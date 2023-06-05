#include "yas_application.h"
#include "yas_searchpage.h"

#include <Wt/WNavigationBar.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WOverlayLoadingIndicator.h>
#include <Wt/WPanel.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTextArea.h>
#include <Wt/WString.h>
#include <Wt/WText.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WMenu.h>


yasApplication::yasApplication(const WEnvironment& env)
    : WApplication(env)
{
    pwdEdit=0;
    pwdContainer=0;
    rightMenu=0;
    layout_main=0;
    navbar=0;

    setLoadingIndicator(std::make_unique<Wt::WOverlayLoadingIndicator>());

    auto bootstrapTheme = std::make_shared<Wt::WBootstrapTheme>();
    bootstrapTheme->setVersion(Wt::WBootstrapTheme::Version::v3);
    bootstrapTheme->setResponsive(true);
    setTheme(bootstrapTheme);
    useStyleSheet("style/bootstrap.min.css");
    useStyleSheet("style/yarra.css");
}


void yasApplication::prepare(yasConfiguration* configInstance)
{
    setTitle("Yarra Archive Search");
    configuration=configInstance;

    auto layout = root()->setLayout(std::make_unique<Wt::WVBoxLayout>());
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    navbar = layout->addWidget(std::make_unique<WNavigationBar>());
    navbar->setTitle("Yarra Archive Search");
    navbar->setResponsive(true);
    navbar->addStyleClass("main-nav");
    navbar->setMargin(0,Wt::Side::Bottom);

    rightMenu = navbar->addMenu(std::make_unique<Wt::WMenu>(), Wt::AlignmentFlag::Right);

    auto container = layout->addWidget(std::make_unique<WContainerWidget>(),1);
    layout_main=container->setLayout(std::make_unique<Wt::WVBoxLayout>());
    layout_main->setContentsMargins(0, 0, 0, 0);

    //container->decorationStyle().setBackgroundColor(WColor(200,200,200));



    createLoginPage();
}


void yasApplication::createLoginPage()
{
    layout_main->clear();

    // Remove the "Logout" menu entry
    for (int i=0; rightMenu->count(); i++)
    {
        rightMenu->removeItem(rightMenu->itemAt(0));
    }

    WContainerWidget* panelcontainer=new WContainerWidget();
    panelcontainer->resize(420, 200);

    Wt::WPanel *panel=new Wt::WPanel(panelcontainer);
    panel->addStyleClass("centered");
    panel->setTitle("Authentication Required");
    panel->addStyleClass("modal-content");
    panel->addStyleClass("panel-primary");

    // TODO: Define CSS class for background color
    WContainerWidget* haligncontainer=new WContainerWidget();
    Wt::WHBoxLayout *halignlayout=new Wt::WHBoxLayout();
    halignlayout->setContentsMargins(0, 0, 0, 0);
    haligncontainer->setLayout(halignlayout);

    WContainerWidget* dummyContainer1=new WContainerWidget();
    WContainerWidget* dummyContainer2=new WContainerWidget();

    layout_main->addWidget(dummyContainer1,1);
    layout_main->addWidget(haligncontainer);
    layout_main->addWidget(dummyContainer2,1);

    halignlayout->addWidget(new WContainerWidget(),1);
    halignlayout->addWidget(panelcontainer);
    halignlayout->addWidget(new WContainerWidget(),1);

    WContainerWidget* innercontainer=new WContainerWidget();
    innercontainer->addStyleClass("form-group");

    Wt::WVBoxLayout *innerlayout=new Wt::WVBoxLayout();
    innerlayout->setContentsMargins(0, 0, 0, 0);
    innerlayout->setSpacing(0);

    pwdContainer=new WContainerWidget();

    Wt::WLabel* pwdLabel=new Wt::WLabel("Password: ",pwdContainer);
    pwdLabel->setMargin(2,Wt::Bottom);
    pwdLabel->setMargin(10,Wt::Top);
    pwdLabel->addStyleClass("control-label");

    pwdEdit=new Wt::WLineEdit(pwdContainer);
    pwdEdit->setEchoMode(Wt::WLineEdit::Password);
    pwdEdit->enterPressed().connect(this, &yasApplication::performLogin);
    pwdLabel->setBuddy(pwdEdit);

    Wt::WPushButton* loginButton=new Wt::WPushButton("Login");
    loginButton->setStyleClass("btn-primary");
    loginButton->setMargin(18,Wt::Top);
    loginButton->setMaximumSize(70,Wt::WLength::Auto);
    loginButton->clicked().connect(this, &yasApplication::performLogin);
    loginButton->setDefault(true);

    innerlayout->addWidget(pwdContainer);
    innerlayout->addStretch(1);
    innerlayout->addWidget(loginButton,0,Wt::AlignRight);

    innercontainer->setLayout(innerlayout);
    panel->setCentralWidget(innercontainer);

    pwdEdit->setFocus();
}


void yasApplication::createSearchPage()
{
    layout_main->clear();
    pwdEdit=0;
    pwdContainer=0;
    rightMenu->addItem("Logout")->triggered().connect(this, &yasApplication::performLogout);

    layout_main->addWidget(new yasSearchPage(this),1);
}


void yasApplication::performLogin()
{
    if ((!pwdEdit) || (!pwdContainer))
    {
        return;
    }

    WString loginPassword=pwdEdit->text();

    bool passwordValid=(loginPassword==configuration->webgui_password);

    if (!passwordValid)
    {
        // Login information is not correct
        pwdContainer->addStyleClass("has-error");
        pwdEdit->setText("");
        pwdEdit->setFocus();
    }
    else
    {
        createSearchPage();
    }
}


void yasApplication::performLogout()
{
    createLoginPage();
}
