#include "tcp_sender.hh"

#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "tcp_state.hh"
#include "wrapping_integers.hh"

#include <algorithm>
#include <iostream>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , RTO(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return bytes_unACKed; }

void TCPSender::fill_window() {
    while (_window_size > 0) {
        auto avail = min(_window_size, min(TCPConfig::MAX_PAYLOAD_SIZE, _stream.buffer_size()));
        if (avail == 0 && _next_seqno != 0 && (!_stream.eof() || fin_sent))
            break;
        TCPSegment seg{};
        seg.header().syn = _next_seqno == 0 ? true : false;
        // ? bug here
        fin_sent = seg.header().fin = _stream.eof() ? true : false;

        if (avail >= _window_size) {
            if (seg.header().syn) {
                avail--;
            }
            if (seg.header().fin) {
                avail--;
            }
        }
        seg.payload() = _stream.read(avail);
        // * incase that the FIN could be piggybacked on the data segment
        fin_sent = seg.header().fin = _stream.eof() ? true : false;
        seg.header().seqno = wrap(_next_seqno, _isn);
        _next_seqno += seg.length_in_sequence_space();
        _segments_out.push(seg);
        _out_segments.push_back(seg);
        bytes_unACKed += seg.length_in_sequence_space();
        if (!timer.started) {
            timer.started = true;
            timer.count = 0;
        }
        _window_size -= seg.length_in_sequence_space();
        // cout << "window size:  " << _window_size << " length  " << seg.length_in_sequence_space() << "\n";
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _window_size = window_size;
    consec_retrx = 0;
    timer.started = true;
    RTO = _initial_retransmission_timeout;
    timer.count = 0;
    for (auto seg : _out_segments) {
        if (unwrap(ackno, _isn, _next_seqno) > unwrap(seg.header().seqno, _isn, _next_seqno)) {
            bytes_unACKed -= seg.length_in_sequence_space();
            _out_segments.pop_front();
        } else
            break;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!timer.started) {
        cout << "error timer\n";
        exit(1);
    }
    timer.count += ms_since_last_tick;
    if (timer.count >= RTO) {
        timer.count = 0;
        _segments_out.push(_out_segments.at(0));
        RTO <<= 1;
        consec_retrx += 1;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return consec_retrx; }

void TCPSender::send_empty_segment() {}
