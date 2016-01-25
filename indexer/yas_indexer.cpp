#include "yas_indexer.h"
#include "../common/yas_global.h"
#include "../common/yas_archiveentry.h"

#include <Wt/WString>

#include <cstring>
#include <iostream>
#include <vector>
#include <chrono>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>
#include <boost/exception/all.hpp>


using namespace Wt;
namespace fs = boost::filesystem;


yasIndexer::yasIndexer(int argc, char* argv[])
{
    returnValue=0;
    action=RUNINDEXER;
    dbBackend=nullptr;
    dbSession=nullptr;

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


yasIndexer::~yasIndexer()
{
    if (dbBackend)
    {
        delete dbBackend;
    }

    if (dbSession)
    {
        delete dbSession;
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

    prepareDatabase();
    processFolders();
    dropUnseenEntries();
}


void yasIndexer::clearIndex()
{
    char input;

    std::cout << ("Do you really want to clear the index? [y/n] ");
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

        if (fs::is_regular_file(fs::path(configuration.db_name.toUTF8())))
        {
            try
            {
                fs::remove(fs::path(configuration.db_name.toUTF8()));
            }
            catch (const boost::exception & e)
            {
                LOG("ERROR: Unable to remove database file " << configuration.db_name);
            }
            LOG("Done.")
        }
        else
        {
            LOG("ERROR: Database file not  found " << configuration.db_name);
        }
    }
}


bool yasIndexer::prepareDatabase()
{
    dbBackend=new Wt::Dbo::backend::Sqlite3(configuration.db_name.toUTF8());
    dbSession=new Wt::Dbo::Session;
    dbSession->setConnection(*dbBackend);

    // Map the database table
    dbSession->mapClass<yasArchiveEntry>("yasArchive");

    // Create the database table if it does not exist. Will not do anything
    // if the database already exists.
    try
    {
        Wt::Dbo::Transaction transaction(*dbSession);
        dbSession->createTables();
        dbSession->execute("CREATE INDEX index_yasArchive on yasArchive (Filename, Path);");
    }
    catch (const Wt::Dbo::Exception & e)
    {
    }

    return true;
}


void yasIndexer::processFolders()
{
    // TODO: Track time

    for (WString& folder : configuration.folders)
    {
        fs::path folderPath(folder.toUTF8());

        if (fs::is_directory(folderPath))
        {
            WString alias="";

            // Get folder alias
            if (configuration.folderAlias.find(folder)!=configuration.folderAlias.end())
            {
                alias=configuration.folderAlias[folder];
            }

            // TODO

            /*
            try
            {
                for (const auto &dir_entry : fs::recursive_directory_iterator(dir_path))
                {
                    if (dir_entry.path().extension() == measFileExtension)
                    {
                        try
                        {
                            Wt::Dbo::ptr<MeasFileMeta> measFileMeta(new MeasFileMeta(dir_entry.path()));
                            std::cout << "Processing " << dir_entry.path();
                            if (!isMetaIndexed(session, *measFileMeta))
                            {
                                std::cout << " -> new entry" << std::endl;
                                measFileMeta.modify()->readMeasFile(dir_entry.path().string());
                                insertMeta(session, measFileMeta);
                            }
                            else
                            {
                                std::cout << " -> existing entry" << std::endl;
                            }
                        }
                        catch (const std::runtime_error &e)
                        {
                            LOG(ERROR, "Exception when reading " << dir_entry.path() << ": " << e.what());
                        }
                    }
                }
            }
            catch (const fs::filesystem_error & e)
            {
                LOG(ERROR, "Exception when recursively iterating " << dir_path << ": " << boost::diagnostic_information(e));
            }
*/
        }
        else
        {
            LOG("ERROR: The path `" << folder << "` does not exist or is not accessible.");
        }
    }
}


std::string yasIndexer::getAliasedPath(std::string fullPath, WString folder, WString alias)
{
    if (!alias.empty())
    {
        fullPath.replace(0, folder.toUTF8().length(), alias.toUTF8());
    }

    return fullPath;
}


void yasIndexer::dropUnseenEntries()
{
    typedef std::chrono::duration<int, std::ratio<3600*24>> days;

    std::time_t dropBefore=std::chrono::system_clock::to_time_t( std::chrono::system_clock::now()-days(configuration.keepUnseenEntries) );

    Wt::Dbo::Transaction transaction(*dbSession);
    dbSession->execute("DELETE FROM yasArchive WHERE LastSeen < ? ").bind(dropBefore);
}


bool yasIndexer::isFileIndexed(std::string path, std::string filename)
{
    Wt::Dbo::Transaction transaction(*dbSession);

    // Search for the entry in the database
    Wt::Dbo::ptr<yasArchiveEntry> entry=dbSession->find<yasArchiveEntry>().
            where("Filename = ?").bind(filename).
            where("Path = ?").bind(path);

    if (entry)
    {
        // Update the LastSeen value (used to remove entries from deleted files)
        entry.modify()->lastSeen=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        return true;
    }

    return false;
}
