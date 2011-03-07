
CONFIG += qt
CONFIG += debug

# On Mac OS X, we rely on a fink installation for some libraries.
macx {
  INCLUDEPATH += /sw/include /sw/include/glib-2.0 /sw/lib/glib-2.0/include
  LIBS += -L/sw/lib -lvipsCC -lvips
}
unix {
  LIBS +=
}

SOURCES += main.cpp

# Describe the main window
FORMS += astrobench.ui
HEADERS += AstrobenchMain.h
SOURCES += AstrobenchMain.cpp

# Describe the StackItem widget
FORMS += stackitem.ui
HEADERS += StackItemWidget.h
SOURCES += StackItemWidget.cpp

TARGET = astrobench
