#ifndef __CUDA_CNTRL_H__
#define __CUDA_CNTRL_H__
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "conway_gol_kernel.cuh"

namespace application {
class cuda_cntrl final {
public:
    cuda_cntrl(const cuda_cntrl &) = delete;
    cuda_cntrl(cuda_cntrl &&) = delete;
    explicit cuda_cntrl(        
        uint64_t height,
        uint64_t width
    ) noexcept;
    ~cuda_cntrl() noexcept;

    void run_kernel(uint8_t* host_grid, bool edited, bool paused) noexcept;

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