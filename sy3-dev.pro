QT += quick core gui sql multimedia
#QT += widgets network websockets virtualkeyboard
QT += widgets network websockets
requires(qtConfig(tablewidget))

CONFIG += c++11 c++14 c++17 c++20
CONFIG(debug,debug|release){
    # debug mode use local file
    win32{
        path=$$system("cd")
        path ~=s,\\\\,/,g
    } else {
        path=$$system("pwd")
    }

    DEFINES += MainImageResourcePath=\\\"file:///$${path}/Resource/\\\"
} else {
    # release mode use qrc file

    # release mode set importPath with 'qrc:///'

    DEFINES += MainImageResourcePath=\\\"qrc:/Resource/\\\"
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += NUMCPP_NO_USE_BOOST

DISTFILES += \
    qml/sy3-dev/*



# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$PWD/TaoQuick/imports/imports.pri)

SOURCES += \
Comm/baseclient.cpp \
    Comm/devserver.cpp \
    Comm/realtimemonclient.cpp \
    DBModels/dbmodelbase.cpp \
    DeviceControl/bedcontrol.cpp \
    DeviceControl/cameracontrol.cpp \
    DeviceControl/robotcontrol.cpp \
    MultiTask/cameragetdataworker.cpp \
    MultiTask/coreprocedureworker.cpp \
    Other/Logger.cpp \
    Other/ImageProvider.cpp \
    Other/ShowImage.cpp \
    Other/frameprovider.cpp \
    Other/myfile.cpp \
    Other/mysignalspy.cpp \
    global.cpp \
    maincontrol.cpp \
    guiinterface.cpp \
    main.cpp

RESOURCES += \
    qml.qrc \
    res.qrc



# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH +=$$PWD/TaoQuick/imports/TaoQuick
#QML_IMPORT_PATH =
# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Comm/baseclient.h \
    Comm/devserver.h \
    Comm/realtimemonclient.h \
    DBModels/dbmodelbase.h \
    DBModels/dbmodels.h \
    DeviceControl/bedcontrol.h \
    DeviceControl/cameracontrol.h \
    DeviceControl/robotcontrol.h \
    MultiTask/cameragetdataworker.h \
    MultiTask/coreprocedureworker.h \
    Other/Logger.h \
    Other/LoggerTemplate.h \
    Other/ImageProvider.h \
    Other/ShowImage.h \
    Other/frameprovider.h \
    Other/myfile.h \
    Other/mysignalspy.h \
    global.h \
    maincontrol.h \
    guiinterface.h \
    pubdef.h

FORMS +=

QMAKE_LFLAGS += -Wl,--rpath,./lib
QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-unused-variable -Wno-unused-parameter

INCLUDEPATH += /usr/local/include/opencv4 \
    /usr/local/include/NumCpp \
    $$PWD/include

DEPENDPATH += /usr/local/include/opencv4 \
    /usr/local/include/NumCpp \
    $$PWD/include

LIBS += -L/usr/local/lib/ -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_calib3d -lopencv_imgcodecs
LIBS += -L$$PWD/lib/ $$PWD/lib/*

#OPEN3D
INCLUDEPATH += /usr/local/open3d/include/open3d/3rdparty \
    /usr/local/open3d/include
DEPENDPATH += /usr/local/open3d/include/open3d/3rdparty \
    /usr/local/open3d/include
LIBS += /usr/local/open3d/lib/*.a
LIBS += -fopenmp -ldl -lX11 -lstdc++fs -lGL -lpthread -lflann -lstdc++
LIBS += -L/usr/lib/llvm-10/lib -lc++ -lc++abi
