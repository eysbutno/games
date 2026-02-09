#include "position.hpp"
#include "transposition_table.hpp"
#include "move_sorter.hpp"
#include <chrono>
#include <array>
#include <iostream>

constexpr int TABLE_SIZE = 1 << 25;
constexpr std::array<int, 7> ORDER = {0, 6, 1, 5, 2, 4, 3};

int nodes = 0;

int negamax(const position &cur, int alpha, int beta) {
    using key_t = uint64_t;
    using val_t = uint8_t;
    static transposition_table<key_t, val_t, TABLE_SIZE> memo;

    ++nodes;

    auto valid = cur.non_losing_moves();
    if (valid == 0) {
        return -(position::WIDTH * position::HEIGHT - cur.moves) / 2;
    }

    if (cur.has_winning_move()) {
        return (position::WIDTH * position::HEIGHT + 1 - cur.moves) / 2;
    }

    if (cur.moves >= position::WIDTH * position::HEIGHT - 2) {
        return 0;
    }

    // since we know neither side can immediately win, we can adjust alpha/beta
    int min = -(position::WIDTH * position::HEIGHT - 2 - cur.moves) / 2;
    int max = (position::WIDTH * position::HEIGHT - 1 - cur.moves) / 2;

    if (alpha < min) {
        // tighten window, prune if needed
        alpha = min;
        if (alpha >= beta) return alpha;
    }

    if (max < beta) {
        // tighten window, prune if needed
        beta = max;
        if (alpha >= beta) return beta;
    }

    auto key = cur.key();
    move_sorter moves;
    uint64_t start = 0;
    if (memo.has(key)) {
        auto val = memo.get_val(key);

        if (val <= 2 * position::WIN) {
            int lower = val - position::WIN;
            if (alpha < lower) alpha = lower;
            if (alpha >= beta) return alpha;
        } else if (val <= 5 * position::WIN) {
            int exact = val - 4 * position::WIN;
            return exact;
        } else {
            int upper = val - 8 * position::WIN;
            if (upper < beta) beta = upper;
            if (alpha >= beta) return beta;
        }

        start = memo.get_move(key);
        moves.add(start, 255);
    }

    
    for (int i : ORDER) {
        if (uint64_t move = valid & position::column_mask(i)) {
            if (move != start) moves.add(move, cur.get_score(move));
        }
    }

    int exact = -position::WIN - 1;
    int original_alpha = alpha;
    uint64_t best_move = 0;
    while (uint64_t move = moves.get_next()) {
        position nxt = cur;
        nxt.play(move);

        int calc = -negamax(nxt, -beta, -alpha);
        if (calc > exact) exact = calc, best_move = move;
        if (calc > alpha) alpha = calc;
        if (alpha >= beta) {
            memo.put(key, best_move, alpha + position::WIN);
            return alpha;
        }
    }

    if (exact <= original_alpha) {
        memo.put(key, best_move, exact + 8 * position::WIN);
    } else {
        memo.put(key, best_move, exact + 4 * position::WIN);
    }

    return exact;
}

int solve(const position &cur, int beta, bool weak) {
    if (weak) {
        if (beta < 1) return negamax(cur, -1, beta);
        return negamax(cur, -1, 1);
    }

    if (cur.has_winning_move()) {
        return (position::WIDTH * position::HEIGHT + 1 - cur.moves) / 2;
    }

    int min = -(position::WIDTH * position::HEIGHT - cur.moves) / 2;
    int max = (position::WIDTH * position::HEIGHT + 1 - cur.moves) / 2;
    if (beta < max) max = beta;

    // we basically use binary search to narrow window until converges
    while (min < max) {
        int med = min + (max - min) / 2;
        if (med <= 0 && min / 2 < med) med = min / 2;
        else if (med >= 0 && max / 2 > med) med = max / 2;
        int calc = negamax(cur, med, med + 1);
        
        if (calc <= med) {
            max = calc;
        } else {
            min = calc;
        }
    }

    return min;
}

int get_best_move(const position &cur, bool weak = false) {
    auto start = std::chrono::steady_clock::now();
    if (cur.has_winning_move()) {
        for (int i = 0; i < position::WIDTH; i++) {
            if (cur.is_winning_move(i)) return i;
        }
    }

    auto valid = cur.non_losing_moves();
    move_sorter moves;
    for (int i : ORDER) {
        if (uint64_t move = valid & position::column_mask(i)) {
            moves.add(move, cur.get_score(move));
        }
    }

    int best = -position::WIN - 1;
    uint64_t optimal = 0;
    int move;
    while (uint64_t move = moves.get_next()) {
        // std::cout << "try " << move << ' ' << cur.get_score(move) << '\n';
        position nxt = cur;
        nxt.play(move);

        int calc = -solve(nxt, -best, weak);
        if (calc > best) best = calc, optimal = move;
        if (weak && best >= 1) break;
    }

    int col_move = -1;
    for (int i = 0; i < position::WIDTH; i++) {
        if (optimal & position::column_mask(i)) col_move = i;
    }

    assert(col_move >= 0);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration_seconds = end - start;
    std::cout << "Time elapsed: " << duration_seconds.count() << " seconds" << '\n';
    std::cout << "found move: " << col_move + 1 << " " << best << " " << nodes << '\n';
    nodes = 0;
    return col_move;
}

void print_board(const position &s) {
    for (int r = position::HEIGHT - 1; r >= 0; r--) {
        for (int c = 0; c < position::WIDTH; c++) {
            int pos = c * (position::HEIGHT + 1) + r;
            if (s.flip >> pos & 1) {
                int type = (s.moves + (s.board >> pos & 1)) & 1;
                std::cout << (type ? "X " : "O ");
            } else {
                std::cout << ". ";
            }
        }

        std::cout << "\n";
    }
}

void print_header() {
    std::cout << "\n";
    for (int c = 1; c <= position::WIDTH; c++) {
        std::cout << c << " ";
    }
    std::cout << "\n";
}

void clear_screen() {
    // portable enough for most terminals
    std::cout << "\033[2J\033[H";
}

int main() {
    position cur;
    // position cur;
    bool human_turn = false;
    bool weak = false; // set false for full-strength solver

    while (true) {
        clear_screen();
        print_header();
        print_board(cur);
        std::cout << "\n";
        // terminal state checks
        if (cur.has_winning_move()) {
            std::cout << (human_turn ? "AI wins!\n" : "You win!\n");
            break;
        }

        if (cur.moves == position::WIDTH * position::HEIGHT) {
            std::cout << "Draw.\n";
            break;
        }

        if (human_turn) {
            int col;
            std::cout << "Your move (1-7): ";
            std::cin >> col;
            col--;

            if (!std::cin || col < 0 || col >= position::WIDTH || !cur.can_play(col)) {
                std::cin.clear();
                std::cin.ignore(1024, '\n');
                std::cout << "Invalid move. Press Enter...";
                std::cin.get();
                std::cin.get();
                continue;
            }

            cur.play_col(col);
            human_turn = false;
        } else {
            std::cout << "AI thinking...\n";
            int ai_move = get_best_move(cur, weak);
            cur.play_col(ai_move);
            human_turn = true;
        }
    }

    print_header();
    print_board(cur);
    return 0;
}