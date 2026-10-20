#ifndef __CUDA_CNTRL_H__
#define __CUDA_CNTRL_H__
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <iostream>

#include "conway_gol_kernel.cuh"

namespace application {
enum class cuda_error_codes : int64_t {
    no_error = 0,
    cuda_malloc_error = -1,
    cuda_memcpy_error = -2,
    cuda_internal_error = -3,
    cuda_device_sync_error = -4
};

class cuda_cntrl final {
public:
    cuda_cntrl(const cuda_cntrl &) = delete;
    cuda_cntrl(cuda_cntrl &&) = delete;
    explicit cuda_cntrl(        
        uint64_t height,
        uint64_t width
    );
    ~cuda_cntrl() noexcept;

    void run_kernel(uint8_t* host_grid, bool edited, bool paused);

private:
    const size_t m_cell_count;
    int m_width;
    int m_height;

    dim3 m_block;
    dim3 m_grid;

    uint8_t* m_pcurrent;
    uint8_t* m_pnext;

};
}//appliation

#endif //__CUDA_CNTRL_H__