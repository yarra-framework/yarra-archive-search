#include "yas_indexer.h"
#include "../common/yas_global.h"

#include <Wt/WString>

#include <cstring>

using namespace Wt;


yasIndexer::yasIndexer(int argc, char* argv[])
{
    returnValue=0;
    action=RUNINDEXER;

    LOG("");
    LOG("Yarra Archive Search -- Indexer Version "+WString(YAS_VERSION_INDEXER));
    LOG("");

    if (argc>1)
    {
        if (strcmp(argv[1], "--help")==0)
        {
            action=SHOWHELP;
        }
        if (strcmp(argv[1], "--clear")==0)
        {
            action=CLEARINDEX;
        }

        // Seems an unkown argument has been given. Abort process.
        if (action==RUNINDEXER)
        {
            action=ABORT;
        }
    }

}


void yasIndexer::perform()
{
    switch (action)
    {
    case SHOWHELP:
        showHelp();
        break;
    case CLEARINDEX:
        clearIndex();
        break;
    case RUNINDEXER:
        runIndexer();
        break;
    case ABORT:
    default:
        LOG("Invalid argument given")
        break;
    }

    LOG("");
}


void yasIndexer::showHelp()
{
    LOG("Without given argument, the indexing process is performed")
    LOG("");
    LOG("--clear  Clears all entries from the database")
    LOG("--help   Shows this help text")
}


void yasIndexer::runIndexer()
{
    LOG("Running indexer...");

    configuration.loadConfiguration();
    if (!configuration.isValid())
    {
        LOG("Configuration is invalid. Aborting");
        return;
    }
}


void yasIndexer::clearIndex()
{
    char input;

    std::cout << ("Do you really want to clear the index? [y/n]  ");
    std::cin >> input;

    if (input=='y')
    {
        std::cout << std::endl;
        LOG("Cleaning index...");

        configuration.loadConfiguration();
        if (!configuration.isValid())
        {
            LOG("Configuration is invalid. Aborting");
            return;
        }

    }
}


