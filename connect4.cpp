#include <vector>
#include "templates/agent_optimized.h"

struct move {
    int column = -1;
    bool operator==(const move &s) const = default;
};

struct state {
    int eval() const {
        // write the heuristic
    }

    bool terminal() const {
        // returns if state is terminal
    }

    std::vector<move> legal_moves() const {
        
    }   

    void apply(const move &m) {
        
    }

    void undo(const move &m) {
        
    }

    bool operator==(const state &s) const = default;
};

struct state_hasher {
    std::size_t operator()(const state &s) const {
        // zobrist hash it? idk
    }
};