#ifndef YAS_GLOBAL_H
#define YAS_GLOBAL_H

#include <iostream>
#include <vector>
#include <Wt/WString>


#define YAS_CONFIGFILE "YAS.ini"

#define YAS_VERSION_INDEXER "0.1b2"
#define YAS_VERSION_WEBGUI  "0.1b2"

// String list, as used at various places in the code
typedef std::vector<Wt::WString> WStringList;

#define LOG(t) std::cout << t << std::endl;


#endif // YAS_GLOBAL_H
