#ifndef YAS_CONFIGURATION_H
#define YAS_CONFIGURATION_H

#include <Wt/WString>
#include <map>

#include "../common/yas_global.h"

using namespace Wt;


class yasConfiguration
{
public:
    yasConfiguration();

    void loadConfiguration();
    bool isValid();

    WString webgui_port;
    WString webgui_password;

    StringList                         folders;
    std::map<std::string, std::string> folderAlias;

    int keepUnseenEntries;
    WString db_name;


private:
    bool configurationValid;

};


inline bool yasConfiguration::isValid()
{
    return configurationValid;
}


#endif // YAS_CONFIGURATION_H

