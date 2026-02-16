#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <optional>
#include "position.hpp"
#include "hash.hpp"

struct opening_book {
    const int MAX_DEPTH = 8;
    std::vector<uint8_t> scores;

    opening_book() = default;

    opening_book(const std::string &file_name) {
        load(file_name);
    }

    void load(const std::string &file_name) {
        scores.clear();
        if (!std::filesystem::exists(file_name)) return;

        std::ifstream fin(file_name, std::ios::binary);
        assert(fin && "failed to open file");

        size_t size = 0;
        fin.read(reinterpret_cast<char*>(&size), sizeof(size));
        std::vector<std::pair<uint64_t, uint8_t>> entries(size);

        uint64_t max_key = 0;
        for (int i = 0; i < size; i++) {
            uint64_t key;
            uint8_t score;
            fin.read(reinterpret_cast<char*>(&key), sizeof(key));
            fin.read(reinterpret_cast<char*>(&score), sizeof(score));
            entries[i] = {key, score};
            if (key > max_key) max_key = key;
        }

        scores.resize(max_key + 1);
        for (const auto &[key, score] : entries) {
            scores[key] = score;
        }
    }

    int get_minimax(const position &cur) {
        if (cur.moves > MAX_DEPTH) return 0;
        auto hash = cur.to_b3();
        return hash < scores.size() ? scores[hash] : 0;
    }
};