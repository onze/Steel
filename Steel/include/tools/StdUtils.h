
#ifndef STEEL_STDUTILS_H
#define STEEL_STDUTILS_H

#include <functional>

namespace Steel
{
    namespace StdUtils
    {
        // http://stackoverflow.com/questions/7110301/generic-hash-for-tuples-in-unordered-map-unordered-set
        // Code from boost
        // Reciprocal of the golden ratio helps spread entropy
        //     and handles duplicates.
        // See Mike Seymour in magic-numbers-in-boosthash-combine:
        //     http://stackoverflow.com/questions/4948780
        /// hash(a,b) combines hash(b) into a.
        template <class T>
        inline void hash_combine(std::size_t &seed, T const &v)
        {
            seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    }
}
#endif //STEEL_STDUTILS_H
