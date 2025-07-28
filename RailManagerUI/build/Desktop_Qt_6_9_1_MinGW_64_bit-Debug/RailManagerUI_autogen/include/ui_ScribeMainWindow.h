/********************************************************************************
** Form generated from reading UI file 'ScribeMainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCRIBEMAINWINDOW_H
#define UI_SCRIBEMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "tabbededitor.h"

QT_BEGIN_NAMESPACE

class Ui_ScribeMainWindow
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSave_As;
    QAction *actionPrint;
    QAction *actionExit;
    QAction *actionUndo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionFind;
    QAction *actionReplace;
    QAction *actionGo_To;
    QAction *actionSelect_All;
    QAction *actionTime_Date;
    QAction *actionFont;
    QAction *actionStatus_Bar;
    QAction *actionRedo;
    QAction *actionAuto_Indent;
    QAction *actionWord_Wrap;
    QAction *actionC_Lang;
    QAction *actionCPP_Lang;
    QAction *actionJava_Lang;
    QAction *actionPython_Lang;
    QAction *actionTool_Bar;
    QAction *actionConnect;
    QAction *actionUpdate;
    QAction *actionSelect_Port_Com;
    QAction *actionCheck_Program;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QLabel *label_2;
    TabbedEditor *tabWidget;
    QLabel *label_3;
    QLineEdit *simpleCommand;
    QLabel *label;
    QTextEdit *CommandResult;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuFormat;
    QMenu *menuLanguage;
    QMenu *menuView;
    QMenu *menuRailRoad;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ScribeMainWindow)
    {
        if (ScribeMainWindow->objectName().isEmpty())
            ScribeMainWindow->setObjectName("ScribeMainWindow");
        ScribeMainWindow->resize(631, 587);
        ScribeMainWindow->setMinimumSize(QSize(530, 230));
        actionNew = new QAction(ScribeMainWindow);
        actionNew->setObjectName("actionNew");
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/res/icons/new.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionNew->setIcon(icon);
        actionOpen = new QAction(ScribeMainWindow);
        actionOpen->setObjectName("actionOpen");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/res/icons/folder.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionOpen->setIcon(icon1);
        actionSave = new QAction(ScribeMainWindow);
        actionSave->setObjectName("actionSave");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/res/icons/save.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionSave->setIcon(icon2);
        actionSave_As = new QAction(ScribeMainWindow);
        actionSave_As->setObjectName("actionSave_As");
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/res/icons/save-as.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionSave_As->setIcon(icon3);
        actionPrint = new QAction(ScribeMainWindow);
        actionPrint->setObjectName("actionPrint");
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/res/icons/printer.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionPrint->setIcon(icon4);
        actionExit = new QAction(ScribeMainWindow);
        actionExit->setObjectName("actionExit");
        actionUndo = new QAction(ScribeMainWindow);
        actionUndo->setObjectName("actionUndo");
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/res/icons/undo.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionUndo->setIcon(icon5);
        actionCut = new QAction(ScribeMainWindow);
        actionCut->setObjectName("actionCut");
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/res/icons/cut.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionCut->setIcon(icon6);
        actionCopy = new QAction(ScribeMainWindow);
        actionCopy->setObjectName("actionCopy");
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons/res/icons/copy.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionCopy->setIcon(icon7);
        actionPaste = new QAction(ScribeMainWindow);
        actionPaste->setObjectName("actionPaste");
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icons/res/icons/paste.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionPaste->setIcon(icon8);
        actionFind = new QAction(ScribeMainWindow);
        actionFind->setObjectName("actionFind");
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icons/res/icons/search.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionFind->setIcon(icon9);
        actionReplace = new QAction(ScribeMainWindow);
        actionReplace->setObjectName("actionReplace");
        actionGo_To = new QAction(ScribeMainWindow);
        actionGo_To->setObjectName("actionGo_To");
        actionSelect_All = new QAction(ScribeMainWindow);
        actionSelect_All->setObjectName("actionSelect_All");
        actionTime_Date = new QAction(ScribeMainWindow);
        actionTime_Date->setObjectName("actionTime_Date");
        actionFont = new QAction(ScribeMainWindow);
        actionFont->setObjectName("actionFont");
        actionStatus_Bar = new QAction(ScribeMainWindow);
        actionStatus_Bar->setObjectName("actionStatus_Bar");
        actionStatus_Bar->setCheckable(true);
        actionStatus_Bar->setChecked(true);
        actionStatus_Bar->setEnabled(true);
        actionRedo = new QAction(ScribeMainWindow);
        actionRedo->setObjectName("actionRedo");
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/icons/res/icons/redo.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::On);
        actionRedo->setIcon(icon10);
        actionAuto_Indent = new QAction(ScribeMainWindow);
        actionAuto_Indent->setObjectName("actionAuto_Indent");
        actionAuto_Indent->setCheckable(true);
        actionAuto_Indent->setChecked(true);
        actionWord_Wrap = new QAction(ScribeMainWindow);
        actionWord_Wrap->setObjectName("actionWord_Wrap");
        actionWord_Wrap->setCheckable(true);
        actionC_Lang = new QAction(ScribeMainWindow);
        actionC_Lang->setObjectName("actionC_Lang");
        actionC_Lang->setCheckable(true);
        actionCPP_Lang = new QAction(ScribeMainWindow);
        actionCPP_Lang->setObjectName("actionCPP_Lang");
        actionCPP_Lang->setCheckable(true);
        actionJava_Lang = new QAction(ScribeMainWindow);
        actionJava_Lang->setObjectName("actionJava_Lang");
        actionJava_Lang->setCheckable(true);
        actionPython_Lang = new QAction(ScribeMainWindow);
        actionPython_Lang->setObjectName("actionPython_Lang");
        actionPython_Lang->setCheckable(true);
        actionTool_Bar = new QAction(ScribeMainWindow);
        actionTool_Bar->setObjectName("actionTool_Bar");
        actionTool_Bar->setCheckable(true);
        actionTool_Bar->setChecked(true);
        actionConnect = new QAction(ScribeMainWindow);
        actionConnect->setObjectName("actionConnect");
        actionUpdate = new QAction(ScribeMainWindow);
        actionUpdate->setObjectName("actionUpdate");
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/icons/res/icons/load.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionUpdate->setIcon(icon11);
        actionUpdate->setMenuRole(QAction::MenuRole::TextHeuristicRole);
        actionSelect_Port_Com = new QAction(ScribeMainWindow);
        actionSelect_Port_Com->setObjectName("actionSelect_Port_Com");
        actionCheck_Program = new QAction(ScribeMainWindow);
        actionCheck_Program->setObjectName("actionCheck_Program");
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/icons/res/icons/check.bmp"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionCheck_Program->setIcon(icon12);
        actionCheck_Program->setMenuRole(QAction::MenuRole::NoRole);
        centralWidget = new QWidget(ScribeMainWindow);
        centralWidget->setObjectName("centralWidget");
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        tabWidget = new TabbedEditor(centralWidget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setMinimumSize(QSize(0, 0));

        verticalLayout->addWidget(tabWidget);

        label_3 = new QLabel(centralWidget);
        label_3->setObjectName("label_3");

        verticalLayout->addWidget(label_3);

        simpleCommand = new QLineEdit(centralWidget);
        simpleCommand->setObjectName("simpleCommand");
        simpleCommand->setEnabled(true);
        simpleCommand->setReadOnly(false);

        verticalLayout->addWidget(simpleCommand);

        label = new QLabel(centralWidget);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        CommandResult = new QTextEdit(centralWidget);
        CommandResult->setObjectName("CommandResult");
        CommandResult->setEnabled(true);
        CommandResult->setMinimumSize(QSize(0, 0));
        CommandResult->setMaximumSize(QSize(16777215, 80));
        CommandResult->setTextInteractionFlags(Qt::TextInteractionFlag::NoTextInteraction);

        verticalLayout->addWidget(CommandResult);

        ScribeMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ScribeMainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 631, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName("menuFile");
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName("menuEdit");
        menuFormat = new QMenu(menuBar);
        menuFormat->setObjectName("menuFormat");
        menuLanguage = new QMenu(menuFormat);
        menuLanguage->setObjectName("menuLanguage");
        menuView = new QMenu(menuBar);
        menuView->setObjectName("menuView");
        menuRailRoad = new QMenu(menuBar);
        menuRailRoad->setObjectName("menuRailRoad");
        ScribeMainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ScribeMainWindow);
        mainToolBar->setObjectName("mainToolBar");
        ScribeMainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ScribeMainWindow);
        statusBar->setObjectName("statusBar");
        statusBar->setEnabled(true);
        ScribeMainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuRailRoad->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuFormat->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_As);
        menuFile->addSeparator();
        menuFile->addAction(actionPrint);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionCut);
        menuEdit->addAction(actionCopy);
        menuEdit->addAction(actionPaste);
        menuEdit->addSeparator();
        menuEdit->addAction(actionFind);
        menuEdit->addAction(actionReplace);
        menuEdit->addAction(actionGo_To);
        menuEdit->addSeparator();
        menuEdit->addAction(actionSelect_All);
        menuEdit->addAction(actionTime_Date);
        menuFormat->addAction(actionFont);
        menuFormat->addAction(menuLanguage->menuAction());
        menuFormat->addAction(actionAuto_Indent);
        menuFormat->addAction(actionWord_Wrap);
        menuLanguage->addAction(actionC_Lang);
        menuLanguage->addAction(actionCPP_Lang);
        menuLanguage->addAction(actionJava_Lang);
        menuLanguage->addAction(actionPython_Lang);
        menuView->addAction(actionStatus_Bar);
        menuView->addAction(actionTool_Bar);
        menuRailRoad->addAction(actionSelect_Port_Com);
        menuRailRoad->addAction(actionCheck_Program);
        menuRailRoad->addAction(actionUpdate);
        mainToolBar->addAction(actionNew);
        mainToolBar->addAction(actionOpen);
        mainToolBar->addAction(actionSave);
        mainToolBar->addAction(actionSave_As);
        mainToolBar->addAction(actionPrint);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionUndo);
        mainToolBar->addAction(actionRedo);
        mainToolBar->addAction(actionCopy);
        mainToolBar->addAction(actionCut);
        mainToolBar->addAction(actionPaste);
        mainToolBar->addAction(actionFind);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionCheck_Program);
        mainToolBar->addAction(actionUpdate);

        retranslateUi(ScribeMainWindow);

        tabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(ScribeMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ScribeMainWindow)
    {
        ScribeMainWindow->setWindowTitle(QCoreApplication::translate("ScribeMainWindow", "ScribeMainWindow", nullptr));
        actionNew->setText(QCoreApplication::translate("ScribeMainWindow", "New", nullptr));
#if QT_CONFIG(shortcut)
        actionNew->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpen->setText(QCoreApplication::translate("ScribeMainWindow", "Open", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave->setText(QCoreApplication::translate("ScribeMainWindow", "Save", nullptr));
#if QT_CONFIG(shortcut)
        actionSave->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave_As->setText(QCoreApplication::translate("ScribeMainWindow", "Save As", nullptr));
#if QT_CONFIG(shortcut)
        actionSave_As->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+Shift+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPrint->setText(QCoreApplication::translate("ScribeMainWindow", "Print...", nullptr));
#if QT_CONFIG(shortcut)
        actionPrint->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+P", nullptr));
#endif // QT_CONFIG(shortcut)
        actionExit->setText(QCoreApplication::translate("ScribeMainWindow", "Exit", nullptr));
        actionUndo->setText(QCoreApplication::translate("ScribeMainWindow", "Undo", nullptr));
#if QT_CONFIG(shortcut)
        actionUndo->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCut->setText(QCoreApplication::translate("ScribeMainWindow", "Cut", nullptr));
#if QT_CONFIG(shortcut)
        actionCut->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCopy->setText(QCoreApplication::translate("ScribeMainWindow", "Copy", nullptr));
#if QT_CONFIG(shortcut)
        actionCopy->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPaste->setText(QCoreApplication::translate("ScribeMainWindow", "Paste", nullptr));
#if QT_CONFIG(shortcut)
        actionPaste->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionFind->setText(QCoreApplication::translate("ScribeMainWindow", "Find...", nullptr));
#if QT_CONFIG(shortcut)
        actionFind->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+F", nullptr));
#endif // QT_CONFIG(shortcut)
        actionReplace->setText(QCoreApplication::translate("ScribeMainWindow", "Replace...", nullptr));
#if QT_CONFIG(shortcut)
        actionReplace->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+H", nullptr));
#endif // QT_CONFIG(shortcut)
        actionGo_To->setText(QCoreApplication::translate("ScribeMainWindow", "Go To...", nullptr));
#if QT_CONFIG(shortcut)
        actionGo_To->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+G", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSelect_All->setText(QCoreApplication::translate("ScribeMainWindow", "Select All", nullptr));
#if QT_CONFIG(shortcut)
        actionSelect_All->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionTime_Date->setText(QCoreApplication::translate("ScribeMainWindow", "Time/Date", nullptr));
#if QT_CONFIG(shortcut)
        actionTime_Date->setShortcut(QCoreApplication::translate("ScribeMainWindow", "F5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionFont->setText(QCoreApplication::translate("ScribeMainWindow", "Font...", nullptr));
        actionStatus_Bar->setText(QCoreApplication::translate("ScribeMainWindow", "Status Bar", nullptr));
        actionRedo->setText(QCoreApplication::translate("ScribeMainWindow", "Redo", nullptr));
#if QT_CONFIG(shortcut)
        actionRedo->setShortcut(QCoreApplication::translate("ScribeMainWindow", "Ctrl+Y", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAuto_Indent->setText(QCoreApplication::translate("ScribeMainWindow", "Auto Indent", nullptr));
        actionWord_Wrap->setText(QCoreApplication::translate("ScribeMainWindow", "Word Wrap", nullptr));
        actionC_Lang->setText(QCoreApplication::translate("ScribeMainWindow", "C", nullptr));
        actionCPP_Lang->setText(QCoreApplication::translate("ScribeMainWindow", "C++", nullptr));
        actionJava_Lang->setText(QCoreApplication::translate("ScribeMainWindow", "Java", nullptr));
        actionPython_Lang->setText(QCoreApplication::translate("ScribeMainWindow", "Python", nullptr));
        actionTool_Bar->setText(QCoreApplication::translate("ScribeMainWindow", "Tool Bar", nullptr));
        actionConnect->setText(QCoreApplication::translate("ScribeMainWindow", "Connect", nullptr));
        actionUpdate->setText(QCoreApplication::translate("ScribeMainWindow", "Update boards", nullptr));
        actionSelect_Port_Com->setText(QCoreApplication::translate("ScribeMainWindow", "Select Port Com", nullptr));
        actionCheck_Program->setText(QCoreApplication::translate("ScribeMainWindow", "Check_Program", nullptr));
        label_2->setText(QCoreApplication::translate("ScribeMainWindow", "Script update", nullptr));
        label_3->setText(QCoreApplication::translate("ScribeMainWindow", "Simple Command", nullptr));
        simpleCommand->setText(QString());
        label->setText(QCoreApplication::translate("ScribeMainWindow", "Master result", nullptr));
        menuFile->setTitle(QCoreApplication::translate("ScribeMainWindow", "File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("ScribeMainWindow", "Edit", nullptr));
        menuFormat->setTitle(QCoreApplication::translate("ScribeMainWindow", "Format", nullptr));
        menuLanguage->setTitle(QCoreApplication::translate("ScribeMainWindow", "Language", nullptr));
        menuView->setTitle(QCoreApplication::translate("ScribeMainWindow", "View", nullptr));
        menuRailRoad->setTitle(QCoreApplication::translate("ScribeMainWindow", "RailRoad", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ScribeMainWindow: public Ui_ScribeMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCRIBEMAINWINDOW_H
