// graphwidget.cpp

#include "graphwidget.h"
#include "parser.h"
#include <QAction>
#include <QCursor>
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QtNodes/internal/NodeGraphicsObject.hpp> // Pour accès complet à nodeId()
#include <QtNodes/internal/ConnectionGraphicsObject.hpp> // si tu utilises setHighlight


using namespace QtNodes;

void GraphWidget::myStyle()
{
    GraphicsViewStyle::setStyle(
        R"(
      {
        "GraphicsViewStyle": {
          "BackgroundColor": [255, 255, 240],
          "FineGridColor": [245, 245, 230],
          "CoarseGridColor": [235, 235, 220]
        }
      }
      )");

    NodeStyle::setNodeStyle(
        R"(
      {
        "NodeStyle": {
          "NormalBoundaryColor": "darkgray",
          "SelectedBoundaryColor": "deepskyblue",
          "GradientColor0": "mintcream",
          "GradientColor1": "mintcream",
          "GradientColor2": "mintcream",
          "GradientColor3": "mintcream",
          "ShadowColor": [200, 200, 200],
          "ShadowEnabled": true,
          "FontColor": [10, 10, 10],
          "FontColorFaded": [100, 100, 100],
          "ConnectionPointColor": "white",
          "PenWidth": 2.0,
          "HoveredPenWidth": 2.5,
          "ConnectionPointDiameter": 10.0,
          "Opacity": 1.0
        }
      }
      )");

    ConnectionStyle::setConnectionStyle(
        R"(
      {
        "ConnectionStyle": {
          "ConstructionColor": "gray",
          "NormalColor": "black",
          "SelectedColor": "gray",
          "SelectedHaloColor": "deepskyblue",
          "HoveredColor": "deepskyblue",

          "LineWidth": 3.0,
          "ConstructionLineWidth": 2.0,
          "PointDiameter": 10.0,

          "UseDataDefinedColors": false
        }
      }
      )");
}

void GraphWidget::updateFromEditor() {
    QString     documentContents = scribeEditor->getCurrentDocument();
    parser      myDocumentParsed;

    QString     newLine;
    qint64      outputIndex;
    QString     data;
    int         lineNumber = 0;
    qreal       posX = 0;
    qreal       posY = 0;
    int         maxTextWidth = 0;
    int         itemNumber = 0;

    gl_scene->clearScene();
    if (!(scribeEditor->getCurrentLanguage()==QString("Language: Rail Manager"))) return;

    if (documentContents.length() >= 0) {
        outputIndex = 0;
        while (outputIndex < documentContents.length()) {
            data = documentContents.at(outputIndex);
            if (data == QChar('/')) {
                outputIndex++;
                if (outputIndex < documentContents.length() && documentContents.at(outputIndex) == QChar('/')) {
                    outputIndex++;
                    while (outputIndex < documentContents.length() && documentContents.at(outputIndex) != QChar('\n')) {
                        outputIndex++;
                    }
                    lineNumber++;
                }
                outputIndex++;
            }
            else {
                if (data != QChar('\n') && outputIndex < documentContents.length()) {
                    newLine.append(data);
                }
                else {
                    if (newLine.isEmpty()) {
                        outputIndex++;
                        lineNumber++;
                        continue;
                    }
                    newLine.append(QChar(' ')).append(QChar('\r'));
                    lineNumber++;

                    struct parser::parserObject* returnedObject;
                    returnedObject = new parser::parserObject;

                    if (myDocumentParsed.parseLine(newLine.toUtf8().data(), returnedObject) == FALSE) {
                        QString LineInfo = QString("Line ").append(QString::number(lineNumber)).append(" \n");
                        myDocumentParsed.gl_errorContext.prepend(LineInfo);
                        myDocumentParsed.traceError();
                        return;
                    }

                    if (!returnedObject->Name.isEmpty()) {
                        bool exists = false;
                        for (const NodeId& nodeId : gl_graphModel->allNodeIds()) {
                            QVariant caption = gl_graphModel->nodeData(nodeId, NodeRole::Caption);
                            if (caption.toString() == returnedObject->Name) {
                                exists = true;
                                break;
                            }
                        }
                        if (exists) {
                            newLine.clear();
                            outputIndex++;
                            continue;
                        }

                        itemNumber++;
                        QFontMetrics metrics(gl_scene->font());
                        int textWidth = (metrics.horizontalAdvance(returnedObject->Event) +
                                         metrics.horizontalAdvance(returnedObject->Name) +
                                         metrics.horizontalAdvance(returnedObject->Action)) * 1.1;
                        if (maxTextWidth < textWidth) maxTextWidth = textWidth;

                        NodeId id = gl_graphModel->addNode();
                        gl_graphModel->setNodeData(id, NodeRole::Caption, returnedObject->Name);
                        gl_graphModel->setNodeData(id, NodeRole::Position, QPointF(0, 0));
                        gl_graphModel->setPortCounts(id, 1, 1);
                        gl_graphModel->setPortData(id, PortType::In, 0, returnedObject->Event, PortRole::Caption);
                        gl_graphModel->setPortData(id, PortType::Out, 0, returnedObject->Action, PortRole::Caption);
                        gl_graphModel->setNodeData(id, NodeRole::Size, QSize(textWidth, 80));
                    }
                    newLine.clear();
                }
                outputIndex++;
            }
        }
    }

    // Connexions
    QMultiMap<NodeId, NodeId> outgoingMap;
    for (const NodeId& nodeIdOut : gl_graphModel->allNodeIds()) {
        QVariant dataOut = gl_graphModel->portData(nodeIdOut, PortType::Out, 0, PortRole::Caption);
        for (const NodeId& nodeIdIn : gl_graphModel->allNodeIds()) {
            QVariant dataIn = gl_graphModel->portData(nodeIdIn, PortType::In, 0, PortRole::Caption);
            if (dataOut.toString().contains(dataIn.toString())) {
                gl_graphModel->addConnection(ConnectionId{nodeIdOut, 0, nodeIdIn, 0});
                outgoingMap.insert(nodeIdOut, nodeIdIn);
            }
        }
    }

    // Counting descendants
    QMap<NodeId, int> descendantsCount;
    std::function<int(const NodeId&, QSet<NodeId>&)> countDescendants;
    countDescendants = [&](const NodeId& nodeId, QSet<NodeId>& visited) -> int {
        if (visited.contains(nodeId)) return 0;
        visited.insert(nodeId);
        int count = 0;
        for (const NodeId& childId : outgoingMap.values(nodeId)) {
            count += 1 + countDescendants(childId, visited);
        }
        return count;
    };
    for (const NodeId& nodeId : gl_graphModel->allNodeIds()) {
        QSet<NodeId> visited;
        descendantsCount[nodeId] = countDescendants(nodeId, visited);
    }

    // Grouping by number of descendants
    QMap<int, QList<NodeId>> groupedByDescendants;
    for (auto it = descendantsCount.begin(); it != descendantsCount.end(); ++it) {
        groupedByDescendants[it.value()].append(it.key());
    }

    // MOD: Grid placement √N with sorting by number of descendants (top-down by column)
    int spacingX = maxTextWidth;
    int spacingY = 150;
    int gridSize = std::ceil(std::sqrt(itemNumber));
    int currentRow = 0;
    int currentCol = 0;

    // Conversion std::unordered_set -> QList
    std::unordered_set<NodeId> nodeSet = gl_graphModel->allNodeIds();
    QList<NodeId> sortedNodes;
    for (const NodeId& nodeId : nodeSet) {
        sortedNodes.append(nodeId);
    }

    // Descending sort: nodes with more descendants first
    std::sort(sortedNodes.begin(), sortedNodes.end(), [&](const NodeId& a, const NodeId& b) {
        return descendantsCount[a] > descendantsCount[b];
    });

    // Column-by-column placement (left → right, top → bottom)
    for (const NodeId& nodeId : sortedNodes) {
        int x = currentCol * spacingX;
        int y = currentRow * spacingY;
        gl_graphModel->setNodeData(nodeId, NodeRole::Position, QPointF(x, y));

        ++currentRow;
        if (currentRow >= gridSize) {
            currentRow = 0;
            ++currentCol;
        }
    }

    // 🔍 Adjust the view to fit all content
    gl_view->fitInView(gl_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void GraphWidget::highlightConnectionsForSelection() {
    QSet<NodeId> selectedNodeIds;
    for (QGraphicsItem* item : gl_scene->selectedItems()) {
        if (auto nodeGraphics = qgraphicsitem_cast<NodeGraphicsObject*>(item)) {
            selectedNodeIds.insert(nodeGraphics->nodeId());
        }
    }

    QSet<NodeId> connectedNodeIds;

    // 1. Connections: highlighting + storing connected nodes
    for (QGraphicsItem* item : gl_scene->items()) {
        auto connGraphics = qgraphicsitem_cast<ConnectionGraphicsObject*>(item);
        if (!connGraphics)
            continue;

        ConnectionId connId = connGraphics->connectionId();
        bool highlight = selectedNodeIds.contains(connId.inNodeId)
                         || selectedNodeIds.contains(connId.outNodeId);

        if (highlight) {
            connectedNodeIds.insert(connId.inNodeId);
            connectedNodeIds.insert(connId.outNodeId);
        }

        connGraphics->setHighlight(highlight);
    }

    // 2. Nodes: highlight with red background if connected to a selected node
    for (QGraphicsItem* item : gl_scene->items()) {
        if (auto nodeGraphics = qgraphicsitem_cast<NodeGraphicsObject*>(item)) {
            NodeId nid = nodeGraphics->nodeId();
            bool highlight = connectedNodeIds.contains(nid);
            nodeGraphics->setHighlight(highlight);  // 🟥 change le fond du noeud
        }
    }

    gl_scene->update();
}


GraphWidget::GraphWidget(ScribeMainWindow *editor, QWidget *parent)
    : QWidget(parent), scribeEditor(editor)
{

    myStyle();

    gl_layout = new QVBoxLayout(this);
    gl_graphModel = new SimpleGraphModel();
    gl_scene = new BasicGraphicsScene(*gl_graphModel);
    gl_view = new GraphicsView(gl_scene);

    connect(gl_scene, &QGraphicsScene::selectionChanged, this, &GraphWidget::highlightConnectionsForSelection);

    gl_layout->addWidget(gl_view);
    setLayout(gl_layout);
}
