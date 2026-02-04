#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream> // for debugging, i'm lazy

enum memo_type {
    LOWER, UPPER, EXACT
};

constexpr int INF = 1e9;

template <class state_t, class move_t, class state_hasher_t> struct agent_optimized {
     struct memo_info {
        int value = 0;
        int depth = -1;
        memo_type bound = EXACT;
        move_t best{};
    };

    state_t cur{};
    std::unordered_map<state_t, memo_info, state_hasher_t> table;

    agent_optimized(int init_size = 0) {
        if (init_size > 0) {
            table.reserve(init_size);
        }
    }

    int negamax(int dep, int alpha, int beta) {
        if (dep == 0 || cur.terminal()) {
            return cur.eval();
        }

        auto g = cur.legal_moves();
        assert(g.size() >= 1);

        int alpha_original = alpha;
        auto &entry = table[cur];
        if (entry.depth >= dep) {
            if (entry.bound == EXACT) {
                return entry.value;
            } else if (entry.bound == LOWER) {
                alpha = std::max(alpha, entry.value);
            } else if (entry.bound == UPPER) {
                beta = std::min(beta, entry.value);
            }

            if (alpha >= beta) {
                return entry.value; 
            }
        } 
        
        if (entry.depth != -1) {
            auto first_move = entry.best;
            for (int i = 0; i < g.size(); i++) {
                if (g[i] == first_move) {
                    std::swap(g[0], g[i]);
                    break;
                }
            }
        }

        int val = -INF;
        move_t optimal;
        for (const auto &m : g) {
            cur.apply(m);

            int calc = -negamax(dep - 1, -beta, -alpha);
            if (calc > val) {
                val = calc;
                optimal = m;
            }

            cur.undo(m);

            alpha = std::max(alpha, val);
            if (alpha >= beta) break;
        }

        entry.depth = dep;
        entry.value = val;
        entry.best = optimal;
        if (val <= alpha_original) {
            entry.bound = UPPER;
        } else if (val >= beta) {
            entry.bound = LOWER;
        } else {
            entry.bound = EXACT;
        }

        return val;
    }

    void apply_move(const move_t &m) {
        cur.apply(m);
    }

    move_t get_best_move(double max_seconds) {
        auto start = std::chrono::high_resolution_clock::now();  
        auto g = cur.legal_moves();

        for (int d = 1; ; d++) {
            int val = -INF;
            move_t optimal{};
            for (const auto &m : g) {
                cur.apply(m);

                int calc = -negamax(d - 1, -INF, INF);
                if (calc > val) {
                    val = calc;
                    optimal = m;
                }

                cur.undo(m);
            }

            auto stop = std::chrono::high_resolution_clock::now();
            double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(stop - start).count();
            if (seconds >= max_seconds) {
                std::cout << d << ' ' << val << ' ' << table.size() << '\n';
                return optimal;
            }
        }

        assert(false);
    }
};