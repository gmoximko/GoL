#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>

#import "lifeprocessor.h"

static NSString* const kernel_src =
    @"#include <metal_stdlib>\n"

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

    "uchar lifeBit(const device uchar* input, uint pos) "
    "{ "
    "  uint byte = pos >> 3; "
    "  uint bit = pos & 7; "
    "  return (input[byte] >> bit) & 1; "
    "}\n"

    "uchar isAlive(const device uchar* input, uint id) "
    "{ "
    "  ushort2 gid = pos(id); "
    "  uint nw = idx(loopPos(gid.x - 1, gid.y + 1)); "
    "  uint n  = idx(loopPos(gid.x,     gid.y + 1)); "
    "  uint ne = idx(loopPos(gid.x + 1, gid.y + 1)); "
    "  uint e  = idx(loopPos(gid.x + 1, gid.y    )); "
    "  uint se = idx(loopPos(gid.x + 1, gid.y - 1)); "
    "  uint s  = idx(loopPos(gid.x    , gid.y - 1)); "
    "  uint sw = idx(loopPos(gid.x - 1, gid.y - 1)); "
    "  uint w  = idx(loopPos(gid.x - 1, gid.y    )); "

    "  uchar neighbours = lifeBit(input, nw) "
    "    + lifeBit(input, n) "
    "    + lifeBit(input, ne) "
    "    + lifeBit(input, e) "
    "    + lifeBit(input, se) "
    "    + lifeBit(input, s) "
    "    + lifeBit(input, sw) "
    "    + lifeBit(input, w); "

    "  uchar self = lifeBit(input, id); "
    "  return static_cast<uchar>((self == 0 && neighbours == 3) "
    "    || (self == 1 && (neighbours == 2 || neighbours == 3))); "
    "}\n"

    "kernel void lifeStep(const device uchar* input [[buffer(0)]], "
                         "      device uchar* output [[buffer(1)]], "
                         "uint id [[thread_position_in_grid]]) "
    "{ "
    "  uint bit = id * 8; "
    "  output[id] = isAlive(input, bit + 0) << 0 "
    "    | isAlive(input, bit + 1) << 1 "
    "    | isAlive(input, bit + 2) << 2 "
    "    | isAlive(input, bit + 3) << 3 "
    "    | isAlive(input, bit + 4) << 4 "
    "    | isAlive(input, bit + 5) << 5 "
    "    | isAlive(input, bit + 6) << 6 "
    "    | isAlive(input, bit + 7) << 7; "
    "}\n";

@interface MetalLifeProcessor : NSObject

- (BOOL) computed;
- (void*) content;
- (CFTimeInterval) computationDuration;
- (void) processLife;

@end

@implementation MetalLifeProcessor
{
  id<MTLCommandQueue> command_queue_;
  id<MTLComputePipelineState> pipeline_state_;
  id<MTLBuffer> input_;
  id<MTLBuffer> output_;

  MTLSize field_size_;
  MTLSize threads_per_group_;

  BOOL computed_;
  CFTimeInterval computation_duration_;
}

- (BOOL) computed
{
  return computed_;
}

- (void*) content
{
  return [input_ contents];
}

- (CFTimeInterval) computationDuration
{
  return computation_duration_;
}

- (id) initWithWidth: (NSUInteger)width Height: (NSUInteger)height
{
  self = [super init];
  assert(self);

  computed_ = YES;
  computation_duration_ = 0;

  NSError* error = nil;
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  if (!device)
  {
    [NSException raise: NSGenericException format: @"Metal is not supported on this device"];
  }

  MTLCompileOptions* options = [MTLCompileOptions new];
  options.preprocessorMacros =
      @{
        @"WIDTH" : [NSNumber numberWithUnsignedLong: width],
        @"HEIGHT" : [NSNumber numberWithUnsignedLong: height]
       };
  id<MTLLibrary> library = [device newLibraryWithSource: kernel_src options: options error: &error];
  assert([error code] == 0);

  id<MTLFunction> function = [library newFunctionWithName: @"lifeStep"];
  assert(function);

  pipeline_state_ = [device newComputePipelineStateWithFunction: function error: &error];
  assert([error code] == 0);
  assert(pipeline_state_);

//  NSUInteger exec_width = [pipeline_state_ threadExecutionWidth];
//  NSUInteger max_threads = [pipeline_state_ maxTotalThreadsPerThreadgroup] / exec_width;
  threads_per_group_ = MTLSizeMake([pipeline_state_ maxTotalThreadsPerThreadgroup], 1, 1);

  command_queue_ = [device newCommandQueue];
  assert(command_queue_);

  NSUInteger bytes = width * height / 8;
  input_ = [device newBufferWithLength: bytes options: MTLResourceStorageModeShared];
  output_ = [device newBufferWithLength: bytes options: MTLResourceStorageModeShared];
  field_size_ = MTLSizeMake(bytes, 1, 1);

  assert(input_);
  assert(output_);

  return self;
}

- (void) processLife
{
  if (!computed_)
  {
    return;
  }
  computed_ = NO;
  NSDate *start = [NSDate date];

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
    assert([[cb error] code] == 0);
    [self handleComputeCompletion];
    computation_duration_ = -[start timeIntervalSinceNow];
  }];
  [command_buffer commit];
}

- (void) handleComputeCompletion
{
  computed_ = YES;

  id<MTLBuffer> tmp = input_;
  input_ = output_;
  output_ = tmp;
}

@end

namespace Logic {

namespace {

class GPULifeProcessor final : public LifeProcessorImpl
{
public:
  explicit GPULifeProcessor(QPoint field_size)
  try
    : LifeProcessorImpl(field_size)
    , self_([[MetalLifeProcessor alloc]
        initWithWidth: static_cast<NSUInteger>(field_size.x())
                Height: static_cast<NSUInteger>(field_size.y())])
  {}
  catch(NSException* e)
  {
    auto const* msg = [[e reason] cStringUsingEncoding: NSUTF8StringEncoding];
    throw std::runtime_error(msg);
  }
  ~GPULifeProcessor() override
  {
    while (!computed());
  }

public: // LifeProcessor
  bool computed() const override
  {
    return [self_ computed];
  }
  int computationDuration() const override
  {
    return static_cast<int>([self_ computationDuration] * 1000);
  }

protected: // LifeProcessorImpl
  void processLife() override
  {
    [self_ processLife];
  }
  uint8_t* data() override
  {
    return static_cast<uint8_t*>([self_ content]);
  }

private:
  MetalLifeProcessor* self_;
};

} // namespace

LifeProcessorPtr createGPULifeProcessor(QPoint field_size)
{
  return std::make_unique<GPULifeProcessor>(field_size);
}

} // Logic

