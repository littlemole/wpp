#ifndef DEFINE_MOL_HTTP2_CLIENT_REQUEST_CLIENT_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP2_CLIENT_REQUEST_CLIENT_DEF_GUARD_DEFINE_

#include "priohttp/conversation.h"
#include "priohttp/response.h"
#include "priohttp/http2.h"
#include <set>
 
namespace prio  {

class Connection;    
class http2_client_session;


struct http2_client_stream : public http2_stream 
{
    http2_client_stream(Request& req,Conversation* con);
    ~http2_client_stream();

    int on_header_callback(
        const nghttp2_frame *frame, 
        const uint8_t *name,
        size_t namelen, 
        const uint8_t *value,
        size_t valuelen, 
        uint8_t flags);

    int data_chunk_recv_callback(
        uint8_t flags, 
        const uint8_t *data, 
        size_t len);

    ssize_t data_provider_callback(
        uint8_t *buf, 
        size_t length,
        uint32_t *data_flags);
};



class http2_client_session : public  http2_session
{
public:

    http2_client_session(Conversation* c) ;
    ~http2_client_session() ;

    http2_client_stream* flush(Request& res);
    
    int on_stream_close_callback(int32_t stream_id,uint32_t error_code);
    int on_frame_recv_callback(const nghttp2_frame *frame);
    int on_begin_headers_callback(const nghttp2_frame *frame);
    int on_request_recv(http2_stream *stream_data);

    int on_header_callback(
        const nghttp2_frame *frame, 
        const uint8_t *name,
        size_t namelen, 
        const uint8_t *value,
        size_t valuelen, 
        uint8_t flags
    ); 

    http2_client_stream* get_stream_by_id(int id);
    virtual void initialize(nghttp2_session_callbacks * cb);

};    


}

#endif
