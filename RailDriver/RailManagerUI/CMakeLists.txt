if(DEFINED CMAKE_PROJECT_INCLUDE_BEFORE AND CMAKE_PROJECT_INCLUDE_BEFORE MATCHES ".qtc/package-manager/auto-setup.cmake")
    unset(CMAKE_PROJECT_INCLUDE_BEFORE CACHE)
endif()

cmake_minimum_required(VERSION 3.11)

project(RailManagerUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trouve Qt (Qt5 ou Qt6), avec Widgets + SerialPort
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort PrintSupport)


set(INNO_MAKER_SOURCE
    inno_maker/delay_sender.cpp
    inno_maker/formatter.cpp
    inno_maker/worker.cpp
    inno_maker/DeviceHotplug.cpp
    inno_maker/InnoMakerMainWindow.cpp
)
set(SCRIBE_SOURCE
    scribe/editor.cpp
    scribe/finddialog.cpp
    scribe/gotodialog.cpp
    scribe/language.cpp
    scribe/metricreporter.cpp
    scribe/ScribeMainWindow.cpp
    scribe/searchhistory.cpp
    scribe/settings.cpp
    scribe/tabbededitor.cpp
    scribe/utilityfunctions.cpp
    scribe/parser.cpp
    scribe/ParserLangHighlighter.cpp
    scribe/highlighters/chighlighter.cpp
    scribe/highlighters/cpphighlighter.cpp
    scribe/highlighters/highlighter.cpp
    scribe/highlighters/javahighlighter.cpp
    scribe/highlighters/pythonhighlighter.cpp
)
set(QNODE_SOURCE
    qnode/graphwidget.cpp
    qnode/SimpleGraphModel.cpp
)
set(SOURCES
    FusionMainWindow.cpp
    main.cpp
)

set(INNO_MAKER_HEADERS
    inno_maker/delay_sender.h
    inno_maker/formatter.h
    inno_maker/worker.h
    inno_maker/DeviceHotplug.h
    inno_maker/InnoMakerMainWindow.h
    inno_maker/libInnoMakerUsb2Can/include/InnoMakerUsb2CanLib.h
    inno_maker/libInnoMakerUsb2Can/include/libusb.h
)
set(SCRIBE_HEADERS
    scribe/documentmetrics.h
    scribe/editor.h
    scribe/finddialog.h
    scribe/gotodialog.h
    scribe/language.h
    scribe/linenumberarea.h
    scribe/metricreporter.h
    scribe/ScribeMainWindow.h
    scribe/searchhistory.h
    scribe/settings.h
    scribe/tabbededitor.h
    scribe/utilityfunctions.h
    scribe/parser.h
    scribe/ParserLangHighlighter.h
    scribe/highlighters/chighlighter.h
    scribe/highlighters/cpphighlighter.h
    scribe/highlighters/highlighter.h
    scribe/highlighters/javahighlighter.h
    scribe/highlighters/pythonhighlighter.h
)
set(QNODE_HEADERS
     qnode/graphwidget.h
     qnode/SimpleGraphModel.hpp
     qnode/CustomNodeModel.h
)
set(HEADERS
    FusionMainWindow.h
)
set(UIS
    scribe/ScribeMainWindow.ui
    inno_maker/InnoMakerMainWindow.ui
)
set(RC
    resources.qrc
    texteditor.rc
    res/icons/app-icon.ico
    res/icons/copy.bmp
    res/icons/cut.bmp
    res/icons/folder.bmp
    res/icons/new.bmp
    res/icons/paste.bmp
    res/icons/printer.bmp
    res/icons/redo.bmp
    res/icons/save-as.bmp
    res/icons/save.bmp
    res/icons/search.bmp
    res/icons/undo.bmp
    res/icons/check.bmp
    res/icons/load.bmp
    res/icons/CAN.png
)
set(LIB
    inno_maker/libInnoMakerUsb2Can/lib/x64/InnoMakerUsb2CanLib.dll
    inno_maker/libInnoMakerUsb2Can/lib/x64/InnoMakerUsb2CanLib.lib
)

# === Création de l'exécutable ===
qt_add_executable(RailManagerUI
    ${INNO_MAKER_SOURCE}
    ${SCRIBE_SOURCE}
    ${QNODE_SOURCE}
    ${SOURCES}
    ${INNO_MAKER_HEADERS}
    ${SCRIBE_HEADERS}
    ${QNODE_HEADERS}
    ${HEADERS}
    ${UIS}
    ${RC}
    ${LIB}
)

target_link_directories(RailManagerUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inno_maker/libInnoMakerUsb2Can/lib/x64
)
target_include_directories(RailManagerUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inno_maker
    ${CMAKE_CURRENT_SOURCE_DIR}/inno_maker/libInnoMakerUsb2Can/include
    ${CMAKE_CURRENT_SOURCE_DIR}/qnode
    ${CMAKE_CURRENT_SOURCE_DIR}/scribe
    ${CMAKE_CURRENT_SOURCE_DIR}/scribe/highlighters
)

# Lien avec Qt et QtNodes
target_link_libraries(RailManagerUI PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::PrintSupport
    QtNodes::QtNodes
    InnoMakerUsb2CanLib
)

# === Définir les propriétés Windows/Mac ===
set_target_properties(RailManagerUI PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
