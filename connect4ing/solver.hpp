#include <array>
#include "transposition_table.hpp"
#include "opening_book.hpp"
#include "thread_pool.hpp"
#include "position.hpp"
#include "move_sorter.hpp"

namespace solver {
    static constexpr int TABLE_SIZE = 1 << 23;
    using key_t = uint64_t;
    using val_t = uint8_t;
    thread_local transposition_table<key_t, val_t, TABLE_SIZE> memo;
    
    static constexpr int INVALID_MOVE = -1000;
    constexpr std::array<int, 7> ORDER = {0, 6, 1, 5, 2, 4, 3};

    opening_book book;

    thread_pool tasks{};

    int negamax(const position &cur, int alpha, int beta);

    int solve(const position &cur, bool weak);

    std::array<int, position::WIDTH> analyze(const position &cur, bool weak);
};