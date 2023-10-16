#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_RESPONSE_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_RESPONSE_DEF_GUARD_

//! \file response.h

#include "priohttp/request.h"
#include "priohttp/header.h"
#include "priohttp/compress.h"
   
//////////////////////////////////////////////////////////////

namespace prio  {


class Conversation;
class HttpClientConversation;


//////////////////////////////////////////////////////////////

//! \brief HTTP Response
class Response
{
friend class HttpConversation;
public:

    //! HTTP response headers
    Headers headers;
    //! custom attributes
    Attributes attributes;

    //! \private
    Response(Conversation* rp);
        
    //! set HTTP content-type
    Response& contentType(const std::string& val);
    //! set arbitary HTTP header
    Response& header(const std::string& key, const std::string& val);
    //! set HTTP response body
    Response& body(const std::string& b);
    //! set HTTP cookie for response
    Response& cookie(const Cookie& c);    
        
    //! set HTTP status code as int, ie 200
    void statusCode(int);
    //! \private    
    void proto(const std::string& p);

    //! set HTTP status as string, ie "HTTP/1.1 200 OK"
    Response& status(const std::string& s);

    //! set HTTP status to OK=200
    Response& ok();
    //! set HTTP status to 500
    Response& error();
    //! set HTTP status to 400
    Response& bad_request();
    //! set HTTP unauthorized
    Response& unauthorized();
    //! set HTTP fobidden
    Response& forbidden();
    //! set HTTP not found=404
    Response& not_found();
    //! send an absolute HTTP redirect
    Response& redirect(const std::string& s, int code = 302);
    //! send a relative HTTP redirect using initial protocol and host domain name
    //! honors X-Forwarded-proto and X-Forwarded-Host a if present
    Response& redirect(Request& req, const std::string& s, int code = 302);

    //! enable gzip compression for response    
    Response& gzip();

    //! return HTTP status code
    const std::string& status() const noexcept;
    //! return HTTP protocol
    const std::string& proto() const noexcept;
    //! return HTTP status code as int
    int statusCode() const noexcept;
    //! return the HTTP response body
    const std::string& body() const noexcept;
    //! return size of HTTP response body
    size_t size() const noexcept;
    
    //! return the whole Response as string suitable for HTTP transmission
    std::string toString();

    //! enable chunked response and send first chunk
    //! repeat for multiple chunks
    void chunk(const std::string& ch);

    //! done with HTTP response construction, send it over the wire asynchronously
    repro::Future<> flush();

    //! check whether chunked response was enabled
    bool isChunked() { return isChunked_; }

    //! check whether gzip compression was enabled
    bool isGzipped() { return isGzipped_; }

protected:

    Response(const Response&) = delete;
    Response(Response&&) = delete;
    Response& operator=(const Response&) = delete;
    Response& operator=(Response&&) = delete;

    Conversation* http_;
    std::string status_;
    std::string proto_;
    int status_code_;
    std::string body_;
    size_t size_;
    bool headersSent_;
    bool isChunked_;
    bool isGzipped_;

};


//! \private
class HttpResponse : public Response
{
friend class HttpConversation;
public:

	HttpResponse(Conversation* rp);

	HttpResponse& reset();

    void size(size_t);

    void onCompletion(std::function<void(Request& req, Response& res)> f);
    void onFlushHeaders(std::function<repro::Future<>(Request& req, Response& res)> f);

    bool headersSent() { return headersSent_; }

    void isChunked(bool b) { isChunked_ = b; }

    void flushHeaders();

private:

    HttpResponse(const HttpResponse&) = delete;
    HttpResponse(HttpResponse&&) = delete;
    HttpResponse& operator=(const HttpResponse&) = delete;
    HttpResponse& operator=(HttpResponse&&) = delete;
};



} // close namespaces
#endif

