#include "byte_stream.hh"
#include "stream_reassembler.hh"
#include "util.hh"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

static constexpr unsigned NREPS = 4;
static constexpr unsigned NSEGS = 128;
static constexpr unsigned MAX_SEG_LEN = 2048;

string read(StreamReassembler &reassembler) {
    return reassembler.stream_out().read(reassembler.stream_out().buffer_size());
}

int main() {
    try {
        auto rd = get_random_generator();

        // overlapping segments
        for (unsigned rep_no = 0; rep_no < NREPS; ++rep_no) {
            StreamReassembler buf{NSEGS * MAX_SEG_LEN};

            vector<tuple<size_t, size_t>> seq_size;
            size_t offset = 0;
            for (unsigned i = 0; i < NSEGS; ++i) {
                const size_t size = 1 + (rd() % (MAX_SEG_LEN - 1));
                const size_t offs = min(offset, 1 + (static_cast<size_t>(rd()) % 1023));
                seq_size.emplace_back(offset - offs, size + offs);
                offset += size;
            }
            shuffle(seq_size.begin(), seq_size.end(), rd);

            string d(offset, 0);
            generate(d.begin(), d.end(), [&] { return rd(); });

            for (auto [off, sz] : seq_size) {
                string dd(d.cbegin() + off, d.cbegin() + off + sz);
                buf.push_substring(move(dd), off, off + sz == offset);
            }

            auto result = read(buf);
            if (buf.stream_out().bytes_written() != offset) {  // read bytes
                cout << "offset" << offset << " written" << buf.stream_out().bytes_written() << "\n";
                cout << buf.unassembled_bytes() << "  unass \n";
                cout << "size : " << buf.ranges.size() << "\n";
                for (auto i : buf.ranges)
                    cout << "start : " << i.start << " end : " << i.end << "\n";
                // cout << buf.ranges.size() << "buf size\n " << buf.ranges[0].start << "start " << buf.next_byte
                //      << " next byte\n";
                throw runtime_error("test 2 - number of RX bytes is incorrect");
            }
            if (!equal(result.cbegin(), result.cend(), d.cbegin())) {
                cout << "~~~~~~~~~equal"
                     << "\n";
                throw runtime_error("test 2 - content of RX bytes is incorrect");
            }
        }
    } catch (const exception &e) {
        cerr << "Exception: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
