#ifndef YAS_APPLICATION_H
#define YAS_APPLICATION_H

#include <Wt/WApplication.h>
#include <Wt/WString.h>
#include <Wt/WVBoxLayout.h>

#include "../common/yas_configuration.h"

using namespace Wt;


class yasApplication : public WApplication
{
public:
    yasApplication(const WEnvironment& env);

    void prepare(yasConfiguration* configInstance);

    WVBoxLayout* layout_main;
    WLineEdit* pwdEdit;
    WContainerWidget* pwdContainer;
    WNavigationBar* navbar;
    WMenu* rightMenu;

    yasConfiguration* configuration;

    void createLoginPage();
    void createSearchPage();

    void performLogin();
    void performLogout();

};


#endif // YAS_APPLICATION_H

