#include "SimpleGraphModel.hpp"

SimpleGraphModel::SimpleGraphModel()
    : _nextNodeId{0}
{}

SimpleGraphModel::~SimpleGraphModel()
{
    //
}

void SimpleGraphModel::setPortCounts(NodeId nodeId, uint32_t inCount, uint32_t outCount)
{
    _inPortCounts[nodeId] = inCount;
    _outPortCounts[nodeId] = outCount;
}

std::unordered_set<NodeId> SimpleGraphModel::allNodeIds() const
{
    return _nodeIds;
}

std::unordered_set<ConnectionId> SimpleGraphModel::allConnectionIds(NodeId const nodeId) const
{
    std::unordered_set<ConnectionId> result;

    std::copy_if(_connectivity.begin(),
                 _connectivity.end(),
                 std::inserter(result, std::end(result)),
                 [&nodeId](ConnectionId const &cid) {
                     return cid.inNodeId == nodeId || cid.outNodeId == nodeId;
                 });

    return result;
}

std::unordered_set<ConnectionId> SimpleGraphModel::connections(NodeId nodeId,
                                                               PortType portType,
                                                               PortIndex portIndex) const
{
    std::unordered_set<ConnectionId> result;

    std::copy_if(_connectivity.begin(),
                 _connectivity.end(),
                 std::inserter(result, std::end(result)),
                 [&portType, &portIndex, &nodeId](ConnectionId const &cid) {
                     return (getNodeId(portType, cid) == nodeId
                             && getPortIndex(portType, cid) == portIndex);
                 });

    return result;
}

bool SimpleGraphModel::connectionExists(ConnectionId const connectionId) const
{
    return (_connectivity.find(connectionId) != _connectivity.end());
}

NodeId SimpleGraphModel::addNode(QString const nodeType)
{
    NodeId newId = newNodeId();
    // Create new node.
    _nodeIds.insert(newId);

    _inPortCounts[newId] = 1;
    _outPortCounts[newId] = 1;

    Q_EMIT nodeCreated(newId);

    return newId;
}

bool SimpleGraphModel::connectionPossible(ConnectionId const connectionId) const
{
    return _connectivity.find(connectionId) == _connectivity.end();
}

void SimpleGraphModel::addConnection(ConnectionId const connectionId)
{
    _connectivity.insert(connectionId);

    Q_EMIT connectionCreated(connectionId);
}

bool SimpleGraphModel::nodeExists(NodeId const nodeId) const
{
    return (_nodeIds.find(nodeId) != _nodeIds.end());
}

QVariant SimpleGraphModel::nodeData(NodeId nodeId, NodeRole role) const
{
    Q_UNUSED(nodeId);

    QVariant result;

    switch (role) {
    case NodeRole::Type:
        result = QString("Default Node Type");
        break;

    case NodeRole::Position:
        result = _nodeGeometryData[nodeId].pos;
        break;

    case NodeRole::Size:
        result = _nodeGeometryData[nodeId].size;
        break;

    case NodeRole::CaptionVisible:
        result = true;
        break;

    case NodeRole::Caption:
    {
        auto it = _nodeCaptions.find(nodeId);
        if (it != _nodeCaptions.end()) {
            result = it->second;
        } else {
            result = QString("Unnamed Node"); // fallback s'il n'existe pas
        }
        break;
    }

    case NodeRole::Style: {
        auto style = StyleCollection::nodeStyle();
        result = style.toJson().toVariantMap();
    } break;

    case NodeRole::InternalData:
        break;

    case NodeRole::InPortCount: {
            auto it = _inPortCounts.find(nodeId);
            result = (it != _inPortCounts.end()) ? it->second : 0u;
            break;
        }

    case NodeRole::OutPortCount: {
        auto it = _outPortCounts.find(nodeId);
        result = (it != _outPortCounts.end()) ? it->second : 0u;
        break;
    }

    case NodeRole::Widget:
        result = QVariant();
        break;
    }

    return result;
}

bool SimpleGraphModel::setNodeData(NodeId nodeId, NodeRole role, QVariant value)
{
    bool result = false;

    switch (role) {
    case NodeRole::Type:
        break;
    case NodeRole::Position: {
        _nodeGeometryData[nodeId].pos = value.value<QPointF>();

        Q_EMIT nodePositionUpdated(nodeId);

        result = true;
    } break;

    case NodeRole::Size: {
        _nodeGeometryData[nodeId].size = value.value<QSize>();
        result = true;
    } break;

    case NodeRole::CaptionVisible:
        break;

    case NodeRole::Caption:
    {
        QString caption = value.toString();
        _nodeCaptions[nodeId] = caption;
        Q_EMIT nodeUpdated(nodeId);
        result = true;
    } break;
    case NodeRole::Style:
        break;

    case NodeRole::InternalData:
        break;

    case NodeRole::InPortCount:
        break;

    case NodeRole::OutPortCount:
        break;

    case NodeRole::Widget:
        break;
    }

    return result;
}

QVariant SimpleGraphModel::portData(NodeId nodeId,
                                    PortType portType,
                                    PortIndex portIndex,
                                    PortRole role) const
{
    switch (role) {
    case PortRole::Data:
        return QVariant();
        break;

    case PortRole::DataType:
        return QVariant();
        break;

    case PortRole::ConnectionPolicyRole:
        return QVariant::fromValue(ConnectionPolicy::One);
        break;

    case PortRole::CaptionVisible:
        return true;
        break;

    case PortRole::Caption:
    {
        auto key = std::make_tuple(nodeId, portType, portIndex);
        auto it = _portCaptions.find(key);

        if (it != _portCaptions.end()) {
            return it->second;
        } else {
            // Valeur par défaut si rien de personnalisé n’est défini
            return (portType == PortType::In)
                       ? QString("In %1").arg(portIndex)
                       : QString("Out %1").arg(portIndex);
        }
    }
    }

    return QVariant();
}

bool SimpleGraphModel::setPortData(
    NodeId nodeId, PortType portType, PortIndex portIndex, QVariant const &value, PortRole role)
{
    if (role == PortRole::Caption) {
        auto key = std::make_tuple(nodeId, portType, portIndex);
        _portCaptions[key] = value.toString();
        Q_EMIT portUpdated(nodeId, portType, portIndex);
        return true;
    }

    return false;
}

bool SimpleGraphModel::deleteConnection(ConnectionId const connectionId)
{
    bool disconnected = false;

    auto it = _connectivity.find(connectionId);

    if (it != _connectivity.end()) {
        disconnected = true;

        _connectivity.erase(it);
    }

    if (disconnected)
        Q_EMIT connectionDeleted(connectionId);

    return disconnected;
}

bool SimpleGraphModel::deleteNode(NodeId const nodeId)
{
    // Delete connections to this node first.
    auto connectionIds = allConnectionIds(nodeId);

    for (auto &cId : connectionIds) {
        deleteConnection(cId);
    }

    _nodeIds.erase(nodeId);
    _nodeGeometryData.erase(nodeId);

    Q_EMIT nodeDeleted(nodeId);

    return true;
}

QJsonObject SimpleGraphModel::saveNode(NodeId const nodeId) const
{
    QJsonObject nodeJson;

    nodeJson["id"] = static_cast<qint64>(nodeId);

    {
        QPointF const pos = nodeData(nodeId, NodeRole::Position).value<QPointF>();

        QJsonObject posJson;
        posJson["x"] = pos.x();
        posJson["y"] = pos.y();
        nodeJson["position"] = posJson;
    }

    return nodeJson;
}

void SimpleGraphModel::loadNode(QJsonObject const &nodeJson)
{
    NodeId restoredNodeId = static_cast<NodeId>(nodeJson["id"].toInt());

    // Next NodeId must be larger that any id existing in the graph
    _nextNodeId = std::max(_nextNodeId, restoredNodeId + 1);

    // Create new node.
    _nodeIds.insert(restoredNodeId);

    Q_EMIT nodeCreated(restoredNodeId);

    {
        QJsonObject posJson = nodeJson["position"].toObject();
        QPointF const pos(posJson["x"].toDouble(), posJson["y"].toDouble());

        setNodeData(restoredNodeId, NodeRole::Position, pos);
    }
}
