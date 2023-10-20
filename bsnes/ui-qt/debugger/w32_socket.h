#ifndef BUFFERED_SOCKET_H
#define BUFFERED_SOCKET_H

#include <windows.h>
#include <queue>
#include <tuple>

const int one_megabayte = 0x400 * 0x400;

class ByteBuffer {
public:
    // perf: instead of sending each ~8 byte message immediately, batch them up and send a full batch at once
    // we're striking a balance between reducing synchronization overhead (so larger = better),
    // vs having to stay small to not have the following:
    // 1) the system reject this buffer for socket sending because it's too big [so smaller = better]
    // 2) if sending on a real network via TCP, packets don't need to be fragment and destroy our performance [so, max size = ~1400 bytes typically]
    const static int _maxSize = 1 * one_megabayte;

    uint8_t _buffer[_maxSize];
    int _bufferLenUsed = 0;

    bool CanAdd(int len);
    bool Append(const uint8_t* buf, int len);
    void Clear();

    ByteBuffer(const ByteBuffer& from);
    ByteBuffer();
};

struct addrinfo;

class SocketServer {
public:
    bool WaitForClientConnect(const char* servName);
    void Shutdown();

    bool Send(const uint8_t *buf, int len, bool &shouldRetryOut);
    bool ClientHadConnected() const;

protected:
    SOCKET _clientSocket = INVALID_SOCKET;

    bool DoOpen(const char *servname, SOCKET &ListenSocket, addrinfo *&result);

    void EnableAsyncIO() const;

    void ReportError(const char *errorMsg, bool printLastError = true);
    void Die(const char *errorMsg, bool printLastError = true);

    bool BlockAndWaitForClientConnect(SOCKET listenSocket, bool enableAsyncIO = false);
};

class ThreadedSocketServer {
public:
    bool Init(const char* servName);
    void Shutdown();

    bool Push(ByteBuffer& byteBuffer);

    // don't call these directly from client code
    void CompressThreadMain();
    void SendThreadMain();

    static ThreadedSocketServer* _threadedSocketServer;

    ThreadedSocketServer();
    ~ThreadedSocketServer();

protected:
    string _servName;
    const int _maxQueueBytes = (100 * one_megabayte);

    volatile HANDLE _compressQueueMutex = INVALID_HANDLE_VALUE;
    volatile HANDLE _compressQueueItemAdded = INVALID_HANDLE_VALUE;
    volatile HANDLE _compressQueueOKToAdd = INVALID_HANDLE_VALUE;
    volatile bool _compressThreadShouldContinue = false;
    std::queue<ByteBuffer> _readyToCompressQueue; // NOTE: this does a LOT of constant and heavy allocations right now, fix if we care.

    volatile HANDLE _sendQueueMutex = INVALID_HANDLE_VALUE;
    volatile HANDLE _sendQueueItemAdded = INVALID_HANDLE_VALUE;
    volatile HANDLE _sendQueueOKToAdd = INVALID_HANDLE_VALUE;
    volatile bool _sendThreadShouldContinue = false;
    std::queue<std::tuple<int, const uint8_t*>> _readyToSendQueue;

    const int _maxQueueLengthAllowed = int(_maxQueueBytes / ByteBuffer::_maxSize);

    SocketServer _socketServer;
    volatile HANDLE _compressThreadID = INVALID_HANDLE_VALUE;
    volatile HANDLE _sendThreadID = INVALID_HANDLE_VALUE;

    bool PushDecompressedData(const uint8_t *buffer, int len);

    bool ProcessNextCompressedItem();
    bool ProcessNextDecompressedItem();

    bool WaitForCompressQueueWrite();
    bool WaitForSendQueueWrite();

    bool SendBuffer(const uint8_t* packetBuffer, int len);


};

// use this for the main thread
class BufferedServer {
public:
    bool Init(const char* servName);
    void Shutdown();

    bool Push(const uint8_t* buf, int len);

    bool FlushWorkingBuffer();

    inline bool IsInitialized() {return _initialized;}

protected:
    ByteBuffer _workingBuffer;
    bool _initialized = false;

    ThreadedSocketServer _thread;
};

#endif // BUFFERED_SOCKET_H