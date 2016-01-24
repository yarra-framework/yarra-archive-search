#ifndef YAS_INDEXER_H
#define YAS_INDEXER_H

#include "../common/yas_configuration.h"

class yasIndexer
{
public:
    enum IndexAction {
        SHOWHELP=0,
        RUNINDEXER=1,
        CLEARINDEX=2,
        ABORT=3
    };

    yasIndexer(int argc, char* argv[]);

    void perform();
    void showHelp();
    void runIndexer();
    void clearIndex();
    int getReturnValue();

    yasConfiguration configuration;

    IndexAction action;
    int returnValue;
};


inline int yasIndexer::getReturnValue()
{
    return returnValue;
}


#endif // YAS_INDEXER_H

