// fusionmainwindow.cpp
#include "fusionmainwindow.h"
#include <QTabWidget>
#include <QWidget>

// Inclure les interfaces refactorisées de chaque application
#include "ScribeMainWindow.h"     // Ton ancien MainWindow de App2
#include "GraphWidget.h"          // Une nouvelle classe que tu crées à partir du code de App1

void FusionMainWindow::onTabChanged(int index)
{
    if (index == 0) {
        scribeEditor->onTabRegainedFocus();
    } else if (index == 1) {
        scribeEditor->onTabLostFocus();
    }
}


FusionMainWindow::FusionMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QTabWidget *tabs = new QTabWidget(this);
    tabs = new QTabWidget(this);
    scribeEditor = new ScribeMainWindow(this);
    graphWidget = new GraphWidget(scribeEditor, this);


    QObject::connect(scribeEditor, &ScribeMainWindow::checkTextSignal,
                     graphWidget, &GraphWidget::updateFromEditor);

    tabs->addTab(scribeEditor, "Program Editor");
    tabs->addTab(graphWidget, "Node Graph Viewer");
    connect(tabs, &QTabWidget::currentChanged, this, &FusionMainWindow::onTabChanged);


    setCentralWidget(tabs);
    resize(1000, 800);
    setWindowTitle("Rail Manager Barraux");
}
