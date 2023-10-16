#include <fcntl.h>

#include "priocpp/api.h"
#include "priocpp/loop.h"
#include "priocpp/connection.h"
#include "priocpp/ssl_connection.h"

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#ifdef _WIN32
#include <inttypes.h>
typedef int ssize_t;
#endif



using namespace repro;
 
namespace prio      {

Future<> timeout( int secs, int ms) noexcept
{
  auto p = repro::promise<>();
  
  auto t = new Timeout();

  ms = ms + 1000*secs;
  
  t->after(ms)
  .then( [p,t]() 
  {
    p.resolve();
    delete t;
  })
  .otherwise( [p,t]( const std::exception& ex) 
  {
    p.reject(ex);
    delete t;
  });

  return p.future();
}

void timeout(const std::function<void()>& f, int secs, int ms) noexcept
{
  auto t = new Timeout();

  ms = ms + 1000*secs;
  
  t->after( [t,f]()
  {
	 f();
	 delete t;
  }, ms);

}


void timeout(const std::function<void()>& f, int secs) noexcept
{
	timeout(f, secs,0);
}

Future<> timeout( int secs) noexcept
{
	return timeout(secs,0);
}




Future<int> signal(int s) noexcept
{
	return theLoop().signal(s);
}



connection_timeout_t& connection_timeouts()
{
	static 	connection_timeout_t ctt { 10000, 10000 };
	return ctt;
}

Future<Connection::Ptr> Connection::connect(const std::string& host, int port)
{
	return TcpConnection::connect(host,port);
}


Future<Connection::Ptr> Connection::connect(const std::string& host, int port, SslCtx& ctx)
{
	return SslConnection::connect(host,port,ctx);
}



} // close namespaces



