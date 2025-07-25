#include "fusionmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle("fusion");
    app.setWindowIcon(QIcon("res/icons/app-icon.ico"));  // chemin dans .qrc


    FusionMainWindow mainWin;
    mainWin.show();

    return app.exec();
}
