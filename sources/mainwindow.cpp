#include "mainwindow.h"

constexpr char C_WINDOW_NAME[] = "CUDA Conway";
constexpr uint32_t C_DELAY_VALUE(16);

namespace application {
main_window::main_window(
    uint64_t num_rows,
    uint64_t num_cols,
    uint64_t cell_size
)
: m_width(num_cols)
, m_height(num_rows)
, m_cellsize(cell_size)
, m_generation(0)
, m_host_grid(num_cols * num_rows, 0)
, m_running(true)
, m_paused(false)
, m_mouse_leftclick(false)
, m_mouse_rightclick(false)
, m_pwindow(nullptr)
, m_prenderer(nullptr)
, m_event()
{
    SDL_Init(SDL_INIT_VIDEO);

    m_pwindow = SDL_CreateWindow(
                    C_WINDOW_NAME,
                    SDL_WINDOWPOS_CENTERED,
                    SDL_WINDOWPOS_CENTERED,
                    m_width * m_cellsize,
                    m_height * m_cellsize,
                    0
                );
    if (!m_pwindow) {
        std::cerr << "Failed to create window." << std::endl;
        throw error_codes::failed_create_window;
    }

    m_prenderer = SDL_CreateRenderer(
                    m_pwindow,
                    -1,
                    SDL_RENDERER_ACCELERATED
                );
    if (!m_prenderer) {
        SDL_DestroyWindow(m_pwindow);
        std::cerr << "Failed to create renderer." << std::endl;
        throw error_codes::failed_create_render;
    }
}
main_window::~main_window() noexcept
{
    SDL_DestroyRenderer(m_prenderer);
    m_prenderer = nullptr;

    SDL_DestroyWindow(m_pwindow);
    m_pwindow = nullptr;

    SDL_Quit();
}

bool main_window::event_polling() noexcept
{
    bool edited = false;

    while(SDL_PollEvent(&m_event)) {
        switch(m_event.type) {
            case SDL_QUIT :
                m_running = false;
            break;

            case SDL_KEYDOWN :
                switch(m_event.key.keysym.sym) {
                    case SDLK_SPACE :
                        m_paused = !m_paused;
                    break;

                    case SDLK_c :
                        std::fill(m_host_grid.begin(), m_host_grid.end(), 0);
                        m_generation = 0;
                        edited = true;
                    break;

                    case SDLK_r :
                        std::generate(
                            m_host_grid.begin(), 
                            m_host_grid.end(), 
                            []{ return rand() & 1; }
                        );
                        m_generation = 0;
                        edited = true;
                    break;
                }
            break;

            case SDL_MOUSEBUTTONDOWN :
                if (m_event.button.button == SDL_BUTTON_LEFT) {
                    m_mouse_leftclick = true;
                }
                if (m_event.button.button == SDL_BUTTON_RIGHT) {
                    m_mouse_rightclick = true;
                }
            break;

            case SDL_MOUSEBUTTONUP :
                if (m_event.button.button == SDL_BUTTON_LEFT) {
                    m_mouse_leftclick = false;
                }
                if (m_event.button.button == SDL_BUTTON_RIGHT) {
                    m_mouse_rightclick = false;
                }
            break;
        }
    }

    edited |= this->mouse_event();
    return edited;
}

bool main_window::mouse_event() noexcept
{
    int mouse_x;
    int mouse_y;

    SDL_GetMouseState(
        &mouse_x,
        &mouse_y
    );

    // Convert pixel coordinates to grid coordinates.
    int cell_x = mouse_x / m_cellsize;
    int cell_y = mouse_y / m_cellsize;

    bool edited = false;

    // If we are over the board, the mouse can paint alive or dead cells.
    if(cell_x >= 0 &&
        cell_x < m_width &&
        cell_y >= 0 &&
        cell_y < m_height)
    {
        if(m_mouse_leftclick)
        {
            // Left click makes a cell alive.
            m_host_grid[cell_y * m_width + cell_x] = 1;
            edited = true;
        }

        if(m_mouse_rightclick)
        {
            // Right click kills a cell.
            m_host_grid[cell_y * m_width + cell_x] = 0;
            edited = true;
        }
    }

    return edited;
}

void main_window::incr_generation() noexcept {
    if (!m_paused) {
        m_generation++;
    }
}

void main_window::render_board() noexcept
{
    // Count live cells for the status text in the window title.
    size_t alive_count = 0;

    // Clear the background to black before drawing the live cells.
    SDL_SetRenderDrawColor(
        m_prenderer,
        0,
        0,
        0,
        255
    );

    SDL_RenderClear(m_prenderer);

    // Set the drawing color for live cells to white.
    SDL_SetRenderDrawColor(
        m_prenderer,
        255,
        255,
        255,
        255
    );

    // Iterate through every cell and draw a filled rectangle for any live one.
    for(int y = 0; y < m_height; y++) {
        for(int x = 0; x < m_width; x++) {
            if(!m_host_grid[y * m_width + x]) {
                continue;
            }

            alive_count++;

            SDL_Rect rect {
                x * static_cast<int>(m_cellsize),
                y * static_cast<int>(m_cellsize),
                static_cast<int>(m_cellsize),
                static_cast<int>(m_cellsize)
            };

            SDL_RenderFillRect(m_prenderer, &rect);
        }
    }

    // Show the results on screen.
    SDL_RenderPresent(m_prenderer);

    // Update the window title to show generation, live cell count, and play state.
    std::stringstream ss;

    ss  << "Generation "
        << m_generation
        << " Alive "
        << alive_count
        << " ["
        << (m_paused ? "PAUSED" : "RUNNING")
        << "]";

    SDL_SetWindowTitle(m_pwindow, ss.str().c_str());

    // Cap the loop to ~60 FPS so the application is smooth but not too CPU-heavy.
    SDL_Delay(C_DELAY_VALUE);    
}
}//application