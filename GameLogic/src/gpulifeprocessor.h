#ifndef GPULIFEPROCESSOR_H
#define GPULIFEPROCESSOR_H

namespace Logic {

class GPULifeProcessor
{
public:
  GPULifeProcessor();
  ~GPULifeProcessor();

  void processLife();

private:
  void* self;
};

} // Logic

#endif // GPULIFEPROCESSOR_H
