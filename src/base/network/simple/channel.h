#ifndef NEP_N_SIMPLE_CHANNEL_H
#define NEP_N_SIMPLE_CHANNEL_H

namespace neptune {
namespace base {

class Channel {
    friend class ChannelPool;

public:

    Channel();

    void setId(uint32_t id);

    uint32_t getId();

    void setArgs(void *args);

    void *getArgs();

    void setHandler(IPacketHandler *handler);

    IPacketHandler *getHandler();

    void setExpireTime(int64_t expireTime);

    int64_t getExpireTime() {
        return _expireTime;
    }

    Channel *getNext() {
        return _next;
    }

private:
    uint32_t _id;      // channel id
    void *_args;    
    IPacketHandler *_handler;
    int64_t _expireTime; 

private:
    Channel *_prev;
    Channel *_next;
};

} //namespace base
} //namespace neptune

#endif
