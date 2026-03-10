TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += C:/Aplicaciones/SFML/SFML-2.6.2/include

LIBS += -LC:/Aplicaciones/SFML/SFML-2.6.2/lib \
        -lsfml-graphics \
        -lsfml-window   \
        -lsfml-system   \
        -lsfml-audio

SOURCES += main.cpp \
    Celda.cpp \
    Tablero.cpp

HEADERS += \
    Celda.h \
    GameScreen.h \
    LevelSelectScreen.h \
    MainMenuScreen.h \
    ScreenManager.h \
    SplashScreen.h \
    Tablero.h \
    UIHelpers.h \
    UserAuthScreen.h \
    UserManager.h
