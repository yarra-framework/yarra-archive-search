#ifndef YAS_ARCHIVEENTRY_H
#define YAS_ARCHIVEENTRY_H

#include <Wt/Dbo/Dbo.h>


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
        Wt::Dbo::field(a, lastSeen,        "last_seen");

        Wt::Dbo::field(a, filename,        "filename");
        Wt::Dbo::field(a, path,            "path");
        Wt::Dbo::field(a, writeTime,       "write_time");

        Wt::Dbo::field(a, patientName,     "patient_name");
        Wt::Dbo::field(a, patientID,       "patient_id");
        Wt::Dbo::field(a, patientAge,      "patient_age");
        Wt::Dbo::field(a, patientGender,   "patient_gender");

        Wt::Dbo::field(a, protocolName,    "protocol_name");
        Wt::Dbo::field(a, acquisitionTime, "acquisition_time");
        Wt::Dbo::field(a, acquisitionDate, "acquisition_date");

        Wt::Dbo::field(a, MRSystem,        "mr_system");
        Wt::Dbo::field(a, accessionNumber, "accession_number");
        Wt::Dbo::field(a, yarraServer,     "yarra_server");
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

