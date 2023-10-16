#ifndef INCLUDE_REPROWEB_I18N_TPL_H_
#define INCLUDE_REPROWEB_I18N_TPL_H_

//! \file i18n.h

#include <regex>
#include "priohttp/common.h"
#include <diycpp/ctx.h>


namespace reproweb {

//! I18n support

class I18N 
{
public:

	//! construct I18n processor with path to property directory and vector of supported locales
	I18N(const std::string& base, const std::vector<std::string>& locales);

	//! find locale using fallbacks
	std::string find_locale(std::string locale);

	//! get the value for a given key
	const std::string& key(std::string locale, const std::string& k);

	//! resolve all keys in txt and return resolved 
	std::string render(std::string locale, const std::string& txt);

private:

	std::map<std::string,std::map<std::string,std::string>> map_;

	const std::string& get_key(std::string locale, const std::string& k);
	void parse(const std::string& locale,const std::string& content);
	void load(const std::string& locale,const std::string& path);
	void load(const std::string& base, const std::vector<std::string>& locales);
};

//! declare i18n properties in WebApplicationContext
class i18n_props
{
public:

	// initialize i18n properties from files at given path with list of supported locales
    i18n_props(const std::string& path,const std::vector<std::string>& locales)
		: path_(path), locales_(locales)
	{}

	//! \private
    void ctx_register(diy::Context* ctx)
	{
		auto i18n = std::make_shared<I18N>(path_,locales_);
		ctx->register_static<I18N>(i18n);
	}

private:

    std::string path_;
    std::vector<std::string> locales_;
};

}


#endif

