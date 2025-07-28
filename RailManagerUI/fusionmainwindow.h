#ifndef FUSIONMAINWINDOW_H
#define FUSIONMAINWINDOW_H
// fusionmainwindow.h
#include "ScribeMainWindow.h"
#include "graphwidget.h"

#pragma once

#include <QMainWindow>


class FusionMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    FusionMainWindow(QWidget *parent = nullptr);

private slots:

    void onTabChanged(int index);

private:
    QTabWidget *tabs;
    ScribeMainWindow *scribeEditor;
    GraphWidget *graphWidget;

};
#endif // FUSIONMAINWINDOW_H
