# ----------------------------------------------------
# 这是工程配置文件，告诉编译器需要加载哪些模块
# ----------------------------------------------------

QT       += core gui
QT       += charts  # <---【关键】这一行绝对不能少！

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 忽略一些过时警告，让编译更清爽
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    containercard.cpp \
    containerdetaildialog.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    containercard.h \
    containerdetaildialog.h \
    logindialog.h \
    mainwindow.h

# 部署规则 (默认即可)
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
