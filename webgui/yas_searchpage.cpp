#include "yas_searchpage.h"
#include "yas_application.h"

#include <boost/algorithm/string/replace.hpp>

#include <Wt/WPanel>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WTable>

#include <ctime>


using namespace Wt;


yasSearchPage::yasSearchPage(yasApplication* parent)
    : WContainerWidget(), dbBackend(parent->configuration->db_name.toUTF8())
{
    app=parent;

    // Set database to write-ahead-logging to avoid locking while the indexer is running
    dbBackend.executeSql("PRAGMA journal_mode=WAL;");

    // Init the db connection
    dbSession.setConnection(dbBackend);

    // Map the database table
    dbSession.mapClass<yasArchiveEntry>("yasArchive");

    // Create the database in case that it does not exist yet
    try
    {
        dbSession.createTables();
    }
    catch (const Wt::Dbo::Exception & e)
    {
    }


    dbQuery = new Wt::Dbo::QueryModel<Wt::Dbo::ptr<yasArchiveEntry>>();
    dbQuery->setQuery(dbSession.find<yasArchiveEntry>());

    dbQuery->addColumn("PatientName",     "Patient Name");
    dbQuery->addColumn("PatientID",       "MRN");
    dbQuery->addColumn("AccessionNumber", "ACC");
    dbQuery->addColumn("AcquisitionDate", "Exam Date");
    dbQuery->addColumn("ProtocolName",    "Protocol");

    tableView=new WTableView();
    tableView->setSelectionMode(SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->setModel(dbQuery);

    tableView->setHeaderHeight(32);
    tableView->setRowHeight(28);
    tableView->setColumnWidth(0, 350);
    tableView->setColumnWidth(1, 150);
    tableView->setColumnWidth(2, 150);
    tableView->setColumnWidth(3, 150);
    tableView->setColumnWidth(4, 350);
    tableView->setHeaderAlignment(0, Wt::AlignCenter);
    tableView->setHeaderAlignment(1, Wt::AlignCenter);
    tableView->setHeaderAlignment(2, Wt::AlignCenter);
    tableView->setHeaderAlignment(3, Wt::AlignCenter);
    tableView->setHeaderAlignment(4, Wt::AlignCenter);

    tableView->sortByColumn(0, AscendingOrder);
    tableView->setMinimumSize(Wt::WLength::Auto,100);
    tableView->selectionChanged().connect(this, &yasSearchPage::showInformation);

    WVBoxLayout* pageLayout=new WVBoxLayout();
    pageLayout->setContentsMargins(30, 30, 30, 10);
    pageLayout->setSpacing(15);
    this->setLayout(pageLayout);

    WContainerWidget* searchWidgets=new WContainerWidget();
    WHBoxLayout* searchLayout=new WHBoxLayout();
    searchLayout->setContentsMargins(150, 0, 150, 20);
    searchLayout->setSpacing(20);
    searchWidgets->setLayout(searchLayout);

    searchEdit=new WLineEdit();
    WPushButton* searchButton=new WPushButton("Search");
    searchButton->setStyleClass("btn-primary");

    searchButton->clicked().connect(this, &yasSearchPage::performSearch);
    searchEdit->enterPressed().connect(this, &yasSearchPage::performSearch);

    searchLayout->addWidget(searchEdit,1);
    searchLayout->addWidget(searchButton);
    pageLayout->addWidget(searchWidgets);

    pageLayout->addWidget(tableView,1);

    WPanel* infoPanel=new WPanel();
    infoPanel->setTitleBar(true);
    infoPanel->setTitle("Information");
    infoPanel->addStyleClass("panel-info");
    infoPanel->setMinimumSize(Wt::WLength::Auto,154);

    informationText=new WText("");
    infoPanel->setCentralWidget(informationText);
    pageLayout->addWidget(infoPanel);
    informationText->setMargin(0);

    if (dbQuery->hasIndex(0,0))
    {
        tableView->select(dbQuery->index(0,0));
    }

    searchEdit->setFocus();
}


void yasSearchPage::performSearch()
{
    std::stringstream searchStringStream(searchEdit->text().toUTF8());

    searchEdit->setSelection(0, searchEdit->text().toUTF8().length());

    std::vector<std::string> searchPhrases;
    while (!searchStringStream.eof())
    {
        std::string searchPhrase;
        searchStringStream >> searchPhrase;
        if (!searchPhrase.empty()) {
            searchPhrases.push_back(searchPhrase);
        }
    }

    if (searchPhrases.empty())
    {
        dbQuery->setQuery(dbSession.find<yasArchiveEntry>(), true);
    }
    else
    {
        auto query=dbSession.query<Wt::Dbo::ptr<yasArchiveEntry>>("select m from yasArchive m");

        for (auto phrase : searchPhrases)
        {
            phrase = "%" + phrase + "%";
            query.where(
                    "m.Filename        LIKE ? COLLATE NOCASE OR "
                    "m.PatientName     LIKE ? COLLATE NOCASE OR "
                    "m.PatientID       LIKE ? OR "
                    "m.ProtocolName    LIKE ? COLLATE NOCASE OR "
                    "m.AcquisitionTime LIKE ? OR "
                    "m.AcquisitionDate LIKE ? OR "
                    "m.MRSystem        LIKE ? COLLATE NOCASE OR "
                    "m.AccessionNumber LIKE ? OR "
                    "m.YarraServer     LIKE ? COLLATE NOCASE")
                    .bind(phrase)
                    .bind(phrase)
                    .bind(phrase)
                    .bind(phrase)
                    .bind(phrase)
                    .bind(phrase)
                    .bind(phrase)
                    .bind(phrase)
                    .bind(phrase);
        }

        dbQuery->setQuery(query, true);
    }

    tableView->sortByColumn(0, AscendingOrder);

    if (dbQuery->hasIndex(0,0))
    {
        tableView->select(dbQuery->index(0,0));
    }
}


void yasSearchPage::showInformation()
{
    WString informationContent="";

    WModelIndexSet selectedRows=tableView->selectedIndexes();

    if (!selectedRows.empty())
    {
        WModelIndex index=*selectedRows.begin();
        Wt::Dbo::ptr<yasArchiveEntry> entry=dbQuery->stableResultRow(index.row());

        // Create link to folder containg file
        std::string htmlPath=entry->path;
        htmlPath=std::string("file:///")+entry->path;;
        boost::replace_all(htmlPath, "#", "%23");

        // Show the path location
        informationContent+="<table><tr><td>Folder&nbsp;&nbsp;</td><td><span style=\"color: #580F8B;\"><strong><a href=\""+WString(htmlPath)+"\" target=\"_blank\">"+WString(entry->path)+"</a></strong></span></td></tr>";
        informationContent+="<tr><td>File&nbsp;&nbsp;</td><td><span style=\"color: #580F8B; margin-bottom: 8px;\"><strong>"+WString(entry->filename)+"</strong></span></td></tr></table>";

        // Show information about acquisition / write time
        WString writeTimeString=WString(ctime(&entry->writeTime));
        informationContent+="<p style=\"margin-top: 6px;\">Acquired on <span style=\"color: #580F8B;\">"+WString(entry->acquisitionDate)+" "+WString(entry->acquisitionTime)+"</span>, archived on <span style=\"color: #580F8B;\">"+writeTimeString+"</span>";

        // If additional information from Yarra task file is available, show it
        if (!entry->MRSystem.empty())
        {
            informationContent+="<br /><span class=\"label label-warning\">Task</span>&nbsp; Submitted from <span style=\"color: #580F8B;\">"+WString(entry->MRSystem)+"</span>";

            if (!entry->yarraServer.empty())
            {
                informationContent+=" to server <span style=\"color: #580F8B;\">"+WString(entry->yarraServer)+"</span>";
            }
        }
        informationContent+="</p>";
    }

    informationText->setText(informationContent);
}


