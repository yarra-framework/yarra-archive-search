#include "yas_indexer.h"
#include "../common/yas_global.h"
#include "../common/yas_archiveentry.h"
#include "yas_twixreader.h"

#include <Wt/WString>

#include <boost/property_tree/ini_parser.hpp>

#include <cstring>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>

using namespace Wt;


yasIndexer::yasIndexer(int argc, char* argv[])
{
    returnValue=0;
    debugOutput=false;
    action=ABORT;

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

        if (strcmp(argv[1], "--debug")==0)
        {
            action=RUNINDEXER;
            debugOutput=true;
        }
    }
    else
    {
        action=RUNINDEXER;
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
    LOG("Without given argument, the indexing process is performed");
    LOG("");
    LOG("--clear  Clears all entries from the database");
    LOG("--help   Shows this help text");
    LOG("--debug  Output debug information during index process");
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
    LOG("");

    std::time_t endTime=std::time(nullptr);
    LOG("Done at " << ctime(&endTime));
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
    try
    {
        dbBackend=new Wt::Dbo::backend::Postgres(configuration.db_name.toUTF8());

        // Set database to write-ahead-logging to avoid locking in the WebGUI while the indexer is running
        // dbBackend->executeSql("PRAGMA journal_mode=WAL;");

        dbSession=new Wt::Dbo::Session;
        dbSession->setConnection(*dbBackend);

        // Map the database table
        dbSession->mapClass<yasArchiveEntry>("yas_archive");
    }
    catch (const Wt::Dbo::Exception & e)
    {
        return false;
    }

    // Create the database table if it does not exist. Will not do anything
    // if the database already exists.
    try
    {
        Wt::Dbo::Transaction transaction(*dbSession);
        dbSession->createTables();
        //dbSession->execute("CREATE INDEX index_yas_archive on yas_archive (Filename, Path);");
    }
    catch (const Wt::Dbo::Exception & e)
    {
        std::cout << e.code() << std::endl;
    }

    return true;
}


void yasIndexer::processFolders()
{
    int totalFilesFound=0;
    int newFilesAdded=0;
    auto startTime=std::chrono::system_clock::now();

    // Sequentially search through configured folders
    for (std::string& folder : configuration.folders)
    {
        fs::path folderPath(folder);

        if (fs::is_directory(folderPath))
        {
            std::string alias="";

            // Get folder alias
            if (configuration.folderAlias.find(folder)!=configuration.folderAlias.end())
            {
                alias=configuration.folderAlias[folder];
            }

            try
            {
                // Recursively iterate through folder
                fs::recursive_directory_iterator dir_entry(folderPath);
                fs::recursive_directory_iterator iter_end;

                while (dir_entry != iter_end)
                {
                    if (dir_entry->path().extension()==".dat")
                    {
                        try
                        {
                            totalFilesFound++;

                            // Inform the user about the progress
                            if (totalFilesFound % 100==0)
                            {
                                LOG("..." << totalFilesFound << " files done...");
                            }

                            //std::cout << "Full path is " << dir_entry->path().parent_path().string() << std::endl;
                            //std::cout << "Folder is " << folder << std::endl;
                            //std::cout << "Alias is " << alias << std::endl;
                            //std::cout << "--------------" << std::endl;

                            std::string aliasedPath=getAliasedPath(dir_entry->path().parent_path().string(), folder, alias);
                            std::string filename=dir_entry->path().filename().string();

                            DEBUG("Processing " << dir_entry->path().string());

                            if (!isFileIndexed(aliasedPath, filename))
                            {
                                DEBUG("New file, adding to index.");

                                // Create new index entry for file
                                if (indexFile(dir_entry->path(), aliasedPath, filename))
                                {
                                    newFilesAdded++;
                                }
                            }
                            else
                            {
                                DEBUG("File already indexed.");
                            }

                        }
                        catch (const std::runtime_error &e)
                        {
                            LOG("ERROR: Unable to read file " << dir_entry->path() << " (" << e.what() << ")");
                        }
                    }

                    ++dir_entry;
                }

            }
            catch (const fs::filesystem_error & e)
            {
                LOG("ERROR: Unable to access directory " << folderPath << " (" << boost::diagnostic_information(e) << ")");
            }
        }
        else
        {
            LOG("ERROR: The path " << folder << " does not exist or is not accessible.");
        }
    }

    auto endTime=std::chrono::system_clock::now();
    auto elapsedTime=std::chrono::duration_cast<std::chrono::seconds>(endTime-startTime);

    LOG("");
    LOG("New files added:       " << newFilesAdded);
    LOG("Total files processed: " << totalFilesFound);
    LOG("Elapsed time:          " << elapsedTime.count() << " sec");
}


std::string yasIndexer::getAliasedPath(std::string fullPath, std::string folder, std::string alias)
{
    if (!alias.empty())
    {
        fullPath.replace(0, folder.length(), alias);
    }

    return fullPath;
}


void yasIndexer::dropUnseenEntries()
{
    typedef std::chrono::duration<int, std::ratio<3600*24>> days;

    std::time_t dropBefore=std::chrono::system_clock::to_time_t( std::chrono::system_clock::now()-days(configuration.keepUnseenEntries) );

    Wt::Dbo::Transaction transaction(*dbSession);
    dbSession->execute("DELETE FROM yas_archive WHERE last_seen < ? ").bind(dropBefore);
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

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
              + system_clock::now());
    return system_clock::to_time_t(sctp);
}
bool yasIndexer::indexFile(fs::path path, std::string aliasedPath, std::string filename)
{
    //LOG("DBG   : " << path.string());
    //LOG("Folder: " << aliasedPath);
    //LOG("File  : " << filename);
    //LOG("");

    // Create new index entry
    Wt::Dbo::ptr<yasArchiveEntry> fileEntry(new yasArchiveEntry());

    bool isEntryValid=true;

    fileEntry.modify()->lastSeen =std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    fileEntry.modify()->filename =filename;
    fileEntry.modify()->path     =aliasedPath;

    try
    {
        fileEntry.modify()->writeTime=to_time_t(fs::last_write_time(path));
    }
    catch (const fs::filesystem_error & e)
    {
        fileEntry.modify()->writeTime=0;
    }

    // Read values from TWIX file
    yasTwixReader twixReader(path.string());
    twixReader.setDebug(debugOutput);

    if (!twixReader.perform())
    {
        LOG("Error parsing file "+path.string())
        LOG("Reason: "+twixReader.getResultString());
        LOG("");
        isEntryValid=false;
    }
    else
    {
        fileEntry.modify()->patientName    =twixReader.getValue(yasTwixReader::patientName);
        fileEntry.modify()->patientID      =twixReader.getValue(yasTwixReader::patientID);
        fileEntry.modify()->patientAge     =twixReader.getValue(yasTwixReader::patientAge);
        fileEntry.modify()->patientGender  =twixReader.getValue(yasTwixReader::patientGender);
        fileEntry.modify()->protocolName   =twixReader.getValue(yasTwixReader::protocolName);
        fileEntry.modify()->acquisitionTime=twixReader.getValue(yasTwixReader::acquisitionTime);
        fileEntry.modify()->acquisitionDate=twixReader.getValue(yasTwixReader::acquisitionDate);

        /*
        LOG("patientName=" + twixReader.getValue(yasTwixReader::patientName));
        LOG("patientID=" + twixReader.getValue(yasTwixReader::patientID));
        LOG("patientAge=" + twixReader.getValue(yasTwixReader::patientAge));
        LOG("patientGender=" + twixReader.getValue(yasTwixReader::patientGender));
        LOG("protocolName=" + twixReader.getValue(yasTwixReader::protocolName));
        LOG("acquisitionTime=" + twixReader.getValue(yasTwixReader::acquisitionTime));
        LOG("acquisitionDate=" + twixReader.getValue(yasTwixReader::acquisitionDate));
        */
    }

    // Read additional values from .task file

    // First, test if a tast file exists and determine the name (.task / .task_prio / .task_night)
    bool taskExists=false;
    fs::path taskFilename = (path.parent_path() / path.stem()).string() + ".task";
    if (fs::exists(taskFilename))
    {
        taskExists=true;
    }
    else
    {
        taskFilename = (path.parent_path() / path.stem()).string() + ".task_prio";
        if (fs::exists(taskFilename))
        {
            taskExists=true;
        }
        else
        {
            taskFilename = (path.parent_path() / path.stem()).string() + ".task_night";
            if (fs::exists(taskFilename))
            {
                taskExists=true;
            }
        }
    }

    // Now read the information from the task
    if (taskExists)
    {
        try
        {
            boost::property_tree::ptree taskFile;
            boost::property_tree::ini_parser::read_ini(taskFilename.string(), taskFile);

            fileEntry.modify()->MRSystem       =taskFile.get<std::string>("Information.SystemName"    , "");
            fileEntry.modify()->accessionNumber=taskFile.get<std::string>("Task.ACC"                  , "");
            fileEntry.modify()->yarraServer    =taskFile.get<std::string>("Information.SelectedServer", "");
        }
        catch (const boost::exception & e)
        {
            LOG("WARNING: Unable to read task file " << taskFilename.string() << " (" << boost::diagnostic_information(e) << ")");
        }
    }

    // Add the entry to the database
    if (isEntryValid)
    {
        Wt::Dbo::Transaction transaction(*dbSession);
        dbSession->add(fileEntry);

        return true;
    }
    else
    {
        return false;
    }
}
