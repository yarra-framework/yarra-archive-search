#ifndef YAS_TWIXREADER_H
#define YAS_TWIXREADER_H

#include <string>
#include <vector>


class yasTwixEntry;

typedef std::vector<yasTwixEntry> yasTwixEntryList;

class yasTwixReader
{
public:

    enum resultType
    {
        OK=0,
        UNABLE_TO_OPEN,
        FILE_INVALID,
        MISSING_ENTRIES
    };

    enum fileVersionType
    {
        UNKNOWN=0,
        VAVB,
        VDVE
    };

    static constexpr int valueCount=7;

    enum valueType
    {
        patientName=0,
        patientID,
        protocolName,
        acquisitionTime,
        acquisitionDate,
        patientAge,
        patientGender
    };


    yasTwixReader(std::string filename);
    ~yasTwixReader();

    bool             perform();
    resultType       getResult();
    std::string      getResultString();
    fileVersionType  getFileType();
    std::string      getValue(valueType value);
    void             evaluateLine(std::string& line, std::ifstream& file);
    bool             splitAcquisitionTime(std::string input, std::string& timeString, std::string& dateString);
    void             removePrecisionTag(std::string& line);
    void             removeLeadingWhitespace(std::string& line);
    void             removeEnclosingWhitespace(std::string& line);
    void             findBraces(std::string& line, std::ifstream& file);

    void             setDebug(bool debugState);
    bool             debugOutput;

    std::string      twixFilename;
    fileVersionType  fileVersion;
    resultType       result;
    std::string      values[valueCount];

    uint64_t         lastMeasOffset;
    uint32_t         headerLength;
    uint32_t         headerEnd;

    yasTwixEntryList searchEntryList;
};


class yasTwixEntry
{
public:
    yasTwixEntry(std::string entryString, yasTwixReader::valueType assignment)
    {
        searchString=entryString;
        valueAssignment=assignment;
    }

    std::string searchString;
    yasTwixReader::valueType valueAssignment;
};



#endif // YAS_TWIXREADER_H

