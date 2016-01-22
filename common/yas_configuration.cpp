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
            folders.push_back(WString::fromUTF8(boost::lexical_cast<std::string>(v.first.data())));
        }

        // Read the list of aliases that should be used for the folder paths
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, inifile.get_child("FolderAlias"))
        {
            folderAlias[WString::fromUTF8(boost::lexical_cast<std::string>(v.first.data()))]=WString::fromUTF8(boost::lexical_cast<std::string>(v.second.data()));
        }

        configurationValid=true;
    }
    catch(const boost::property_tree::ptree_error &e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
        configurationValid=false;
    }
}


