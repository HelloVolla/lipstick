include(../common.pri)
TARGET = ut_screenlock
QT += dbus qml quick

INCLUDEPATH += $$SCREENLOCKSRCDIR $$UTILITYSRCDIR $$TOUCHSCREENSRCDIR $$SRCDIR/xtools $$DEVICESTATE

SOURCES += ut_screenlock.cpp \
    $$SCREENLOCKSRCDIR/screenlock.cpp \
    $$TOUCHSCREENSRCDIR/touchscreen.cpp \
    $$STUBSDIR/homeapplication.cpp \
    $$STUBSDIR/stubbase.cpp

HEADERS += ut_screenlock.h \
    $$SCREENLOCKSRCDIR/screenlock.h \
    $$TOUCHSCREENSRCDIR/touchscreen.h \
    $$DEVICESTATE/displaystate.h \
    $$SRCDIR/homeapplication.h \
    $$UTILITYSRCDIR/closeeventeater.h
