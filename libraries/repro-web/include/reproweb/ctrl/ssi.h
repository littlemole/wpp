#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_SSI_REQUEST_HANDLER_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_SSI_REQUEST_HANDLER_DEF_GUARD_

//! \file ssi.h
//! \defgroup view

#include "reproweb/ctrl/front_controller.h"

//////////////////////////////////////////////////////////////

namespace reproweb  {


//! SSI resolver
//! \ingroup view
class SSIResolver : public std::enable_shared_from_this<SSIResolver>
{
public:

	//! asynchronously resolve given SSI template and return Document with includes resolved
	static 	repro::Future<std::string> resolve( prio::Request& req, const std::string& tmpl);

	//! lookup static SSI include from request path
    static std::string tmpl(prio::Request& req, const std::string& htdocs);

private:

	std::shared_ptr<SSIResolver> self_;
	int cnt_ = 0;

	std::vector<std::string> includes_;
	std::vector<std::string> parts_;
	std::vector<std::string> resolves_;

	repro::Future<std::string> fetch( prio::Request& req, const std::string& tmpl);
	std::string combine();
	bool match(const std::string& tmpl);
};

//! declare static content handler in WebApplicationContext
//! \ingroup view
class ssi_content
{
public:

	//! \private
	typedef ssi_content type;

	//! \private
    ssi_content(const std::string& htdocs_path,const std::string& filter);

	//! \private
    void ctx_register(diy::Context* ctx)
	{
        register_static_handler(ctx);
	}

private:

    std::string htdocs_;
    std::string filter_;

    void register_static_handler(diy::Context* ctx);

};



} // end namespace mol

#endif

