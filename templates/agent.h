#include <climits>

/**
 * Implements minimax with alpha-beta pruning.
 * 
 * By default, the maximizer is the agent, and the minimizer is the opponent.
 */

template<class move_t, class state_t, int MAX_DEP> struct agent {
    state_t cur;

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

// below is the regular minimax impl lol

/*
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

    
*/