#-------------------------------------------------
#
# Project created by QtCreator 2019-01-23T17:15:35
#
#-------------------------------------------------

QT += core gui
QT += 3dextras
QT += 3dcore 3drender 3dinput 3dextras multimedia
QT += widgets

TARGET = WebCam
TEMPLATE = app

#INCLUDEPATH += "D:/Dev/OpenCV/opencv/OpenCV-MinGW/include"

#LIBS += L"D:/Dev/OpenCV/opencv/OpenCV-MinGW/x64/mingw/bin" \
#        -lopencv_core400        \
#        -lopencv_highgui400     \
#        -lopencv_imgcodecs400   \
#        -lopencv_imgproc400     \
#        -lopencv_features2d400  \
#        -lopencv_calib3d400

#LIBS += L"D:/Dev/OpenCV/opencv/OpenCV-MinGW/x64/mingw/lib" \
#        -lopencv_core400.dll        \
#        -lopencv_highgui400.dll     \
#        -lopencv_imgcodecs400.dll   \
#        -lopencv_imgproc400.dll     \
#        -lopencv_features2d400.dll  \
#        -lopencv_calib3d400.dll

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11 c++17
#QMAKE_CXXFLAGS += -Wall -Werror

SOURCES += \
        main.cpp \
        MainWindow.cpp \
    Viwe3dWidget.cpp \
    CaptureCamera.cpp \
    PrivateCapture.cpp

HEADERS += \
        MainWindow.h \
    Viwe3dWidget.h \
    CaptureCamera.h \
    PrivateCapture.h \
    nlohmann/*.hpp \
    nlohmann/detail/*.hpp \
    nlohmann/detail/conversions/*.hpp \
    nlohmann/detail/input/*.hpp \
    nlohmann/detail/iterators/*.hpp \
    nlohmann/detail/output/*.hpp

FORMS += \
        MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    LegoObject.qrc \
    Resources.qrc
