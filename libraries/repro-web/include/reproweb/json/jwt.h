#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_CRYPT_JWT_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_CRYPT_JWT_DEF_GUARD_

//! \file jwt.h

#include "priohttp/common.h"
#include "priohttp/response.h"
#include "reproweb/json/json.h"
#include "cryptoneat/cryptoneat.h"

namespace reproweb  {

//! JWT Json Web Tokens
//! \ingroup json
class JWT
{
public:
    //! construct empty JWT
    JWT();
    //! construct JWT from plaintext
    JWT(const std::string& token);
    //! construct JWT using Json::Value as JWT claim
    JWT(const Json::Value& c);    
    //! construct JWT using given JSON header and claim objects
    JWT(const Json::Value& h,const Json::Value& c);
    ~JWT();

    //! parse plaintext into JWT object
    bool parse(const std::string& token);

    //! sign token given expriry date using secret
    std::string sign(const std::string& secret,size_t expires_s);
    //! verify given token using secret
    bool verify(const std::string& secret);

    //! sign token given expiry date and private key
    std::string sign(cryptoneat::EVP_PKEY* privkey,size_t expires_s);
    //! verify JWT using public key
    bool verify( cryptoneat::EVP_PKEY* pubkey);

    //! specify the JWT header
    JWT& header(const Json::Value& h);
    //! specify the JWT claim
    JWT& claim(const Json::Value& c);
    
    //! return the JWT header
    Json::Value header();
    //! return the JWT claim
    Json::Value claim();

    //! return the JWT signature
    std::string signature();

private:

    void expire(size_t expires_s);
    std::string payload();
    std::string token(const std::string& body);
    size_t inspect(const std::string& token);

    Json::Value header_;
    Json::Value claim_;
    std::string signature_;
    std::string payload_;
};

namespace http {

	class Request;
	class Response;
}

//! render JSON value as HTTP response
//! \ingroup json
void json_response(prio::Response& res, Json::Value json);

//! return a lambda that renders any exceptions as JSON error
//! \ingroup json
auto inline json_err(prio::Response& res)
{
    return  [&res] ( const std::exception& ex)
    {
        Json::Value r(Json::objectValue);
        r["error"] = ex.what();

        res
            .body(JSON::stringify(r))
            .contentType("application/json")
            .error()
            .flush();
    };
}

} // end namespace csgi

#endif

