#ifndef YAS_APPLICATION_H
#define YAS_APPLICATION_H

#include <Wt/WApplication>
#include <Wt/WString>
#include <Wt/WVBoxLayout>

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

