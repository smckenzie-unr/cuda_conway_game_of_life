#include <thread>
#include <chrono>
#include <memory>

#include "arg_parser.h"
#include "mainwindow.h"
#include "cuda_cntrl.h"

constexpr int C_MAINWINDOW_ERROR_CODES(-10);
constexpr int C_CUDACNTRL_ERROR_CODES(-20);

int main(int argc, char* argv[]) {
    int return_val(0);

    std::unique_ptr<arguments::arg_parser> arguments;
    std::unique_ptr<application::main_window> window;
    std::unique_ptr<application::cuda_cntrl> cuda;

    try {
        arguments = std::make_unique<arguments::arg_parser>(argc, argv);
        window = std::make_unique<application::main_window>(
            arguments->get_rows(),
            arguments->get_columns(),
            arguments->get_cell_size()
        );
        cuda = std::make_unique<application::cuda_cntrl>(
            arguments->get_rows(),
            arguments->get_columns()
        );
    }
    catch (const arguments::error_codes& code) {
        if (static_cast<int>(code) == -1) {
            arguments::arg_parser::display_helpmenu();
        }
        else {
            return_val = static_cast<int>(code);
        }
    }
    catch (const application::error_codes &code) {
        return_val = static_cast<int>(code) - C_MAINWINDOW_ERROR_CODES;
    }
    catch (const application::cuda_error_codes &code) {
        return_val = static_cast<int>(code) - C_CUDACNTRL_ERROR_CODES;
    }
    
    while(!return_val && window->is_running()) {
        try {
            cuda->run_kernel(
                window->get_grid_data(),
                window->event_polling(),
                window->is_paused()
            );
        }
        catch (const application::cuda_error_codes& code) {
            return_val = static_cast<int>(code) - C_CUDACNTRL_ERROR_CODES;
            break;
        }
        window->incr_generation();
        window->render_board();
    }

    return return_val;
}