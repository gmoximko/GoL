#import <Foundation/Foundation.h>
#import <objc/NSObject.h>

#import "gpulifeprocessor.h"

@interface MetalLifeProcessor : NSObject
{}

- (void) processLife;

@end

@implementation MetalLifeProcessor

- (void) processLife
{
  NSLog(@"Msg = %@", @"Hello from Obj-C!");
}

@end

namespace Logic {

GPULifeProcessor::GPULifeProcessor()
  : self([[MetalLifeProcessor alloc] init])
{}

GPULifeProcessor::~GPULifeProcessor()
{
  [(id)self dealloc];
}

void GPULifeProcessor::processLife()
{
  [(id)self processLife];
}

} // Logic

