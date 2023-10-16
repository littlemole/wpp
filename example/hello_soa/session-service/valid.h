#ifndef _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VALID_CONTROLLER_DEFINE_
#define _DEF_GUARD_DEFINE_REPROWEB_HELLO_WORLD_VALID_CONTROLLER_DEFINE_

#include "reproweb/tools/validation.h"

using namespace reproweb;
using namespace repro;
using namespace prio;


class NoSessionEx : public repro::ReproEx<NoSessionEx> 
{
public:
	NoSessionEx() {}
	NoSessionEx(const std::string& s) : repro::ReproEx<NoSessionEx> (s) {}
};


class Valid
{
public:

	static const std::string session_id(const std::string& sid)
	{
		static std::regex r("repro_web_sid::[0-9a-f]*");

		return valid<NoSessionEx>( 
			sid, 
			r,
			"invalid session id."
		);
	}   
};


#endif