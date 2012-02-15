#include "a.h"
#include <cassert>
#include <cstdlib>

A::A(int32_t _m, uint16_t _d, uint16_t _g, uint8_t _l) {
    m = _m;
    d = _d;
    g = _g;
    l = _l;
}

A::A() {
    g = d = 0;
    m = 0.0;
    l = 0x0;
}


bool A::maj(const struct A& other) const {
    int first = 0;
    int o_first = 0;
    int second = 0;
    int o_second = 0;

    first = g;
    o_first = other.g;
    second = d;
    o_second = other.g;
    
    bool majFirst = first <= o_first;
    if (majFirst) {
        bool majM = m >= other.m;
        if ( majM ) {
            if ( first==o_first && m == other.m )
                return second <= o_second;
            else
                return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
    return false;
}

bool operator==(const struct A& left, const struct A& right) {
    return left.g==right.g && left.m==right.m;
}
bool operator<(const struct A& left, const struct A& right) {
    /* We need sorting from max to min, so operation is reverted */
    if (left==right)
        return false;
    else {
        int first_l, first_r;
        int second_l, second_r;
        first_l = left.g;
        first_r = right.g;
        second_l = left.d;
        second_r = right.d;
        if (first_l==first_r) {
            if ( left.m == right.m ) {
                return second_l < second_r;
            }
            else
                return left.m > right.m;
        }
        else
            return first_l < first_r;
    }
}
