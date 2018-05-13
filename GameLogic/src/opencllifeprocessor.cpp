#if defined(__APPLE__)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <vector>
#include <string>
#include <set>

#include <QDebug>

#include "gpulifeprocessor.h"

namespace Logic {

namespace {

std::string const kernel_src =
    "ushort2 pos(uint id) "
    "{ "
    "  return (ushort2)(id % WIDTH, id / HEIGHT); "
    "}\n"

    "uint idx(ushort2 pos) "
    "{ "
    "  return pos.x + pos.y * HEIGHT; "
    "}\n"

    "ushort2 loopPos(short x, short y) "
    "{ "
    "  return (ushort2)((x + WIDTH) % WIDTH, (y + HEIGHT) % HEIGHT); "
    "}\n"

    "kernel void lifeStep(global const uchar* input, "
    "                     global       uchar* output) "
    "{ "
    "  ushort2 gid = pos(get_global_id(0)); "
    "  uint nw = idx(loopPos(gid.x - 1, gid.y + 1)); "
    "  uint n  = idx(loopPos(gid.x,     gid.y + 1)); "
    "  uint ne = idx(loopPos(gid.x + 1, gid.y + 1)); "
    "  uint e  = idx(loopPos(gid.x + 1, gid.y)); "
    "  uint se = idx(loopPos(gid.x + 1, gid.y - 1)); "
    "  uint s  = idx(loopPos(gid.x    , gid.y - 1)); "
    "  uint sw = idx(loopPos(gid.x - 1, gid.y - 1)); "
    "  uint w  = idx(loopPos(gid.x - 1, gid.y)); "

    "  uchar neighbours = "
    "    input[nw] + input[n] + input[ne] + input[e] + input[se] + input[s] + input[sw] + input[w]; "

    "  uchar self = input[idx(gid)]; "

    "  output[idx(gid)] = (self == 0) ? (neighbours == 3) : (neighbours == 2 || neighbours == 3); "
    "}\n";

class OpenCLLifeProcessor
{
public:
  explicit OpenCLLifeProcessor(QPoint field_size)
    : field_size_(field_size)
  {
    cl_int error = 0;
    cl::Platform platform = cl::Platform::getDefault(&error);
    if (error != 0)
    {
      throw std::runtime_error("OpenCL is not supported on this device");
    }
    qDebug() << platform.getInfo<CL_PLATFORM_NAME>().c_str();

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    Q_ASSERT(!devices.empty());

    for (auto const& device : devices)
    {
      qDebug() << device.getInfo<CL_DEVICE_NAME>().c_str();
    }
    cl::Device gpu_device = devices.front();
    cl::Context context({ gpu_device });

    cl::Program::Sources src;
    src.push_back({ kernel_src.data(), kernel_src.size() });

    cl::Program program(context, src);
    std::string options = "-D WIDTH=" + std::to_string(field_size_.x())
        + " -D HEIGHT=" + std::to_string(field_size_.y());
    qDebug() << options.c_str();
    cl_int result = program.build({ gpu_device }, options.c_str());
    if (result != CL_SUCCESS)
    {
      qDebug() << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(gpu_device).c_str();
    }

    in_pinned_buffer_ = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, fieldSize());
    out_pinned_buffer_ = cl::Buffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, fieldSize());

    in_buffer_ = cl::Buffer(context, CL_MEM_READ_ONLY, fieldSize());
    out_buffer_ = cl::Buffer(context, CL_MEM_WRITE_ONLY, fieldSize());

    command_queue_ = cl::CommandQueue(context, gpu_device);

    input_ = static_cast<uint8_t*>(
          command_queue_.enqueueMapBuffer(in_pinned_buffer_, CL_TRUE, CL_MAP_WRITE, 0, fieldSize()));
    output_ = static_cast<uint8_t*>(
          command_queue_.enqueueMapBuffer(out_pinned_buffer_, CL_TRUE, CL_MAP_READ, 0, fieldSize()));

    kernel_ = cl::Kernel(program, "lifeStep");
    kernel_.setArg(0, in_buffer_);
    kernel_.setArg(1, out_buffer_);
    Q_ASSERT(error == 0);
  }

  uint8_t unitAt(size_t position) const
  {
    Q_ASSERT(position < fieldSize());
    return input_[position];
  }

  void processLife()
  {
    cl_int error = 0;
    if (!completion_.empty())
    {
      error = cl::Event::waitForEvents(completion_);
      Q_ASSERT(error == 0);
      completion_.clear();
      handleComputeCompletion();
    }

    error = command_queue_.enqueueWriteBuffer(in_buffer_, CL_TRUE, 0, fieldSize(), input_);
    Q_ASSERT(error == 0);
    completion_.emplace_back();
    auto& event = completion_.back();

    error = command_queue_
        .enqueueNDRangeKernel(kernel_, cl::NullRange, cl::NDRange(fieldSize()), cl::NullRange, nullptr, &event);
    Q_ASSERT(error == 0);
  }

  void addUnit(size_t position)
  {
    if (completion_.empty())
    {
      Q_ASSERT(position < static_cast<size_t>(field_size_.x() * field_size_.y()));
      input_[position] = 1;
    }
    else
    {
      position_cache_.insert(position);
    }
  }

private:
  size_t fieldSize() const
  {
    return field_size_.x() * field_size_.y();
  }

  void handleComputeCompletion()
  {
    auto error = command_queue_.enqueueReadBuffer(out_buffer_, CL_TRUE, 0, fieldSize(), output_);
    Q_ASSERT(error == 0);
    std::memset(input_, 0, fieldSize());
    std::swap(input_, output_);

    for (auto const pos : position_cache_)
    {
      addUnit(pos);
    }
    position_cache_.clear();
  }

  QPoint const field_size_;
  uint8_t* input_ = nullptr;
  uint8_t* output_ = nullptr;

  cl::Buffer in_pinned_buffer_;
  cl::Buffer out_pinned_buffer_;
  cl::Buffer in_buffer_;
  cl::Buffer out_buffer_;
  cl::CommandQueue command_queue_;
  cl::Kernel kernel_;
  std::vector<cl::Event> completion_;

  std::set<size_t> position_cache_;
};

} // namespace

GPULifeProcessor::GPULifeProcessor(QPoint field_size)
  : self_(new OpenCLLifeProcessor(field_size))
  , field_size_(field_size)
{}

GPULifeProcessor::~GPULifeProcessor()
{
  delete static_cast<OpenCLLifeProcessor*>(self_);
}

void GPULifeProcessor::processLife()
{
  auto* impl = static_cast<OpenCLLifeProcessor*>(self_);
  life_units_.clear();

  auto const field_size = static_cast<size_t>(field_size_.x() * field_size_.y());
  for (size_t idx = 0; idx < field_size; ++idx)
  {
    if (impl->unitAt(idx) != 0)
    {
      int x = idx % field_size_.x();
      int y = idx / field_size_.y();
      life_units_.insert(LifeUnit(x, y));
    }
  }

  impl->processLife();
}

void GPULifeProcessor::addUnit(LifeUnit const& unit)
{
  static_cast<OpenCLLifeProcessor*>(self_)->addUnit(unit.x() + unit.y() * field_size_.y());
}

} // Logic
