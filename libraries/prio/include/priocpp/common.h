#ifndef _MOL_DEF_GUARD_DEFINE_MOD_LIBEVENT_COMMON_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_LIBEVENT_COMMON_DEF_GUARD_

/**
 * \file common.h
 * common helpers
 */

#ifndef _WIN32
#include <unistd.h>
#else
typedef unsigned short ushort;
#include <winsock2.h>
#endif

#include <vector>
#include <deque>
#include <string.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "reprocpp/promise.h"
#include "reprocpp/ex.h"
   
namespace prio  {

//! generic IO exception
class IoEx  : public repro::ReproEx<IoEx> { public : IoEx(const std::string& s) : repro::ReproEx<IoEx>(s) {} };

//! generic IO error exception
class IoErr : public repro::ReproEx<IoErr> { public : IoErr(const std::string& s) : repro::ReproEx<IoErr>(s) {} };
//! end-of-file IO exception indicating closed connection
class IoEof : public repro::ReproEx<IoEof> { public : IoEof(const std::string& s) : repro::ReproEx<IoEof>(s) {} };
//! exception thrown on timeouts
class IoTimeout : public repro::ReproEx<IoTimeout> { public : IoTimeout(const std::string& s) : repro::ReproEx<IoTimeout>(s) {} };

//! remove whitespace on both tails of a string
std::string trim(const std::string& input);

//! generate random nonce of length n
std::string nonce(unsigned int n);

//! encode input string to base64
std::string base64_encode(const std::string& bytes_to_encode);
//! encode input c-style string to base64
std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len);
//! decode a base64 encoded string
std::string base64_decode(const std::string& encoded_string);

//! current time as UNIX timestamp
size_t unix_timestamp();

#ifndef _WIN32
typedef int socket_t;
#else
typedef uintptr_t socket_t;
#endif


//////////////////////////////////////////////////////////////
// forwards
//////////////////////////////////////////////////////////////

struct TimeoutImpl;
struct SignalImpl;
struct ListenerImpl;
struct IOImpl;
struct SslCtxImpl;

class Loop;
class ThreadPool;
class Connection;
class Timeout;
class Signal;
class Listener;
class IO;
class SslCtx;

typedef std::shared_ptr<Connection> ConnectionPtr;

}

#endif

