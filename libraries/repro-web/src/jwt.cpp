#include "reproweb/json/jwt.h"
#include "cryptoneat/cryptoneat.h"
#include "reproweb/json/json.h"
#include "cryptoneat/base64.h"
#include "priohttp/request.h"
#include "priohttp/response.h"

#include <sstream>

using namespace cryptoneat;

namespace reproweb  {



std::string EventBus::subscribe( const std::string& topic, std::function<void(Json::Value)> observer)
{
    std::string id = cryptoneat::nonce(32);
    prio::nextTick( [this,id,topic,observer] ()
    {
        subscriptions_[topic].insert(std::make_pair(id, subscription(observer)));
    });
    return id;
}

void EventBus::unsubscribe( const std::string& topic,  const std::string& id)
{
    prio::nextTick( [this,topic,id] ()
    {
        subscriptions_[topic].erase(id);
    });
}

void EventBus::notify(const std::string& topic, Json::Value value)
{
    for( auto fun : subscriptions_[topic])
    {
        prio::nextTick( [fun,value]()
        {
            fun.second.fun(value);
        });
    }
}

void EventBus::clear()
{
    subscriptions_.clear();
}


JWT::JWT()
{}

JWT::~JWT()
{}

JWT::JWT(const std::string& token)
{
	parse(token);
}


JWT::JWT(const Json::Value& c)
    : header_(Json::objectValue), claim_(c)
{
    header_["typ"] = "JWT";
}


JWT::JWT(const Json::Value& h,const Json::Value& c)
    : header_(h), claim_(c)
{}

JWT& JWT::header(const Json::Value& h)
{
    header_ = h;
    return *this;
}

JWT& JWT::claim(const Json::Value& c)
{
    claim_ = c;
    return *this;
}

std::string JWT::payload()
{
    std::ostringstream oss;
    oss << Base64Url::encode(JSON::stringify(header_));
    oss << ".";
    oss << Base64Url::encode(JSON::stringify(claim_));

    return oss.str();
}

void JWT::expire(size_t expires_s)
{
    unsigned int exp = (unsigned int)time(NULL);
    exp += expires_s;
    claim_["exp"] = exp;
}

std::string JWT::token(const std::string& body)
{
    std::ostringstream oss;
    oss << body << "." << signature();
    return oss.str();
}

std::string JWT::sign(const std::string& secret,size_t expires_s)
{
    header_["alg"] = "HS256";
	expire(expires_s);

	std::string body = payload();
    
    Hmac hmac("sha256",secret);
    signature_ = hmac.hash(body);
    
    return token(body);
}

std::string JWT::sign(EVP_PKEY* privkey,size_t expires_s)
{
    header_["alg"] = "RS256";
	expire(expires_s);

	std::string body = payload();

    cryptoneat::Signature signer(digest("sha256"),privkey);
    signature_ = signer.sign(body);

    return token(body);
}

size_t JWT::inspect(const std::string& token)
{
    std::vector<std::string> vjwt = prio::split(token,'.');
    if( vjwt.size() != 3) {
        return std::string::npos;
    }

    std::string jwt_header    = Base64Url::decode(vjwt[0]);
    std::string jwt_claim     = Base64Url::decode(vjwt[1]);
    signature_ 				  = Base64Url::decode(vjwt[2]);

    try {
        header_ = JSON::parse( jwt_header );
        claim_ = JSON::parse( jwt_claim );
    } catch( JSON::ParseEx& ex)
    {
        return std::string::npos;
    }

    unsigned int exp = claim_.get("exp",0).asInt();
    unsigned int now = (unsigned int)time(NULL);
    if ( now > exp )
    {
        return std::string::npos;
    }

    size_t pos = token.find_last_of(".");
    payload_ = token.substr(0,pos);
    return pos;
 }

bool JWT::parse(const std::string& token)
{
	size_t pos = inspect(token);
    if ( pos == std::string::npos )
    {
        return false;
    }
    return true;
}

bool JWT::verify(const std::string& secret)
{
    Hmac hmac("sha256",secret);
    std::string hash = hmac.hash(payload_);

    return hash == signature_;
}


bool JWT::verify( EVP_PKEY* pubkey)
{
    cryptoneat::Signature verifier(digest("sha256"),pubkey);

    return verifier.verify(payload_,signature_);
}

Json::Value JWT::header()
{
    return header_;
}

Json::Value JWT::claim()
{
    return claim_;
}

std::string JWT::signature()
{
    return Base64Url::encode(signature_);
}


void json_response(prio::Response& res, Json::Value json)
{
	res.body(JSON::stringify(json)).contentType("application/json").ok().flush();
}


} // end namespace csgi



