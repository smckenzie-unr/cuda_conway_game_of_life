#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace arguments {
enum class error_codes : int64_t {
    successful        = 0,
    help_menu         = -1,
    invalid_arguments = -2
};

class arg_parser final {
public:
    arg_parser(const arg_parser &) = delete;
    arg_parser(arg_parser &&) = delete;
    explicit arg_parser(
        int argc, 
        char* argv[]
    );
    ~arg_parser() = default;

    inline uint64_t get_rows() const noexcept {
        return m_num_rows;
    }
    inline uint64_t get_columns() const noexcept {
        return m_num_cols;
    }
    inline uint64_t get_cell_size() const noexcept {
        return m_cell_size;
    }
    static void display_helpmenu() noexcept;

    arg_parser& operator =(const arg_parser &) = delete;
    arg_parser& operator =(arg_parser &&) = delete;

private:
    uint64_t m_num_rows;
    uint64_t m_num_cols;
    uint64_t m_cell_size;

    error_codes parse_arguments(
        int argc,
        char* argv[]
    ) noexcept;
};
}//arguments
#endif //__ARG_PARSER_H__