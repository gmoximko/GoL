#include "gpulifeprocessor.h"

namespace Logic {

namespace {

class OpenCLLifeProcessor
{

};

} // namespace

GPULifeProcessor::GPULifeProcessor()
  : self(new OpenCLLifeProcessor)
{}

GPULifeProcessor::~GPULifeProcessor()
{
  delete static_cast<OpenCLLifeProcessor*>(self);
}

void GPULifeProcessor::processLife()
{}

} // Logic
