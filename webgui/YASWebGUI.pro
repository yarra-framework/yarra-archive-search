TARGET = YASWebGUI

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
LIBS += /usr/local/lib/libwthttp.a
LIBS += /usr/local/lib/libwt.a
LIBS += /usr/local/lib/libwtdbosqlite3.a
LIBS += /usr/local/lib/libwtdbo.a

LIBS += /usr/lib/libGraphicsMagick.a

LIBS += /usr/lib/x86_64-linux-gnu/libfreetype.a
LIBS += /usr/lib/x86_64-linux-gnu/libtiff.a
LIBS += /usr/lib/x86_64-linux-gnu/liblcms.a
LIBS += $$ICU_PATH/libicui18n.a
LIBS += $$ICU_PATH/libicuuc.a

LIBS += /usr/lib/x86_64-linux-gnu/libglib-2.0.a
LIBS += /usr/lib/x86_64-linux-gnu/libgobject-2.0.a
LIBS += /usr/lib/x86_64-linux-gnu/libpango-1.0.a
LIBS += /usr/lib/x86_64-linux-gnu/libpangoft2-1.0.a
LIBS += /usr/lib/x86_64-linux-gnu/libpng12.a
LIBS += $$ICU_PATH/libicudata.a
LIBS += /usr/lib/x86_64-linux-gnu/libgmodule-2.0.a
LIBS += /usr/lib/x86_64-linux-gnu/libfontconfig.a
LIBS += /usr/lib/x86_64-linux-gnu/libltdl.a
LIBS += /usr/lib/x86_64-linux-gnu/libxml2.a

LIBS += /usr/lib/x86_64-linux-gnu/libbz2.a
LIBS += /usr/lib/x86_64-linux-gnu/libwmflite.a
LIBS += /usr/lib/x86_64-linux-gnu/libjasper.a
LIBS += /usr/lib/x86_64-linux-gnu/libjpeg.a

LIBS += $$BOOST_PATH/libboost_thread.a
LIBS += $$BOOST_PATH/libboost_filesystem.a
LIBS += $$BOOST_PATH/libboost_signals.a
LIBS += $$BOOST_PATH/libboost_regex.a
LIBS += $$BOOST_PATH/libboost_random.a
LIBS += $$BOOST_PATH/libboost_date_time.a
LIBS += $$BOOST_PATH/libboost_program_options.a
LIBS += $$BOOST_PATH/libboost_system.a

LIBS += -lrt

LIBS += /usr/lib/x86_64-linux-gnu/libssl.a
LIBS += /usr/lib/x86_64-linux-gnu/libcrypto.a
LIBS += /usr/lib/x86_64-linux-gnu/libcrypt.a

LIBS += /usr/lib/x86_64-linux-gnu/libz.a
LIBS += /usr/lib/x86_64-linux-gnu/libm.a
LIBS += /usr/lib/gcc/x86_64-linux-gnu/4.8/libgomp.a
LIBS += /usr/lib/x86_64-linux-gnu/libX11.a
LIBS += /usr/lib/x86_64-linux-gnu/libXext.a

LIBS += /usr/lib/x86_64-linux-gnu/libxcb.a
LIBS += /usr/lib/x86_64-linux-gnu/libXau.a
LIBS += /usr/lib/x86_64-linux-gnu/libXdmcp.a

LIBS += -ldl

TEMPLATE = app

SOURCES += main.cpp \ 
    yas_application.cpp \
    ../common/yas_configuration.cpp \
    yas_searchpage.cpp

HEADERS += ../common/yas_global.h \
    yas_application.h \
    ../common/yas_configuration.h \
    yas_searchpage.h \
    ../common/yas_archiveentry.h

