#ifndef _MOL_DEF_GUARD_REPROWEB_TRAITS_DEF_GUARD_
#define _MOL_DEF_GUARD_REPROWEB_TRAITS_DEF_GUARD_

#include <functional>
#include <set>
#include <type_traits>
#include <sstream>
#include <metacpp/traits.h>

#ifndef _WIN32
#include <experimental/type_traits>
#endif



namespace diy {

    class Context;
}

namespace reproweb {

    template<class T>
    using has_meta_t = decltype(std::declval<T>().meta());

    template<class T>
    using has_meta = std::experimental::is_detected<has_meta_t, T>;

    template<class T>
    using has_valid_t = decltype(std::declval<T>().validate());

    template<class T>
    using has_valid = std::experimental::is_detected<has_valid_t, T>;

    template<class T>
    using has_http_root_t = decltype(T::http_root());

    template<class T>
    using has_http_root = std::experimental::is_detected<has_http_root_t, T>;

    class call_valid
    {
    public:

        template <class T, typename std::enable_if<has_valid<T>::value>::type* = nullptr >
        static void invoke( T& t) 
        {
            t.validate();
        }

        template <class T , typename  std::enable_if<!has_valid<T>::value>::type* = nullptr >
        static void invoke( T& t) 
        {}
    };

    class call_http_root
    {
    public:

        template <class T, typename std::enable_if<has_http_root<T>::value>::type* = nullptr >
        static std::string invoke() 
        {
            return T::http_root();
        }

        template <class T , typename  std::enable_if<!has_http_root<T>::value>::type* = nullptr >
        static std::string invoke() 
        {
            return "";
        }
    };    

    template<class T>
    std::string http_root()
    {
        return call_http_root::invoke<T>();
    }


//////////////////////////////////////////////////////////////

typedef ::repro::Future<> Async;


inline std::string csv_quote(const std::string& in)
{
	std::ostringstream oss;
    oss << '"';

	for(char c : in)
	{
		if(c=='"')
		{
			oss << '"';
		}
        if(c=='\t' || c=='\r' || c=='\n' || c=='\\')
        {
            oss << '\\';
        }
		oss << c;
	}
    oss << '"';

	return oss.str();
}

}


#endif

