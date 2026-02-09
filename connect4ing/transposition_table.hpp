#include <cstddef>
#include <cmath>

/**
 * Just a hash table that sorta acts like a cache.
 * 
 * SIZE must be a power of 2, to make indexing fast and easy.
 * 
 * As our board representation is a number, and our associated value is 
 * also a number, we need to be able to do bit operations on themn.
 */
template <typename key_t, typename value_t, int SIZE> 
struct transposition_table {
    static_assert((SIZE & (SIZE - 1)) == 0, "SIZE must be a power of 2");

    key_t key[SIZE];
    key_t opt[SIZE];
    value_t val[SIZE];

    uint64_t hash(key_t key) const {
        static const uint64_t C = uint64_t(4e18 * std::acos(0)) + 71;
        static const int XOR_VAL = 998244353;
        return __builtin_bswap64((key ^ XOR_VAL) * C);
    }

    size_t index(key_t key) const {
        return hash(key) & (SIZE - 1);
    }

    bool has(key_t cur) {
        auto loc = index(cur);
        return key[loc] == cur;
    }

    value_t get_val(key_t cur) {
        auto loc = index(cur);
        return val[loc];
    }

    key_t get_move(key_t cur) { 
        auto loc = index(cur);
        return opt[loc]; 
    }

    void put(key_t cur, key_t nxt, value_t new_val) {
        auto loc = index(cur);
        key[loc] = cur;
        opt[loc] = nxt;
        val[loc] = new_val;
    }
};