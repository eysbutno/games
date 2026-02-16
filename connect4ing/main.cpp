#include "solver.hpp"

using namespace solver;

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
    position cur("12345671");
    // position cur;
    bool human_turn = false;

    auto start = std::chrono::steady_clock::now();
    // std::cout << solver::solve(cur, false, position::WIN) << '\n';
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration_seconds = end - start;
    std::cout << "Time elapsed: " << duration_seconds.count() << " seconds" << '\n';
    // std::cout << solver::num_good << '\n';
    /*

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
    */
}