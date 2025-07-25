cmake_minimum_required(VERSION 3.16)

project(RailManagerUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trouve Qt (Qt5 ou Qt6), avec Widgets + SerialPort
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets SerialPort PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SerialPort PrintSupport)

# Ajoute la bibliothèque QtNodes
add_subdirectory(../QtNodesLibrary QtNodes_build)

# Sources de ton application
set(PROJECT_SOURCES
    documentmetrics.h
    editor.h
    finddialog.h
    fusionmainwindow.h
    gotodialog.h
    graphwidget.h
    language.h
    linenumberarea.h
    mainwindow.h
    metricreporter.h
    parser.h
    ScribeMainWindow.h
    searchhistory.h
    serialport.h
    settings.h
    tabbededitor.h
    utilityfunctions.h
    SimpleGraphModel.hpp
    editor.cpp
    finddialog.cpp
    fusionmainwindow.cpp
    gotodialog.cpp
    graphwidget.cpp
    language.cpp
    main.cpp
    mainwindow.cpp
    metricreporter.cpp
    parser.cpp
    ScribeMainWindow.cpp
    searchhistory.cpp
    serialport.cpp
    settings.cpp
    SimpleGraphModel.cpp
    tabbededitor.cpp
    utilityfunctions.cpp
    CMakeLists.txt
    resources.qrc
    mainwindow.ui
    ScribeMainWindow.ui
    texteditor.rc
)

# Crée l'exécutable
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(RailManagerUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        highlighters/chighlighter.cpp highlighters/chighlighter.h
        highlighters/cpphighlighter.cpp highlighters/cpphighlighter.h
        highlighters/highlighter.cpp highlighters/highlighter.h
        highlighters/javahighlighter.cpp highlighters/javahighlighter.h
        highlighters/pythonhighlighter.cpp highlighters/pythonhighlighter.h
        res/icons/app-icon.ico res/icons/copy.bmp res/icons/cut.bmp res/icons/folder.bmp
        res/icons/new.bmp res/icons/paste.bmp res/icons/printer.bmp res/icons/redo.bmp
        res/icons/save-as.bmp res/icons/save.bmp res/icons/search.bmp res/icons/undo.bmp
        res/icons/check.bmp res/icons/load.bmp
        CustomNodeModel.h


    )
else()
    add_executable(RailManagerUI
        ${PROJECT_SOURCES}
    )
endif()

# Lien avec Qt et QtNodes
target_link_libraries(RailManagerUI
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::SerialPort
        Qt${QT_VERSION_MAJOR}::PrintSupport
        QtNodes::QtNodes
)

# Propriétés pour bundle
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.RailManagerUI)
endif()
set_target_properties(RailManagerUI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Installation
include(GNUInstallDirs)
install(TARGETS RailManagerUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Finalise pour Qt6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RailManagerUI)
endif()

target_include_directories(RailManagerUI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
