#include <array>
#include <vector>
#include "move.h"

/**
 * Stores the state of our game.
 * 
 * Needs to implement the following functions for the code to work.
 */

struct state {
    int eval() const {
        
    }

    bool terminal() const {
        
    }

    std::vector<move> legal_moves() const {
        
    }   

    void apply(const move &m) {
        
    }

    void undo(const move &m) {
        
    }

    bool operator==(const state &s) const = default;
};