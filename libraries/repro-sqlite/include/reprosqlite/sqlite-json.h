#ifndef _MOL_DEF_GUARD_DEFINE_MOD_HTTP_SQLITE_JSON_DEF_GUARD_
#define _MOL_DEF_GUARD_DEFINE_MOD_HTTP_SQLITE_JSON_DEF_GUARD_

#include "reprosqlite/sqlite.h"
#include <json/json.h>

//////////////////////////////////////////////////////////////


namespace reprosqlite {


inline Json::Value toJson(Result& r)
{
    Json::Value result(Json::arrayValue);

    for (unsigned int i = 0; i < r.rows(); i++)
    {
        Json::Value obj(Json::objectValue);

        for (unsigned int j = 0; j < r.cols(); j++)
        {
            std::string name = r.columns[j];
            obj[name] = r.row(i)[j];
        }
        result.append(obj);
    }
    return result;
}


} // end namespace sqlite

#endif

