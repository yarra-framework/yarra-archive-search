#include "yas_twixreader.h"
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

    // Determin TWIX file type: VA/VB or VD/VE?

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
        uint32_t id, ndset;
        std::vector<VD::EntryHeader> veh;

        file.read((char*)&id,   sizeof(uint32_t));  // ID
        file.read((char*)&ndset,sizeof(uint32_t));  // # data sets

        if (ndset>30)
        {
            // If there are more than 30 measurements, it's unlikely that the
            // file is a valid TWIX file
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


void yasTwixReader::evaluateLine(std::string& line, std::ifstream& file)
{
    int indexFound=-1;

    for (int i=0; i<searchEntryList.size(); i++)
    {
        if (line.find(searchEntryList.at(i).searchString)!=std::string::npos)
        {
            // TODO: Get value from line and write into result array
            std::string value="";

            valueType target=searchEntryList.at(i).valueAssignment;
            std::cout << line << std::endl;


            if (target==acquisitionTime)
            {
                // TOOD: Split the string into date and time
                values[acquisitionTime]="";
                values[acquisitionDate]="";
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

