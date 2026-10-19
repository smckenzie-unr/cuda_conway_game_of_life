#include "arg_parser.h"

constexpr uint64_t C_DEFAULT_ROWS(100);
constexpr uint64_t C_DEFAULT_COLS(100);
constexpr uint64_t C_DEFAULT_CELL_SIZE(8);

namespace arguments {

arg_parser::arg_parser(
    int argc, 
    char* argv[]
)
: m_num_rows(C_DEFAULT_ROWS)
, m_num_cols(C_DEFAULT_COLS)
, m_cell_size(C_DEFAULT_CELL_SIZE)
{
    error_codes code(error_codes::successful);

    code = this->parse_arguments(argc, argv);
    if (code != error_codes::successful) {
        throw code;
    }

#ifdef DEBUG
    std::cout << std::endl 
              << "Num Rows: " << m_num_rows << std::endl
              << "Num Cols: " << m_num_cols << std::endl
              << "Cell Size: " << m_cell_size << std::endl
              << std::endl;
#endif
}

void arg_parser::display_helpmenu() noexcept 
{
    using namespace std;

    const int set_width(25);

    cout << endl << "Usage: ./nvidia_conways_gol [-h] [-r] [-c] [-k]" << endl
         << endl
         << "Simulation of Conways game of life using a Nvidia gpu to calculate and display Cells." 
         << endl << endl
         << "Optional Arguments:" << endl;
    cout << left << setw(set_width) << "    -h , --help" << "Display help menu for this application."
         << endl;
    cout << left << setw(set_width) << "    -r , --rows" << "Set the number of rows." << endl;
    cout << left << setw(set_width) << "    -c , --columns" << "Set the number of columns." << endl;
    cout << left << setw(set_width) << "    -k , --cell_size" << "Sets the cell size." << endl;
    cout << endl;
}

error_codes arg_parser::parse_arguments(
    int argc,
    char* argv[]
) noexcept 
{
    for (uint64_t i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);

        if (arg == "-h" || arg == "--help") {
            return error_codes::help_menu;
        }

        if ((arg == "-r" || arg == "--rows") && i + 1 < argc) {
            try {
                m_num_rows = static_cast<decltype(m_num_rows)>(std::stoi(argv[++i]));
            }
            catch(const std::invalid_argument& error) {
                std::cerr << "Invalid argument given for number of rows. Received: " << argv[i]
                          << " Caught error: " << error.what() << std::endl;
                return error_codes::invalid_arguments;
            }
        }
        else if ((arg == "-c" || arg == "--columns") && i + 1 < argc)
        {
            try {
                m_num_cols = std::stoi(argv[++i]);
            }
            catch(const std::invalid_argument& error) {
                std::cerr << "Invalid argument given for number of columns. Received: " << argv[i]
                          << " Caught error: " << error.what() << std::endl;
                return error_codes::invalid_arguments;
            }
        }
        else if ((arg == "-k" || arg == "--cell_size") && i + 1 < argc)
        {
            try {
                m_cell_size = std::stoi(argv[++i]);
            }
            catch(const std::invalid_argument& error) {
                std::cerr << "Invalid argument given for cell size. Received: " << argv[i]
                          << " Caught error: " << error.what() << std::endl;
                return error_codes::invalid_arguments;
            }
        }
    }
    
    return error_codes::successful;
}

} //arguments