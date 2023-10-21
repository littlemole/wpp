#include <event2/event.h>
#include "priohttp/http2_conversation.h"
#include "priocpp/api.h"
#include "priohttp/server/reader.h"
#include "priohttp/server/writer.h"
#include "priohttp/http2_client.h"
#include <set>
#include <sstream>
#include <deque>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

namespace prio  {

http2_client_stream::http2_client_stream(Request& request,Conversation* con)
    : http2_stream(request,con)
{}

http2_client_stream::~http2_client_stream()
{}

int http2_client_stream::on_header_callback(
    const nghttp2_frame* /*frame*/, 
    const uint8_t *name,
    size_t namelen, 
    const uint8_t *value,
    size_t /*valuelen*/, 
    uint8_t /*flags*/ ) 
{
    static const char* pseudo_headers[] = { ":status" };
    static std::function<void(HttpResponse& r,char* c)> pseudo_handlers[] = { 
        [](HttpResponse& res,char* c)
        {
            std::ostringstream oss;
            oss << "HTTP/2 " << c; 
            res.status(oss.str()); 
        } 
    };
    
    HttpResponse& response = (HttpResponse&)(res);

    // check for http2 pseudo headers
    for ( size_t i = 0; i < 4; i++)
    {
        size_t len = strlen(pseudo_headers[i]);
        if ( namelen != len ) 
            continue;
        if ( memcmp(pseudo_headers[i],name,namelen)!=0 ) 
            continue;
           
        pseudo_handlers[i](response,(char*)value);
        return 0;
    }

    // plain old header
    response.header((char*)name,(char*)value);

    //std::cout << (char*)name << " " << (char*)value << std::endl;
    
    return 0;
}

int http2_client_stream::data_chunk_recv_callback(
    uint8_t /*flags*/, 
    const uint8_t *data, 
    size_t len) 
{
    if(len==0)
        return 0;
       
    oss.write((char*)data,len);
    return 0;
}    

ssize_t http2_client_stream::data_provider_callback(
    uint8_t *buf, 
    size_t length,
    uint32_t *data_flags) 
{    
    size_t want = req.body().size() - written;
    if(want > length) want = length;

    memcpy( buf, req.body().data() + written, want );

    written += want;

    if ( written >= req.body().size())
    {                    
        *data_flags |= NGHTTP2_DATA_FLAG_EOF;
    }
    return want;
}    

/////////////////////////////////////////////////////

http2_client_session::http2_client_session(Conversation* c) 
    : http2_session(c)
{}
  
http2_client_session::~http2_client_session() 
{
}

// http2 header received callback
int http2_client_session::on_header_callback(
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
            if (frame->headers.cat != NGHTTP2_HCAT_RESPONSE) 
            {
                break;
            }

            http2_client_stream* stream_data = get_stream_by_id(frame->hd.stream_id);
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
int http2_client_session::on_begin_headers_callback(const nghttp2_frame* /*frame*/ ) 
{
    return 0;
}

// an http2 frame has been received. check if request is complete.
int http2_client_session::on_frame_recv_callback(const nghttp2_frame *frame) 
{
    switch (frame->hd.type) 
    {
        case NGHTTP2_DATA:
        case NGHTTP2_HEADERS:
        {        
            // look for stream EOF
            if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) 
            {
                http2_client_stream* stream_data = get_stream_by_id(frame->hd.stream_id);
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
int http2_client_session::on_request_recv(http2_stream* stream) 
{    
    HttpResponse& res = (HttpResponse&)(stream->res);

    // set response body
    res.body(stream->oss.str());

    // invoke the http handler
    con_->resolve(stream->req,stream->res);

    return 0;
}


ssize_t data_provider_callback(
    nghttp2_session *session, 
    int32_t stream_id,
    uint8_t *buf, 
    size_t length,
    uint32_t *data_flags,
    nghttp2_data_source *source,
    void *user_data);


http2_client_stream* http2_client_session::flush(Request& req)
{            
    auto stream = std::make_shared<http2_client_stream>(req,con_);

    streams_.insert(stream);

    std::vector<nghttp2_nv> hdrs;
    
    // assign mandatory http2 headers
    hdrs.push_back(
        nghttp2_nv{
            (uint8_t*)":method",
            (uint8_t*)(req.path.method().c_str()),
            7,
            req.path.method().size(), 
            NGHTTP2_NV_FLAG_NONE
        }
    );

    hdrs.push_back(
        nghttp2_nv{
            (uint8_t*)":scheme",
            (uint8_t*)"https",
            7,
            5, 
            NGHTTP2_NV_FLAG_NONE
        }
    );

    std::string host = req.headers.get("host");
    hdrs.push_back(
        nghttp2_nv{
            (uint8_t*)":authority",
            (uint8_t*)(host.c_str()),
            10,
            host.size(), 
            NGHTTP2_NV_FLAG_NONE
        }
    );    

    std::string path = req.path.url();
    hdrs.push_back(
        nghttp2_nv{
            (uint8_t*)":path",
            (uint8_t*)(path.c_str()),
            5,
            path.size(), 
            NGHTTP2_NV_FLAG_NONE
        }
    );       
    
    // create headers for cookies
    if ( !req.headers.cookies().empty())
    {
        const std::vector<Cookie>& cookies = req.headers.cookies().all();
        std::ostringstream oss;
        for ( size_t i = 0; i < cookies.size(); i++ )
        {
            oss << cookies[i].str();
            if ( i < cookies.size()-1)
            {
                oss << " ";
            }
        }
        req.headers.set("Cookie",oss.str());
    }
    
    // all custom headers
    auto& headers = req.headers.raw();
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

    // prepare post data if any
    nghttp2_data_provider* pdata = NULL;

    nghttp2_data_provider data_prd;
    data_prd.source.ptr = stream.get();
    data_prd.read_callback = data_provider_callback;

    if ( strcasecmp(req.path.method().c_str(),"POST") == 0 ||
         strcasecmp(req.path.method().c_str(),"PUT") == 0)
    {
        pdata = &data_prd;
    }
    
    // submit request to nghttp2
    int stream_id = nghttp2_submit_request(session_, NULL, &hdrs[0], hdrs.size(), pdata, stream.get());
    if (stream_id < 0) 
    {
       return 0;
    }

    // have valid stream, associate it for later 
    stream->stream_id = stream_id;
    nghttp2_session_set_stream_user_data(session_, stream_id,stream.get());    
    
    return stream.get();
}

http2_client_stream* http2_client_session::get_stream_by_id(int id)
{
    return (http2_client_stream*)::prio::get_stream_by_id(session_,id);
} 

void http2_client_session::initialize(nghttp2_session_callbacks *callbacks) 
{
    nghttp2_session_client_new(&session_, callbacks, this);
}


} // end namespaces
