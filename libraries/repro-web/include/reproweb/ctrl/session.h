#ifndef INCLUDE_PROMISE_WEB_CONTROLLER_SESSION_H_
#define INCLUDE_PROMISE_WEB_CONTROLLER_SESSION_H_

//! \file session.h

#include "reproweb/ctrl/controller.h"
#include "reproweb/ctrl/filter.h"
#include "reproweb/json/json.h"
#include "metacpp/meta.h"
#include "cryptoneat/cryptoneat.h"
#include <ctime>

namespace reproweb  
{


//! HTTP Session
//! \ingroup session

class Session
{
public:

    //! \private
    Session() 
    {}

    //! \private
	Session(const std::string& id,Json::Value p) 
		:sid(id), data(p)
	{}

    //! session is valid
    bool valid = true;
    //! session i authenticated
    bool authenticated = false;
    //! unique session identifies
	std::string sid;
    //! arbitary JSON data associated with this session
	Json::Value data;	

    //! \private
    static constexpr auto meta()
    {
        return meta::data(
            "authenticated", &Session::authenticated,
            "sid", &Session::sid,
            "data", &Session::data
        );
    }

private:
};

//! Session Provider Interface
//! \ingroup session

class SessionProvider
{
public:

    virtual ~SessionProvider() {}

    //! fetch session for session-id
    virtual repro::Future<Session> get_session( std::string sid) = 0;
    //! set session for session-id
    virtual repro::Future<> set_session(Session session) = 0;
    //! remove session from session store
    virtual repro::Future<> remove_user_session(Session session) = 0;
};

//! In Memory Session provider
//! \ingroup session

class MemorySessionProvider : public SessionProvider
{
public:

    //! construct default MemorySessionProvider
    MemorySessionProvider();
    //! construct default MemorySessionProvider with given session timeout in seconds
    MemorySessionProvider( int timeout);

    virtual ~MemorySessionProvider();

    virtual repro::Future<Session> get_session(std::string sid);
    virtual repro::Future<> set_session(Session session);
    virtual repro::Future<> remove_user_session(Session session);

private:

    void cleanup();
    int now();

    prio::Timeout timer_;
    int session_timeout_;

    std::map<std::string,Session> map_;    
    std::map<std::string,int> timeouts_;    
};

//! HTTP filter to handle session management
class SessionFilter
{
public:

    //! \private
    SessionFilter(std::shared_ptr<SessionProvider> sp);
    //! \private
    SessionFilter(const std::string& cn, std::shared_ptr<SessionProvider> sp);

    //! \private
    void before( prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain);
    //! \private
    void after( prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain);

private:

    std::string sid_cookie_name_;
    std::shared_ptr<SessionProvider> session_provider_;

    std::string make_session_id();

};

//! declare HTTP filter in WebApplicationContext
template<class F = SessionFilter>
struct session_filter
{
    //! \private
    typedef F type;

    //! specify HTTP method, HTTP path and filter priority    
    session_filter(const std::string& m, const std::string& p, int prio = 0 )
        : before(m,p,&F::before,prio), 
          after(m,p,&F::after,prio)
    {}

    //! \private
	void ctx_register(diy::Context* ctx)
	{
        before.ctx_register(ctx);
        after.ctx_register(ctx);
	}    

    //! \private
    filter_router<decltype(& F::before)> before;
    //! \private
    flush_filter_router<decltype(& F::after)> after;
};

//! fetch HTTP Session from Reqquest
inline std::shared_ptr<Session> req_session(prio::Request& req)
{
    if(!req.attributes.exists("_session_"))
    {
        auto json = Json::Value(Json::objectValue);
        return std::make_shared<Session>("",json);
    }
    return req.attributes.attr<std::shared_ptr<Session>>("_session_");
}

//! determine whether HTTP request was authenticated
inline bool is_authenticated(prio::Request& req)
{
    return req_session(req)->authenticated;
}

//! invalidate HTTP session
inline void invalidate_session(prio::Request& req)
{
    auto s = req_session(req);
    s->valid = false;
    s->authenticated = false;
}

}

#endif
