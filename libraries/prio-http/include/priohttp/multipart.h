
#ifndef INCLUDE_PROMISE_HTTP_MULTIPART_H_
#define INCLUDE_PROMISE_HTTP_MULTIPART_H_

#include "priohttp/request.h"

namespace prio  {


class MultiPart
{
public:

	MultiPart(const std::string& h, const std::string& b)
		: body(b)
	{
		std::istringstream iss(h);
		headers.parse(iss);
	}

	Headers headers;
	std::string body;

	HeaderValues disposition()
	{
		return headers.values("Content-Disposition");
	}
};

class MultiParts
{
public:

	MultiParts( const std::string& s, const std::string& b)
	{
		std::string delim = "--";
		delim.append(b);

		std::vector<std::string> ps = split(s,delim);

		for( auto& part : ps)
		{
			if( part.size() < 5 )
				continue;

			std::string tmp = part.substr(2,part.size()-4);

			if(trim(tmp).empty())
				continue;

			size_t pos = tmp.find("\r\n\r\n");
			if ( pos == std::string::npos )
			{
				continue;
			}

			parts.push_back( MultiPart(tmp.substr(0,pos),tmp.substr(pos+4)) );
		}
	}

	std::vector<MultiPart> parts;

};

}


#endif /* INCLUDE_PROMISE_HTTP_MULTIPART_H_ */
