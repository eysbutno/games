#include <vector>
#include <array>
#include <iostream>
#include "templates/agent_optimized.h"

struct move {
    int column = -1;
    bool operator==(const move &s) const = default;
};

constexpr int WIN = 100;

struct state {
    std::array<int, 7> next_cell{};
    int player = 1;

    std::array<std::uint64_t, 3> hashes{};
    
    inline int get_val(int col, int row) const {
        const int pos = col * 7 + row;
        if (hashes[1] >> pos & 1) return 1;
        if (hashes[2] >> pos & 1) return 2;
        return 0;
    }

    int eval() const {
        constexpr int WIN_SCORE = 1'000'000;
        constexpr int W3 = 1'000;
        constexpr int W2 = 50;
        constexpr int W1 = 1;

        auto other = 3 - player;
        int score = 0;

        auto eval_window = [&](int c0, int r0, int dc, int dr) -> int {
            int cnt_player = 0;
            int cnt_other = 0;
            int empties = 0;

            for (int k = 0; k < 4; ++k) {
                int c = c0 + dc * k;
                int r = r0 + dr * k;

                int v = get_val(c, r);
                if (v == player) ++cnt_player;
                else if (v == other) ++cnt_other;
                else ++empties;
            }

            // dead window
            if (cnt_player > 0 && cnt_other > 0) return 0;

            // immediate win / loss
            if (cnt_player == 4) return WIN_SCORE;
            if (cnt_other == 4) return -WIN_SCORE;

            // score for player
            if (cnt_player == 3 && empties == 1) return W3;
            if (cnt_player == 2 && empties == 2) return W2;
            if (cnt_player == 1 && empties == 3) return W1;

            // score for opponent (negative, scaled stronger)
            if (cnt_other == 3 && empties == 1) return -W3;
            if (cnt_other == 2 && empties == 2) return -W2;
            if (cnt_other == 1 && empties == 3) return -W1;

            return 0;
        };

        for (int r = 0; r < 6; ++r) {
            for (int c = 0; c <= 7 - 4; ++c) {
                score += eval_window(c, r, 1, 0);
            }
        }

        for (int c = 0; c < 7; ++c) {
            for (int r = 0; r <= 6 - 4; ++r) {
                score += eval_window(c, r, 0, 1);
            }
        }

        for (int c = 0; c <= 7 - 4; ++c) {
            for (int r = 0; r <= 6 - 4; ++r) {
                score += eval_window(c, r, 1, 1);
            }
        }

        for (int c = 0; c <= 7 - 4; ++c) {
            for (int r = 3; r < 6; ++r) {
                score += eval_window(c, r, 1, -1);
            }
        }

        for (int r = 0; r < 6; ++r) {
            int v = get_val(3, r);
            if (v == player) score += 6;
            else if (v == other) score -= 6;
        }

        return score;
    };

    bool terminal() const {
        for (int t = 1; t <= 2; t++) {
            for (int i = 0; i < 7; i++) {
                for (int j = 0; j < 6; j++) {
                    bool up = j + 3 < 6;
                    bool ur = up && i + 3 < 7;
                    bool dr = i + 3 < 7 && j >= 3;
                    bool ri = i + 3 < 7;

                    for (int k = 0; k <= 3; k++) {
                        if (up) up &= get_val(i, j + k) == t;
                        if (ur) ur &= get_val(i + k, j + k) == t;
                        if (dr) dr &= get_val(i + k, j - k) == t;
                        if (ri) ri &= get_val(i + k, j) == t;
                    }

                    if (up || ur || dr || ri) return true;
                }
            }
        }

        for (int i = 0; i < 7; i++) {
            if (next_cell[i] < 6) return false;
        }

        return true;
    }

    std::vector<move> legal_moves() const {
        std::vector<move> res;
        for (int i = 0; i < 7; i++) {
            if (next_cell[i] != 6) res.push_back(move{i});
        }

        return res;
    }   

    void apply(const move &m) {
        int v = m.column;
        assert(0 <= v && v < 7);
        assert(next_cell[v] < 6);

        int loc = v * 7 + next_cell[v];
        hashes[player] ^= std::uint64_t(1) << loc;

        player = 3 - player;
        next_cell[v]++;
    }

    void undo(const move &m) {
        int v = m.column;
        assert(0 <= v && v < 7);
        assert(next_cell[v] > 0);
        --next_cell[v];

        int loc = v * 7 + next_cell[v];
        hashes[get_val(v, next_cell[v])] ^= std::uint64_t(1) << loc;

        player = 3 - player;
    }

    bool operator==(const state &s) const = default;
};

struct state_hasher {
    static uint64_t splitmix64(uint64_t x) {
		// http://xorshift.di.unimi.it/splitmix64.c
		x += 0x9e3779b97f4a7c15;
		x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
		x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
		return x ^ (x >> 31);
	}

    static std::uint64_t pair_hasher(std::uint64_t first, std::uint64_t second) {
        static const uint64_t FIXED_RANDOM = std::chrono::steady_clock::now().time_since_epoch().count();
		return splitmix64(first + FIXED_RANDOM)^(splitmix64(second + FIXED_RANDOM) >> 1);
    }

    std::size_t operator()(const state &s) const {
        // should rewrite to properly use zobrist hashing later so can avoid rehashing lmao
        return pair_hasher(s.hashes[1], s.hashes[2]);
    }
};

void print_board(const state &s) {
    for (int r = 5; r >= 0; --r) { // print top row first
        for (int c = 0; c < 7; ++c) {
            int cell = s.get_val(c, r);
            if (cell == 0) std::cout << ". ";
            else if (cell == 1) std::cout << "X ";
            else std::cout << "O ";
        }
        std::cout << "\n";
    }
    std::cout << "1 2 3 4 5 6 7\n\n";
}

int main() {
    const int table_size = 1e8;
    agent_optimized<state, move, state_hasher> ai(table_size);

    bool player_turn = false; // human = X]
    int so_far = 0;
    while (!ai.cur.terminal()) {
        print_board(ai.cur);

        if (player_turn) {
            int col;
            std::cout << "Your move (1-7): ";
            std::cin >> col;
            --col;

            if (col < 0 || col > 6 || ai.cur.next_cell[col] >= 6) {
                std::cout << "Invalid move! Try again.\n";
                continue;
            }

            ai.cur.apply(move{col});
        } else {
            std::cout << "AI is thinking...\n";
            move best{};
            if (so_far == 0) {
                best = move{3};
            } else {
                best = ai.get_best_move(300.0); // 1 second per move
            }
            
            ai.cur.apply(best);
            std::cout << "AI plays column " << best.column + 1 << "\n";
        }

        player_turn = !player_turn;
        so_far++;
    }

    print_board(ai.cur);

    // Determine winner
    bool x_win = false, o_win = false;
    for (int t = 1; t <= 2; t++) {
        // reuse your terminal check code partially
        for (int i = 0; i < 7 && !(x_win || o_win); i++) {
            for (int j = 0; j < 6 && !(x_win || o_win); j++) {
                bool up = j + 3 < 6;
                bool ur = up && i + 3 < 7;
                bool dr = i + 3 < 7 && j >= 3;
                bool ri = i + 3 < 7;

                for (int k = 0; k <= 3; k++) {
                    if (up) up &= ai.cur.get_val(i, j + k) == t;
                    if (ur) ur &= ai.cur.get_val(i + k, j + k) == t;
                    if (dr) dr &= ai.cur.get_val(i + k, j - k) == t;
                    if (ri) ri &= ai.cur.get_val(i + k, j) == t;
                }

                if (up || ur || dr || ri) {
                    if (t == 1) x_win = true;
                    else o_win = true;
                }
            }
        }
    }

    if (x_win) std::cout << "You win!\n";
    else if (o_win) std::cout << "AI wins!\n";
    else std::cout << "Draw!\n";

    return 0;
}