#pragma once

#include <cstdint>
#include <cassert>
#include <string>
#include <array>

/**
 * store position w/ two integers
 * 
 * flip: all played positions so far
 * board: the board itself
 * 
 * when we play a move, we do the move then swap POVs
 */
struct position {
    using pos_t = uint64_t;
    static constexpr int WIDTH = 7;
    static constexpr int HEIGHT = 6;
    static constexpr int WIN = 1 + WIDTH * HEIGHT / 2;
    template<int width, int height> struct bottom {static constexpr pos_t mask = bottom<width - 1, height>::mask | pos_t(1) << (width - 1) * (height + 1);};
    template <int height> struct bottom<0, height> {static constexpr pos_t mask = 0;};

    static constexpr pos_t bottom_mask = bottom<WIDTH, HEIGHT>::mask;
    static constexpr pos_t board_mask = bottom_mask * ((pos_t(1) << HEIGHT) - 1);

    static constexpr std::array<int, 7> pow3 = {1, 3, 9, 27, 81, 243, 729};

    // board = bitmap of all pieces current player has
    // flip  = all the currently played positions
    pos_t board = 0;
    pos_t flip = 0;
    int moves = 0;

    position() : board(0), flip(0), moves(0) {}

    position(pos_t _board, pos_t _flip, int _moves) 
           : board(_board), flip(_flip), moves(_moves) {}

    position(const std::string &seq) : board(0), flip(0), moves(0) {
        for (char c : seq) {
            int move = c - '1';
            assert(can_play(move));
            play_col(move);
        }
    }

    void partial_key(uint64_t &key, int col) const {
        for (uint64_t bit = uint64_t(1) << (col * (HEIGHT + 1)); bit & flip; bit <<= 1) {
            key *= 3;
            if (bit & board) key += 1;
            else key += 2;
        }

        key *= 3;
    }

    /**
     * @return a symmetric, base 3 representation of our board
     */
    uint64_t to_b3() const {
        uint64_t key_f = 0;
        for (int i = 0; i < WIDTH; i++) {
            partial_key(key_f, i);
        }

        uint64_t key_r = 0;
        for (int i = WIDTH - 1; i >= 0; i--) {
            partial_key(key_r, i);
        }

        return key_f < key_r ? key_f / 3 : key_r / 3;
    }

    /**
     * @param col: the column we want to play in
     * @return if we can play in this column
     */
    bool can_play(int col) const {
        // check if the top cell in col (besides buffer) is empty
        // if so, we can place it here
        return (flip & top_mask_col(col)) == 0;
    }

    /**
     * Plays a possible move.
     * 
     * @param move: bitmap of the cell we place at.
     */
    void play(uint64_t move) {
        board ^= flip;
        flip |= move;
        moves++;
    }

    /**
     * Plays a possible move.
     * 
     * @param col: the column we want to play in
     */
    void play_col(int col) {
        assert(can_play(col));
        board ^= flip;

        // to adjust mask, we use bit carryover
        // 1111 + 0001 => 10000
        // can just use bitwise OR

        flip |= flip + bottom_mask_col(col);
        moves++;
    }

    /**
     * @param col: the column we want to play in
     * @return if this move results in a win.
     */
    bool is_winning_move(int col) const {
        pos_t upd = board | ((flip + bottom_mask_col(col)) & column_mask(col));
        return alignment(upd);
    }

    /**
     * @return if current player has a legal winning move.
     */
    bool has_winning_move() const {
        return possible() & compute_winning_position(board, flip);
    }

    /**
     * @return bitmap of all non-losing moves
     */
    pos_t non_losing_moves() const {
        pos_t possible_mask = possible();
        pos_t opponent_win = compute_winning_position(board ^ flip, flip);
        pos_t forced_moves = possible_mask & opponent_win;
        if (forced_moves) {
            if (forced_moves & (forced_moves - 1)) {
                return 0;
            }

            possible_mask = forced_moves;
        }

        // can't play into a win for opponent
        return possible_mask & ~(opponent_win >> 1);
    }

    /**
     * @return bitmap of all legal moves
     */
    pos_t possible() const {
        return (flip + bottom_mask) & board_mask;
    }

    /**
     * @return uniquely identifiable key for this position
     */
    pos_t key() const {
        return board + flip;
    }

    /**
     * @return winning cells for this position
     */
    pos_t get_winning() const {
        return compute_winning_position(board, flip);
    }

    /** 
     * @param move: bitmap of the move we want to play
     * @return how many "winning" cells are created after playing col
     */
    int get_score(uint64_t move) const {
        pos_t upd_board = board | move;
        pos_t upd_flip = flip | move;
        return __builtin_popcountll(compute_winning_position(upd_board, upd_flip));
    }

    /**
     * @param col: the column we want to play in
     * @return the topmost bit in a column (not including the buffer bit)
     */
    static constexpr pos_t top_mask_col(int col) {
        return pos_t(1) << (HEIGHT - 1 + col * (HEIGHT + 1));
    }

    /**
     * @param col: the column we want to play in
     * @return the lowest bit in the column
     */
    static constexpr pos_t bottom_mask_col(int col) {
        return pos_t(1) << (col * (HEIGHT + 1));
    }

    /**
     * @param col: the column we want to play in
     * @return a bitmap of all the legal cells in the column
     */
    static constexpr pos_t column_mask(int col) {
        return ((pos_t(1) << HEIGHT) - 1) << (col * (HEIGHT + 1));
    }

    /**
     * @param pos: the bitmap of the current player's tiles
     * @return if there exists a 4 in a row here
     */
    static bool alignment(pos_t pos) {
        // horizontal 
        pos_t m = pos & (pos >> (HEIGHT + 1));
        if(m & (m >> (2 * (HEIGHT + 1)))) return true;

        // diagonal 1
        m = pos & (pos >> HEIGHT);
        if(m & (m >> (2 * HEIGHT))) return true;

        // diagonal 2 
        m = pos & (pos >> (HEIGHT + 2));
        if(m & (m >> (2 * (HEIGHT + 2)))) return true;

        // vertical
        m = pos & (pos >> 1);
        if(m & (m >> 2)) return true;

        return false;
    }

    /**
     * @return all unfilled slots resulting in a win for current side
     */
    static pos_t compute_winning_position(pos_t position, pos_t mask) {
        // vertical
        pos_t r = (position << 1) & (position << 2) & (position << 3);

        // horizontal
        pos_t p = (position << (HEIGHT + 1)) & (position << 2 * (HEIGHT + 1));
        r |= p & (position << 3 * (HEIGHT + 1));
        r |= p & (position >> (HEIGHT + 1));
        p = (position >> (HEIGHT + 1)) & (position >> 2 * (HEIGHT + 1));
        r |= p & (position << (HEIGHT + 1));
        r |= p & (position >> 3 * (HEIGHT + 1));

        // diagonal 1
        p = (position << HEIGHT) & (position << 2 * HEIGHT);
        r |= p & (position << 3 * HEIGHT);
        r |= p & (position >> HEIGHT);
        p = (position >> HEIGHT) & (position >> 2 * HEIGHT);
        r |= p & (position << HEIGHT);
        r |= p & (position >> 3 * HEIGHT);

        // diagonal 2
        p = (position << (HEIGHT + 2)) & (position << 2 * (HEIGHT + 2));
        r |= p & (position << 3 * (HEIGHT + 2));
        r |= p & (position >> (HEIGHT + 2));
        p = (position >> (HEIGHT + 2)) & (position >> 2 * (HEIGHT + 2));
        r |= p & (position << (HEIGHT + 2));
        r |= p & (position >> 3 * (HEIGHT + 2));

        return r & (board_mask ^ mask);
    }
};