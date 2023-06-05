#include "yas_searchpage.h"
#include "yas_application.h"

#include <boost/algorithm/string/replace.hpp>

#include <Wt/WPanel.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WMemoryResource.h>
#include <Wt/WMessageBox.h>

#include <ctime>


using namespace Wt;


saTableView::saTableView(WContainerWidget *parent)
    : WTableView(parent)
{
    setLayoutSizeAware(true);
    prepared=false;
}


void saTableView::layoutSizeChanged(int width, int)
{
    if (prepared)
    {
        int fullWidth=width-50;

        if (fullWidth<1150)
        {
            fullWidth=1150;
        }

        setColumnWidth(0, int(fullWidth*0.28));
        setColumnWidth(1, int(fullWidth*0.11));
        setColumnWidth(2, int(fullWidth*0.11));
        setColumnWidth(3, int(fullWidth*0.05));
        setColumnWidth(4, int(fullWidth*0.05));
        setColumnWidth(5, int(fullWidth*0.11));
        setColumnWidth(6, int(fullWidth*0.28));
    }
}



yasSearchPage::yasSearchPage(yasApplication* parent)
    : WContainerWidget())
{
    app=parent;

    try
    {
        auto dbBackend=std::make_unique<Wt::Dbo::backend::Sqlite3>(parent->configuration->db_connection.toUTF8());

        // Set database to write-ahead-logging to avoid locking in the WebGUI while the indexer is running
        dbBackend->executeSql("PRAGMA journal_mode=WAL;");

        dbSession = std::make_unique<Wt::Dbo::Session>();
        dbSession->setConnection(std::move(dbBackend));

        // Map the database table
        dbSession->mapClass<yasArchiveEntry>("yasArchive");
    }
    catch (const Wt::Dbo::Exception & e)
    {
        std::cout << "ERROR: Unable to open database" << std::endl;
        return;
    }


    // Create the database in case that it does not exist yet
    try
    {
        dbSession->createTables();
    }
    catch (const Wt::Dbo::Exception & e)
    {
    }


    dbQuery = std::make_unique<Wt::Dbo::QueryModel<Wt::Dbo::ptr<yasArchiveEntry>>>();
    dbQuery->setQuery(dbSession->find<yasArchiveEntry>());

    dbQuery->addColumn("PatientName",     "Patient Name");
    dbQuery->addColumn("PatientID",       "MRN");
    dbQuery->addColumn("AccessionNumber", "ACC");
    dbQuery->addColumn("PatientAge",      "Age");
    dbQuery->addColumn("PatientGender",   "Gender");
    dbQuery->addColumn("AcquisitionDate", "Exam Date");
    dbQuery->addColumn("ProtocolName",    "Protocol");

    tableView=new saTableView();
    tableView->setSelectionMode(SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->setModel(std::move(dbQuery));

    tableView->setHeaderHeight(32);
    tableView->setRowHeight(28);
    tableView->setColumnWidth(0, 350);
    tableView->setColumnWidth(1, 150);   
    tableView->setColumnWidth(2, 150);
    tableView->setColumnWidth(3, 50);
    tableView->setColumnWidth(4, 50);
    tableView->setColumnWidth(5, 150);
    tableView->setColumnWidth(6, 350);
    tableView->setHeaderAlignment(0, Wt::AlignCenter);
    tableView->setHeaderAlignment(1, Wt::AlignCenter);
    tableView->setHeaderAlignment(2, Wt::AlignCenter);
    tableView->setHeaderAlignment(3, Wt::AlignCenter);
    tableView->setHeaderAlignment(4, Wt::AlignCenter);
    tableView->setHeaderAlignment(5, Wt::AlignCenter);
    tableView->setHeaderAlignment(6, Wt::AlignCenter);

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

    Wt::WPopupMenu* popupMenu = new Wt::WPopupMenu();
    popupMenu->addItem("Information...")->triggered().connect(this, &yasSearchPage::showStatistics);
    popupMenu->addItem("Export as CSV...")->triggered().connect(this, &yasSearchPage::exportCSV);
    popupMenu->addSeparator();
    popupMenu->addItem("YAS Version " + WString(YAS_VERSION_WEBGUI));

    WPushButton* menuButton=new WPushButton();
    menuButton->setStyleClass("btn-default");
    menuButton->setMenu(popupMenu);

    searchLayout->addWidget(searchEdit,1);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(menuButton);
    pageLayout->addWidget(searchWidgets);

    pageLayout->addWidget(tableView,1);

    WPanel* infoPanel=new WPanel();
    infoPanel->setTitleBar(true);
    infoPanel->setTitle("&nbsp;&nbsp;Information");
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
    updateTooltip();

    // Table columns have been setup, resizing is possible now.
    tableView->prepared=true;
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
        if (!searchPhrase.empty())
        {
            searchPhrases.push_back(searchPhrase);
        }
    }

    if (searchPhrases.empty())
    {
        dbQuery->setQuery(dbSession->find<yasArchiveEntry>(), true);
    }
    else
    {
        auto query=dbSession->query<Wt::Dbo::ptr<yasArchiveEntry>>("select m from yasArchive m");

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

    updateTooltip();
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

        if (!entry->patientID.empty())
        {
            informationContent+="<p style=\"margin-bottom: 0;\">MRN = <span style=\"color: #580F8B;\">"+WString(entry->patientID)+"</span>";

            if (!entry->accessionNumber.empty())
            {
                informationContent+="&nbsp;&nbsp;ACC = <span style=\"color: #580F8B;\">"+WString(entry->accessionNumber)+"</span>";
            }
            informationContent+="</p>";
        }
    }

    informationText->setText(informationContent);
}


void yasSearchPage::updateTooltip()
{
    WString toolTipText=WString("{1} entries found in archive.").arg(dbQuery->rowCount());

    if (dbQuery->rowCount()==1)
    {
        toolTipText="1 entry found in archive.";
    }

    tableView->setToolTip(toolTipText);
}


void yasSearchPage::showStatistics()
{
    WString messageText=WString("{1} entries found in archive.").arg(dbQuery->rowCount());

    if (dbQuery->rowCount()==1)
    {
        messageText="1 entry found in archive.";
    }

    Wt::WMessageBox* messageBox=new Wt::WMessageBox("Search Result", messageText, Wt::Information, Wt::Ok);
    messageBox->setModal(true);
    messageBox->buttonClicked().connect(std::bind([=] ()
    {
        delete messageBox;
    }));
    messageBox->show();
}


void yasSearchPage::exportCSV()
{
    std::stringstream csv;

    for (int j=0; j<dbQuery->columnCount(); j++)
    {
           csv << dbQuery->fieldName(j) << ",";
    }
    csv << "Filename" << ",";
    csv << "\r\n";
    csv << "\r\n";

    for (int i=0; i<dbQuery->rowCount(); i++)
    {
        Wt::Dbo::ptr<yasArchiveEntry> entry=dbQuery->stableResultRow(i);

        for (int j=0; j<dbQuery->columnCount(); j++)
        {
            std::string s = Wt::asString(dbQuery->data(i,j)).toUTF8();
            boost::replace_all(s, "\"", "\"\"");
            csv << "\"" << s << "\",";
        }

        // Additionally store the path and filename (not shown in the table)
        std::string s = Wt::asString(entry->path + "/" + entry->filename).toUTF8();
        boost::replace_all(s, "\"", "\"\"");
        csv << "\"" << s << "\",";

        csv << "\r\n";
    }

    std::string csvStr = csv.str();
    std::vector<unsigned char> csvData(csvStr.begin(), csvStr.end());

    Wt::WMemoryResource *csvResource = new Wt::WMemoryResource("text/csv", this);
    csvResource->setData(csvData);
    csvResource->suggestFileName("search.csv");
    WApplication::instance()->redirect(csvResource->url());
}
