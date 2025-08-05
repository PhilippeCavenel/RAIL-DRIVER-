
#include <FusionMainWindow.h>
#include <QTabWidget>
#include <QWidget>

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
    innoMakerView = new InnoMakerMainWindow(this);

    QObject::connect(scribeEditor, &ScribeMainWindow::checkTextSignal,
                     graphWidget, &GraphWidget::updateFromEditor);

    tabs->addTab(scribeEditor, "Program Editor");
    tabs->addTab(graphWidget, "Node Graph Viewer");
    tabs->addTab(innoMakerView, "CAN Viewer");

    connect(tabs, &QTabWidget::currentChanged, this, &FusionMainWindow::onTabChanged);
    QObject::connect(innoMakerView, &InnoMakerMainWindow::characterReceived,scribeEditor, &ScribeMainWindow::innoMakerDataReceived);
    setCentralWidget(tabs);
    resize(1000, 800);
    setWindowTitle("Rail Manager");
}
