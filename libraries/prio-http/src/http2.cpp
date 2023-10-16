#include <event2/event.h>
#include "priohttp/http2_conversation.h"
#include "priocpp/api.h"
#include "priohttp/server/reader.h"
#include "priohttp/server/writer.h"
#include "priohttp/http2.h"
#include <set>
#include <sstream>
#include <deque>

using namespace repro;

namespace prio  {


http2_stream::http2_stream(int32_t id,Conversation* con)
    : res(con),
      stream_id(id),
      written(0)
{
    reset_callbacks();
}

http2_stream::http2_stream(Request& request,Conversation* con)
:   req(request),
    res(0),
    stream_id(0),
    written(0)          
{}

void http2_stream::reset()
{
    ((HttpRequest&)req).reset();
    ((HttpResponse&)res).reset();
    written = 0;
    oss.str("");
    oss.clear();

    reset_callbacks();
}    

void http2_stream::reset_callbacks()
{
	flusheaders_func = [](Request&,Response&)
     {
        auto p = repro::promise<>();
        nextTick([p]()
        {
            p.resolve();
        });
        return p.future();
     };

     completion_func =  [](Request&,Response&){};      
}           

http2_server_stream::http2_server_stream(int32_t id,Conversation* con)
    : http2_stream(id,con)
{}

http2_server_stream::~http2_server_stream()
{}

int http2_server_stream::on_header_callback(
    const nghttp2_frame *frame, 
    const uint8_t *name,
    size_t namelen, 
    const uint8_t *value,
    size_t valuelen, 
    uint8_t flags) 
{
    static const char* pseudo_headers[] = { ":method", ":scheme",":authority",":path" };
    static std::function<void(HttpRequest& r,char* c)> pseudo_handlers[] = { 
        [](HttpRequest& req,char* c){ req.path.method(c); },
        [](HttpRequest& req,char* c){ ; },
        [](HttpRequest& req,char* c){ req.header("Host",c); },
        [](HttpRequest& req,char* c){ req.path.path(c); }
    };
    
    HttpRequest& request = (HttpRequest&)(req);

    // check for http2 pseudo headers
    for ( size_t i = 0; i < 4; i++)
    {
        size_t len = strlen(pseudo_headers[i]);
        if ( namelen != len ) 
            continue;
        if ( memcmp(pseudo_headers[i],name,namelen)!=0 ) 
            continue;

        pseudo_handlers[i](request,(char*)value);
        return 0;
    }

    // plain old header
    request.header((char*)name,(char*)value);

    return 0;
}

int http2_server_stream::data_chunk_recv_callback(
    uint8_t flags, 
    const uint8_t *data, 
    size_t len) 
{
    if(len==0)
        return 0;

    oss.write((char*)data,len);
    return 0;
}    

ssize_t http2_server_stream::data_provider_callback(
    uint8_t *buf, 
    size_t length,
    uint32_t *data_flags) 
{    
    size_t want = res.body().size() - written;
    if(want > length) want = length;

    memcpy( buf, res.body().data() + written, want );

    written += want;

    if ( written >= res.body().size())
    {                    
        *data_flags |= NGHTTP2_DATA_FLAG_EOF;
    }
    return want;
}    

///////////////////////////////////////////////////////////////////


http2_session::http2_session(Conversation* c) 
: con_(c)
{}

http2_session::~http2_session() 
{
    streams_.clear();
    nghttp2_session_del(session_);
    
}


// serialize frame and send over the socket
Future<> http2_session::send() 
{
    auto p = repro::promise<>();
    
    std::ostringstream oss;
    const uint8_t *data_ptr = 0;

    ssize_t rv = nghttp2_session_mem_send(session_, &data_ptr);
    if(rv==0)
    {
        return p.resolved();
    }

    while(rv>0)
    {
        oss.write((char*)data_ptr,rv);
        rv = nghttp2_session_mem_send(session_, &data_ptr);        
    }

    if(rv<0)
    {
        repro::Ex ex("err http2_server_session::send() ");
        return p.rejected(ex);
    }

    std::string tmp = oss.str();
    if(tmp.empty())
        return p.resolved();

    con_->con()->write(tmp)
    .then([p](Connection::Ptr)
    {
       p.resolve();
    })
    .otherwise([p](const std::exception_ptr& ex)
    {
        p.reject(ex);
    });   
    return p.future();
}

// feed data received from socket to nghttp2
Future<> http2_session::recv(const std::string& s) 
{
    auto p = repro::promise<>();
    
    size_t datalen = s.size();
    unsigned char *data = (unsigned char*)(s.data());
  
    ssize_t readlen = nghttp2_session_mem_recv(session_, data, datalen);
    if (readlen < 0) 
    {
        repro::Ex ex("http2_server_session::recv");
        return p.rejected(ex);
    }
    
    send()
    .then([p]()
    {
        p.resolve();
    })
    .otherwise([p](const std::exception_ptr& ex)
    {
        p.reject(ex);
    });
    
    return p.future();
}

// initial server http2 frame
int http2_session::send_connection_header() 
{
    nghttp2_settings_entry iv[1] = {
        {NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 100}
    };

    int rv = nghttp2_submit_settings(session_, NGHTTP2_FLAG_NONE, iv,1);
    if (rv != 0) 
    {
      return -1;
    }
    return 0;
}

// stream has been closed
int http2_session::on_stream_close_callback(int32_t stream_id, uint32_t error_code) 
{
    http2_stream* stream_data = get_stream_by_id(session_,stream_id);
    if (!stream_data) 
    {
        return 0;
    }

    for ( auto it = streams_.begin(); it != streams_.end(); it++)
    {
        if( (*it).get() == stream_data)
        {
            streams_.erase(it);
            return 0;
        }
    }
    return 0;
}



///////////////////////////////////////////////////////////////////
void http2_server_session::initialize(nghttp2_session_callbacks *callbacks) 
{
    nghttp2_session_server_new(&session_, callbacks, this);
}

http2_server_session::http2_server_session(Conversation* c) 
    : http2_session(c)
{}
  
http2_server_session::~http2_server_session() 
{}


ssize_t data_provider_callback(
    nghttp2_session *session, 
    int32_t stream_id,
    uint8_t *buf, 
    size_t length,
    uint32_t *data_flags,
    nghttp2_data_source *source,
    void *user_data);

// start sending a response with given headers and response body
int http2_server_session::send_response(int32_t stream_id,nghttp2_nv *nva, size_t nvlen)
{
    http2_server_stream* stream = get_stream_by_id(stream_id);

    nghttp2_data_provider data_prd;
    data_prd.source.ptr = stream;
    data_prd.read_callback = data_provider_callback;

    int rv = nghttp2_submit_response(session_, stream_id, nva, nvlen, &data_prd);
    if (rv != 0) 
    {
        std::cout << "http2_server_session::send_response failed " << rv << std::endl;
        return -1;
    }
    return 0;
}

// http2 header received callback
int http2_server_session::on_header_callback(
    const nghttp2_frame *frame, 
    const uint8_t *name,
    size_t namelen, 
    const uint8_t *value,
    size_t valuelen, 
    uint8_t flags) 
{
    switch (frame->hd.type) 
    {
        case NGHTTP2_HEADERS:
        {
            if (frame->headers.cat != NGHTTP2_HCAT_REQUEST) 
            {
                break;
            }

            http2_server_stream* stream_data = get_stream_by_id(frame->hd.stream_id);
            if (!stream_data) 
            {
                break;
            }

            return stream_data->on_header_callback(frame, name, namelen, value, valuelen,flags);

            break;
        }
    }
    return 0;
}

// called when stream is first started
int http2_server_session::on_begin_headers_callback(const nghttp2_frame *frame) 
{
    if ( frame->hd.type != NGHTTP2_HEADERS || frame->headers.cat != NGHTTP2_HCAT_REQUEST) 
    {
        return 0;
    }

    auto s = std::make_shared<http2_server_stream>(frame->hd.stream_id,con_);
    streams_.insert(s);
    nghttp2_session_set_stream_user_data(session_, frame->hd.stream_id,s.get());

    return 0;
}

// an http2 frame has been received. check if request is complete.
int http2_server_session::on_frame_recv_callback(const nghttp2_frame *frame) 
{
    switch (frame->hd.type) 
    {
        case NGHTTP2_DATA:
        case NGHTTP2_HEADERS:
        {        
            // look for stream EOF
            if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) 
            {
                http2_server_stream* stream_data = get_stream_by_id(frame->hd.stream_id);
                if (!stream_data) 
                {
                    return 0;
                }
                return on_request_recv(stream_data);
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return 0;
}

// http2 request has been completely received
// prepare and call the user supplied http handler.
int http2_server_session::on_request_recv(http2_stream* stream) 
{
    HttpRequest& req = (HttpRequest&)(stream->req);

    // set request body
    req.body(stream->oss.str());

    // set stream id for late use when responding
    stream->res.attributes.set(":http2:stream:id",stream->stream_id);

    // invoke the http handler
    con_->resolve(stream->req,stream->res);

    return 0;
}


http2_server_stream* http2_server_session::flush(Response& res)
{        
    int stream_id = res.attributes.attr<int>(":http2:stream:id");    

    http2_server_stream* stream = get_stream_by_id(stream_id);

    std::vector<nghttp2_nv> hdrs;
    
    // status code pseudo header as string
    std::ostringstream oss;
    oss << res.statusCode();
    std::string c = oss.str();

    hdrs.push_back(
        nghttp2_nv{
            (uint8_t*)":status",
            (uint8_t*)(c.c_str()),
            7,
            c.size(), 
            NGHTTP2_NV_FLAG_NONE
        }
    );
    
    // create headers for cookies
    if ( !res.headers.cookies().empty())
    {
        std::string key = "Set-Cookie";
        const std::vector<Cookie>& cookies = res.headers.cookies().all();
        for ( size_t i = 0; i < cookies.size(); i++ )
        {
            std::ostringstream oss;
            oss << cookies[i].str();
            res.headers.set(key,oss.str());
        }
    }
     
    // all custom headers
    auto& headers = res.headers.raw();
    for ( auto& h : headers)
    {        
        hdrs.push_back(
            nghttp2_nv{
                (uint8_t*)(h.first.c_str()),
                (uint8_t*)(h.second.c_str()),
                h.first.size(),
                h.second.size(),
                NGHTTP2_NV_FLAG_NONE
            }
        );    
    }
    
    if(send_response(
        stream_id, 
        &hdrs[0], 
        hdrs.size()
    ) == -1)
    {
        return 0;
    }  

    return stream;
}

http2_server_stream* http2_server_session::get_stream_by_id(int id)
{
    return (http2_server_stream*) ::prio::get_stream_by_id(session_,id);
} 



// static c-style trampoline callbacks

static int on_frame_recv_callback(nghttp2_session *session,const nghttp2_frame *frame, void *user_data) 
{
    http2_session* s = (http2_session *)user_data;
    return s->on_frame_recv_callback(frame);
}

static int on_header_callback(
    nghttp2_session *session,
    const nghttp2_frame *frame, 
    const uint8_t *name,
    size_t namelen, 
    const uint8_t *value,
    size_t valuelen, 
    uint8_t flags, 
    void *user_data) 
{
    http2_session* s = (http2_session*)user_data;
    return s->on_header_callback(frame,name,namelen,value,valuelen,flags);
}

static int on_begin_headers_callback(nghttp2_session *session,const nghttp2_frame *frame,void *user_data) 
{
    http2_session* s = (http2_session *)user_data;
    return s->on_begin_headers_callback(frame);
}

static int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,uint32_t error_code, void *user_data) 
{
    http2_session* s = (http2_session *)user_data;
    return s->on_stream_close_callback(stream_id,error_code);
}

ssize_t data_provider_callback(
    nghttp2_session *session, 
    int32_t stream_id,
    uint8_t *buf, 
    size_t length,
    uint32_t *data_flags,
    nghttp2_data_source *source,
    void *user_data) 
{
    http2_stream* stream = (http2_stream*)(source->ptr);
    return stream->data_provider_callback(buf,length,data_flags);
}

static int on_data_chunk_recv_callback(
    nghttp2_session *session,
    uint8_t flags, 
    int32_t stream_id,
    const uint8_t *data, 
    size_t len,
    void *user_data) 
{
    http2_stream* stream = get_stream_by_id(session,stream_id);
    return stream->data_chunk_recv_callback(flags,data,len);
}

// initialize nghttp2 library callbacks
void http2_session::initialize_nghttp2_session() 
{
    nghttp2_session_callbacks *callbacks;

    nghttp2_session_callbacks_new(&callbacks);

    nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, on_data_chunk_recv_callback);

    nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, ::prio::on_frame_recv_callback);

    nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, ::prio::on_stream_close_callback);

    nghttp2_session_callbacks_set_on_header_callback(callbacks, ::prio::on_header_callback);

    nghttp2_session_callbacks_set_on_begin_headers_callback( callbacks, ::prio::on_begin_headers_callback);

    initialize(callbacks);

    nghttp2_session_callbacks_del(callbacks);
}

} // end namespaces
