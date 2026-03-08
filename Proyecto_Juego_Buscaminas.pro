TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

SFML_DIR = C:/SFML-2.6.2

INCLUDEPATH += $$SFML_DIR/include

CONFIG(debug, debug|release) {
    LIBS += -L$$SFML_DIR/lib \
            -lsfml-graphics-d \
            -lsfml-window-d \
            -lsfml-system-d
} else {
    LIBS += -L$$SFML_DIR/lib \
            -lsfml-graphics \
            -lsfml-window \
            -lsfml-system
}
