#ifndef _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_WEB_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_REPROWEB_SERIALIZER_WEB_DEF_GUARD_


#include <reproweb/serialization/parameter.h>


namespace reproweb {

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

inline void fromParams(const std::string& from, int& to)
{
	std::istringstream iss(from);
	iss >> to;
}

inline void fromParams(const std::string& from, long& to)
{
	std::istringstream iss(from);
	iss >> to;
}

inline void fromParams(const std::string& from, long long& to)
{
	std::istringstream iss(from);
	iss >> to;
}

inline void fromParams(const std::string& from, float& to)
{
	std::istringstream iss(from);
	iss >> to;
}

inline void fromParams(const std::string& from, double& to)
{
	std::istringstream iss(from);
	iss >> to;
}


inline void fromParams(const std::string& from, bool& to)
{
	std::istringstream iss(from);
	iss >> to;
}


inline void fromParams(const std::string& from, std::string& to)
{
	to = from;
}

inline void fromParams(const std::string& from, prio::HeaderValues& to)
{

}

inline void fromParams(const std::string& from, prio::Cookie& to)
{

}

inline void fromParams( const prio::Cookie& from, prio::Cookie& to )
{
	to = from;
}

inline void fromParams( const prio::Cookie& from, int& to )
{
	to = std::stoi(from.value());	
}

inline void fromParams( const prio::Cookie& from, long& to )
{
	to = std::stol(from.value());	
}

inline void fromParams( const prio::Cookie& from, long long& to )
{
	to = std::stoll(from.value());	
}

inline void fromParams( const prio::Cookie& from, float& to )
{
	std::istringstream iss(from.value());
	iss >> to;
}

inline void fromParams( const prio::Cookie& from, double& to )
{
	std::istringstream iss(from.value());
	iss >> to;
}

inline void fromParams( const prio::Cookie& from, bool& to )
{
	std::istringstream iss(from.value());
	iss >> to;
}

inline void fromParams( const prio::Cookie& from, std::string& to )
{
	to = from.value();
}



inline void fromParams( const prio::Cookie& from, prio::HeaderValues& to )
{
	// no op
}


inline void fromParams( const prio::HeaderValues& from, std::string& to )
{
	to = from.value().main();
}


inline void fromParams( const prio::HeaderValues& from, prio::HeaderValues& to )
{
	to = from;
}


inline void fromParams( const prio::HeaderValues& from, int& to )
{
	to = std::stoi(from.value().main());
}

inline void fromParams( const prio::HeaderValues& from, long& to )
{
	to = std::stol(from.value().main());
}

inline void fromParams( const prio::HeaderValues& from, long long& to )
{
	to = std::stoll(from.value().main());
}

inline void fromParams( const prio::HeaderValues& from, float& to )
{
	std::istringstream iss(from.value().main());
	iss >> to;
}

inline void fromParams( const prio::HeaderValues& from, double& to )
{
	std::istringstream iss(from.value().main());
	iss >> to;
}

inline void fromParams( const prio::HeaderValues& from, bool& to )
{
	std::istringstream iss(from.value().main());
	iss >> to;
}

inline void fromParams( const prio::HeaderValues& from, prio::Cookie& to )
{
	// no op
}


template<class T>
void fromParams( prio::QueryParams& qp, T& t);

template<class T>
void fromParams( const std::string& val, std::vector<T>& to);

template<class T>
void fromParams( prio::QueryParams& qp, T& t)
{
	meta::visit(t, [&qp](auto n, auto m) 
	{	
		std::remove_reference_t<typename decltype(m)::setter_value_t> value;
		std::string val = qp.get(n.name);
		std::cout << "val: " << val << std::endl;
		//std::cout << typeid(value).name() << std::endl;
		fromParams(val,value);
		m = value;
	});
}

template<class T>
void fromParams( const std::string& val, std::vector<T>& to)
{
	to.clear();
	auto v = prio::split(val,",");
	for ( auto& i : v)
	{
		T t;
		fromParams(i,t);
		to.push_back(std::move(t));
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

template<class T>
void fromRequest( prio::Request& req, T& t)
{
	meta::visit(t, [&req](auto n, auto m) 
	{	
		std::remove_reference_t<typename decltype(m)::setter_value_t> value;

		// path param

		prio::Args args = req.path.args();
		if ( args.exists(n.name) )
		{
			reproweb::fromParams( args.get(n.name), value );
			m = value;
			return;
		}

		// query param

		prio::QueryParams qp = req.path.queryParams();
		if (qp.exists(n.name))
		{
			reproweb::fromParams( qp.get(n.name), value );
			m = value;
			return;
		}

		// cookie value

		const prio::Cookies& c = req.headers.cookies();

		if(c.exists(n.name))
		{
			reproweb::fromParams( c.get(n.name), value );
			m = value;
			return;
		}

		// header

		if(req.headers.exists(n.name))
		{
			reproweb::fromParams( req.headers.values(n.name), value );
			m = value;
			return;
		}	

	});
}

//////////////////////////////////////////////////////////////

template<class T>
class HandlerParam<Parameter<T>>
{
public:

	static Parameter<T> get(prio::Request& req,  prio::Response& res)
	{
		Parameter<T> t;
		fromRequest(req,t.value);
		validate(t.value);

		return t;
	}
};

//////////////////////////////////////////////////////////////

template<class T>
class HandlerParam<Form<T>>
{
public:

	static Form<T> get(prio::Request& req,  prio::Response& res)
	{
		prio::QueryParams qp(req.body());

		Form<T> t;
		fromParams(qp,t.value);
		validate(t.value);

		return t;
	}
};



//////////////////////////////////////////////////////////////

template<>
class HandlerParam<prio::Request&>
{
public:

	static prio::Request& get(prio::Request& req,  prio::Response& res)
	{
		return req;
	}
};

//////////////////////////////////////////////////////////////

template<>
class HandlerParam<prio::Response&>
{
public:

	static prio::Response& get(prio::Request& req,  prio::Response& res)
	{
		return res;
	}
};

//////////////////////////////////////////////////////////////

template<>
class HandlerParam<prio::Cookies>
{
public:

	static prio::Cookies get(prio::Request& req,  prio::Response& res)
	{
		return req.headers.cookies();
	}
};

//////////////////////////////////////////////////////////////

template<>
class HandlerParam<prio::MultiParts>
{
public:

	static prio::MultiParts get(prio::Request& req,  prio::Response& res)
	{
		std::string delim = req.headers.values("Content-Type").value().params()["boundary"];

		// unquote
		if(delim.size() > 1 && delim[0] == '"'  && delim[delim.size()-1] == '"' ) delim = delim.substr(1,delim.size()-2);
		if(delim.size() > 1 && delim[0] == '\'' && delim[delim.size()-1] == '\'') delim = delim.substr(1,delim.size()-2);

		return prio::MultiParts(req.body(),delim);
	}
};
	

//////////////////////////////////////////////////////////////

template<>
class HandlerParam<prio::QueryParams>
{
public:

	static prio::QueryParams get(prio::Request& req,  prio::Response& res)
	{
		return req.path.queryParams();
	}
};
				
//////////////////////////////////////////////////////////////

class FormParams : public prio::QueryParams
{
public:
	FormParams() {};
	FormParams(const std::string& s) : prio::QueryParams(s) {}
};


template<>
class HandlerParam<FormParams>
{
public:

	static FormParams get(prio::Request& req,  prio::Response& res)
	{
		FormParams fp(req.body());
		return fp;
	}
};




///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

}

#endif

