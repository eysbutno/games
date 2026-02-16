#include <cstddef>
#include <cmath>
#include <chrono>

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

    uint64_t splitmix64(uint64_t x) const {
        // http://xorshift.di.unimi.it/splitmix64.c
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    uint64_t hash(key_t key) const {
        static const uint64_t FIXED_RANDOM = std::chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(key + FIXED_RANDOM);
    }

    size_t index(key_t key) const {
        return hash(key) & (SIZE - 1);
    }

    bool has(key_t cur) {
        auto loc = index(cur);
        return key[loc] == cur && val[loc];
    }

    value_t get_val(key_t cur) {
        auto loc = index(cur);
        return key[loc] == cur ? val[loc] : 0;
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