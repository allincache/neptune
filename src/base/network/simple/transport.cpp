#include "net.h"

namespace neptune {
namespace base {

Transport::Transport() {
  _stop = false;
  _iocListHead = _iocListTail = NULL;
  _delListHead = _delListTail = NULL;
  _iocListChanged = false;
  _iocListCount = 0;
}

Transport::~Transport() {
  destroy();
}

bool Transport::start() {
  signal(SIGPIPE, SIG_IGN);
  _readWriteThread.start(this, &_socketEvent);
  _timeoutThread.start(this, NULL);
  return true;
}

bool Transport::stop() {
  _stop = true;
  return true;
}

bool Transport::wait() {
  _readWriteThread.join();
  _timeoutThread.join();
  destroy();
  return true;
}

void Transport::eventLoop(SocketEvent *socketEvent) {
  IOEvent events[MAX_SOCKET_EVENTS];
  while (!_stop) {
    int cnt = socketEvent->getEvents(1000, events, MAX_SOCKET_EVENTS);
    if (cnt < 0) {
        //LOG(INFO, "�õ�events������: %s(%d)\n", strerror(errno), errno);
    }
    for (int i = 0; i < cnt; i++) {
      IOComponent *ioc = events[i]._ioc;
      if (ioc == NULL) {
        continue;
      }
      if (events[i]._errorOccurred) { 
        removeComponent(ioc);
        continue;
      }
      ioc->addRef();
      bool rc = true;
      if (events[i]._readOccurred) {
        rc = ioc->handleReadEvent();
      }
      if (rc && events[i]._writeOccurred) {
        rc = ioc->handleWriteEvent();
      }
      ioc->subRef();
      if (!rc) {
        removeComponent(ioc);
      }
    }
  }
}

void Transport::timeoutLoop() {
  IOComponent *mydelHead = NULL;
  IOComponent *mydelTail = NULL;
  std::vector<IOComponent*> mylist;
  while (!_stop) {
    _iocsMutex.lock();
    if (_iocListChanged) {
        mylist.clear();
        IOComponent *iocList = _iocListHead;
        while (iocList) {
            mylist.push_back(iocList);
            iocList = iocList->_next;
        }
        _iocListChanged = false;
    }
    if (_delListHead != NULL && _delListTail != NULL) {
        if (mydelTail == NULL) {
            mydelHead = _delListHead;
        } else {
            mydelTail->_next = _delListHead;
            _delListHead->_prev = mydelTail;
        }
        mydelTail = _delListTail;
        _delListHead = _delListTail = NULL;
    }
    _iocsMutex.unlock();
    for (int i=0; i<(int)mylist.size(); i++) {
        IOComponent *ioc = mylist[i];
        ioc->checkTimeout(CTimeUtil::getTime());
    }
    IOComponent *tmpList = mydelHead;
    int64_t nowTime = CTimeUtil::getTime() - static_cast<int64_t>(900000000); // 15min
    while (tmpList) {
        if (tmpList->getRef() <= 0) {
            tmpList->subRef();
        }
        if (tmpList->getRef() <= -10 || tmpList->getLastUseTime() < nowTime) {
            if (tmpList == mydelHead) { // head
                mydelHead = tmpList->_next;
            }
            if (tmpList == mydelTail) { // tail
                mydelTail = tmpList->_prev;
            }
            if (tmpList->_prev != NULL)
                tmpList->_prev->_next = tmpList->_next;
            if (tmpList->_next != NULL)
                tmpList->_next->_prev = tmpList->_prev;

            IOComponent *ioc = tmpList;
            tmpList = tmpList->_next;
            //LOG(INFO, "DELIOC, %s, IOCount:%d, IOC:%p\n",
            //          ioc->getSocket()->getAddr().c_str(), _iocListCount, ioc);
            delete ioc;
        } else {
            tmpList = tmpList->_next;
        }
    }
    usleep(500000);  
  }
  _iocsMutex.lock();
  if (mydelHead != NULL) {
      if (_delListTail == NULL) {
          _delListHead = mydelHead;
      } else {
          _delListTail->_next = mydelHead;
          mydelHead->_prev = _delListTail;
      }
      _delListTail = mydelTail;
  }
  _iocsMutex.unlock();
}

void Transport::run(CThread *thread, void *arg) {
  if (thread == &_timeoutThread) {
    timeoutLoop();
  } else {
    eventLoop((SocketEvent*)arg);
  }
}

int Transport::parseAddr(char *src, char **args, int cnt) {
  int index = 0;
  char *prev = src;
  while (*src) {
      if (*src == ':') {
          *src = '\0';
          args[index++] = prev;
          if (index >= cnt) {
              return index;
          }
          prev = src + 1;
      }
      src ++;
  }
  args[index++] = prev;
  return index;
}

IOComponent *Transport::listen(const char *spec, IPacketStreamer *streamer, IServerAdapter *serverAdapter) {
  char tmp[1024];
  char *args[32];
  strncpy(tmp, spec, 1024);
  tmp[1023] = '\0';
  if (parseAddr(tmp, args, 32) != 3) {
      return NULL;
  }
  if (strcasecmp(args[0], "tcp") == 0) {
      char *host = args[1];
      int port = atoi(args[2]);
      // Server Socket
      ServerSocket *socket = new ServerSocket();
      if (!socket->setAddress(host, port)) {
          delete socket;
          return NULL;
      }
      // TCPAcceptor
      TCPAcceptor *acceptor = new TCPAcceptor(this, socket, streamer, serverAdapter);
      if (!acceptor->init()) {
          delete acceptor;
          return NULL;
      }
      addComponent(acceptor, true, false);
      return acceptor;
  } else if (strcasecmp(args[0], "udp") == 0) {}
  return NULL;
}

Connection *Transport::connect(const char *spec, IPacketStreamer *streamer, bool autoReconn) {
  char tmp[1024];
  char *args[32];
  strncpy(tmp, spec, 1024);
  tmp[1023] = '\0';
  if (parseAddr(tmp, args, 32) != 3) {
      return NULL;
  }
  if (strcasecmp(args[0], "tcp") == 0) {
      char *host = args[1];
      int port = atoi(args[2]);
      // Socket
      Socket *socket = new Socket();
      if (!socket->setAddress(host, port)) {
          delete socket;
          //LOG(ERROR, "����setAddress����: %s:%d, %s", host, port, spec);
          return NULL;
      }
      // TCPComponent
      TCPComponent *component = new TCPComponent(this, socket, streamer, NULL);
      component->setAutoReconn(autoReconn);
      if (!component->init()) {
          delete component;
          //LOG(ERROR, "��ʼ��ʧ��TCPComponent: %s:%d", host, port);
          return NULL;
      }
      addComponent(component, true, true);
      component->addRef();
      return component->getConnection();
  } else if (strcasecmp(args[0], "udp") == 0) {}
  return NULL;
}

bool Transport::disconnect(Connection *conn) {
  IOComponent *ioc = NULL;
  if (conn == NULL || (ioc = conn->getIOComponent()) == NULL) {
      return false;
  }
  ioc->setAutoReconn(false);
  ioc->subRef();
  if (ioc->_socket) {
      ioc->_socket->shutdown();
  }
  return true;
}

void Transport::addComponent(IOComponent *ioc, bool readOn, bool writeOn) {
  assert(ioc != NULL);
  _iocsMutex.lock();
  if (ioc->isUsed()) {
      //LOG(ERROR, "�Ѹ��ӹ�addComponent: %p", ioc);
      _iocsMutex.unlock();
      return;
  }
  ioc->_prev = _iocListTail;
  ioc->_next = NULL;
  if (_iocListTail == NULL) {
      _iocListHead = ioc;
  } else {
      _iocListTail->_next = ioc;
  }
  _iocListTail = ioc;
  ioc->setUsed(true);
  _iocListChanged = true;
  _iocListCount ++;
  _iocsMutex.unlock();
  Socket *socket = ioc->getSocket();
  ioc->setSocketEvent(&_socketEvent);
  _socketEvent.addEvent(socket, readOn, writeOn);
  //LOG(INFO, "ADDIOC, SOCK: %d, %s, RON: %d, WON: %d, IOCount:%d, IOC:%p\n",
  //          socket->getSocketHandle(), ioc->getSocket()->getAddr().c_str(),
  //          readOn, writeOn, _iocListCount, ioc);
}

void Transport::removeComponent(IOComponent *ioc) {
  assert(ioc != NULL);
  CThreadGuard guard(&_iocsMutex);
  ioc->close();
  if (ioc->isAutoReconn()) {
      return;
  }
  if (ioc->isUsed() == false) {
      return;
  }
  if (ioc == _iocListHead) { // head
      _iocListHead = ioc->_next;
  }
  if (ioc == _iocListTail) { // tail
      _iocListTail = ioc->_prev;
  }
  if (ioc->_prev != NULL)
      ioc->_prev->_next = ioc->_next;
  if (ioc->_next != NULL)
      ioc->_next->_prev = ioc->_prev;
  ioc->_prev = _delListTail;
  ioc->_next = NULL;
  if (_delListTail == NULL) {
      _delListHead = ioc;
  } else {
      _delListTail->_next = ioc;
  }
  _delListTail = ioc;
  ioc->setUsed(false);
  _iocListChanged = true;
  _iocListCount --;
  //LOG(INFO, "RMIOC, %s IOCount:%d, IOC:%p\n",
  //          ioc->getSocket()->getAddr().c_str(),
  //          _iocListCount, ioc);
}

void Transport::destroy() {
  CThreadGuard guard(&_iocsMutex);
  IOComponent *list, *ioc;
  list = _iocListHead;
  while (list) {
      ioc = list;
      list = list->_next;
      _iocListCount --;
      //LOG(INFO, "DELIOC, IOCount:%d, IOC:%p\n",
      //          _iocListCount, ioc);
      delete ioc;
  }
  _iocListHead = _iocListTail = NULL;
  _iocListCount = 0;
  list = _delListHead;
  while (list) {
      ioc = list;
      assert(ioc != NULL);
      list = list->_next;
      //LOG(INFO, "DELIOC, IOCount:%d, IOC:%p\n",
      //          _iocListCount, ioc);
      delete ioc;
  }
  _delListHead = _delListTail = NULL;
}

bool* Transport::getStop()
{
  return &_stop;
}

}//namespace base
}//namespace neptune
