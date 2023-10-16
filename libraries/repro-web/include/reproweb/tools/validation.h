#ifndef INCLUDE_REPROWEB_VALIDATION_TPL_H_
#define INCLUDE_REPROWEB_VALIDATION_TPL_H_

//! \file valid.h

#include "priohttp/request.h"

namespace reproweb {



class ValidationEx : public repro::ReproEx<ValidationEx> 
{
public:
	ValidationEx() {}
	ValidationEx(const std::string& s) : repro::ReproEx<ValidationEx> (s) {}
};



//! generic validator
template<class E>
class validator
{
public:

	//! construct validator woth given regex
	validator(const std::regex & r)
		: r_(r)
	{}

	//! get string from tainted input string, throwing msg if not matching the regex
	std::string getString(const std::string& tainted, const std::string& msg)
	{
		std::smatch match;
		 if ( !std::regex_match(tainted,match,r_) )
		 {
			 throw E(msg);
		 }

		 return match[0];
	}

	//! get int from tainted input string, throwing msg if not matching the regex
	int getInteger(const std::string& tainted, const std::string& msg)
	{
		std::smatch match;
		if ( !std::regex_match(tainted,match,r_) )
		{
			throw E(msg);
		}

		std::istringstream iss(match[0]);
		int i;
		iss >> i;		 
		return i;
	}

	//! get double from tainted input string, throwing msg if not matching the regex
	double getDouble(const std::string& tainted, const std::string& msg)
	{
		std::smatch match;
		if ( !std::regex_match(tainted,match,r_) )
		{
			throw E(msg);
		}

		std::istringstream iss(match[0]);
		double d;
		iss >> d;		 
		return d;
	}


private:
	std::regex r_;	
};

//! return s if s matches regex, other throw Exception E with given error message
template<class E>
inline std::string valid(const std::string& s, const std::regex& r, const std::string& msg)
{
	validator<E> v(r);
	return v.getString(s,msg);
}

//! return s if s matches regex, other throw Exception E with default error message
template<class E>
inline std::string valid(const std::string& s, const std::regex& r)
{
	static const std::string msg("string validation failed!");
	return valid<E>(s,r,msg);
}

//! return s if s matches regex, other throw ValidationException with default error message
inline std::string valid(const std::string& s, const std::regex& r)
{
	return valid<ValidationEx>(s,r);
}

//! return s if s matches regex, other throw ValidationException with given error message
inline std::string valid(const std::string& s, const std::regex& r, const std::string& msg)
{
	return valid<ValidationEx>(s,r,msg);
}

//! return vector if all strings in vector match given regex, other throw Exception E with default error message
template<class E>
std::vector<std::string>& valid( std::vector<std::string>& v, const std::regex& r, const std::string& msg)
{
	for ( auto& i : v)
	{
		valid<E>(i,r,msg);
	}
	return v;
}

//! return vector if all strings in vector match given regex, other throw ValidationException with default error message
inline std::vector<std::string>& valid( std::vector<std::string>& v, const std::regex& r, const std::string& msg)
{
	for ( auto& i : v)
	{
		valid<ValidationEx>(i,r,msg);
	}
	return v;
}

//! return int if input s matches given regex, other throw Exception E with given error message
template<class E>
inline int valid_int(const std::string& s,const std::string& msg)
{
	validator<E> v(std::regex("[0-9]*"));
	return v.getInteger(s,msg);
}


//! return int if input s matches given regex, other throw Exception E with default error message
template<class E>
inline int valid_int(const std::string& s)
{
	static const std::string msg("integer validation failed!");
	return valid_int<E>(s,msg);
}

//! return int if input s matches given regex, other throw ValidationException with default error message
inline int valid_int(const std::string& s)
{
	return valid_int<ValidationEx>(s);
}

//! return int if input s matches given regex, other throw ValidationException with given error message
inline int valid_int(const std::string& s,const std::string& msg)
{
	return valid_int<ValidationEx>(s,msg);
}

//! return double if input s matches given regex, other throw Exception E with given error message

template<class E>
inline double valid_double(const std::string& s,const std::string& msg)
{
	validator<E> v(std::regex("[0-9\\.]*"));
	return v.getDouble(s,msg);
}


//! return double if input s matches given regex, other throw Exception E with default error message
template<class E>
inline double valid_double(const std::string& s)
{
	static const std::string msg("double validation failed!");
	return valid_double<E>(s,msg);
}

//! return double if input s matches given regex, other throw ValidationException with given error message
inline double valid_double(const std::string& s,const std::string& msg)
{
	return valid_double<ValidationEx>(s,msg);
}


//! return double if input s matches given regex, other throw ValidationException with default error message
inline double valid_double(const std::string& s)
{
	return valid_double<ValidationEx>(s);
}

//! return s if input s matches given regex for email addresses, other throw Exception E with given error message

template<class E>
std::string valid_email(const std::string& email, const std::string& msg)
{
	validator<E> v( std::regex("^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\\.[a-zA-Z0-9-]+)*$"));
	return v.getString(email,msg);
}

//! return s if input s matches given regex for email addresses, other throw Exception E with default error message
template<class E>
std::string valid_email(const std::string& email)
{
	return valid_email<E>(email,"invalid email");
}

//! return s if input s matches given regex for email addresses, other throw ValidationException  with given error message
inline std::string valid_email(const std::string& email, const std::string& msg)
{
	return valid_email<ValidationEx>(email,msg);
}


//! return s if input s matches given regex for email addresses, other throw ValidationException  with default error message
inline std::string valid_email(const std::string& email)
{
	return valid_email<ValidationEx>(email);
}

//! return s if input s matches given regex for URLs, other throw Exception E  with given error message

template<class E>
std::string valid_url(const std::string& url, const std::string& msg)
{
	validator<E> v( std::regex("(http|https)://(\\w+:{0,1}\\w*@)?(\\S+)(:[0-9]+)?(/|/([\\w#!:.?+=&%@!-/]))?"));
	return v.getString(url,msg);
}

//! return s if input s matches given regex for URLs, other throw Exception E  with default error message
template<class E>
std::string valid_url(const std::string& url)
{
	return valid_email<E>(url,"invalid url");
}

//! return s if input s matches given regex for URLs, other throw ValidationException  with given error message

inline std::string valid_url(const std::string& url, const std::string& msg)
{
	return valid_email<ValidationEx>(url,msg);
}

//! return s if input s matches given regex for URLs, other throw ValidationException with default error message

inline std::string valid_url(const std::string& url)
{
	return valid_email<ValidationEx>(url);
}

} // end namespace


#endif
