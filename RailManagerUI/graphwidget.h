#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H
#pragma once
#include <QWidget>
#include <QtNodes/BasicGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/StyleCollection>
#include <QVBoxLayout>
#include "ScribeMainWindow.h"
#include "SimpleGraphModel.hpp"


class GraphWidget : public QWidget {

    Q_OBJECT

public:
    explicit GraphWidget(ScribeMainWindow *editor,QWidget *parent = nullptr);
    void updateFromEditor(); // méthode pour récupérer les données
    void myStyle();
    void highlightConnectionsForSelection();


private:
    ScribeMainWindow *scribeEditor;

    QVBoxLayout *gl_layout;
    SimpleGraphModel *gl_graphModel;
    QtNodes::BasicGraphicsScene *gl_scene;
    QGraphicsView *gl_view;
};
#endif // GRAPHWIDGET_H
