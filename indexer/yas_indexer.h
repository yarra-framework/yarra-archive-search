#ifndef YAS_INDEXER_H
#define YAS_INDEXER_H

#include "../common/yas_configuration.h"

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>


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

    std::string getAliasedPath(std::string fullPath, WString folder, WString alias);

    Wt::Dbo::backend::Sqlite3* dbBackend;
    Wt::Dbo::Session* dbSession;

    yasConfiguration configuration;

    IndexAction action;
    int returnValue;
};


inline int yasIndexer::getReturnValue()
{
    return returnValue;
}


#endif // YAS_INDEXER_H

