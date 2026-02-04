#include "position.hpp"

struct move_sorter {
    struct {
        int move;
        int score;
    } entries[position::WIDTH];
    
    int size = 0;

    void add(int move, int score) {
        int loc = size++;
        for (; loc && entries[loc - 1].score > score; loc--) {
            entries[loc] = entries[loc - 1];
        }

        entries[loc].move = move;
        entries[loc].score = score;
    }

    int get_next() {
        if (size == 0) return -1;
        return entries[--size].move;
    }
};