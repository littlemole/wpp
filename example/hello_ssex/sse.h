
#ifndef _MOL_DEF_GUARD_DEFINE_CHAT_PROMISE_EXAMPLE_SSE_DEF_
#define _MOL_DEF_GUARD_DEFINE_CHAT_PROMISE_EXAMPLE_SSE_DEF_

#include "model.h"
#include "repo.h"
#include "reproweb/ws/sse.h"
#include "priocpp/timeout.h"

using namespace reproweb;

class SSEController
{
public:

	// c'tor
	SSEController(
		std::shared_ptr<TplStore> tpls,
		std::shared_ptr<EventBus> bus
	)
	  : templates_(tpls),eventBus_(bus)
	{}

	// sse connects
    void onConnect(SSEConnection::Ptr sse)
    {
		// subscribe to topic and pass any Json downwards on invocation
		std::string view = templates_->get("msg");
    	std::string sid = eventBus_->subscribe("chat-topic", [sse,view] (Json::Value value)
    	{
			std::string content = mustache::render(view,value);

			auto v = split(content, "\n");

			std::ostringstream oss;
			for( auto& i : v)
			{
				oss << i;
			}
			oss << "\n\n";

			std::string body = oss.str();
			std::cout << body << std::endl;
    		sse->send(body);
    	});

		// make this sse remember the subscriber id
		set_subscriber_id(sse,sid);
    };

	// sse disconnects
    void onClose(SSEConnection::Ptr sse)
    {
		std::cout << "SSE CLOSE" << std::endl;
		// unsubscribe from topic using supscription id
    	eventBus_->unsubscribe("chat-topic", subscriber_id(sse));
    };


private:


	void set_subscriber_id(SSEConnection::Ptr sse, const std::string& id)
	{
		sse->req.attributes.set("subscription-id",id);
	}

	std::string subscriber_id(SSEConnection::Ptr sse)
	{
		return sse->req.attributes.attr<std::string>("subscription-id");
	}

	std::shared_ptr<TplStore> templates_;
    std::shared_ptr<EventBus> eventBus_;
};

#endif


