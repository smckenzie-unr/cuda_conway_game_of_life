#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__
#include <cstdint>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <SDL2/SDL.h>

namespace application {
enum class error_codes : int64_t {
    no_error = 0,
    failed_create_window = -1,
    failed_create_render = -2
};

class main_window final {
public:
    main_window(const main_window &) = delete;
    main_window(main_window &&) = delete;
    explicit main_window(
        uint64_t num_rows,
        uint64_t num_cols,
        uint64_t cell_size
    );
    ~main_window() noexcept;

    bool event_polling() noexcept;
    void render_board() noexcept;

    inline bool is_running() const noexcept {
        return m_running;
    }
    inline uint8_t* get_grid_data() noexcept {
        return m_host_grid.data();
    }
    inline bool is_paused() const noexcept {
        return m_paused;
    }

    void incr_generation() noexcept;

    main_window& operator =(const main_window &) = delete;
    main_window& operator =(main_window &&) = delete;

private:
    const uint64_t m_width;
    const uint64_t m_height;
    const uint64_t m_cellsize;

    uint64_t m_generation;

    std::vector<uint8_t> m_host_grid;

    bool m_running;
    bool m_paused;

    bool m_mouse_leftclick;
    bool m_mouse_rightclick;

    SDL_Window* m_pwindow;
    SDL_Renderer* m_prenderer;
    SDL_Event m_event;
    
    bool mouse_event() noexcept;

};
}//application

#endif //__MAINWINDOW_H__