#include "solver.hpp"

int solver::negamax(const position &cur, int alpha, int beta) {
    auto valid = cur.non_losing_moves();
    if (valid == 0) {
        return -(position::WIDTH * position::HEIGHT - cur.moves) / 2;
    }

    if (cur.has_winning_move()) {
        return (position::WIDTH * position::HEIGHT + 1 - cur.moves) / 2;
    }

    if (cur.moves >= position::WIDTH * position::HEIGHT - 2) {
        return 0;
    }

    // since we know neither side can immediately win, we can adjust alpha/beta
    int min = -(position::WIDTH * position::HEIGHT - 2 - cur.moves) / 2;
    int max = (position::WIDTH * position::HEIGHT - 1 - cur.moves) / 2;

    if (alpha < min) {
        // tighten window, prune if needed
        alpha = min;
        if (alpha >= beta) return alpha;
    }

    if (max < beta) {
        // tighten window, prune if needed
        beta = max;
        if (alpha >= beta) return beta;
    }

    key_t key = cur.key();
    move_sorter moves;
    uint64_t start = 0;
    if (auto val = memo.get_val(key)) {
        if (val <= 2 * position::WIN) {
            int lower = val - position::WIN;
            if (alpha < lower) alpha = lower;
            if (alpha >= beta) return alpha;
        } else if (val <= 5 * position::WIN) {
            int exact = val - 4 * position::WIN;
            return exact;
        } else {
            int upper = val - 8 * position::WIN;
            if (upper < beta) beta = upper;
            if (alpha >= beta) return beta;
        }

        start = memo.get_move(key);
        moves.add(start, 255);
    }

    for (int i : ORDER) {
        if (uint64_t move = valid & position::column_mask(i)) {
            if (move != start) moves.add(move, cur.get_score(move));
        }
    }

    int exact = -position::WIN - 1;
    int original_alpha = alpha;
    uint64_t best_move = 0;
    while (uint64_t move = moves.get_next()) {
        position nxt = cur;
        nxt.play(move);

        int calc = -negamax(nxt, -beta, -alpha);
        if (calc > exact) exact = calc, best_move = move;
        if (calc > alpha) alpha = calc;
        if (alpha >= beta) {
            memo.put(key, best_move, alpha + position::WIN);
            return alpha;
        }
    }

    if (exact <= original_alpha) {
        memo.put(key, best_move, exact + 8 * position::WIN);
    } else {
        memo.put(key, best_move, exact + 4 * position::WIN);
    }
    
    return exact;
}

int solver::solve(const position &cur, bool weak) {
    if (cur.has_winning_move()) {
        return (position::WIDTH * position::HEIGHT + 1 - cur.moves) / 2;
    }

    if (!cur.non_losing_moves()) {
        return -(position::WIDTH * position::HEIGHT - cur.moves) / 2;
    }

    int min = -(position::WIDTH * position::HEIGHT - cur.moves) / 2;
    int max = (position::WIDTH * position::HEIGHT + 1 - cur.moves) / 2;

    // we basically use binary search to narrow window until converges
    while (min < max) {
        int med = min + (max - min) / 2;
        if (med <= 0 && min / 2 < med) med = min / 2;
        else if (med >= 0 && max / 2 > med) med = max / 2;
        int calc = solver::negamax(cur, med, med + 1);
        
        if (calc <= med) {
            max = calc;
        } else {
            min = calc;
        }
    }

    return min;
}

std::array<int, position::WIDTH> solver::analyze(const position &cur, bool weak) {
    std::array<std::future<int>, position::WIDTH> results{};
    for (int i = 0; i < position::WIDTH; i++) {
        results[i] = tasks.submit([=] {
            if (!cur.can_play(i)) return solver::INVALID_MOVE;
            auto nxt = cur;
            nxt.play_col(i);
            return solver::solve(nxt, weak);
        });
    }

    std::array<int, position::WIDTH> pulled{};
    for (int i = 0; i < position::WIDTH; i++) {
        pulled[i] = results[i].get();
    }

    return pulled;
}