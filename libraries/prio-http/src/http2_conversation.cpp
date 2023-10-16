#include <event2/event.h>
#include "priohttp/http2_conversation.h"
#include "priocpp/api.h"
#include "priohttp/server/reader.h"
#include "priohttp/server/writer.h"
#include "priohttp/http2.h"
#include <set>
#include <sstream>
#include <deque>


namespace prio  {



Http2Conversation::Http2Conversation(Connection::Ptr f)
	: con_(f),
      http2_(std::make_unique<http2_server_session>(this))
{
    REPRO_MONITOR_INCR(Http2Conversation);    
}
 
Http2Conversation::~Http2Conversation()
{
    REPRO_MONITOR_DECR(Http2Conversation);    
}

std::string Http2Conversation::common_name()
{
	return con_->common_name();
}

prio::Callback<Request&,Response&>& Http2Conversation::on(Connection::Ptr s)
{
	auto r =  Ptr( new Http2Conversation(s));
	r->self_ = r;

    r->http2_->initialize_nghttp2_session();
    r->http2_->send_connection_header();
    r->http2_->send()
    .then([r]()
    {
       r->schedule_read();
    })
    .otherwise([r](const std::exception_ptr& ex)
	{
		r->onRequestError(ex);
	});     
    
	return r->cb_;
}

void Http2Conversation::schedule_read()
{
    auto ptr = shared_from_this();

    con_->read()
    .then([ptr](Connection::Ptr,std::string s)
    {
        return ptr->http2_->recv(s);
    })
    .then([ptr]()
    {
        ptr->schedule_read();
    })
    .otherwise([ptr](const std::exception_ptr& ex)
	{
		ptr->onRequestError(ex);
	}); 
}

void Http2Conversation::resolve(Request& req, Response& res)
{ 
    cb_.resolve(req,res);
}

repro::Future<> Http2Conversation::flush(Response& res)
{  
    auto p = repro::promise();

    int stream_id = res.attributes.attr<int>(":http2:stream:id");    

    http2_server_stream* s = http2_->get_stream_by_id(stream_id);
    if(!s)
    {
        std::cout << "stream id is gone " << stream_id << std::endl;
        nextTick([p](){ p.resolve(); });
        return p.future();
    }
 
    auto stream = s->shared_from_this();
    auto ptr = shared_from_this();

	stream->flusheaders_func(stream->req,res)
	.then( [ptr,p,stream,stream_id,&res]()
	{
        auto s = ptr->http2_->flush(res);
        if(!s)
        {
            std::cout << "stream id went away " << stream_id << std::endl;
            stream->reset();     
            p.resolve();
            return;
        }

        ptr->http2_->send()
        .then([ptr,p,stream]()
        {
            if(stream->completion_func)
            {
                stream->completion_func(stream->req,stream->res);
            }
        
            if(stream->req.detached())
            {
                ptr->self_.reset();
                stream->reset_callbacks();
                p.resolve();
                return;
            }

            p.resolve();
            stream->reset();        
        })
        .otherwise([ptr,p,stream](const std::exception_ptr& ex)
        {
            ptr->onRequestError(ex);
            stream->reset();     
            p.resolve();
        });    
    });

    return p.future();
}

void Http2Conversation::chunk(const std::string& ch)
{
   // TODO ?
}

void Http2Conversation::onRequestError(const std::exception_ptr& ex)
{
	cb_.reject(ex);
	con_->close();
	self_.reset();
}

Connection::Ptr Http2Conversation::con()
{
	return con_;
}

bool Http2Conversation::keepAlive()
{
	return false;
}

void Http2Conversation::onCompletion(std::function<void(Request& req, Response& res)> f, Response& res)
{
    int stream_id = res.attributes.attr<int>(":http2:stream:id");    

    http2_server_stream* s = http2_->get_stream_by_id(stream_id);
    if(!s)
    {
        std::cout << "no stream id for onCompletion " << stream_id << std::endl;
        return;
    }

    s->completion_func = f;
}

void Http2Conversation::onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> f, Response& res)
{
    int stream_id = res.attributes.attr<int>(":http2:stream:id");    

    http2_server_stream* s = http2_->get_stream_by_id(stream_id);
    if(!s)
    {
        std::cout << "no stream id for onFlushHeaders " << stream_id << std::endl;
        return;
    }

	s->flusheaders_func = f;
}


} // close namespaces

