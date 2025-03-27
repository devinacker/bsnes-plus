// win32-specific socket and multithreaded functionality
//
// 3 threads work together to deal with the insane volume of data generated from a non-tracemasked binary output:
// 1) main thread (emulation normal thread). creates buffers with a few hundred trace items in them,
//    sends a big batch over to compression thread
// 2) compression thread: uses zlib [badly] to compress buffer batches, puts in queue for sending via network
// 3) socket sending thread: receives compressed buffers and sends to the network
//
// The entire system's output speed is currently limited by the ability to send the compressed data, or the ability to
// compress data fast enough.  With tracemasking on, we greatly reduce stuff being output and performance is a piece
// of cake.  With tracemasking off, the full firehose is sent through this pipeline. This implementation achieves
// ~45FPS realtime sending to an external client. That's not too shabby.  Emulation is paused if network transmission is
// too slow to keep up with our volume of data, so the end result is a limiting effect on max FPS.
//
// Previous text-only implementations written to disk crawled at around 2FPS when tracing was enabled. This implementation
// allows minimal impact on a user, so large volumes of data can be logged with an external tool (Like "Diztinguish")
//
// This entire class was hacked together and surely could be tuned better, and with a non-platform-specific
// threading and socket-implementation.
//
//
// TODO: demo: proof of concept only. for merging, use (probably) QtNetwork instead for cross-platform
// and better async IO notifications so UI doesn't block. for now, it's good enough to demo the idea.
//
// TODO: There are multiple classes in here, break them out.

#include "w32_socket.h"

inline QTextStream& qStdout()
{
    // TODO: use real logging, this is a hack. it doesn't even really work
    static QTextStream r{stdout};
    return r;
}

void SocketServer::ReportError(const char *errorMsg, bool printLastError) {
    qStdout() << "Error: " << errorMsg;

    if (printLastError)
        qStdout() << " (error# " << WSAGetLastError() << ")";

    qStdout() << endl;
}

void SocketServer::Die(const char* errorMsg, bool printLastError) {
    ReportError(errorMsg, printLastError);
    Shutdown();
}

void SocketServer::Shutdown() {
    if (!ClientHadConnected())
        return;

    int iResult = shutdown(_clientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        ReportError("Shutdown");
    }

    closesocket(_clientSocket);
    _clientSocket = INVALID_SOCKET;
    WSACleanup();
}

// Perf: This function is tuned to be able to run basically in realtime with the emulation now.
//
// If even THAT is even too slow, try the following for speedups:
// 1) Switch from TCP to UDP or Unix domain sockets
// 2) Make sure both side of the connection are always running locally (not across a real network, especially not wifi)
// 3) Add a separate thread and push bytes to it, let the other thread deal with the slight delays on the IO
// 4) Make sure client is able to read data fast enough so we don't fill up the system buffer and block.
// 5) Switch all this over to a shared memory buffer or memory mapped file implementation
//
// batch up bytes in a buffer for sending to the network.
// things are faster if we minimize the # of send() calls, so queue up a lot of data first
// before allowing it to send.
//
// returns true if we queued or sent the data.
// returns false if we were unable to queue or send this data. try again later?
bool SocketServer::Send(const uint8_t* buf, int len, bool &shouldRetryOut) {
    shouldRetryOut = false;

    if (!ClientHadConnected() || len == 0 || !buf)
        return false;

    // blocks if AsyncIO is disabled (default).
    // this is OK because we're on a dedicated sending thread
    // this function is called HEAVILY and tuning buffer size is key to performance improvements
    int iSendResult = send(_clientSocket, (const char *)(buf), len, 0);

    if (iSendResult != SOCKET_ERROR)
        return true;

    int lastError = WSAGetLastError();
    if (lastError == WSAENOBUFS || lastError == WSAEWOULDBLOCK) {
        shouldRetryOut = true;
    } else if (lastError == WSAECONNRESET) {
        shouldRetryOut = true;
        Die("Connection reset");
    } else {
        Die("Send failed.");
    }

    return false;
}

bool SocketServer::WaitForClientConnect(const char* servname) {
    if (ClientHadConnected())
        return true;

    SOCKET ListenSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL;

    bool success =
        DoOpen(servname, ListenSocket, result) &&
        BlockAndWaitForClientConnect(ListenSocket);

    // pretty sure I'm doing this wrong and we should hang onto listening socket and wait for other connectons on it
    if (ListenSocket != INVALID_SOCKET) {
        closesocket(ListenSocket);
    }

    if (result)
        freeaddrinfo(result); // todo: move above? i think. yea.

    if (success)
        return true;

    WSACleanup();
    if (_clientSocket)
        closesocket(_clientSocket);

    return false;
}

bool SocketServer::BlockAndWaitForClientConnect(SOCKET listenSocket, bool enableAsyncIO)
{
    // TODO: this blocks til a client connects. temporary demo / *not very friendly* way to do this.
    // what we should do instead is pause emulation (but not the UI) and then get notified when a client connects.
    // when that happens, unpause emulation and write the trace data to the socket.
    _clientSocket = accept(listenSocket, NULL, NULL);

    if (_clientSocket == INVALID_SOCKET) {
        ReportError("accept() failed");
        return false;
    }

    if (enableAsyncIO)
        EnableAsyncIO();

    return true;
}

bool SocketServer::DoOpen(const char *servname, SOCKET &ListenSocket, addrinfo *&result)
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        Die("WSAStartup() failed");
        return false;
    }

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, servname, &hints, &result);
    if (iResult != 0) {
        ReportError("getaddrinfo() failed");
        return false;
    }

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        ReportError("socket() failed");
        return false;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen); // Setup the TCP listening socket
    if (iResult == SOCKET_ERROR) {
        ReportError("bind() failed");
        return false;
    }

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        ReportError("listen() failed");
        return false;
    }

    return true;
}

void SocketServer::EnableAsyncIO() const {
    // 1 to enable non-blocking socket writes (doesn't return immediately, just won't block)
    u_long ioctl_mode = 1;
    ioctlsocket(_clientSocket, FIONBIO, &ioctl_mode);
}

bool SocketServer::ClientHadConnected() const {
    return _clientSocket != INVALID_SOCKET;
}

// ----------------------------------------------------

bool ByteBuffer::CanAdd(int len) {
    return _bufferLenUsed + len <= _maxSize;
}

void ByteBuffer::Clear() {
    _bufferLenUsed = 0;
}

bool ByteBuffer::Append(const uint8_t* buf, int len) {
    if (!CanAdd(len))
        return false;

    memcpy(_buffer + _bufferLenUsed, buf, len);
    _bufferLenUsed += len;
    return true;
}

ByteBuffer::ByteBuffer() {}

ByteBuffer::ByteBuffer(const ByteBuffer &from) {
    memcpy(_buffer, from._buffer, from._bufferLenUsed);
    _bufferLenUsed = from._bufferLenUsed;
}

// -----------------------------------------------------------------

bool BufferedServer::Init(const char* servName) {
    if (_initialized)
        return false;

    _workingBuffer.Clear();
    _initialized = true;

    return _thread.Init(servName);
}

bool BufferedServer::FlushWorkingBuffer() {
    if (!_initialized)
        return false;

    if (_workingBuffer._bufferLenUsed == 0)
        return true;

    if (!_thread.Push(_workingBuffer))
        return false;

    _workingBuffer.Clear();
    return true;
}

void BufferedServer::Shutdown() {
    if (!_initialized)
        return;

    FlushWorkingBuffer();

    _thread.Shutdown();

    _workingBuffer.Clear();
    _initialized = false;
}

bool BufferedServer::Push(const uint8_t* buf, int len) {
    if (!_initialized || len == 0 || !buf)
        return false;

    if (len > _workingBuffer._maxSize)
        return false;

    while (!_workingBuffer.Append(buf, len)) {
        FlushWorkingBuffer();
    }

    return true;
}

ThreadedSocketServer* ThreadedSocketServer::_threadedSocketServer = NULL;

DWORD WINAPI CompressThreadProc(void*) {
    ThreadedSocketServer* ts = ThreadedSocketServer::_threadedSocketServer;

    if (ts)
        ts->CompressThreadMain();

    return 0;
}

DWORD WINAPI SendThreadProc(void*) {
    ThreadedSocketServer* ts = ThreadedSocketServer::_threadedSocketServer;

    if (ts)
        ts->SendThreadMain();

    return 0;
}

bool ThreadedSocketServer::Init(const char *servName) {
    if (_compressThreadID != INVALID_HANDLE_VALUE)
        return false;

    _servName = servName;

    _compressQueueMutex = CreateMutex(
            NULL,              // default security attributes
            FALSE,             // initially not owned
            NULL);             // unnamed mutex

    _compressQueueItemAdded = CreateEvent(
            NULL,
            FALSE,              // = auto-reset event
            FALSE,              // initial state is nonsignaled
            NULL
    );

    _compressQueueOKToAdd = CreateEvent(
            NULL,
            TRUE,              // manual reset event
            TRUE,             // initial state is signalled
            NULL
    );

    _sendQueueMutex = CreateMutex(
            NULL,              // default security attributes
            FALSE,             // initially not owned
            NULL);             // unnamed mutex


    _sendQueueItemAdded = CreateEvent(
            NULL,
            FALSE,              // = auto-reset event
            FALSE,              // initial state is nonsignaled
            NULL
    );

    _sendQueueOKToAdd = CreateEvent(
            NULL,
            TRUE,              // manual reset event
            TRUE,             // initial state is signalled
            NULL
    );

    _compressThreadShouldContinue = _sendThreadShouldContinue = true;
    _threadedSocketServer = this;

    _compressThreadID = CreateThread(NULL, 0, CompressThreadProc, 0, 0, NULL);
    _sendThreadID = CreateThread(NULL, 0, SendThreadProc, 0, 0, NULL);

    return true;
}

// call from main thread
void ThreadedSocketServer::Shutdown() {
    _compressThreadShouldContinue = false;
    _sendThreadShouldContinue = false;

    if (_compressThreadID != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(_compressThreadID, INFINITE);
        CloseHandle(_compressThreadID);
    }

    if (_sendThreadID != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(_sendThreadID, INFINITE);
        CloseHandle(_sendThreadID);
    }

    // ----------------

    if (_compressQueueMutex != INVALID_HANDLE_VALUE)
        CloseHandle(_compressQueueMutex);

    if (_compressQueueItemAdded != INVALID_HANDLE_VALUE)
        CloseHandle(_compressQueueItemAdded);

    if (_compressQueueOKToAdd = INVALID_HANDLE_VALUE)
        CloseHandle(_compressQueueOKToAdd);


    if (_sendQueueMutex != INVALID_HANDLE_VALUE)
        CloseHandle(_sendQueueMutex);

    if (_sendQueueItemAdded != INVALID_HANDLE_VALUE)
        CloseHandle(_sendQueueItemAdded);

    if (_sendQueueOKToAdd = INVALID_HANDLE_VALUE)
        CloseHandle(_sendQueueOKToAdd);

    _compressThreadID = _compressQueueMutex = _compressQueueItemAdded = _compressQueueOKToAdd = INVALID_HANDLE_VALUE;
    _sendThreadID = _sendQueueMutex = _sendQueueItemAdded = _sendQueueOKToAdd = INVALID_HANDLE_VALUE;

    ThreadedSocketServer::_threadedSocketServer = NULL;
}

// call from main thread (producer)
bool ThreadedSocketServer::Push(ByteBuffer &_buffer) {

    HANDLE events[2];
    events[0] = _compressQueueMutex;
    events[1] = _compressQueueOKToAdd;

    // NOTE: this can block the main thread here if socket can't keep up, resulting in emulation stutter.
    // that's probably the right call, but, we could also discard data if we didn't care about completeness
    // and were going more for sampling.
    if (WaitForMultipleObjects(2, events, true, INFINITE)  != WAIT_OBJECT_0)
        return false;
    _readyToCompressQueue.push(_buffer);

    if (_readyToCompressQueue.size() >= _maxQueueLengthAllowed)
        ResetEvent(_compressQueueOKToAdd);

    ReleaseMutex(_compressQueueMutex);

    SetEvent(_compressQueueItemAdded);

    return true;
}

#include <zlib/zlib.h>

// call on compress worker thread
// return false to kill the thread.
bool ThreadedSocketServer::ProcessNextCompressedItem() {

    // get latest off the queue, if we can
    if (WaitForSingleObject(_compressQueueMutex, 500) != WAIT_OBJECT_0)
        return true; // go around again.
    ByteBuffer buffer = _readyToCompressQueue.front();
    _readyToCompressQueue.pop();
    SetEvent(_compressQueueOKToAdd);
    ReleaseMutex(_compressQueueMutex);

    // ------------------------------------------------------------------------------------------------
    // compress
    // perf: takes longer than realtime to shove these bytes into a socket.
    // spend CPU time to reduce the amount of bytes that need to go out, and hopefully reduce jittering
    // ------------------------------------------------------------------------------------------------

    // TODO: all this casting from Bytef and char* can likely be simplified

    // COMPRESS BUFFER ASSUMPTION for zlib:
    // data must start out being AT LEAST 0.1% larger than
    // the original size of the data, + 12 extra bytes.
    // So, we'll just play it safe and alloated 1.1x
    // as much memory + 12 bytes (110% original + 12 bytes)
    const int headerSize = 1 + sizeof(ULONG) + sizeof(ULONG);

    ULONG zlibCompressBufferSize = (ByteBuffer::_maxSize * 1.1) + 12;
    ULONG packetFullSize = zlibCompressBufferSize + headerSize;

    Bytef* packetBuffer = new Bytef[packetFullSize];

    // create a pointer inside packetBuffer to just the compressed data (it starts after our header)
    Bytef* zlibCompressBuffer = packetBuffer + headerSize;

    ULONG zlibCompressedDataSizeOut = zlibCompressBufferSize;

    int z_result = compress(zlibCompressBuffer, &zlibCompressedDataSizeOut, reinterpret_cast<const Bytef*>(buffer._buffer), buffer._bufferLenUsed);

    if (z_result == Z_MEM_ERROR || z_result == Z_BUF_ERROR)
        return false;

    // populate the header:
    packetBuffer[0] = 'Z'; // watermark saying this is a compressed bit of data coming up

    packetBuffer[1] = (buffer._bufferLenUsed >> 0) & 0xFF; // store length of original data
    packetBuffer[2] = (buffer._bufferLenUsed >> 8) & 0xFF;
    packetBuffer[3] = (buffer._bufferLenUsed >> 16) & 0xFF;
    packetBuffer[4] = (buffer._bufferLenUsed >> 24) & 0xFF;

    packetBuffer[5] = (zlibCompressedDataSizeOut >> 0) & 0xFF; // store length of compressed data that follows
    packetBuffer[6] = (zlibCompressedDataSizeOut >> 8) & 0xFF;
    packetBuffer[7] = (zlibCompressedDataSizeOut >> 16) & 0xFF;
    packetBuffer[8] = (zlibCompressedDataSizeOut >> 24) & 0xFF;

    int len = headerSize + zlibCompressedDataSizeOut;
    bool result = PushDecompressedData(reinterpret_cast<const uint8_t*>(packetBuffer), len);

    return result;
}

// call from compress worker thread
bool ThreadedSocketServer::PushDecompressedData(const uint8_t* buffer, int len) {
    HANDLE events[2];
    events[0] = _sendQueueMutex;
    events[1] = _sendQueueOKToAdd;

    // NOTE: this can block if sending socket can't keep up, resulting in emulation stutter.
    if (WaitForMultipleObjects(2, events, true, INFINITE)  != WAIT_OBJECT_0)
        return false;
    _readyToSendQueue.push(std::make_tuple(len, buffer));

    if (_readyToSendQueue.size() >= _maxQueueLengthAllowed)
        ResetEvent(_sendQueueOKToAdd);

    ReleaseMutex(_sendQueueMutex);

    SetEvent(_sendQueueItemAdded);
    return true;
}

// socket thread
bool ThreadedSocketServer::ProcessNextDecompressedItem() {
    // get latest off the queue, if we can
    if (WaitForSingleObject(_sendQueueMutex, 500) != WAIT_OBJECT_0)
        return true; // go around again.
    auto item = _readyToSendQueue.front();
    _readyToSendQueue.pop();
    SetEvent(_sendQueueOKToAdd);
    ReleaseMutex(_sendQueueMutex);

    auto len=std::get<0>(item);
    auto buffer=reinterpret_cast<const uint8_t*>(std::get<1>(item));

    bool result = SendBuffer(buffer, len);

    delete reinterpret_cast<const Bytef*>(buffer);

    return result;
}

// call on socket thread
bool ThreadedSocketServer::SendBuffer(const uint8_t* buffer, int len) {
    // this sequence could probably use a better coat of paint

    const int delay_time_ns = 500;
    bool shouldRetry, sent=false;

    do {
        if (!_socketServer.WaitForClientConnect(_servName))
            return true;

        if (_socketServer.Send(buffer, len, shouldRetry)) {
            sent=true;
        }

        if (shouldRetry) {
            // NOTE: DUMB WAY TO HANDLE THIS, use select() instead or overlapped IO stuff probably.
            usleep(delay_time_ns);
        }
    } while (shouldRetry && _compressThreadShouldContinue);

    // don't return false here if we disconnected, just try again.
    return sent;
}

// call on compress thread
bool ThreadedSocketServer::WaitForCompressQueueWrite() {
    DWORD result = WaitForSingleObject(_compressQueueItemAdded, 500);

    if (result == WAIT_OBJECT_0) {
        return ProcessNextCompressedItem();
    } else if (result == WAIT_TIMEOUT) {
        // timeout, just fall through, try again.
        return true;
    } else {
        // error
        return false;
    }
}

// call on compress thread
bool ThreadedSocketServer::WaitForSendQueueWrite() {
    DWORD result = WaitForSingleObject(_sendQueueItemAdded, 500);

    if (result == WAIT_OBJECT_0) {
        return ProcessNextDecompressedItem();
    } else if (result == WAIT_TIMEOUT) {
        // timeout, just fall through, try again.
        return true;
    } else {
        // error
        return false;
    }
}

void ThreadedSocketServer::CompressThreadMain() {
    while (_compressThreadShouldContinue) {
        if (!WaitForCompressQueueWrite())
            break;
    }
}

void ThreadedSocketServer::SendThreadMain() {
    if (!_socketServer.WaitForClientConnect(_servName))
        return;

    while (_sendThreadShouldContinue) {
        if (!WaitForSendQueueWrite())
            break;
    }

    _socketServer.Shutdown();
}

ThreadedSocketServer::ThreadedSocketServer() {
}

ThreadedSocketServer::~ThreadedSocketServer() {
    Shutdown();
}
