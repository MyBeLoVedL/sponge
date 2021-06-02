#include "byte_stream.hh"

#include "common.hh"

#include <cstddef>
#include <cstdint>
#include <string>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in
// `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : buf(std::deque<std::uint8_t>{})
    , count(0)
    , cap(capacity)
    , bytes_written_count(0)
    , bytes_read_count(0)
    , whether_input_end(false) {}

size_t ByteStream::write(const string &data) {
    auto avail = min(data.size(), static_cast<size_t>(cap - count));
    count += avail;
    buf.insert(buf.end(), data.begin(), data.begin() + avail);
    bytes_written_count += avail;
    return avail;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    auto avail = min(count, len);
    return string(buf.begin(), buf.begin() + avail);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    auto avail = min(count, len);
    buf.erase(buf.begin(), buf.begin() + avail);
    count -= avail;
    bytes_read_count += avail;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    auto avail = min(count, len);
    auto res = string(buf.begin(), buf.begin() + avail);
    buf.erase(buf.begin(), buf.begin() + avail);
    bytes_read_count += avail;
    count -= avail;
    return res;
}

void ByteStream::end_input() { whether_input_end = true; }

bool ByteStream::input_ended() const { return whether_input_end; }

size_t ByteStream::buffer_size() const { return count; }

bool ByteStream::buffer_empty() const { return count == 0; }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return bytes_written_count; }

size_t ByteStream::bytes_read() const { return bytes_read_count; }

size_t ByteStream::remaining_capacity() const { return cap - count; }
