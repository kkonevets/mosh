#ifndef TRANSPORT_SENDER_HPP
#define TRANSPORT_SENDER_HPP

#include <string>
#include <list>

#include "network.hpp"
#include "transportinstruction.pb.h"
#include "transportstate.hpp"

using namespace std;
using namespace TransportBuffers;

namespace Network {
  template <class MyState>
  class TransportSender
  {
  private:
    /* timing parameters */
    static const int SEND_INTERVAL_MIN = 20; /* ms between frames */
    static const int SEND_INTERVAL_MAX = 250; /* ms between frames */
    static const int ACK_INTERVAL = 1000; /* ms between empty acks */
    static const int ACK_DELAY = 10; /* ms before delayed ack */
    static const int SEND_MINDELAY = 20; /* ms to collect all input */
    static const int HEADER_LEN = 120;

    /* helper methods for tick() */
    unsigned int send_interval( void );
    void update_assumed_receiver_state( void );
    void rationalize_states( void );
    void send_to_receiver( string diff );
    void send_empty_ack( void );
    void send_in_fragments( string diff, uint64_t new_num, bool send_timestamp = true );

    /* state of sender */
    Connection *connection;

    MyState current_state;

    list< TimestampedState<MyState> > sent_states;
    /* first element: known, acknowledged receiver state */
    /* last element: last sent state */

    /* somewhere in the middle: the assumed state of the receiver */
    typename list< TimestampedState<MyState> >::iterator assumed_receiver_state;

    /* for fragment creation */
    uint16_t next_instruction_id;
    Instruction last_instruction_sent;

    /* timing state */
    uint64_t next_ack_time;
    uint64_t next_send_time;

    bool verbose;
    bool shutdown_in_progress;

    /* information about receiver state */
    uint64_t ack_num;

  public:
    /* constructor */
    TransportSender( Connection *s_connection, MyState &initial_state );

    /* Send data or an ack if necessary */
    void tick( void );

    /* Returns the number of ms to wait until next possible event. */
    int wait_time( void );

    /* Executed upon receipt of ack */
    void process_acknowledgment_through( uint64_t ack_num );

    /* Executed upon entry to new receiver state */
    void set_ack_num( uint64_t s_ack_num ) { ack_num = s_ack_num; }

    /* Starts shutdown sequence */
    void start_shutdown( void ) { shutdown_in_progress = true; }
    bool get_shutdown_in_progress( void ) { return shutdown_in_progress; }
    bool get_shutdown_acknowledged( void ) { return sent_states.front().num == uint64_t(-1); }
    bool get_counterparty_shutdown_acknowledged( void ) { return last_instruction_sent.ack_num() == uint64_t(-1); }

    /* Misc. getters and setters */
    /* Cannot modify current_state while shutdown in progress */
    MyState &get_current_state( void ) { assert( !shutdown_in_progress ); return current_state; }
    void set_current_state( const MyState &x ) { assert( !shutdown_in_progress ); current_state = x; }
    void set_verbose( void ) { verbose = true; }

    /* nonexistent methods to satisfy -Weffc++ */
    TransportSender( const TransportSender &x );
    TransportSender & operator=( const TransportSender &x );
  };
}

#endif
