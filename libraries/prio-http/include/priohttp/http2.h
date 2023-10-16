#ifndef DEFINE_MOL_HTTP2_SERVER_REQUEST_CLIENT_PROCESSOR_DEF_GUARD_DEFINE_
#define DEFINE_MOL_HTTP2_SERVER_REQUEST_CLIENT_PROCESSOR_DEF_GUARD_DEFINE_

#include "priohttp/response.h"
#include <set>

#ifdef _WIN32
#include <inttypes.h>
typedef int ssize_t;
#endif

#include <nghttp2/nghttp2.h>


namespace prio  {

class Connection;    


struct http2_stream : public std::enable_shared_from_this<http2_stream>
{
    http2_stream(int32_t id,Conversation* con);
    http2_stream(Request& req,Conversation* con);    
    virtual ~http2_stream() {};

    virtual int on_header_callback(
        const nghttp2_frame *frame, 
        const uint8_t *name,
        size_t namelen, 
        const uint8_t *value,
        size_t valuelen, 
        uint8_t flags) = 0;

    virtual int data_chunk_recv_callback(
        uint8_t flags, 
        const uint8_t *data, 
        size_t len) = 0;

    virtual  ssize_t data_provider_callback(
        uint8_t *buf, 
        size_t length,
        uint32_t *data_flags) = 0;

    void reset();
    void reset_callbacks();

    Request req;
    Response res;    

    int32_t stream_id;         
    size_t written;            
    std::ostringstream oss;

	std::function<repro::Future<>(Request& req, Response& res)> flusheaders_func;
	std::function<void(Request& req, Response& res)> completion_func;    
};

struct http2_server_stream : public http2_stream
{
    http2_server_stream(int32_t id,Conversation* con);
    virtual ~http2_server_stream();

    virtual int on_header_callback(
        const nghttp2_frame *frame, 
        const uint8_t *name,
        size_t namelen, 
        const uint8_t *value,
        size_t valuelen, 
        uint8_t flags);

    virtual int data_chunk_recv_callback(
        uint8_t flags, 
        const uint8_t *data, 
        size_t len);

    virtual ssize_t data_provider_callback(
        uint8_t *buf, 
        size_t length,
        uint32_t *data_flags);
};


inline http2_stream* get_stream_by_id(nghttp2_session* session,int id)
{
    return (http2_stream*)nghttp2_session_get_stream_user_data(session, id);
} 

class http2_session
{
public:

    http2_session(Conversation* c);
    virtual ~http2_session();

    repro::Future<> send();
    repro::Future<> recv(const std::string& s);

    void initialize_nghttp2_session();
    int send_connection_header();
    int on_stream_close_callback(int32_t stream_id,uint32_t error_code);

    virtual void initialize(nghttp2_session_callbacks * cb) = 0;
    virtual int on_frame_recv_callback(const nghttp2_frame *frame) = 0;
    virtual int on_begin_headers_callback(const nghttp2_frame *frame) = 0;
    virtual int on_request_recv(http2_stream *stream_data) = 0;

    virtual int on_header_callback(
        const nghttp2_frame *frame, 
        const uint8_t *name,
        size_t namelen, 
        const uint8_t *value,
        size_t valuelen, 
        uint8_t flags
    ) = 0; 
    
protected:

    nghttp2_session* session_;
    std::set<std::shared_ptr<http2_stream>> streams_;
    Conversation* con_;
}; 

class http2_server_session : public http2_session
{
public:

    http2_server_session(Conversation* c);
    ~http2_server_session();

    int send_response(int32_t stream_id,nghttp2_nv *nva, size_t nvlen);

    http2_server_stream* flush(Response& res);

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

    http2_server_stream* get_stream_by_id(int id);

    virtual void initialize(nghttp2_session_callbacks * cb);
};    


}

#endif
