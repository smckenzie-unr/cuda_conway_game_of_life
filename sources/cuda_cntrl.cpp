#include "cuda_cntrl.h"

namespace application{
cuda_cntrl::cuda_cntrl(        
    uint64_t height,
    uint64_t width
) noexcept
: m_cell_count(width * height)
, m_width(width)
, m_height(height)
, m_block(BLOCK_WIDTH_DIMENSION, BLOCK_HEIGHT_DIMENSION)
, m_grid(
    (width + m_block.x - 1) / m_block.x,
    (height + m_block.y - 1) / m_block.y
)
, m_pcurrent(nullptr)
, m_pnext(nullptr)
{
    cudaMalloc(&m_pcurrent, m_cell_count);
    cudaMalloc(&m_pnext, m_cell_count);
}
cuda_cntrl::~cuda_cntrl() noexcept
{
    cudaFree(m_pcurrent);
    cudaFree(m_pnext);
}

void cuda_cntrl::run_kernel(uint8_t* host_grid, bool edited, bool paused) noexcept
{
    // If the user edited the board, copy the latest host data to the GPU.
    if(edited) {
        cudaMemcpy(
            m_pcurrent,
            host_grid,
            m_cell_count,
            cudaMemcpyHostToDevice
        );
    }

    if (!paused) {
        game_of_life_kernel<<<m_grid, m_block>>>(
            m_pcurrent,
            m_pnext,
            m_width,
            m_height
        );

        // Check that the kernel launched without errors.
        cudaGetLastError();

        // Wait until the GPU finishes the kernel before reading results.
        cudaDeviceSynchronize();

        // Swap the buffers so m_pcurrent points to the freshly computed data.
        std::swap(m_pcurrent, m_pnext);

        // Copy the newly computed generation back to the host so we can draw it.
        cudaMemcpy(
            host_grid,
            m_pcurrent,
            m_cell_count,
            cudaMemcpyDeviceToHost
        );
    }
    
}
}//application