#include "tcp_connection.hh"

#include "tcp_segment.hh"
#include "tcp_state.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.available_window_size(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _period_since_last_segment; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _period_since_last_segment = 0;
    if (seg.header().rst) {
        cout << "-------------------RST received\n";
    }
    if (seg.header().rst &&
        TCPState(_sender, _receiver, active(), _linger_after_streams_finish) != TCPState(TCPState::State::LISTEN)) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _conn_state = false;
    }
    _receiver.segment_received(seg);
    _sender._receiver_ackno = _receiver.ackno().value();
    if (seg.length_in_sequence_space() > 0) {
        _sender.send_empty_segment(_receiver.ackno().value());
    }
    cout << "reply with " << _receiver.ackno().value() << "\n";
    if (seg.header().ack) {
        // * in state LAST_ACK,and recieved seg equals to the sent fin + 1
        if (TCPState(_sender, _receiver, active(), _linger_after_streams_finish) ==
                TCPState(TCPState::State::LAST_ACK) &&
            seg.header().ackno == _sender._out_segments.front().header().seqno + 1) {
            cout << "passive close\n";
            _conn_state = false;
        }
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }

    // * negative clsoe
    if (seg.header().fin && !_sender.fin_sent) {
        _linger_after_streams_finish = false;
    }
}

bool TCPConnection::active() const { return _conn_state; }

size_t TCPConnection::write(const string &data) {
    auto t = _sender.stream_in().write(data);
    _sender.fill_window();

    // * positive close side
    if (_sender.fin_sent && !_linger_after_streams_finish) {
        _conn_state = false;
    }
    return t;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _period_since_last_segment += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    if (_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS) {
        // * sender side send packet with RST set
        auto seg = TCPSegment{};
        seg.header().rst = true;
        segments_out().push(seg);

        // * sender side close connection
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _conn_state = false;
    }
    //* postive close side
    if (_linger_after_streams_finish && _period_since_last_segment >= 10 * _cfg.rt_timeout) {
        _conn_state = false;
    }
}

void TCPConnection::end_input_stream() {
    cout << "closing the connection\n";
    _sender.stream_in().end_input();
    _sender.fill_window();
}

void TCPConnection::connect() {
    _sender.fill_window();
    _conn_state = true;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
