#include "yas_configuration.h"
#include "../common/yas_global.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/exception/all.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <errno.h>
#include <iostream>


yasConfiguration::yasConfiguration()
{
    configurationValid=false;

    webgui_port    ="9090";
    webgui_password="none";

    keepUnseenEntries=14;
    db_name="yas.db";
}


void yasConfiguration::loadConfiguration()
{
    try
    {
        boost::property_tree::ptree inifile;
        boost::property_tree::ini_parser::read_ini(YAS_CONFIGFILE, inifile);

        // Read main settings
        webgui_port    =WString::fromUTF8(inifile.get<std::string>("WebGUI.Port"    ,webgui_port.toUTF8()));
        webgui_password=WString::fromUTF8(inifile.get<std::string>("WebGUI.Password",webgui_password.toUTF8()));

        // Read the list of folders that should be parsed by the indexer
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, inifile.get_child("Folders"))
        {
            std::string folder=boost::lexical_cast<std::string>(v.first.data());

            // Remove possible "/" at the end
            if (folder.at(folder.length()-1)=='/')
            {
                folder.pop_back();
            }

            folders.push_back(folder);
        }

        // Read the list of aliases that should be used for the folder paths.
        // Catch exceptions for the case that no folder aliases have been defined.
        try
        {
            BOOST_FOREACH(boost::property_tree::ptree::value_type &v, inifile.get_child("FolderAlias"))
            {
                std::string folder=boost::lexical_cast<std::string>(v.first.data());
                std::string alias =boost::lexical_cast<std::string>(v.second.data());

                // Remove possible "/" at the end
                if (folder.at(folder.length()-1)=='/')
                {
                    folder.pop_back();
                }
                if (alias.at(alias.length()-1)=='/')
                {
                    alias.pop_back();
                }

                folderAlias[folder]=alias;

                //std::cout << "Setting alias " << folder << " --> " << alias << std::endl;
            }
        }
        catch(const boost::property_tree::ptree_error &e)
        {
            //std::cout << "Problem while reading FolderAlias" << std::endl;
        }

        keepUnseenEntries=inifile.get<int>("Indexer.KeepUnseenEntries", keepUnseenEntries);

        configurationValid=true;
    }
    catch(const boost::property_tree::ptree_error &e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
        configurationValid=false;
    }
}


