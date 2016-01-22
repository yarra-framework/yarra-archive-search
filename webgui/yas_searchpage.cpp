#include "yas_searchpage.h"
#include "yas_application.h"

#include <Wt/WPanel>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>


using namespace Wt;


yasSearchPage::yasSearchPage(yasApplication* parent)
    : WContainerWidget(), dbBackend(parent->configuration->db_name.toUTF8())
{
    app=parent;

    // Init the db connection
    dbSession.setConnection(dbBackend);

    // Map the database table
    dbSession.mapClass<yasArchiveEntry>("yasArchive");

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

    //tableView->resize(1000, 600);
    //tableView->setWidth(Wt::WLength("100%"));

    tableView->setSelectionMode(SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->setModel(dbQuery);

    tableView->setColumnWidth(0, 350);
    tableView->setColumnWidth(1, 150);
    tableView->setColumnWidth(2, 150);
    tableView->setColumnWidth(3, 150);
    tableView->setColumnWidth(4, 350);

    tableView->sortByColumn(0, AscendingOrder);
    tableView->setMinimumSize(Wt::WLength::Auto,100);

    WVBoxLayout* pageLayout=new WVBoxLayout();
    pageLayout->setContentsMargins(30, 30, 30, 10);
    pageLayout->setSpacing(10);
    this->setLayout(pageLayout);

    WContainerWidget* searchWidgets=new WContainerWidget();
    WHBoxLayout* searchLayout=new WHBoxLayout();
    searchLayout->setContentsMargins(120, 0, 120, 20);
    searchLayout->setSpacing(20);
    searchWidgets->setLayout(searchLayout);

    WLineEdit* searchEdit=new WLineEdit();
    WPushButton* searchButton=new WPushButton("Search");
    searchButton->setStyleClass("btn-primary");

    searchLayout->addWidget(searchEdit,1);
    searchLayout->addWidget(searchButton);
    pageLayout->addWidget(searchWidgets);

    pageLayout->addWidget(tableView,1);

    WPanel* infoPanel=new WPanel();
    infoPanel->setTitleBar(true);
    infoPanel->setTitle("Information");
    infoPanel->addStyleClass("panel-info");
    pageLayout->addWidget(infoPanel);
    infoPanel->setMinimumSize(Wt::WLength::Auto,150);

    searchEdit->setFocus();
}
