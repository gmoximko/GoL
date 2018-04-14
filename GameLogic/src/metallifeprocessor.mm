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

    "  uchar self = input[idx(gid)];"

    "  output[idx(gid)] = (self == 0) ? (neighbours == 3) : (neighbours == 2 || neighbours == 3); "
    "}\n";

@interface MetalLifeProcessor : NSObject

- (UInt8* const) lifeBuffer;
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
  MTLSize thread_group_size_;
  MTLSize thread_group_count_;

  BOOL computed_;
}

- (UInt8* const) lifeBuffer
{
  return (UInt8* const)[input_ contents];
}

- (id) initWithSize: (MTLSize)size
{
  self = [super init];
  assert(self);

  field_size_ = size;
  computed_ = YES;

  thread_group_size_ = MTLSizeMake(1, 1, 1);
  thread_group_count_ = MTLSizeMake(field_size_.width, field_size_.height, 1);

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

  command_queue_ = [device newCommandQueue];
  assert(command_queue_);

  NSUInteger const buffer_length = field_size_.width * field_size_.height;
  input_ = [device newBufferWithLength: buffer_length options: MTLResourceStorageModeShared];
  output_ = [device newBufferWithLength: buffer_length options: MTLResourceStorageModeShared];

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

  id<MTLCommandBuffer> command_buffer = [command_queue_ commandBuffer];
  command_buffer.label = @"LifeStep";

  id<MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder];
  [compute_encoder setComputePipelineState: pipeline_state_];
  [compute_encoder setBuffer: input_ offset: 0 atIndex: 0];
  [compute_encoder setBuffer: output_ offset: 0 atIndex: 1];
  [compute_encoder dispatchThreadgroups: thread_group_count_ threadsPerThreadgroup: thread_group_size_];
  [compute_encoder endEncoding];

  [command_buffer addCompletedHandler: ^(id<MTLCommandBuffer> cb)
  {
    assert([[cb error] code] == 0);

    computed_ = YES;
    memset([input_ contents], 0, field_size_.width * field_size_.height);

    id<MTLBuffer> tmp = input_;
    input_ = output_;
    output_ = tmp;
  }];
  [command_buffer commit];
}

- (void) addUnit: (NSUInteger)position
{
  if (computed_)
  {
    assert(position < field_size_.width * field_size_.height);
    ((UInt8*)[input_ contents])[position] = 1;
  }
}

@end

namespace Logic {

GPULifeProcessor::GPULifeProcessor(QPoint field_size)
  : self_([[MetalLifeProcessor alloc] initWithSize: MTLSizeMake(field_size.x(), field_size.y(), 1)])
  , field_size_(field_size)
{}

GPULifeProcessor::~GPULifeProcessor()
{
  [(id)self_ dealloc];
}

void GPULifeProcessor::processLife()
{
  life_units_.clear();

  UInt8* storage = [(id)self_ lifeBuffer];
  for (int idx = 0; idx < field_size_.x() * field_size_.y(); ++idx)
  {
    if (storage[idx] != 0)
    {
      int x = idx % field_size_.x();
      int y = idx / field_size_.y();
      Q_ASSERT(!life_units_.contains(LifeUnit(x, y)));
      life_units_.push_back(LifeUnit(x, y));
    }
  }

  [(id)self_ processLife];
}

void GPULifeProcessor::addUnit(const LifeUnit &unit)
{
  [(id)self_ addUnit: (unit.x() + unit.y() * field_size_.y())];
}

} // Logic

