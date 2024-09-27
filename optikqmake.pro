
WINDOWS_OPENCV_INCLUDEPATH = C:\opencv\build\include
WINDOWS_OPENCV_LIBPATH = C:\opencv\build\install\x64\mingw\bin
WINDOWS_OPENCV_LIBS = -lopencv_core490 -lopencv_highgui490 -lopencv_imgcodecs490 -lopencv_dnn490 -lopencv_imgproc490 -lopencv_ml490 -lopencv_photo490 -lopencv_video490 -lopencv_objdetect490

ANDROID_OPENCV_INCLUDEPATH = C:\OpenCV-android-sdk\sdk\native\jni\include

ANDROID_OPENCV_LIBPATH_BASE = C:\OpenCV-android-sdk\sdk\native\libs
ANDROID_OPENCV_LIBS = -lopencv_java4

android {
    INCLUDEPATH += $$ANDROID_OPENCV_INCLUDEPATH

    # farklı türde android mimarileri için
    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
        ANDROID_OPENCV_LIBPATH = $$ANDROID_OPENCV_LIBPATH_BASE/armeabi-v7a
    } else:contains(ANDROID_TARGET_ARCH, arm64-v8a) {
        ANDROID_OPENCV_LIBPATH = $$ANDROID_OPENCV_LIBPATH_BASE/arm64-v8a
    } else:contains(ANDROID_TARGET_ARCH, x86) {
        ANDROID_OPENCV_LIBPATH = $$ANDROID_OPENCV_LIBPATH_BASE/x86
    } else:contains(ANDROID_TARGET_ARCH, x86_64) {
        ANDROID_OPENCV_LIBPATH = $$ANDROID_OPENCV_LIBPATH_BASE/x86_64
    } else {
        error("Unsupported architecture: $$ANDROID_TARGET_ARCH")
    }

    LIBS += -L$$ANDROID_OPENCV_LIBPATH $$ANDROID_OPENCV_LIBS

    # android opencv kütüphanesi
    ANDROID_EXTRA_LIBS += $$ANDROID_OPENCV_LIBPATH/libopencv_java4.so
}


# windows
win32 {
    INCLUDEPATH += $$WINDOWS_OPENCV_INCLUDEPATH
    LIBS += -L$$WINDOWS_OPENCV_LIBPATH $$WINDOWS_OPENCV_LIBS

    # tesseract linkage
    # INCLUDEPATH += C:\Users\melih\vcpkg\installed\x64-windows-static\include
    # LIBS += -LC:\Users\melih\vcpkg\installed\x64-windows-static\lib
    # LIBS += -lleptonica-1.84.1
    # LIBS += -ltesseract54
}

QT       += core gui
QT       += multimediawidgets
QT       += multimedia
QT       += core-private
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20
QMAKE_LFLAGS += -v

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    camera.cpp \
    database.cpp \
    gallery.cpp \
    main.cpp \
    mainwindow.cpp \
    opencv.cpp

HEADERS += \
    camera.h \
    database.h \
    gallery.h \
    mainwindow.h \
    opencv.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES +=
