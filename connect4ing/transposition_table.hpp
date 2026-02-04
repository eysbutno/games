#include <cstddef>

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
    // static_assert((SIZE & (SIZE - 1)) == 0, "SIZE must be a power of 2");

    key_t key[SIZE];
    value_t val[SIZE];

    inline size_t index(key_t key) const {
        return key % SIZE;
    }

    bool has(const key_t &cur) {
        auto loc = index(cur);
        return key[loc] == cur;
    }

    value_t get(const key_t &cur) {
        auto loc = index(cur);
        return key[loc] != cur ? value_t{} : val[loc];
    }

    void put(const key_t &cur, const value_t &new_val) {
        auto loc = index(cur);
        key[loc] = cur;
        val[loc] = new_val;
    }
};