TARGET = YASIndexer

# Define identifier for Ubuntu Linux version (UBUNTU_1204 / UBUNTU_1404)
BUILD_OS=UBUNTU_2204
equals( BUILD_OS, "UBUNTU_2204" ) {
    message( "Configuring for Ubuntu 22.04" )
    QMAKE_CXXFLAGS += -DUBUNTU_2204
    ICU_PATH=/usr/lib/x86_64-linux-gnu
    BOOST_PATH=/usr/lib/x86_64-linux-gnu
}

equals( BUILD_OS, "UBUNTU_1604" ) {
    message( "Configuring for Ubuntu 16.04" )
    QMAKE_CXXFLAGS += -DUBUNTU_1604
    ICU_PATH=/usr/lib/x86_64-linux-gnu
    BOOST_PATH=/usr/lib/x86_64-linux-gnu
}


equals( BUILD_OS, "UBUNTU_1404" ) {
    message( "Configuring for Ubuntu 14.04" )
    QMAKE_CXXFLAGS += -DUBUNTU_1404
    ICU_PATH=/usr/lib/x86_64-linux-gnu
    BOOST_PATH=/usr/lib/x86_64-linux-gnu
}

equals( BUILD_OS, "UBUNTU_1204" ) {
    message( "Configuring for Ubuntu 12.04" )
    QMAKE_CXXFLAGS += -DUBUNTU_1204
    ICU_PATH=/usr/lib
    BOOST_PATH=/usr/local/lib
}

QMAKE_CXXFLAGS += -DDEBUG -std=c++17 -DWT_DEPRECATED_4_0_0

# Statically link dependencies
LIBS += /usr/local/lib/libwt.a
LIBS += /usr/local/lib/libwtdbosqlite3.a
LIBS += /usr/local/lib/libwtdbopostgres.a
LIBS += /usr/local/lib/libwtdbo.a
# LIBS += /usr/local/lib/libwthttp.a
# LIBS += /usr/local/lib/libwt.a

LIBS += /usr/lib/x86_64-linux-gnu/libssl.a
LIBS += /usr/lib/x86_64-linux-gnu/libcrypto.a
LIBS += /usr/lib/x86_64-linux-gnu/libcrypt.a
LIBS += /usr/lib/postgresql/14/lib/libpgcommon.a
LIBS += /usr/lib/postgresql/14/lib/libpgport.a

LIBS += /usr/local/lib/libwthttp.a
LIBS += /usr/local/lib/libwt.a
LIBS += /usr/lib/x86_64-linux-gnu/libssl.a
LIBS += /usr/lib/x86_64-linux-gnu/libcrypto.a
LIBS += /usr/lib/x86_64-linux-gnu/libcrypt.a


LIBS += /usr/lib/x86_64-linux-gnu/libssl.a
LIBS += /usr/lib/x86_64-linux-gnu/libcrypto.a
LIBS += /usr/lib/x86_64-linux-gnu/libcrypt.a

LIBS += $$BOOST_PATH/libboost_thread.a
LIBS += $$BOOST_PATH/libboost_filesystem.a
# LIBS += $$BOOST_PATH/libboost_signals.a
LIBS += $$BOOST_PATH/libboost_regex.a
LIBS += $$BOOST_PATH/libboost_random.a
LIBS += $$BOOST_PATH/libboost_date_time.a
LIBS += $$BOOST_PATH/libboost_system.a

LIBS += -lrt
LIBS += -ldl
LIBS += -lstdc++fs
LIBS += -lpq
TEMPLATE = app

SOURCES += main.cpp \ 
    yas_indexer.cpp \
    ../common/yas_configuration.cpp \
    yas_twixreader.cpp

HEADERS += ../common/yas_global.h \
    yas_indexer.h \
    ../common/yas_configuration.h \
    ../common/yas_archiveentry.h \
    yas_twixreader.h

DISTFILES += \
    YAS.ini

