#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>

#import "gpulifeprocessor.h"

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

    "kernel void lifeStep(const device uchar* input [[buffer(0)]], "
                         "      device uchar* output [[buffer(1)]], "
                         "ushort2 gid [[thread_position_in_grid]]) "
    "{ "
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

@interface MetalLifeProcessor : NSObject

- (NSUInteger) fieldSize;
- (UInt8) unitAt: (NSUInteger)position;
- (void) processLife;
- (void) addUnit: (NSUInteger)position;

@end

@implementation MetalLifeProcessor
{
  id<MTLCommandQueue> command_queue_;
  id<MTLComputePipelineState> pipeline_state_;
  id<MTLBuffer> input_;
  id<MTLBuffer> output_;

  MTLSize field_size_;
  MTLSize threads_per_group_;

  NSMutableSet* position_cache_;
  BOOL computed_;
}

- (NSUInteger) fieldSize
{
  return field_size_.width * field_size_.height;
}

- (UInt8) unitAt: (NSUInteger)position
{
  assert(position < [self fieldSize]);
  return ((UInt8*)[input_ contents])[position];
}

- (id) initWithWidth: (NSUInteger)width Height:(NSUInteger)height
{
  self = [super init];
  assert(self);

  field_size_ = MTLSizeMake(width, height, 1);
  computed_ = YES;

  NSError* error = nil;
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();

  MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
  options.preprocessorMacros =
      @{
        @"WIDTH" : [NSNumber numberWithUnsignedInt: field_size_.width],
        @"HEIGHT" : [NSNumber numberWithUnsignedInt: field_size_.height]
       };
  id<MTLLibrary> library = [device newLibraryWithSource: kernel_src options: options error: &error];
  assert([error code] == 0);

  id<MTLFunction> function = [library newFunctionWithName: @"lifeStep"];
  assert(function);

  pipeline_state_ = [device newComputePipelineStateWithFunction: function error: &error];
  assert([error code] == 0);
  assert(pipeline_state_);

  NSUInteger exec_width = [pipeline_state_ threadExecutionWidth];
  NSUInteger max_threads = [pipeline_state_ maxTotalThreadsPerThreadgroup] / exec_width;
  threads_per_group_ = MTLSizeMake(exec_width, max_threads, 1);

  command_queue_ = [device newCommandQueue];
  assert(command_queue_);

  input_ = [device newBufferWithLength: [self fieldSize] options: MTLResourceStorageModeShared];
  output_ = [device newBufferWithLength: [self fieldSize] options: MTLResourceStorageModeShared];

  assert(input_);
  assert(output_);

  position_cache_ = [[NSMutableSet alloc] init];
  return self;
}

- (void) processLife
{
  if (!computed_)
  {
    return;
  }
  computed_ = NO;

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
  }];
  [command_buffer commit];
}

- (void) addUnit: (NSUInteger)position
{
  if (computed_)
  {
    assert(position < [self fieldSize]);
    ((UInt8*)[input_ contents])[position] = 1;
  }
  else
  {
    [position_cache_ addObject: [NSNumber numberWithUnsignedInt: position]];
  }
}

- (void) handleComputeCompletion
{
  computed_ = YES;
  memset([input_ contents], 0, [self fieldSize]);

  id<MTLBuffer> tmp = input_;
  input_ = output_;
  output_ = tmp;

  for (id pos in position_cache_)
  {
    [self addUnit: [pos unsignedIntegerValue]];
  }
  [position_cache_ removeAllObjects];
}

@end

namespace Logic {

GPULifeProcessor::GPULifeProcessor(QPoint field_size)
  : self_([[MetalLifeProcessor alloc] initWithWidth: field_size.x() Height: field_size.y()])
  , field_size_(field_size)
{}

GPULifeProcessor::~GPULifeProcessor()
{
  [(id)self_ dealloc];
}

void GPULifeProcessor::processLife()
{
  id impl = (id)self_;
  life_units_.clear();

  auto const field_size = static_cast<size_t>(field_size_.x() * field_size_.y());
  for (size_t idx = 0; idx < field_size; ++idx)
  {
    if ([impl unitAt: idx] != 0)
    {
      int x = idx % field_size_.x();
      int y = idx / field_size_.y();
      life_units_.insert(LifeUnit(x, y));
    }
  }

  [impl processLife];
}

void GPULifeProcessor::addUnit(const LifeUnit &unit)
{
  [(id)self_ addUnit: (unit.x() + unit.y() * field_size_.y())];
}

} // Logic

