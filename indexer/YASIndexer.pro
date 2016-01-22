TARGET = YASIndexer

# Define identifier for Ubuntu Linux version (UBUNTU_1204 / UBUNTU_1404)
BUILD_OS=UBUNTU_1204

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

QMAKE_CXXFLAGS += -DNDEBUG -std=c++11 -DWT_DEPRECATED_3_0_0

# Statically link dependencies
LIBS += /usr/local/lib/libwt.a
LIBS += /usr/local/lib/libwtdbo.a
LIBS += /usr/local/lib/libwtdbosqlite3.a

LIBS += $$ICU_PATH/libicui18n.a
LIBS += $$ICU_PATH/libicuuc.a
LIBS += $$ICU_PATH/libicudata.a

LIBS += $$BOOST_PATH/libboost_thread.a
LIBS += $$BOOST_PATH/libboost_filesystem.a
LIBS += $$BOOST_PATH/libboost_signals.a
LIBS += $$BOOST_PATH/libboost_regex.a
LIBS += $$BOOST_PATH/libboost_random.a
LIBS += $$BOOST_PATH/libboost_date_time.a
LIBS += $$BOOST_PATH/libboost_program_options.a
LIBS += $$BOOST_PATH/libboost_system.a

LIBS += -lrt

LIBS += /usr/lib/x86_64-linux-gnu/libz.a
LIBS += /usr/lib/x86_64-linux-gnu/libm.a

LIBS += -ldl

TEMPLATE = app

SOURCES += main.cpp 

HEADERS += 


