#include "reproweb/ctrl/session.h"

namespace reproweb {


MemorySessionProvider::MemorySessionProvider()
    : session_timeout_(60*5)
{
    cleanup();
}


MemorySessionProvider::MemorySessionProvider( int timeout)
    : session_timeout_(timeout)
{
    cleanup();
}

MemorySessionProvider::~MemorySessionProvider() 
{
    timer_.cancel();
}

repro::Future<Session> MemorySessionProvider::get_session(std::string sid)
{
    auto p = repro::promise<Session>();

    prio::nextTick( [this,p,sid]() 
    {
        if(map_.count(sid) == 0)
        {
            auto json = Json::Value(Json::objectValue);
            map_[sid] = Session(sid,json);
        }

        timeouts_[sid] = now() + session_timeout_;
        p.resolve(map_[sid]);
    });

    return p.future();
}

repro::Future<> MemorySessionProvider::set_session(Session session)
{
    auto p = repro::promise<>();

    prio::nextTick( [this,p,session]() 
    {
        map_[session.sid] = session;
        timeouts_[session.sid] = now() + session_timeout_;
        p.resolve();
    });

    return p.future();
}

repro::Future<> MemorySessionProvider::remove_user_session(Session session)
{
    auto p = repro::promise<>();

    prio::nextTick( [this,p,session]() 
    {        
        if(map_.count(session.sid)!=0)
        {
            map_.erase(session.sid);
            timeouts_.erase(session.sid);
        }
        p.resolve();
    });

    return p.future();
}


void MemorySessionProvider::cleanup()
{
    int n = now();

    std::vector<std::string> purgatory;

    for( auto& it : timeouts_)
    {
        if ( n > it.second)
        {
            purgatory.push_back(it.first);
        }
    }

    for( auto& k : purgatory)
    {
        map_.erase(k);
        timeouts_.erase(k);
    }

    timer_.after([this]()
    {
        cleanup();
    }, 1000*60*5);
}


int MemorySessionProvider::now()
{
    std::time_t t = std::time(0);
    return t;
}


SessionFilter::SessionFilter(std::shared_ptr<SessionProvider> sp)
    : sid_cookie_name_("repro_web_sid"), session_provider_(sp)
{}

SessionFilter::SessionFilter(const std::string& cn, std::shared_ptr<SessionProvider> sp)
    : sid_cookie_name_(cn), session_provider_(sp)
{}    

void SessionFilter::before( prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain)
{
    const prio::Cookies& cookies = req.headers.cookies();

    std::string sid;
    if(cookies.exists(sid_cookie_name_))
    {
        sid = cookies.get(sid_cookie_name_).value();
    }

    if(sid.empty())
    {
        sid = make_session_id();

        Session session(sid,Json::objectValue);
        req.attributes.set("_session_", std::make_shared<Session>(session) );   
        chain->next(req,res);
        return;
    }

    session_provider_->get_session(sid)
    .then([&req,&res,chain]( Session session ) 
    {
        req.attributes.set("_session_", std::make_shared<Session>(session) );

        chain->next(req,res);
    })
    .otherwise([this,&req,&res,chain]( const std::exception& ex)
    {
        std::cout << "before ex: " << ex.what() << std::endl;
        //res.error().body(ex.what()).flush();

        std::string sid = make_session_id();

        Session session(sid,Json::objectValue);
        req.attributes.set("_session_", std::make_shared<Session>(session) );   
        chain->next(req,res);        
    });
}


void SessionFilter::after( prio::Request& req, prio::Response& res, std::shared_ptr<FilterChain> chain)
{
    auto session = req.attributes.attr<std::shared_ptr<Session>>("_session_");
    if(session->sid.empty())
    {
        session->sid = make_session_id();
    }

    if ( session && session->valid)
    {
        
        res.cookie(prio::Cookie(sid_cookie_name_,session->sid).path("/"));

        session_provider_->set_session(*session)
        .then([&req,&res,chain]()
        {
            chain->next(req,res);     
        })
        .otherwise([&req,&res,chain]( const std::exception& ex)
        {
            std::cout << "after ex: " << ex.what() << std::endl;
            chain->next(req,res);     
        });
    }
    else
    {
        res.cookie(prio::Cookie(sid_cookie_name_,make_session_id()).path("/"));

        if(session)
        {
            session_provider_->remove_user_session(*session)
            .then([&req,&res,chain]()
            {
                chain->next(req,res);     
            })
            .otherwise([&req,&res,chain]( const std::exception& ex)
            {
                chain->next(req,res);     
            });
        }
        else
        {
             chain->next(req,res); 
        }
    }
}    


std::string SessionFilter::make_session_id()
{
    std::string sid = sid_cookie_name_ + "::";
    sid += cryptoneat::toHex(cryptoneat::nonce(64));
    return sid;
}    


}