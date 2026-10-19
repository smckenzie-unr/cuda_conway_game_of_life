#ifndef __CONWAY_GOL_KERNEL_CUH__
#define __CONWAY_GOL_KERNEL_CUH__
#include <cstdint>
#include <cuda_runtime.h>

__global__
void game_of_life_kernel(
    const uint8_t* current,
    uint8_t* next,
    int width,
    int height
);

#endif //__CONWAY_GOL_KERNEL_CUH__