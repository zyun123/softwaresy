QT += quick core gui sql
QT += widgets network
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

#开启日志记录
 DEFINES += USE_TAO_LOG
#开启测试子线程
#DEFINES += USE_TEST_THREAD
#打印机械臂解包信息
#DEFINES += PRINT_ROBOT_INFO
#切换离线和实机模式
DEFINES += LINK_REAL_ROBOT

DISTFILES += \
    qml/qtDemo/*



# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$PWD/TaoQuick/imports/imports.pri)
include($$PWD/TaoCommon/TaoCommon.pri)
include($$PWD/xlsx/qtxlsx.pri)


SOURCES += \
    DeviceAddTable/ChoiseCaseItem.cpp \
    DeviceAddTable/ChoiseCaseModel.cpp \
    DeviceAddTable/ChoiseCasePlanListModel.cpp \
        DeviceAddTable/DeviceAddItem.cpp \
        DeviceAddTable/DeviceAddModel.cpp \
    DeviceAddTable/DoctorManageItem.cpp \
    DeviceAddTable/DoctorManagerModel.cpp \
    DeviceAddTable/GLPatientManagerModel.cpp \
    DeviceAddTable/PatientCaseListModel.cpp \
    DeviceAddTable/PatientCaseListNewModel.cpp \
        DeviceAddTable/PatientCaseModel.cpp \
        DeviceAddTable/PatientDb.cpp \
    DeviceAddTable/PatientPresAddModel.cpp \
    DeviceAddTable/PatientTreatmentRecordModel.cpp \
    DeviceAddTable/TreatCaseManageListModel.cpp \
    DeviceAddTable/TreatCaseModel.cpp \
        DeviceAddTable/patientcaseitem.cpp \
    DeviceControl/baseclient.cpp \
    DeviceControl/bedcontrol.cpp \
    DeviceControl/calibrateworker.cpp \
    DeviceControl/cameracontrol.cpp \
    DeviceControl/cameragetdataworker.cpp \
    DeviceControl/cureworker.cpp \
    DeviceControl/devserver.cpp \
    DeviceControl/identifypointsdata.cpp \
    DeviceControl/identifyworker.cpp \
    DeviceControl/maincontrol.cpp \
    DeviceControl/realtimemonclient.cpp \
    DeviceControl/robotcontrol.cpp \
    DeviceControl/secmonclient.cpp \
    DeviceControl/testworker.cpp \
    Patient/ImageProvider.cpp \
    Patient/ShowImage.cpp \
    comconqml.cpp \
    global.cpp \
        main.cpp \
    myfile.cpp \


RESOURCES += qml.qrc



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
    DeviceAddTable/ChoiseCaseItem.h \
    DeviceAddTable/ChoiseCaseModel.h \
    DeviceAddTable/ChoiseCasePlanListModel.h \
    DeviceAddTable/DeviceAddItem.h \
    DeviceAddTable/DeviceAddModel.h \
    DeviceAddTable/DoctorManageItem.h \
    DeviceAddTable/DoctorManagerModel.h \
    DeviceAddTable/GLPatientManagerModel.h \
    DeviceAddTable/PatientCaseListModel.h \
    DeviceAddTable/PatientCaseListNewModel.h \
    DeviceAddTable/PatientCaseModel.h \
    DeviceAddTable/PatientPresAddModel.h \
    DeviceAddTable/PatientTreatmentRecordModel.h \
    DeviceAddTable/TreatCaseManageListModel.h \
    DeviceAddTable/TreatCaseModel.h \
    DeviceAddTable/patientcaseitem.h \
    DeviceControl/baseclient.h \
    DeviceControl/bedcontrol.h \
    DeviceControl/calibrateworker.h \
    DeviceControl/cameracontrol.h \
    DeviceControl/cameragetdataworker.h \
    DeviceControl/cureworker.h \
    DeviceControl/devserver.h \
    DeviceControl/identifypointsdata.h \
    DeviceControl/identifyworker.h \
    DeviceControl/maincontrol.h \
    DeviceControl/pubdef.h \
    DeviceControl/realtimemonclient.h \
    DeviceControl/robotcontrol.h \
    DeviceControl/secmonclient.h \
    DeviceControl/testworker.h \
    Patient/ImageProvider.h \
    Patient/ShowImage.h \
    PatientDb.h \
    comconqml.h \
    global.h \
    myfile.h


FORMS +=

QMAKE_LFLAGS += -Wl,--rpath=./lib

INCLUDEPATH += $$PWD/../../../../../usr/local/include/spdlog
DEPENDPATH += $$PWD/../../../../../usr/local/include/spdlog

INCLUDEPATH += /usr/local/include/opencv4 \
    /usr/local/include/librealsense2 \
    /usr/local/include/NumCpp \
    $$PWD/include

DEPENDPATH += /usr/local/include/opencv4 \
    /usr/local/include/librealsense2 \
    /usr/local/include/NumCpp \
    $$PWD/include

LIBS += -L/usr/local/lib/ -lrealsense2 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_calib3d -lopencv_imgcodecs
LIBS += -L$$PWD/lib/ -lkppredict
