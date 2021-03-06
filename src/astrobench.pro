
CONFIG += qt
CONFIG += debug

# On Mac OS X, we rely on a fink installation for some libraries.
macx {
  INCLUDEPATH += /sw/include /sw/include/glib-2.0 /sw/lib/glib-2.0/include
  LIBS += -L/sw/lib -lvipsCC -lvips
}
unix {
  INCLUDEPATH += /usr/include/glib-2.0 /usr/lib64/glib-2.0/include
  LIBS += -lvipsCC -lvips -lpthread
}

SOURCES += main.cpp

# Describe the main window
FORMS += astrobench.ui
HEADERS += AstrobenchMain.h
SOURCES += AstrobenchMain.cpp AstrobenchMain_proj.cpp proc_tone_map.cpp

# Describe the StackItem widget
FORMS += stackitem.ui
HEADERS += StackItemWidget.h
SOURCES += StackItemWidget.cpp

TARGET = astrobench
