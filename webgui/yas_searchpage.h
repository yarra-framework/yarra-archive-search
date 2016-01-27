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


class yasSearchPage : public WContainerWidget
{
public:
    yasSearchPage(yasApplication* parent);

    Wt::Dbo::backend::Sqlite3 dbBackend;
    Wt::Dbo::Session          dbSession;
    Wt::Dbo::QueryModel<Wt::Dbo::ptr<yasArchiveEntry>>* dbQuery;

    WTableView* tableView;
    WLineEdit*  searchEdit;
    WText*      informationText;

    yasApplication* app;

    void performSearch();
    void showInformation();

};


#endif // YAS_SEARCHPAGE_H

