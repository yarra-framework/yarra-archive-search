#include <functional>
#include <iostream>

#include <Wt/WApplication>

#include "yas_application.h"


// The global configuration instance
yasConfiguration configurationInstance;


WApplication* createApplication(const WEnvironment& env)
{
    yasApplication* app=new yasApplication(env);
    app->prepare(&configurationInstance);

    return (WApplication*) app;
}


int main(int argc, char **argv)
{
    std::cout << std::endl;

    configurationInstance.loadConfiguration();

    if (!configurationInstance.isValid())
    {
        std::cout << "ERROR: Reading configuration failed" << std::endl;
        std::cout << "ERROR: Unable to start the YAS WebGUI" << std::endl << std::endl;
        return 1;
    }

    // Prepare the arguments for calling the WRun commands
    char arg1[] = "--docroot=""html""";
    char arg2[] = "--http-address";
    char arg3[] = "0.0.0.0";
    char arg4[] = "--http-port";

    // Overwrite the port number with the setting from the configuration file
    char arg5[8] = "    ";
    strcpy(arg5,configurationInstance.webgui_port.toUTF8().data());

    // Pass path to local configuration file (needed to set maximum allowed upload size)
    char arg6[] = "--config=wt_config.xml";

    int argCount=7;
    char* args[7];
    args[0]=argv[0];
    args[1]=arg1;
    args[2]=arg2;
    args[3]=arg3;
    args[4]=arg4;
    args[5]=arg5;
    args[6]=arg6;

    return WRun(argCount, args, &createApplication);
}
