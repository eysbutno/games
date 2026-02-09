#include "position.hpp"
#include <cstdint>

struct move_sorter {
    struct {
        position::pos_t move;
        int score;
    } entries[position::WIDTH];
    
    int size = 0;

    void add(position::pos_t move, int score) {
        int loc = size++;
        for (; loc && entries[loc - 1].score > score; loc--) {
            entries[loc] = entries[loc - 1];
        }

        entries[loc].move = move;
        entries[loc].score = score;
    }

    position::pos_t get_next() {
        if (size == 0) return 0;
        return entries[--size].move;
    }
};