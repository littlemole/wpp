#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_API_URL_HANDLER_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_REQUEST_API_URL_HANDLER_DEF_GUARD_

/**
 * \file url.h
 */

#include "priocpp/common.h"


namespace prio      {


//////////////////////////////////////////////////////////////
//! simple Url class
//////////////////////////////////////////////////////////////

class Url
{
public:
  Url ();
  Url (const std::string& url );

  bool set( const std::string& url );

  const std::string& getHost() const  { return host_;  }
  const std::string& getProto() const { return proto_; }
  const std::string& getPath() const  { return path_;  }
  const std::string& getUser() const  { return user_;  }
  const std::string& getPwd() const   { return pwd_;   }
  const std::string  getFullPath() const;
  const std::string  getParentDir() const;
  int getPort()	const			      { return port_;  }
  const std::string getQuery() const ;
  const std::string& getAnchor() const { return anchor_;  }

  void setHost     ( const std::string& host  );
  void setProto    ( const std::string& proto );
  void setPath     ( const std::string& path  );
  void setFullPath (const std::string& p );
  void setPort     ( int p );
  void setQuery    ( const std::string& query );
  void setAnchor   ( const std::string& anchor);

  void setUser (const std::string& user );
  void setPwd (const std::string& p );

  void addParam  ( const std::string& key, const std::string& val );
  std::string getParam(const std::string& key);
  std::vector<std::string> getParams(const std::string& key);
  std::string toString();

private:
  std::string   host_;
  int           port_;
  std::string   proto_;
  std::string   path_;
  std::string   anchor_;
  std::string   user_;
  std::string   pwd_;

  typedef std::pair<std::string,std::string> KeyVal;

  std::vector<KeyVal> queryParams_;
};

} // close namespaces

#endif

