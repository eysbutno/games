#include "solver.cpp"
#include "thread_pool.hpp"
#include <map>
#include <unordered_set>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>

std::vector<position> generate_positions(int depth) {
    std::unordered_set<position::pos_t> seen;
    std::vector<position> positions;

    const auto dfs = [&](const position &cur, int dep, auto &&self) -> void {
        if (cur.has_winning_move() || !cur.non_losing_moves()) return;

        if (dep == 0) {
            auto key = cur.to_b3();
            if (!seen.count(key)) {
                seen.insert(key);
                positions.push_back(cur);
            }

            return;
        }

        for (int i = 0; i < position::WIDTH; i++) {
            if (cur.can_play(i)) {
                position nxt = cur;
                nxt.play_col(i);
                self(nxt, dep - 1, self);
            }
        }
    };

    dfs(position{}, depth, dfs);

    return positions;
}

int main() {
    constexpr int DEPTH = 8;
    const std::string FILE_PATH = "8-ply.bin";
    
    auto start = std::chrono::steady_clock::now();
    thread_pool tasks{};

    std::vector<std::pair<uint64_t, uint8_t>> results;

    for (int d = DEPTH; d >= 0; d--) {
        solver::book.load(FILE_PATH);
        
        auto positions = generate_positions(DEPTH);
        std::vector<std::pair<uint64_t, std::future<int>>> pending(positions.size());
        for (int i = 0; i < positions.size(); i++) {
            auto key = positions[i].to_b3();
            auto score = tasks.submit(solver::solve, positions[i], false);
            pending[i] = {key, std::move(score)};
        }

        for (auto &[key, score] : pending) {
            uint8_t mapped = score.get() + position::WIN;
            results.push_back({key, mapped});
        }

        std::ofstream fout(FILE_PATH, std::ios::binary);

        size_t size = results.size();
        fout.write(reinterpret_cast<const char*>(&size), sizeof(size));
        for (auto &[key, score] : results) {
            fout.write(reinterpret_cast<const char*>(&key), sizeof(key));
            fout.write(reinterpret_cast<const char*>(&score), sizeof(score));
        }
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration_seconds = end - start;
    std::cout << "Time elapsed: " << duration_seconds.count() << " seconds" << '\n';
}