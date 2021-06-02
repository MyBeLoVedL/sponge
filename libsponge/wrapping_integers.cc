#include "wrapping_integers.hh"

#include <bits/stdint-uintn.h>
#include <cassert>
#include <cstdint>
#include <iostream>
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) >= 0 ? (a) : (-(a)))

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    std::uint32_t n32 = (n << 32) >> 32;
    return WrappingInt32(n32 + isn.raw_value());
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.

#define half_gap (1ul << 31)

//* a hard lesson about overflow
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t lower, upper;
    if (checkpoint <= half_gap) {
        lower = 0;
        upper = 2 * half_gap;
    } else {
        lower = checkpoint - half_gap;
        upper = checkpoint + half_gap;
    }

    uint64_t base = (checkpoint >> 32) << 32;
    int32_t t = n - isn;
    base += static_cast<uint64_t>(t);
    if (!(base >= lower && base <= upper))
        base += half_gap * 2;
    if (!(base >= lower && base <= upper))
        assert("exam failed");
    return base;
}
