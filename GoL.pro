QT += quick
CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    GameView/gameview.cpp \
    GameView/mainwindow.cpp \
    Utilities/rleparser.cpp \
    GameLogic/gamemodel.cpp \
    GameLogic/src/gamecontrollerimpl.cpp \
    GameLogic/src/cpulifeprocessor.cpp

RESOURCES += qml.qrc \
             patterns.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    GameView/gameview.h \
    GameView/mainwindow.h \
    Utilities/rleparser.h \
    Utilities/qtutilities.h \
    GameLogic/gamecontroller.h \
    GameLogic/gamemodel.h \
    GameLogic/src/gamecontrollerimpl.h \
    GameLogic/src/patterns.h \
    GameLogic/src/gpulifeprocessor.h \
    GameLogic/src/cpulifeprocessor.h

macx|ios {
#    QMAKE_OBJECTIVE_CFLAGS += -fobjc-arc
#    QMAKE_OBJECTIVE_CXXFLAGS += -fobjc-arc
    LIBS += -framework Foundation -framework Metal -framework MetalKit
    OBJECTIVE_SOURCES += GameLogic/src/metallifeprocessor.mm
}
else:unix|win32 {
    LIBS += -lOpenCL
    SOURCES += GameLogic/src/opencllifeprocessor.cpp
}
