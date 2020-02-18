#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>

#import "lifeprocessor.h"

namespace Logic {

namespace {

constexpr auto c_kernel_src =
    "#include <metal_stdlib>\n"
    "#include <metal_integer>\n"

    "using namespace metal;\n"

    "ushort2 pos(uint id) "
    "{ "
    "  return ushort2(id % WIDTH, id / HEIGHT); "
    "}\n"

    "uint idx(ushort2 pos) "
    "{ "
    "  return pos.x + pos.y * HEIGHT; "
    "}\n"

    "ushort2 loopPos(short x, short y) "
    "{ "
    "  return ushort2((x + WIDTH) % WIDTH, (y + HEIGHT) % HEIGHT); "
    "}\n"

    "template<uint Bit> "
    "struct Mask "
    "{ "
    "  constant constexpr static uint c_n_e_s_w = 0x70007 << (Bit - 1); "
    "  constant constexpr static uint c_nw_ne_se_sw = 0x0; "
    "  constant constexpr static uint c_self = 0x5 << (Bit - 1); "
    "};\n"
    "template<> "
    "struct Mask<0> "
    "{ "
    "  constant constexpr static uint c_n_e_s_w = 0x80030003; "
    "  constant constexpr static uint c_nw_ne_se_sw = 0x80000080; "
    "  constant constexpr static uint c_self = 0x2; "
    "};\n"
    "template<> "
    "struct Mask<7> "
    "{ "
    "  constant constexpr static uint c_n_e_s_w = 0xC001C0; "
    "  constant constexpr static uint c_nw_ne_se_sw = 0x10100; "
    "  constant constexpr static uint c_self = 0x40; "
    "};\n"

    "template<uint Bit>"
    "uint isAlive(uint self, uint n_e_s_w, uint nw_ne_se_sw) "
    "{ "
    "/*"
    "  nw0       n0        ne1 "
    "  [|||||||0][00333|77][7|||||||] "
    "w3[|||||||0][*03*3|7*][7|||||||]e1 "
    "  [|||||||0][00333|77][7|||||||] "
    "  sw3       s2        se2 "
    "*/"
    "  uint neighbours = popcount(Mask<Bit>::c_n_e_s_w & n_e_s_w) "
    "    + popcount(Mask<Bit>::c_nw_ne_se_sw & nw_ne_se_sw) "
    "    + popcount(Mask<Bit>::c_self & self); "

    "  return static_cast<uint>((self >> Bit & 1) == 0 "
    "    ? neighbours == 3 "
    "    : neighbours == 2 || neighbours == 3) << Bit;"
    "}\n"

    "template<uint Bit>"
    "uint calculateLife(uint self, uint n_e_s_w, uint nw_ne_se_sw) "
    "{ "
    "  return isAlive<Bit>(self, n_e_s_w, nw_ne_se_sw) "
    "    | calculateLife<Bit - 1>(self, n_e_s_w, nw_ne_se_sw); "
    "}\n"
    "template<>"
    "uint calculateLife<0>(uint self, uint n_e_s_w, uint nw_ne_se_sw)"
    "{"
    "  return isAlive<0>(self, n_e_s_w, nw_ne_se_sw); "
    "}\n"

    "kernel void lifeStep(constant uchar* input [[buffer(0)]], "
                         "  device uchar* output [[buffer(1)]], "
                         "uint id [[thread_position_in_grid]]) "
    "{ "
    "  ushort2 gid = pos(id * 8); "
    "  uint nw = idx(loopPos(gid.x - 8, gid.y + 1)); "
    "  uint n  = idx(loopPos(gid.x,     gid.y + 1)); "
    "  uint ne = idx(loopPos(gid.x + 8, gid.y + 1)); "
    "  uint e  = idx(loopPos(gid.x + 8, gid.y    )); "
    "  uint se = idx(loopPos(gid.x + 8, gid.y - 1)); "
    "  uint s  = idx(loopPos(gid.x    , gid.y - 1)); "
    "  uint sw = idx(loopPos(gid.x - 8, gid.y - 1)); "
    "  uint w  = idx(loopPos(gid.x - 8, gid.y    )); "

    "  uint self = static_cast<uint>(input[id]); "
    "  uint n_e_s_w = static_cast<uint>(input[n >> 3]) << 0 * 8 "
    "    | static_cast<uint>(input[e >> 3]) << 1 * 8 "
    "    | static_cast<uint>(input[s >> 3]) << 2 * 8 "
    "    | static_cast<uint>(input[w >> 3]) << 3 * 8; "

    "  uint nw_ne_se_sw = static_cast<uint>(input[nw >> 3]) << 0 * 8 "
    "    | static_cast<uint>(input[ne >> 3]) << 1 * 8 "
    "    | static_cast<uint>(input[se >> 3]) << 2 * 8 "
    "    | static_cast<uint>(input[sw >> 3]) << 3 * 8; "

    "  output[id] = static_cast<uchar>(calculateLife<7>(self, n_e_s_w, nw_ne_se_sw)); "
    "}\n";

class GPULifeProcessor final : public LifeProcessorImpl
{
public:
  explicit GPULifeProcessor(QPoint field_size)
    : LifeProcessorImpl(field_size)
  {
    mainThread().check();
    NSError* error = nil;
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device)
    {
      [NSException raise: NSGenericException format: @"Metal is not supported on this device"];
    }

    NSUInteger width = cols();
    NSUInteger height = rows();
    MTLCompileOptions* options = [MTLCompileOptions new];
    options.preprocessorMacros =
        @{
          @"WIDTH" : [NSNumber numberWithUnsignedLong: width],
          @"HEIGHT" : [NSNumber numberWithUnsignedLong: height]
         };
    NSString* source =
        [[NSString alloc] initWithCString: c_kernel_src encoding: NSASCIIStringEncoding];
    id<MTLLibrary> library = [device newLibraryWithSource: source options: options error: &error];
    Q_ASSERT([error code] == 0);

    id<MTLFunction> function = [library newFunctionWithName: @"lifeStep"];
    Q_ASSERT(function);

    MTLComputePipelineDescriptor* descriptor = [MTLComputePipelineDescriptor new];
    descriptor.computeFunction = function;
    descriptor.buffers[0].mutability = MTLMutabilityImmutable;
    descriptor.buffers[1].mutability = MTLMutabilityMutable;

    pipeline_state_ = [device
        newComputePipelineStateWithDescriptor: descriptor
        options: MTLPipelineOptionNone
        reflection: nil
        error: &error];
    Q_ASSERT([error code] == 0);
    Q_ASSERT(pipeline_state_);

    NSUInteger exec_width = [pipeline_state_ threadExecutionWidth];
    NSUInteger max_threads = [pipeline_state_ maxTotalThreadsPerThreadgroup] / exec_width;
    threads_per_group_ = MTLSizeMake(exec_width, max_threads, 1);
//    threads_per_group_ = MTLSizeMake([pipeline_state_ maxTotalThreadsPerThreadgroup], 1, 1);

    command_queue_ = [device newCommandQueue];
    Q_ASSERT(command_queue_);

    NSUInteger bytes = fieldLength() / 8;
    input_ = [device newBufferWithLength: bytes options: MTLResourceStorageModeShared];
    output_ = [device newBufferWithLength: bytes options: MTLResourceStorageModeShared];
    field_size_ = MTLSizeMake(bytes, 1, 1);

    Q_ASSERT(input_);
    Q_ASSERT(output_);
    Q_ASSERT(computed());
  }
  ~GPULifeProcessor() override
  {
    mainThread().check();
    qDebug() << "~MetalLifeProcessor min computaion duration ";
  }

public: // LifeProcessor
  bool computed() const override
  {
    return computed_;
  }

protected: // LifeProcessorImpl
  void onDestroy() override
  {
    mainThread().check();
    while (!computed());
  }
  void processLife() override
  {
    computeThread().check();
    Q_ASSERT(computed());
    computed_.deref();
    Q_ASSERT(!computed());

    id<MTLCommandBuffer> command_buffer = [command_queue_ commandBuffer];
    command_buffer.label = @"LifeStep";

    id<MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder];
    [compute_encoder setComputePipelineState: pipeline_state_];
    [compute_encoder setBuffer: input_ offset: 0 atIndex: 0];
    [compute_encoder setBuffer: output_ offset: 0 atIndex: 1];
    [compute_encoder dispatchThreads: field_size_ threadsPerThreadgroup: threads_per_group_];
    [compute_encoder endEncoding];

    [command_buffer addCompletedHandler: ^(id<MTLCommandBuffer> cb)
    {
      Q_ASSERT([[cb error] code] == 0);
//      qDebug() << "CFTimeInterval executionDuration " << (cb.GPUEndTime - cb.GPUStartTime);
      handleComputeCompletion();
    }];
    [command_buffer commit];
  }
  uint8_t const* data() const override
  {
    return static_cast<uint8_t const*>([input_ contents]);
  }
  uint8_t* data() override
  {
    return static_cast<uint8_t*>([input_ contents]);
  }

private:
  void handleComputeCompletion()
  {
    id<MTLBuffer> tmp = input_;
    input_ = output_;
    output_ = tmp;
    computed_.ref();
    Q_ASSERT(computed());
  }

  id<MTLCommandQueue> command_queue_;
  id<MTLComputePipelineState> pipeline_state_;
  id<MTLBuffer> input_;
  id<MTLBuffer> output_;

  MTLSize field_size_;
  MTLSize threads_per_group_;

  QAtomicInt computed_ = 1;
};

} // namespace

LifeProcessorPtr createGPULifeProcessor(QPoint field_size)
{
  try
  {
    return std::make_unique<GPULifeProcessor>(field_size);
  }
  catch (NSException* e)
  {
    auto const* msg = [[e reason] cStringUsingEncoding: NSUTF8StringEncoding];
    throw std::runtime_error(msg);
  }
}

} // Logic

