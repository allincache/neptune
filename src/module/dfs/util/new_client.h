#ifndef N_DFS_UTIL_NEW_CLIENT_H_
#define N_DFS_UTIL_NEW_CLIENT_H_

#include <map>
#include <ext/hash_map>
#include "dfs/util/dfs.h"
#include "base/network/simple/net.h"
#include "base/common/Monitor.h"
#include "base/concurrent/Mutex.h"
#include "base_packet.h"

namespace neptune {
namespace dfs {

struct WaitId
{
  uint32_t seq_id_:24;
  uint8_t  send_id_;
};
class NewClientManager;
class NewClient 
{
  friend class NewClientManager;
  friend class LocalPacket;
  public:
    typedef std::map<uint8_t, std::pair<uint64_t, Packet*> > RESPONSE_MSG_MAP;
    typedef RESPONSE_MSG_MAP::iterator RESPONSE_MSG_MAP_ITER;
    typedef std::pair<uint8_t, uint64_t> SEND_SIGN_PAIR;
    typedef int (*callback_func)(NewClient* client);
  public:
    explicit NewClient(const uint32_t& seq_id);
    virtual ~NewClient();
    bool wait(const int64_t timeout_in_ms = DEFAULT_NETWORK_CALL_TIMEOUT);
    int post_request(const uint64_t server, Packet* packet, uint8_t& send_id);
    int async_post_request(const std::vector<uint64_t>& servers, Packet* packet, callback_func func, bool save_source_msg = true);
    inline RESPONSE_MSG_MAP* get_success_response() { return complete_ ? &success_response_ : NULL;}
    inline RESPONSE_MSG_MAP* get_fail_response() { return complete_ ? &fail_response_ : NULL;}
    inline Packet* get_source_msg() { return source_msg_;}
    inline std::vector<SEND_SIGN_PAIR>& get_send_id_sign() { return send_id_sign_;}

  private:
    NewClient();
    DISALLOW_COPY_AND_ASSIGN(NewClient);
    Monitor<Mutex> monitor_;
    RESPONSE_MSG_MAP success_response_;
    RESPONSE_MSG_MAP fail_response_;
    std::vector<SEND_SIGN_PAIR> send_id_sign_;
    callback_func callback_;
    Packet* source_msg_;
    const uint32_t seq_id_;
    uint8_t generate_send_id_;
    static const uint8_t MAX_SEND_ID = 0xFF - 1;
    bool complete_;// receive all response(data packet, timeout packet) complete or timeout
    bool post_packet_complete_;
  private:
    SEND_SIGN_PAIR* find_send_id(const WaitId& id);
    bool push_fail_response(const uint8_t send_id, const uint64_t server);
    bool push_success_response(const uint8_t send_id, const uint64_t server, Packet* packet);
    bool handlePacket(const WaitId& id, Packet* packet, bool& is_callback);

    uint8_t create_send_id(const uint64_t server);
    bool destroy_send_id(const WaitId& id);

    inline const uint32_t get_seq_id() const { return seq_id_;}

    bool async_wait();
};
int send_msg_to_server(uint64_t server, Packet* message, int32_t& status, 
                      const int64_t timeout = DEFAULT_NETWORK_CALL_TIMEOUT/*ms*/);
int send_msg_to_server(uint64_t server, NewClient* client, Packet* msg, Packet*& output/*not free*/,
                      const int64_t timeout = DEFAULT_NETWORK_CALL_TIMEOUT/*ms*/);
int post_msg_to_server(const std::vector<uint64_t>& servers, NewClient* client, Packet* message, NewClient::callback_func func,
                      bool save_source_msg = true);
int post_msg_to_server(uint64_t servers, NewClient* client, Packet* message, NewClient::callback_func func,
                      bool save_source_msg = true);
int test_server_alive(const uint64_t server_id, const int64_t timeout = DEFAULT_NETWORK_CALL_TIMEOUT/*ms*/);
  
} //namespace dfs
} //namespace neptune

#endif
