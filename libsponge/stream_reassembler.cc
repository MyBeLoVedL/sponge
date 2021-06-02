#include "stream_reassembler.hh"

#include "common.hh"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <vector>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), ranges(vector<Range>()) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (eof)
        stream_end = index + data.size();
    if (data.size() == 0) {
        if (next_byte == static_cast<uint64_t>(stream_end))
            _output.end_input();
        return;
    }
    Range cur(index, index + data.size() - 1, data);
    insert_range(cur);
}
void StreamReassembler::insert_range(Range &cur) {
    auto threshold = next_byte + _capacity - _output.buffer_size() - 1;
    std::uint32_t s = static_cast<std::uint32_t>(cur.start);
    if (s > threshold)
        return;
    else if (static_cast<std::uint32_t>(cur.end) > threshold) {
        cur.end = threshold;
        cur.data = cur.data.substr(0, cur.end - cur.start + 1);
    }
    ranges.push_back(cur);
    merge_range();
}

bool cmp(Range &a, Range &b) { return a.start < b.start; }

void StreamReassembler::merge_range() {
    vector<Range> res{};
    std::uint32_t tail = 0;
    std::sort(ranges.begin(), ranges.end(), cmp);
    res.push_back(ranges[0]);
    for (size_t i = 1; i < ranges.size(); i++) {
        if (ranges[i].start > res[tail].end + 1) {
            res.push_back(ranges[i]);
            tail++;
        } else if (ranges[i].end <= res[tail].end)
            continue;
        else if (ranges[i].start <= res[tail].end + 1 && ranges[i].end > res[tail].end) {
            auto gap = res[tail].end - ranges[i].start;
            res[tail].end = ranges[i].end;
            res[tail].data += ranges[i].data.substr(gap + 1, ranges[i].data.size());
        } else {
            cout << "can't get here~"
                 << "\n";
            exit(EXIT_FAILURE);
        }
    }
    if (res[0].start == next_byte) {
        _output.write(res[0].data);
        next_byte = res[0].end + 1;
        if (next_byte == static_cast<uint64_t>(stream_end))
            _output.end_input();
        res.erase(res.begin());
    } else if (res[0].start < next_byte && res[0].end >= next_byte) {
        res[0].data = res[0].data.substr(next_byte - res[0].start, res[0].data.size());
        next_byte = res[0].end + 1;
        _output.write(res[0].data);
        if (next_byte == static_cast<uint64_t>(stream_end))
            _output.end_input();
        res.erase(res.begin());
    }
    consumed = 0;
    for (auto i = res.begin(); i < res.end(); i++) {
        consumed += i->data.size();
    }
    ranges = std::move(res);
}

size_t StreamReassembler::unassembled_bytes() const { return consumed; }

bool StreamReassembler::empty() const { return consumed == 0; }
