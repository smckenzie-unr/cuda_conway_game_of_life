#include "conway_gol_kernel.cuh"

// A CUDA kernel is a function that runs on the GPU. It is launched with many
// threads in parallel.
//
// Each thread is assigned one cell coordinate (x, y). It checks the 8 neighbors
// around that cell and applies the rules of Conway's Game of Life:
// - Any live cell with 2 or 3 live neighbors stays alive.
// - Any dead cell with exactly 3 live neighbors becomes alive.
// - Otherwise the cell dies or stays dead.
__global__
void game_of_life_kernel(
    const uint8_t* current,
    uint8_t* next,
    int width,
    int height
)
{
    // CUDA uses a grid of blocks, and each block contains many threads.
    // We map the thread's 2D location to a 2D cell coordinate in the board.
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    // If the thread is outside the board, stop early.
    if (x >= width || y >= height) {
        return;
    }

    // Count how many neighbors are alive.
    int neighbors = 0;

    // Loop over the 3x3 neighborhood around this cell, excluding the center.
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int nx = x + dx;
            int ny = y + dy;

            // Keep only valid positions inside the board.
            if (nx >= 0 &&
                nx < width &&
                ny >= 0 &&
                ny < height)
            {
                // Flatten the 2D index into a 1D index: row * width + column.
                neighbors += current[ny * width + nx];
            }
        }
    }

    // Read the current state of this cell.
    uint8_t alive = current[y * width + x];

    // Apply the Game of Life rules.
    // In this representation, 1 means alive and 0 means dead.
    next[y * width + x] = alive ? (neighbors == 2 || neighbors == 3) : (neighbors == 3);
}