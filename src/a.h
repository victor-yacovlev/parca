#ifndef A_H
#define A_H

#include <boost/cstdint.hpp>
#include <vector>

#define NIL 0x0
#define NO_DEL 0x1
#define DEL_H 0x2
#define DEL_V 0x4

#define STOP_FLAG 0x8


struct A {
    /* matches score */
    int32_t m;
    /* deletions count */
    uint16_t d;
    /* gaps count */
    uint16_t g;
    /* link: bitmask '.....xyz', where 'x' is DEL_H, 'y' is DEL_V,
       'z' is NO_DEL, and '.' are unused bits
    */
    uint8_t l;

    A(int32_t _m, uint16_t _d, uint16_t _g, uint8_t _l);
    A();

    bool maj(const A& other) const;
};



bool operator<(const struct A& left, const struct A& right);
bool operator==(const struct A& left, const struct A& right);

#endif // A_H
