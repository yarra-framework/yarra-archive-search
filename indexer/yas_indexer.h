#ifndef YAS_INDEXER_H
#define YAS_INDEXER_H

#include "../common/yas_configuration.h"

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>
#include <boost/exception/all.hpp>

namespace fs = boost::filesystem;


class yasIndexer
{
public:
    enum IndexAction {
        SHOWHELP=0,
        RUNINDEXER=1,
        CLEARINDEX=2,
        ABORT=3
    };

    yasIndexer(int argc, char* argv[]);
    ~yasIndexer();

    void perform();
    void showHelp();
    void runIndexer();
    void clearIndex();
    int  getReturnValue();

    bool prepareDatabase();
    void processFolders();
    void dropUnseenEntries();

    bool isFileIndexed(std::string path, std::string filename);
    bool indexFile(fs::path path, std::string aliasedPath, std::string filename);

    std::string getAliasedPath(std::string fullPath, std::string folder, std::string alias);

    Wt::Dbo::backend::Sqlite3* dbBackend;
    Wt::Dbo::Session* dbSession;

    yasConfiguration configuration;

    IndexAction action;
    bool debugOutput;
    int  returnValue;
};


inline int yasIndexer::getReturnValue()
{
    return returnValue;
}


#endif // YAS_INDEXER_H

