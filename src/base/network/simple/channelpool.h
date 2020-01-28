#ifndef N_BASE_NET_CHANNEL_POOL_H_
#define N_BASE_NET_CHANNEL_POOL_H_

#define CHANNEL_CLUSTER_SIZE 25

namespace neptune {
namespace base {

class ChannelPool {

public:
    /*
     * ���캯��
     */
    ChannelPool();

    /*
     * ��������
     */
    ~ChannelPool();

    /*
     * �õ�һ���µ�channel
     *
     * @return һ��Channel
     */
    Channel *allocChannel();

    /*
     * �ͷ�һ��channel
     *
     * @param channel: Ҫ�ͷŵ�channel
     * @return
     */
    bool freeChannel(Channel *channel);
    bool appendChannel(Channel *channel);

    /*
     * ����һ��channel
     *
     * @param id: channel id
     * @return Channel
     */
    Channel* offerChannel(uint32_t id);

    /*
     * ��useList���ҳ���ʱ��channel��list,����hashmap�ж�Ӧ��ɾ��
     *
     * @param now: ��ǰʱ��
     */
    Channel* getTimeoutList(int64_t now);

    /*
     * ��addList���������뵽freeList��
     *
     * @param addList���ӵ�list
     */
    bool appendFreeList(Channel *addList);

    /*
     * ���������ĳ���
     */
    int getUseListCount() {
        return static_cast<int32_t>(_useMap.size());
    }

    void setExpireTime(Channel *channel, int64_t now); 

private:
    __gnu_cxx::hash_map<uint32_t, Channel*> _useMap; // ʹ�õ�map
    std::list<Channel*> _clusterList;                // cluster list
    CThreadMutex _mutex;

    Channel *_freeListHead;             // �յ�����
    Channel *_freeListTail;
    Channel *_useListHead;              // ��ʹ�õ�����
    Channel *_useListTail;
    int _maxUseCount;                   // ���������ĳ���

    static atomic_t _globalChannelId;   // ����ͳһ��id
    static atomic_t _globalTotalCount;
};

} //namespace base
} //namespace neptune

#endif
