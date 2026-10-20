#include "conway_gol_kernel.cuh"

#include <cuda_runtime.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int kBlockWidth = 16;
constexpr int kBlockHeight = 16;
constexpr int kSkipReturnCode = 77;

bool check_cuda(cudaError_t result, const char* operation)
{
    if (result == cudaSuccess) {
        return true;
    }

    std::cerr << operation << " failed: " << cudaGetErrorString(result) << '\n';
    return false;
}

std::vector<uint8_t> reference_step(
    const std::vector<uint8_t>& current,
    int width,
    int height
)
{
    std::vector<uint8_t> next(current.size(), 0);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int neighbors = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }

                    const int neighbor_x = x + dx;
                    const int neighbor_y = y + dy;
                    if (neighbor_x >= 0 && neighbor_x < width &&
                        neighbor_y >= 0 && neighbor_y < height) {
                        neighbors += current[neighbor_y * width + neighbor_x];
                    }
                }
            }

            const auto index = static_cast<std::size_t>(y * width + x);
            next[index] = current[index]
                ? static_cast<uint8_t>(neighbors == 2 || neighbors == 3)
                : static_cast<uint8_t>(neighbors == 3);
        }
    }

    return next;
}

bool run_case(
    const std::string& name,
    const std::vector<uint8_t>& current,
    int width,
    int height
)
{
    uint8_t* device_current = nullptr;
    uint8_t* device_next = nullptr;
    const std::size_t bytes = current.size() * sizeof(uint8_t);

    if (!check_cuda(cudaMalloc(&device_current, bytes), "cudaMalloc current")) {
        return false;
    }
    if (!check_cuda(cudaMalloc(&device_next, bytes), "cudaMalloc next")) {
        check_cuda(cudaFree(device_current), "cudaFree current");
        return false;
    }

    bool success = check_cuda(
        cudaMemcpy(
            device_current,
            current.data(),
            bytes,
            cudaMemcpyHostToDevice
        ),
        "copy input to device"
    );

    if (success) {
        const dim3 block(kBlockWidth, kBlockHeight);
        const dim3 grid(
            (width + kBlockWidth - 1) / kBlockWidth,
            (height + kBlockHeight - 1) / kBlockHeight
        );
        game_of_life_kernel<<<grid, block>>>(
            device_current,
            device_next,
            width,
            height
        );
        success = check_cuda(cudaGetLastError(), "launch kernel") &&
            check_cuda(cudaDeviceSynchronize(), "synchronize kernel");
    }

    std::vector<uint8_t> actual(current.size());
    if (success) {
        success = check_cuda(
            cudaMemcpy(
                actual.data(),
                device_next,
                bytes,
                cudaMemcpyDeviceToHost
            ),
            "copy output from device"
        );
    }

    success = check_cuda(cudaFree(device_current), "cudaFree current") && success;
    success = check_cuda(cudaFree(device_next), "cudaFree next") && success;

    if (!success) {
        return false;
    }

    const auto expected = reference_step(current, width, height);
    if (actual != expected) {
        std::cerr << "Test case '" << name << "' produced an incorrect board\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const auto index = static_cast<std::size_t>(y * width + x);
                if (actual[index] != expected[index]) {
                    std::cerr << "  mismatch at (" << x << ", " << y
                              << "): expected "
                              << static_cast<int>(expected[index])
                              << ", got " << static_cast<int>(actual[index])
                              << '\n';
                    return false;
                }
            }
        }
        return false;
    }

    std::cout << "Passed: " << name << '\n';
    return true;
}

std::vector<uint8_t> make_board(
    int width,
    int height,
    const std::vector<std::pair<int, int>>& live_cells
)
{
    std::vector<uint8_t> board(static_cast<std::size_t>(width * height), 0);
    for (const auto& cell : live_cells) {
        board[static_cast<std::size_t>(cell.second * width + cell.first)] = 1;
    }
    return board;
}

} // namespace

int main()
{
    int device_count = 0;
    const cudaError_t device_status = cudaGetDeviceCount(&device_count);
    if (device_status == cudaErrorNoDevice ||
        device_status == cudaErrorInsufficientDriver ||
        (device_status == cudaSuccess && device_count == 0)) {
        std::cout << "Skipped: no usable CUDA device is available\n";
        return kSkipReturnCode;
    }
    if (!check_cuda(device_status, "cudaGetDeviceCount")) {
        return 1;
    }

    bool success = true;
    success = run_case("empty board", make_board(5, 3, {}), 5, 3) && success;
    success = run_case(
        "single-cell board",
        make_board(1, 1, {{0, 0}}),
        1,
        1
    ) && success;
    success = run_case(
        "corner boundary",
        make_board(3, 2, {{0, 0}, {1, 0}, {0, 1}}),
        3,
        2
    ) && success;
    success = run_case(
        "still life",
        make_board(6, 6, {{2, 2}, {3, 2}, {2, 3}, {3, 3}}),
        6,
        6
    ) && success;
    success = run_case(
        "blinker oscillator",
        make_board(7, 7, {{3, 2}, {3, 3}, {3, 4}}),
        7,
        7
    ) && success;

    constexpr int width = 35;
    constexpr int height = 19;
    std::vector<uint8_t> irregular_board(
        static_cast<std::size_t>(width * height),
        0
    );
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            irregular_board[static_cast<std::size_t>(y * width + x)] =
                static_cast<uint8_t>((x * 17 + y * 31 + x * y) % 7 < 3);
        }
    }
    success = run_case(
        "non-block-multiple dimensions",
        irregular_board,
        width,
        height
    ) && success;

    return success ? 0 : 1;
}
