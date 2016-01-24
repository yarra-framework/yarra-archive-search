#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <chrono>

#include "yas_indexer.h"


int main(int argc, char* argv[])
{
    //Speed up report output
    std::ios_base::sync_with_stdio(false);

    yasIndexer indexer(argc, argv);
    indexer.perform();
    return indexer.getReturnValue();
}
