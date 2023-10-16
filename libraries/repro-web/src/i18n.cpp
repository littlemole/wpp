#include "reproweb/view/i18n.h"

namespace reproweb {


I18N::I18N(const std::string& base, const std::vector<std::string>& locales)
{
    load(base,locales);
}

std::string I18N::find_locale(std::string locale)
{
    if(map_.count(locale) == 0)
    {
        auto v = prio::split(locale,'_');
        if ( v.size() == 2 )
        {				
            if ( map_.count(v[0]) == 0 )
            {
                locale = "";
            }
            else
            {
                locale = v[0];
            }
        }
        else
        {
            locale = "";
        }
    }

    return locale;
}

const std::string& I18N::key(std::string locale, const std::string& k)
{
    locale = find_locale(locale);
    return get_key(locale,k);
}

std::string I18N::render(std::string locale, const std::string& txt)
{
    locale = find_locale(locale);

    std::map<std::string,std::string>& props = map_[locale];
    static std::regex r("<!--#i18n\\s+key\\s*=\\s*[\"']([^'\"]*)[\"']\\s*-->");

    size_t start = 0;
    size_t startpos = 0;
    size_t endpos = std::string::npos;

    startpos = txt.find("<!--#i18n");

    std::ostringstream result;

	while (startpos != std::string::npos)
	{
		endpos = txt.find("-->", startpos + 9);
		if (endpos == std::string::npos)
		{
			return "error unterminated i18n comment";
		}

		std::string i18n = txt.substr(startpos, endpos-startpos + 3);
		std::smatch m;
		if (!std::regex_search(i18n, m, r))
		{
			return "invalid i18n comment";
		}

		if (m.size() < 2)
		{
			return "invalid i18n key";
		}        

        //result << txt.substr(start, startpos-start);
        result.write( txt.c_str() + start, startpos-start);

        std::string key = m[1];
        if( props.count(key)==0 && map_[""].count(key)==0 )
        {
            result << key;
        }
        else
        {
            const std::string& value = get_key(locale,key);
            result.write(value.c_str(),value.size());
        }        
        
        start = endpos + 3;
		startpos = txt.find("<!--#i18n",start);
    }

    //result << txt.substr(start);
    result.write( txt.c_str()+start, txt.size()-start );
    return result.str();
}



const std::string& I18N::get_key(std::string locale, const std::string& k)
{
    if( map_[locale].count(k) > 0 )
    {
        return map_[locale][k];
    }
    if( map_[""].count(k) > 0 )
    {
        return map_[""][k];
    }
    return k;
}

void I18N::parse(const std::string& locale,const std::string& content)
{
    std::vector<std::string> lines;
    std::istringstream iss(content);
    while(!iss.eof())
    {
        std::string line;
        std::getline(iss,line);
        line = prio::trim(line);
        if(line.empty())
        {
            continue;
        }
        if(line[0] == '#')
        {
            continue;
        }

        if ( line[ line.length() - 1 ] == '\\' )
        {
            lines.push_back( line.substr(0, line.length()-1) );

            do
            {
                std::getline(iss,line);
                line = prio::trim(line);
                if(line.empty())
                {
                    continue;
                }

                if ( line[ line.length() - 1 ] == '\\' )
                {
                    lines.back().append(line.substr(0,line.length()-1));
                }
                else 
                {
                    lines.back().append(line);
                }
            }
            while(line[line.length()-1] == '\\');
        }
        else
        {
            lines.push_back(line);
        }
    }

    for(auto& line : lines)
    {
        std::size_t pos = line.find("=");
        if (pos == std::string::npos)
        {
            continue;
        }
        std::string key = line.substr(0,pos);
        std::string value = line.substr(pos+1);
        map_[locale][key] = value;
    }
}

void I18N::load(const std::string& locale,const std::string& path)
{
    std::cout << "load locales: " << locale << " from " << path << std::endl;
    
    std::string content = prio::slurp(path);
    if ( content.empty() )
    {
        return;
    }
    
    parse(locale,content);
}


void I18N::load(const std::string& base, const std::vector<std::string>& locales)
{
    std::string path_base = prio::get_current_work_dir()+ base;
    std::string path = prio::real_path(path_base );		

#ifndef _WIN32
    if ( path.substr(0,path_base.length()) != path_base )
    {
        return;
    } 
#endif

    load("",path);
    for( auto& l : locales)
    {
        load(l,path + "." + l);
    }
}


}