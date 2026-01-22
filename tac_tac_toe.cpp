#include <bits/stdc++.h>

/**
 * tic tac toe impl-ing lol
 * 
 * player 1 win = +1 => maximizer
 * player 2 win = -1 => minimizer
 * 
 * player 1 = agent, goes first
 */

struct move {
    int loc;
    int player;
};

struct state {
    static constexpr std::array<std::array<int, 3>, 8> WIN_LINES {{
        std::array<int,3>{0, 1, 2}, std::array<int,3>{3, 4, 5}, std::array<int,3>{6, 7, 8},
        std::array<int,3>{0, 3, 6}, std::array<int,3>{1, 4, 7}, std::array<int,3>{2, 5, 8},
        std::array<int,3>{0, 4, 8}, std::array<int,3>{2, 4, 6}
    }};

    std::array<int, 9> board{};
    int played = 0;

    int eval() const {
        for (int t = 1; t <= 2; t++) {
            for (const auto &[a, b, c] : WIN_LINES) {
                if (board[a] == t && board[b] == t && board[c] == t) {
                    return (t == 1 ? 1 : -1);
                }
            }
        }

        return 0;
    }

    bool terminal() const {
        if (eval() != 0) return true;
        for (int i = 0; i < 9; i++) {
            if (board[i] == 0) return false;
        }

        return true;
    }

    std::vector<move> legal_moves() const {
        std::vector<move> moves;
        int player = (played % 2 == 0 ? 1 : 2);
        for (int i = 0; i < 9; i++) {
            if (!board[i]) moves.push_back(move{i, player});
        }

        return moves;
    }   

    void apply(const move &m) {
        const auto &[loc, player] = m;
        board[loc] = player;
        played++;
    }

    void undo(const move &m) {
        const auto &[loc, player] = m;
        board[loc] = 0;
        played--;
    }
};

template<class move_t, class state_t, int MAX_DEP> struct agent {
    state_t cur;

    int minimax(bool is_maximizer) {
        if (cur.terminal()) {
            return cur.eval();
        }

        auto g = cur.legal_moves();
        if (is_maximizer) {
            int val = INT_MIN;
            for (const auto &m: g) {
                cur.apply(m);
                val = std::max(val, minimax(!is_maximizer));
                cur.undo(m);
            }

            return val;
        } else {
            int val = INT_MAX;
            for (const auto &m : g) {
                cur.apply(m);
                val = std::min(val, minimax(!is_maximizer));
                cur.undo(m);
            }

            return val;
        }
    }

    int minimax_alpha_beta(int dep, int alpha, int beta, bool is_maximizer) {
        if (dep == 0 || cur.terminal()) {
            return cur.eval();
        }

        auto g = cur.legal_moves();
        if (is_maximizer) {
            int val = INT_MIN;
            for (const auto &m: g) {
                cur.apply(m);
                val = std::max(val, minimax_alpha_beta(dep - 1, alpha, beta, !is_maximizer));
                cur.undo(m);

                if (val >= beta) break;
                alpha = std::max(alpha, val);
            }

            return val;
        } else {
            int val = INT_MAX;
            for (const auto &m : g) {
                cur.apply(m);
                val = std::min(val, minimax_alpha_beta(dep - 1, alpha, beta, !is_maximizer));
                cur.undo(m);

                if (val <= alpha) break;
                beta = std::min(beta, val);
            }

            return val;
        }
    } 

    void apply_move(const move_t &m) {
        cur.apply(m);
    }

    int get_best_move() {
        int pos = -1;
        int val = INT_MIN;
        auto g = cur.legal_moves();
        for (const auto &m : g) {
            cur.apply(m);
            int get = minimax_alpha_beta(MAX_DEP - 1, INT_MIN, INT_MAX, false);
            if (get > val) {
                pos = m.loc;
                val = get;
            }

            cur.undo(m);
        }

        return pos;
    }
};

// all the actual game stuff

void print_board(const state &s) {
    auto sym = [&](int v) {
        if (v == 1) return 'X';
        if (v == 2) return 'O';
        return '.';
    };

    std::cout << "\n";
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            std::cout << sym(s.board[r * 3 + c]);
            if (c < 2) std::cout << " | ";
        }
        std::cout << "\n";
        if (r < 2) std::cout << "--+---+--\n";
    }
    std::cout << "\n";
}

int read_human_move(const state &s) {
    while (true) {
        std::cout << "Your move (0-8): ";
        int pos;
        if (!(std::cin >> pos)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        if (pos < 0 || pos > 8 || s.board[pos] != 0) {
            std::cout << "Invalid move.\n";
            continue;
        }
        return pos;
    }
}

int main() {
    agent<move, state, 9> ai;

    std::cout << "Tic-Tac-Toe\n";
    std::cout << "You are O (player 2)\n";
    std::cout << "Positions:\n";
    std::cout << "0 | 1 | 2\n";
    std::cout << "--+---+--\n";
    std::cout << "3 | 4 | 5\n";
    std::cout << "--+---+--\n";
    std::cout << "6 | 7 | 8\n";

    print_board(ai.cur);

    while (!ai.cur.terminal()) {
        // AI move (player 1)
        int best = ai.get_best_move();
        std::cout << "AI plays: " << best << "\n";
        ai.apply_move(move{best, 1});
        print_board(ai.cur);

        if (ai.cur.terminal()) break;

        // Human move (player 2)
        int human = read_human_move(ai.cur);
        ai.apply_move(move{human, 2});
        print_board(ai.cur);
    }

    int result = ai.cur.eval();
    if (result == 1) {
        std::cout << "AI wins!\n";
    } else if (result == -1) {
        std::cout << "You win!\n";
    } else {
        std::cout << "Draw.\n";
    }

    return 0;
}