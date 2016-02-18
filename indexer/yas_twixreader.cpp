#include "yas_twixreader.h"
#include "../common/yas_global.h"
#include "yas_twixheader.h"

#include <iostream>
#include <fstream>


yasTwixReader::yasTwixReader(std::string filename)
{
    twixFilename=filename;
    fileVersion=UNKNOWN;
    result=OK;

    for (int i=0; i<valueCount; i++)
    {
        values[i]="";
    }

    lastMeasOffset=0;
    headerLength=0;
}


yasTwixReader::~yasTwixReader()
{
    searchEntryList.clear();
}


bool yasTwixReader::perform()
{
    searchEntryList.clear();
    searchEntryList.push_back(yasTwixEntry("<ParamString.\"tPatientName\">"    ,patientName));
    searchEntryList.push_back(yasTwixEntry("<ParamString.\"PatientID\">"       ,patientID));
    searchEntryList.push_back(yasTwixEntry("<ParamString.\"tProtocolName\">"   ,protocolName));
    searchEntryList.push_back(yasTwixEntry("<ParamString.\"FrameOfReference\">",acquisitionTime));

    std::ifstream file;
    file.open(twixFilename.c_str());

    if (!file.is_open())
    {
        result=UNABLE_TO_OPEN;
        return false;
    }

    // Determine TWIX file type: VA/VB or VD/VE?

    uint32_t x[2];
    file.read ((char*)x, 2*sizeof(uint32_t));

    if ((x[0]==0) && (x[1]<=64))
    {
        fileVersion=VDVE;
    }
    else
    {
        fileVersion=VAVB;
    }
    file.seekg(0);

    if (fileVersion==VDVE)
    {
        uint32_t id=0, ndset=0;
        std::vector<VD::EntryHeader> veh;

        file.read((char*)&id,   sizeof(uint32_t));  // ID
        file.read((char*)&ndset,sizeof(uint32_t));  // # data sets

        if (ndset>30)
        {
            // If there are more than 30 measurements, it's unlikely that the
            // file is a valid TWIX file
            LOG("WARNING: Number of measurements in file " << ndset);

            result=FILE_INVALID;
            file.close();
            return false;
        }

        veh.resize(ndset);

        for (size_t i=0; i<ndset; ++i)
        {
            file.read((char*)&veh[i], VD::ENTRY_HEADER_LEN);
        }

        lastMeasOffset=veh.back().MeasOffset;

        // Go to last measurement
        file.seekg(lastMeasOffset);
    }

    // Find header length
    file.read((char*)&headerLength, sizeof(uint32_t));

    if ((headerLength<=0) || (headerLength>1000000))
    {        
        // File header is invalid

        std::string fileType="VB";
        if (fileVersion==VDVE)
        {
            fileType="VD/VE";
        }
        LOG("WARNING: Unusual header size " << headerLength << " (file type " << fileType << ")");

        result=FILE_INVALID;
        file.close();
        return false;
    }

    // Jump back to start of measurement block
    file.seekg(lastMeasOffset);

    uint32_t headerEnd=lastMeasOffset+headerLength;

    // Parse header

    bool terminateParsing=false;

    while ((!file.eof()) && (file.tellg()<headerEnd) && (!terminateParsing))
    {
        std::string line="";
        std::getline(file, line);
        //std::cout << line << std::endl;

        evaluateLine(line, file);

        if (searchEntryList.empty())
        {
            terminateParsing=true;
        }

        // Terminate the parsing once the acquisiton protocol is reached
        if (line.find("### ASCCONV BEGIN ###")!=std::string::npos)
        {
            terminateParsing=true;
        }
    }

    file.close();

    if (!searchEntryList.empty())
    {
        result=MISSING_ENTRIES;
        return false;
    }

    return true;
}


bool yasTwixReader::splitAcquisitionTime(std::string input, std::string& timeString, std::string& dateString)
{
    // Format is 1.3.12.2.1107.5.2.30.25654.1.20130506212248515.0.0.0
    timeString="";
    dateString="";

    // Remove the numbers and dots in front of the date/time section
    int dotPos=0;

    for (int i=0; i<10; i++)
    {
        dotPos=input.find(".",dotPos+1);

        if (dotPos==std::string::npos)
        {
            // String does not match the usual format
            return false;
        }
    }
    input.erase(0,dotPos+1);

    dateString=input.substr(4,2)+"/"+input.substr(6,2)+"/"+input.substr(0,4);
    timeString=input.substr(8,2)+":"+input.substr(10,2)+":"+input.substr(12,2);

    //std::cout << "ACQ:  " << input << std::endl;
    //std::cout << "Date: " << dateString << std::endl;
    //std::cout << "Time: " << timeString << std::endl;

    return true;
}


void yasTwixReader::evaluateLine(std::string& line, std::ifstream& file)
{
    int indexFound=-1;
    int searchPos=std::string::npos;

    for (int i=0; i<searchEntryList.size(); i++)
    {
        searchPos=line.find(searchEntryList.at(i).searchString);

        if (searchPos!=std::string::npos)
        {
            // Get value from line and write into result array
            std::string value=line;
            valueType target=searchEntryList.at(i).valueAssignment;

            value.erase(0,searchPos+searchEntryList.at(i).searchString.length());

            int quotePos=value.find("\"");

            if (quotePos==std::string::npos)
            {
                // If the line does not contain quotation marks, the value might be in the next line
                // TODO: Read two additional lines from the file
            } else
            {
                // Delete the quotation marks including preceeding white space
                value.erase(0,quotePos+1);
            }

            // Remove the trailing quotation mark if it exists
            quotePos=value.find("\"");
            if (quotePos!=std::string::npos)
            {
                value.erase(quotePos);
            }

            //std::cout << value << std::endl;

            if (target==acquisitionTime)
            {
                // Split the string into date and time
                splitAcquisitionTime(value, values[acquisitionTime], values[acquisitionDate]);
            }
            else
            {
                values[target]=value;
            }

            indexFound=i;
            break;
        }
    }

    // Remove entry from search list
    if (indexFound>=0)
    {
        searchEntryList.erase(searchEntryList.begin()+indexFound);
    }
}


yasTwixReader::resultType yasTwixReader::getResult()
{
    return result;
}


std::string yasTwixReader::getResultString()
{
    std::string returnText="";

    switch (result)
    {
    case UNABLE_TO_OPEN:
        returnText="Unable to open file";
        break;

    case FILE_INVALID:
        returnText="File is not valid TWIX file";
        break;

    case MISSING_ENTRIES:
        {
            returnText="Unable to find all entries (missing:";

            for (const yasTwixEntry& missingEntry : searchEntryList)
            {
                returnText+=" "+missingEntry.searchString;
            }

            returnText+=")";
            break;
        }

    case OK:
    default:
        returnText="OK";
        break;
    }

    return returnText;
}



std::string yasTwixReader::getValue(valueType value)
{
    if (value>=valueCount)
    {
        return "";
    }

    return values[value];
}

