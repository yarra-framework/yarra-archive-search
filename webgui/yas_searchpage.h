#ifndef YAS_SEARCHPAGE_H
#define YAS_SEARCHPAGE_H

#include "../common/yas_archiveentry.h"

#include <Wt/WTableView>
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/QueryModel>
#include <Wt/WContainerWidget>

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

    Wt::Dbo::backend::Sqlite3 dbBackend;
    Wt::Dbo::Session          dbSession;
    Wt::Dbo::QueryModel<Wt::Dbo::ptr<yasArchiveEntry>>* dbQuery;

    saTableView* tableView;
    WLineEdit*   searchEdit;
    WText*       informationText;

    yasApplication* app;

    void performSearch();
    void showInformation();
    void updateTooltip();
};


#endif // YAS_SEARCHPAGE_H

