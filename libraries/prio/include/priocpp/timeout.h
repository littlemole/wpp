#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_API_TIMEOUT_HANDLER_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_API_TIMEOUT_HANDLER_DEF_GUARD_

/**
 * \file timeout.h
 */

#include "priocpp/common.h"


namespace prio      {


//////////////////////////////////////////////////////////////
//! do somethin on event loop after timer expires
//! \param secs int 
//! \param ms int
//! returns a future with no arguments
//////////////////////////////////////////////////////////////

repro::Future<> timeout(int secs, int ms) noexcept;

//////////////////////////////////////////////////////////////
//! do somethin on event loop after timer expires
//! \param secs int
//! returns a future with no arguments
//////////////////////////////////////////////////////////////
repro::Future<> timeout(int secs) noexcept;

//////////////////////////////////////////////////////////////
//! do something next on EventLoop
//! returns a future with no arguments
//////////////////////////////////////////////////////////////

repro::Future<> nextTick() noexcept;

//////////////////////////////////////////////////////////////
//! timeout events.
//! threadsafe version, callable from worker thread
//! must specify callback directly
//////////////////////////////////////////////////////////////

void timeout( const std::function<void()>& f, int secs, int ms) noexcept;

//////////////////////////////////////////////////////////////
//! timeout events.
//! threadsafe version, callable from worker thread
//! must specify callback directly
//////////////////////////////////////////////////////////////
void timeout( const  std::function<void()>& f, int secs) noexcept;
    

//////////////////////////////////////////////////////////////
// Timeout, cancel-able
//////////////////////////////////////////////////////////////

/// @private
class Timeout
{
public:

   Timeout();
   ~Timeout();
   
   repro::Future<> after(int ms);
   void after(const std::function<void()>& f,int ms);
   void after(std::function<void()>&& f, int ms);

   void cancel();
   
private:
   Timeout(const Timeout&) = delete;
   Timeout& operator=(const Timeout&) = delete;
   std::unique_ptr<TimeoutImpl> impl_;
};

/// @private
class TimeoutEx : public repro::ReproEx<TimeoutEx> {};


} // close namespaces

#endif

