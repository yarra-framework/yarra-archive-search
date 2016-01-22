#include "yas_searchpage.h"
#include "yas_application.h"

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
    tableView->clicked().connect(this, &yasSearchPage::showInformation);

    WVBoxLayout* pageLayout=new WVBoxLayout();
    pageLayout->setContentsMargins(30, 30, 30, 10);
    pageLayout->setSpacing(10);
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
    infoPanel->setMinimumSize(Wt::WLength::Auto,150);

    informationText=new WText("");
    infoPanel->setCentralWidget(informationText);
    pageLayout->addWidget(infoPanel);

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

}


void yasSearchPage::showInformation(const WModelIndex& index)
{
    WString informationContent="";

    if (index.isValid())
    {
        Wt::Dbo::ptr<yasArchiveEntry> entry=dbQuery->stableResultRow(index.row());

        // Show the path location
        informationContent+=WString(entry->path) + "/" + WString(entry->filename);

        WString writeTimeString=WString(ctime(&entry->writeTime));

        informationContent+="<br />Acquired on " + WString(entry->acquisitionDate) + " " + WString(entry->acquisitionTime) + ", archived on " + writeTimeString;

        // If information from a Yarra task file is available, show it
        if (!entry->yarraServer.empty())
        {
            informationContent+="<br />Submitted from " + WString(entry->MRSystem) + " to YarraServer " + WString(entry->yarraServer);
        }

        return;
    }

    informationText->setText(informationContent);
}


