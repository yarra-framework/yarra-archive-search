#ifndef YAS_SEARCHPAGE_H
#define YAS_SEARCHPAGE_H

#include "../common/yas_archiveentry.h"

#include <Wt/WTableView.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/QueryModel.h>
#include <Wt/WContainerWidget.h>

using namespace Wt;


class yasApplication;

// Size-aware table view that expands the columns to cover the whole table
// after resizing the window
class saTableView : public WTableView
{
public:
    bool prepared;

    saTableView(WContainerWidget *parent=0);
    void layoutSizeChanged(int width, int height);
};


class yasSearchPage : public WContainerWidget
{
public:
    yasSearchPage(yasApplication* parent);

    // Wt::Dbo::backend::Sqlite3 dbBackend;
    std::unique_ptr<Wt::Dbo::Session> dbSession;
    std::unique_ptr<Wt::Dbo::QueryModel<Wt::Dbo::ptr<yasArchiveEntry>>> dbQuery;

    saTableView* tableView;
    WLineEdit*   searchEdit;
    WText*       informationText;

    yasApplication* app;

    void performSearch();
    void showInformation();
    void updateTooltip();

    void showStatistics();
    void exportCSV();

};


#endif // YAS_SEARCHPAGE_H

