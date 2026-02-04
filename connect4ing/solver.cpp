#include "position.hpp"
#include "transposition_table.hpp"
#include "move_sorter.hpp"
#include <array>
#include <iostream>

constexpr int TABLE_SIZE = (1 << 23) + 9;

int negamax(const position &cur, int alpha, int beta) {
    using key_t = uint64_t;
    using val_t = uint8_t;
    static transposition_table<key_t, val_t, TABLE_SIZE> memo;

    auto valid = cur.non_losing_moves();
    if (valid == 0) {
        return -(position::WIDTH * position::HEIGHT - cur.moves) / 2;
    }

    if (cur.has_winning_move()) {
        return (position::WIDTH * position::HEIGHT + 1 - cur.moves) / 2;
    }

    if (cur.moves == position::HEIGHT * position::WIDTH) {
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
    if (memo.has(key)) {
        auto val = memo.get(key);

        // encode the values like:
        // lower bound:  0 <= x <= 44,  (true value) + WIN
        // exact bound: 45 <= x <= 89,  (true value) + 3 * WIN + 1
        if (val <= 2 * position::WIN) {
            int lower = val - position::WIN;
            if (alpha < lower) alpha = lower;
            if (alpha >= beta) return alpha;
        } else {
            int exact = val - 3 * position::WIN - 1;
            return exact;
        }
    }

    move_sorter moves;
    for (int i = 0; i < position::WIDTH; i++) {
        if ((cur.column_mask(i) & valid) == 0) continue;
        moves.add(i, cur.get_score(i));
    }

    int move = -1;
    int exact = -position::WIN - 1;
    while ((move = moves.get_next()) != -1) {
        if ((position::column_mask(move) & valid) == 0) {
            // we already know this move sucks, ignore
            continue;
        }

        position nxt = cur;
        nxt.play(move);

        int calc = negamax(nxt, -beta, -alpha);
        if (calc > exact) exact = calc;
        if (calc > alpha) alpha = calc;
        if (alpha >= beta) {
            memo.put(key, alpha + position::WIN);
            return alpha;
        }
    }

    memo.put(key, exact + 3 * position::WIN + 1);
    return exact;
}

int solve(const position &cur, bool weak) {
    if (cur.has_winning_move()) {
        return (position::WIDTH * position::HEIGHT + 1 - cur.moves) / 2;
    }

    int min = -(position::WIDTH * position::HEIGHT - cur.moves) / 2;
    int max = (position::WIDTH * position::HEIGHT + 1 - cur.moves) / 2;

    if (weak) {
        min = -1, max = 1;
    }

    // we basically use binary search to narrow window until converges
    while (min < max) {
        int med = min + (max - min) / 2;
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
    if (cur.has_winning_move()) {
        for (int i = 0; i < position::WIDTH; i++) {
            if (cur.is_winning_move(i)) return i;
        }
    }

    auto valid = cur.non_losing_moves();
    move_sorter moves;
    for (int i = 0; i < position::WIDTH; i++) {
        if ((cur.column_mask(i) & valid) == 0) continue;
        moves.add(i, cur.get_score(i));
        // std::cout << i << " " << cur.get_score(i) << "\n";
    }

    int best = -position::WIN - 1;
    int optimal = -1;
    int move;
    while ((move = moves.get_next()) != -1) {
        // std::cout << "try " << move << '\n';
        position nxt = cur;
        nxt.play(move);

        int calc = -negamax(cur, -1, 1); // -solve(nxt, weak);
        if (calc > best) best = calc, optimal = move;
        if (weak && best >= 1) break;
    }

    std::cout << "found move: " << optimal + 1 << " " << best << "\n";
    return optimal;
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
    for (int c = 0; c < position::WIDTH; c++) {
        std::cout << c << " ";
    }
    std::cout << "\n";
}

void clear_screen() {
    // portable enough for most terminals
    std::cout << "\033[2J\033[H";
}

int main() {
    position cur("44444136666452");
    bool human_turn = false;
    bool weak = true; // set false for full-strength solver

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

            cur.play(col);
            human_turn = false;
        } else {
            std::cout << "AI thinking...\n";
            int ai_move = get_best_move(cur, weak);
            cur.play(ai_move);
            human_turn = true;
        }
    }

    print_header();
    print_board(cur);
    return 0;
}