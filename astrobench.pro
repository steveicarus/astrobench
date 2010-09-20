
CONFIG += qt
#CONFIG += debug

# On Mac OS X, we rely on a fink installation for some libraries.
macx {
  INCLUDEPATH += /sw/include /sw/include/glib-2.0 /sw/lib/glib-2.0/include
  LIBS += -L/sw/lib -lvipsCC
}
unix {
  LIBS +=
}

SOURCES += main.cpp

FORMS += astrobench.ui
HEADERS += AstrobenchMain.h
SOURCES += AstrobenchMain.cpp

TARGET = astrobench

