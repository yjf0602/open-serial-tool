win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lqtadvanceddocking
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lqtadvanceddockingd
else:unix: LIBS += -L$$PWD/lib/ -lqtadvanceddocking

INCLUDEPATH += $$PWD/include
