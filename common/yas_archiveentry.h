#ifndef YAS_ARCHIVEENTRY_H
#define YAS_ARCHIVEENTRY_H

#include <Wt/Dbo/Dbo>


class yasArchiveEntry
{
public:
    std::time_t lastSeen;

    std::string filename;
    std::string path;
    std::time_t writeTime;

    std::string patientName;
    std::string patientID;
    std::string patientAge;
    std::string patientGender;

    std::string protocolName;
    std::string acquisitionTime;
    std::string acquisitionDate;   

    std::string MRSystem;
    std::string accessionNumber;
    std::string yarraServer;


    // Mapping method for Wt::Dbo
    template<class Action>
    void persist(Action &a)
    {
        Wt::Dbo::field(a, lastSeen,        "LastSeen");

        Wt::Dbo::field(a, filename,        "Filename");
        Wt::Dbo::field(a, path,            "Path");
        Wt::Dbo::field(a, writeTime,       "WriteTime");

        Wt::Dbo::field(a, patientName,     "PatientName");
        Wt::Dbo::field(a, patientID,       "PatientID");
        Wt::Dbo::field(a, patientAge,      "PatientAge");
        Wt::Dbo::field(a, patientGender,   "PatientGender");

        Wt::Dbo::field(a, protocolName,    "ProtocolName");
        Wt::Dbo::field(a, acquisitionTime, "AcquisitionTime");
        Wt::Dbo::field(a, acquisitionDate, "AcquisitionDate");

        Wt::Dbo::field(a, MRSystem,        "MRSystem");
        Wt::Dbo::field(a, accessionNumber, "AccessionNumber");
        Wt::Dbo::field(a, yarraServer,     "YarraServer");
    }


    yasArchiveEntry()
    {
        lastSeen=0;

        filename="";
        path="";
        writeTime=0;

        patientName="";
        patientID="";
        patientAge="";
        patientGender="";

        protocolName="";
        acquisitionTime="";
        acquisitionDate="";

        MRSystem="";
        accessionNumber="";
        yarraServer="";
    }
};


#endif // YAS_ARCHIVEENTRY_H

