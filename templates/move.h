/**
 * Moves are just information for transitioning to a state.
 */

struct move {
    bool operator==(const move &s) const = default;
};