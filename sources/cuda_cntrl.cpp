#include "cuda_cntrl.h"

namespace application{
cuda_cntrl::cuda_cntrl(        
    uint64_t height,
    uint64_t width
)
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
    if (cudaMalloc(&m_pcurrent, m_cell_count) != cudaSuccess) {
        throw cuda_error_codes::cuda_malloc_error;
    }

    if (cudaMalloc(&m_pnext, m_cell_count) != cudaSuccess) {
        throw cuda_error_codes::cuda_malloc_error;
    }

    std::vector<uint8_t> temp(m_cell_count, 0);

    if (cudaMemcpy(
            m_pcurrent,
            temp.data(),
            m_cell_count,
            cudaMemcpyHostToDevice
        ) != cudaSuccess
    )
    {
        throw cuda_error_codes::cuda_memcpy_error;
    }
}
cuda_cntrl::~cuda_cntrl() noexcept
{
    if (m_pcurrent) {
        cudaFree(m_pcurrent);
        m_pcurrent = nullptr;
    }

    if (m_pnext) {
        cudaFree(m_pnext);
        m_pnext = nullptr;
    }
}

void cuda_cntrl::run_kernel(uint8_t* host_grid, bool edited, bool paused)
{
    cudaError_t err;

    // If the user edited the board, copy the latest host data to the GPU.
    if(edited) {
        err = cudaMemcpy(
                m_pcurrent,
                host_grid,
                m_cell_count,
                cudaMemcpyHostToDevice
            );
        if (err) {
            throw cuda_error_codes::cuda_memcpy_error;
        }
    }

    if (!paused) {
        game_of_life_kernel<<<m_grid, m_block>>>(
            m_pcurrent,
            m_pnext,
            m_width,
            m_height
        );

        // Check that the kernel launched without errors.
        err = cudaGetLastError();
        if (err) {
            std::cerr << "Internal cuda error. Error code: " << err << std::endl;
            throw cuda_error_codes::cuda_internal_error;
        }

        // Wait until the GPU finishes the kernel before reading results.
        err = cudaDeviceSynchronize();
        if (err) {
            std::cerr << "Device synchronize error: " << err << std::endl;
            throw cuda_error_codes::cuda_device_sync_error;
        }

        // Swap the buffers so m_pcurrent points to the freshly computed data.
        std::swap(m_pcurrent, m_pnext);

        // Copy the newly computed generation back to the host so we can draw it.
        err = cudaMemcpy(
                host_grid,
                m_pcurrent,
                m_cell_count,
                cudaMemcpyDeviceToHost
            );
        if (err) {
            throw cuda_error_codes::cuda_memcpy_error;
        }
    }
}
}//application