#include "net.h"

namespace neptune {
namespace base {

Connection::Connection(Socket *socket, IPacketStreamer *streamer, IServerAdapter *serverAdapter) {
    _socket = socket;
    _streamer = streamer;
    _serverAdapter = serverAdapter;
    _defaultPacketHandler = NULL;
    _iocomponent = NULL;
    _queueTimeout = 5000;
    _queueLimit = 50;
    _queueTotalSize = 0;
}

/*
 * ��������
 */
Connection::~Connection() {
    disconnect();
    _socket = NULL;
    _iocomponent = NULL;
}

/*
 * ���ӶϿ��������з��Ͷ����е�packetȫ����ʱ
 */
void Connection::disconnect() {
    _outputCond.lock();
    _myQueue.moveTo(&_outputQueue);
    _outputCond.unlock();
    checkTimeout(NET_MAX_TIME);
}

/*
 * ����packet�����Ͷ���
 */
bool Connection::postPacket(Packet *packet, IPacketHandler *packetHandler, void *args, bool noblocking) {
    if (!isConnectState()) {
        if (_iocomponent == NULL ||  _iocomponent->isAutoReconn() == false) {
            return false;
        } else if (_outputQueue.size()>10) {
            return false;
        } else {
            TCPComponent *ioc = dynamic_cast<TCPComponent*>(_iocomponent);
            bool ret = false;
            if (ioc != NULL) {
                _outputCond.lock();
                ret = ioc->init(false);
                _outputCond.unlock();
            }
            if (!ret) return false;
        }
    }
    // �����client, ������queue���ȵ�����
    _outputCond.lock();
    _queueTotalSize = _outputQueue.size() + _channelPool.getUseListCount() + _myQueue.size();
    if (!_isServer && _queueLimit > 0 && noblocking && _queueTotalSize >= _queueLimit) {
        _outputCond.unlock();
        return false;
    }
    _outputCond.unlock();
    Channel *channel = NULL;
    packet->setExpireTime(_queueTimeout);           // ���ó�ʱ
    if (_streamer->existPacketHeader()) {           // ���ڰ�ͷ
        uint32_t chid = packet->getChannelId();     // ��packet��ȡ
        if (_isServer) {
            assert(chid != 0);                      // ����Ϊ��
        } else {
            channel = _channelPool.allocChannel();

            // channelû�ҵ���
            if (channel == NULL) {
            //    LOG(WARN, "����channel����, id: %u", chid);
                return false;
            }

            channel->setHandler(packetHandler);
            channel->setArgs(args);
            packet->setChannel(channel);            // ���û�ȥ
        }
    }
    _outputCond.lock();
    // д�뵽outputqueue��
    _outputQueue.push(packet);
    if (_iocomponent != NULL && _outputQueue.size() == 1U) {
        _iocomponent->enableWrite(true);
    }
    _outputCond.unlock();
    if (!_isServer && _queueLimit > 0) {
        _outputCond.lock();
        _queueTotalSize = _outputQueue.size() + _channelPool.getUseListCount() + _myQueue.size();
        if ( _queueTotalSize > _queueLimit && noblocking == false) {
            bool *stop = NULL;
            if (_iocomponent && _iocomponent->getOwner()) {
                stop = _iocomponent->getOwner()->getStop();
            }
            while (_queueTotalSize > _queueLimit && stop && *stop == false) {
                if (_outputCond.wait(1000) == false) {
                    if (!isConnectState()) {
                        break;
                    }
                    _queueTotalSize = _outputQueue.size() + _channelPool.getUseListCount() + _myQueue.size();
                }
            }
        }
        _outputCond.unlock();
    }

    if (_isServer && _iocomponent) {
        _iocomponent->subRef();
    }

    return true;
}

/*
 * handlePacket ����
 */
bool Connection::handlePacket(DataBuffer *input, PacketHeader *header) {
    Packet *packet;
    IPacketHandler::HPRetCode rc;
    void *args = NULL;
    Channel *channel = NULL;
    IPacketHandler *packetHandler = NULL;

    if (_streamer->existPacketHeader() && !_isServer) { // ���ڰ�ͷ
        uint32_t chid = header->_chid;    // ��header��ȡ
        chid = (chid & 0xFFFFFFF);
        channel = _channelPool.offerChannel(chid);

        // channelû�ҵ�
        if (channel == NULL) {
            input->drainData(header->_dataLen);
        //    LOG(WARN, "û�ҵ�channel, id: %u, %s", chid, CNetUtil::addrToString(getServerId()).c_str());
            return false;
        }

        packetHandler = channel->getHandler();
        args = channel->getArgs();
    }

    // ����
    packet = _streamer->decode(input, header);
    if (packet == NULL) {
        packet = &ControlPacket::BadPacket;
    } else {
        packet->setPacketHeader(header);
        // ����������, ֱ�ӷ���queue, ����
        if (_isServer && _serverAdapter->_batchPushPacket) {
            if (_iocomponent) _iocomponent->addRef();
            _inputQueue.push(packet);
            if (_inputQueue.size() >= 15) { // ����15��packet�͵���һ��
                _serverAdapter->handleBatchPacket(this, _inputQueue);
                _inputQueue.clear();
            }
            return true;
        }
    }

    // ����handler
    if (_isServer) {
        if (_iocomponent) _iocomponent->addRef();
        rc = _serverAdapter->handlePacket(this, packet);
    } else {
        if (packetHandler == NULL) {    // ��Ĭ�ϵ�
            packetHandler = _defaultPacketHandler;
        }
        assert(packetHandler != NULL);

        rc = packetHandler->handlePacket(packet, args);
        channel->setArgs(NULL);
        // ���ջ����ͷŵ�
        if (channel) {
            _channelPool.appendChannel(channel);
        }
    }

    return true;
}

/*
 * ��鳬ʱ
 */
bool Connection::checkTimeout(int64_t now) {
    // �õ���ʱ��channel��list
    Channel *list = _channelPool.getTimeoutList(now);
    Channel *channel = NULL;
    IPacketHandler *packetHandler = NULL;

    if (list != NULL) {
        if (!_isServer) { // client endpoint, ��ÿ��channel��һ����ʱpacket, �������˰�channel����
            channel = list;
            while (channel != NULL) {
                packetHandler = channel->getHandler();
                if (packetHandler == NULL) {    // ��Ĭ�ϵ�
                    packetHandler = _defaultPacketHandler;
                }
                // �ص�
                if (packetHandler != NULL) {
                    packetHandler->handlePacket(&ControlPacket::TimeoutPacket, channel->getArgs());
                    channel->setArgs(NULL);
                }
                channel = channel->getNext();
            }
        }
        // �ӵ�freelist��
        _channelPool.appendFreeList(list);
    }

    // ��PacketQueue��ʱ���
    _outputCond.lock();
    Packet *packetList = _outputQueue.getTimeoutList(now);
    _outputCond.unlock();
    while (packetList) {
        Packet *packet = packetList;
        packetList = packetList->getNext();
        channel = packet->getChannel();
        packet->free();
        if (channel) {
            packetHandler = channel->getHandler();
            if (packetHandler == NULL) {    // ��Ĭ�ϵ�
                packetHandler = _defaultPacketHandler;
            }
            // �ص�
            if (packetHandler != NULL) {
                packetHandler->handlePacket(&ControlPacket::TimeoutPacket, channel->getArgs());
                channel->setArgs(NULL);
            }
            _channelPool.freeChannel(channel);
        }
    }

    // �����client, ������queue���ȵ�����
    if (!_isServer && _queueLimit > 0 &&  _queueTotalSize > _queueLimit) {
        _outputCond.lock();
        _queueTotalSize = _outputQueue.size() + _channelPool.getUseListCount() + _myQueue.size();
        if (_queueTotalSize <= _queueLimit) {
            _outputCond.broadcast();
        }
        _outputCond.unlock();
    }

    return true;
}

/**
 * ����״̬
 */
bool Connection::isConnectState() {
    if (_iocomponent != NULL) {
        return _iocomponent->isConnectState();
    }
    return false;
}

} //namespace base
} //namespace neptune
