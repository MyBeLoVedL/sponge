#include "tcp_receiver.hh"

#include "wrapping_integers.hh"

#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (seg.header().syn) {
        _state = IN_PROGRESS;
        _next_seq = seg.header().seqno + 1;
        _isn = WrappingInt32(seg.header().seqno);
    }

    if (_state == IN_PROGRESS) {
        WrappingInt32 seqno(0);
        if (seg.header().seqno == _isn) {
            seqno = seg.header().seqno + 1;
            // cout << "seqno : _isn : _next_idx   : index : " << seqno << " " << _isn << ": " << _next_idx << ": "
            //      << unwrap(seqno, _isn, _next_idx - _isn.raw_value()) << "\n";
        } else {
            seqno = seg.header().seqno;
        }
        bool fin = seg.header().fin;
        _reassembler.push_substring(string(seg.payload().str()), unwrap(seqno, _isn, _next_idx) - 1, fin);
        _next_idx = _reassembler.next_byte + 1;
        _next_seq = wrap(_next_idx, _isn);
        if (_next_seq == _seq_end) {
            _next_seq = _next_seq + 1;
        }
    }
    if (seg.header().fin) {
        _seq_end = seg.header().seqno + seg.payload().size();
        if (_state == IN_PROGRESS && seg.header().seqno.raw_value() <= _next_seq.raw_value()) {
            _state = TERMINATED;
            _next_seq = _next_seq + 1;
        }
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_state == WAITING)
        return {};
    return {_next_seq};
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
