#pragma once

#include "spec_parser/automaton-data.hpp"
#include <QtCore/QJsonObject>
#include <QtCore/QPointF>
#include <QtCore/QSize>

#include <QtNodes/AbstractGraphModel>
#include <QtNodes/StyleCollection>

using ConnectionId = QtNodes::ConnectionId;
using ConnectionPolicy = QtNodes::ConnectionPolicy;
using NodeFlag = QtNodes::NodeFlag;
using NodeId = QtNodes::NodeId;
using NodeRole = QtNodes::NodeRole;
using PortIndex = QtNodes::PortIndex;
using PortRole = QtNodes::PortRole;
using PortType = QtNodes::PortType;
using StyleCollection = QtNodes::StyleCollection;
using QtNodes::InvalidNodeId;

class PortAddRemoveWidget;

/**
 * The class implements a bare minimum required to demonstrate a model-based
 * graph.
 */
class DynamicPortsModel : public QtNodes::AbstractGraphModel
{
    Q_OBJECT
public:
    struct NodeGeometryData
    {
        QSize size;
        QPointF pos;
    };

public:
    DynamicPortsModel();

    ~DynamicPortsModel() override;

    std::unordered_set<NodeId> allNodeIds() const override;

    std::unordered_set<ConnectionId> allConnectionIds(NodeId const nodeId) const override;

    std::unordered_set<ConnectionId> connections(NodeId nodeId,
                                                 PortType portType,
                                                 PortIndex portIndex) const override;

    QString caption() const  { return QStringLiteral("Result"); }


    QString GetNodeName(NodeId const nodeId) { return _nodeNames[nodeId]; }
    void SetNodeName(NodeId const nodeId, QString name) {
        _nodeNames[nodeId] = name;
        setNodeData(nodeId, QtNodes::NodeRole::Caption, QVariant::fromValue(name));
    }

    void SetNodeActionCode(NodeId const nodeId, QString code) { _nodeActionCodes[nodeId] = code; }
    QString GetNodeActionCode(NodeId const nodeId) { return _nodeActionCodes[nodeId]; }

    void SetConnectionCode(ConnectionId const connId, QString code){ _connectionCodes[connId] = code; }
    QString GetConnectionCode(ConnectionId const connId) { return _connectionCodes[connId]; }

    void SetNodeFinalState(NodeId const nodeId, bool value){ _nodeFinalStates[nodeId] = value; }
    bool GetNodeFinalState(NodeId const nodeId) { return _nodeFinalStates[nodeId]; }

    void SetStartNode(NodeId const nodeId) { _startStateId = nodeId; }
    bool IsStartNode(NodeId const nodeId) { return nodeId == _startStateId; }

    void SetFsmName(QString const name){ _fsmName = name; }

    void ToFile(std::string const filename) const;

    bool connectionExists(ConnectionId const connectionId) const override;

    Automaton* ToAutomaton() const;

    NodeId addNode(QString const nodeType = QString()) override;

    /**
   * Connection is possible when graph contains no connectivity data
   * in both directions `Out -> In` and `In -> Out`.
   */
    bool connectionPossible(ConnectionId const connectionId) const override;

    void addConnection(ConnectionId const connectionId) override;

    bool nodeExists(NodeId const nodeId) const override;

    QVariant nodeData(NodeId nodeId, NodeRole role) const override;

    bool setNodeData(NodeId nodeId, NodeRole role, QVariant value) override;

    QVariant portData(NodeId nodeId,
                      PortType portType,
                      PortIndex portIndex,
                      PortRole role) const override;

    bool setPortData(NodeId nodeId,
                     PortType portType,
                     PortIndex portIndex,
                     QVariant const &value,
                     PortRole role = PortRole::Data) override;

    bool deleteConnection(ConnectionId const connectionId) override;

    bool deleteNode(NodeId const nodeId) override;

    QJsonObject saveNode(NodeId const) const override;

    QJsonObject save() const;

    /// @brief Creates a new node based on the informatoin in `nodeJson`.
    /**
   * @param nodeJson conains a `NodeId`, node's position, internal node
   * information.
   */
    void loadNode(QJsonObject const &nodeJson) override;

    void load(QJsonObject const &jsonDocument);

    void addPort(NodeId nodeId, PortType portType, PortIndex portIndex);

    void removePort(NodeId nodeId, PortType portType, PortIndex first);

    NodeId newNodeId() override { return _nextNodeId++; }

    std::vector<std::pair<std::string, std::string>> variables;

private:
    std::unordered_set<NodeId> _nodeIds;
    std::unordered_map<NodeId, QString> _nodeNames;
    std::unordered_map<NodeId, QString> _nodeActionCodes;
    std::unordered_map<ConnectionId, QString> _connectionCodes;
    std::unordered_map<NodeId, bool> _nodeFinalStates;
    NodeId _startStateId = 0;
    QString _fsmName = "my_fsm";


    std::unordered_set<ConnectionId> _connectivity;

    mutable std::unordered_map<NodeId, NodeGeometryData> _nodeGeometryData;

    struct NodePortCount
    {
        unsigned int in = 0;
        unsigned int out = 0;
    };

    PortAddRemoveWidget *widget(NodeId) const;

    mutable std::unordered_map<NodeId, NodePortCount> _nodePortCounts;
    mutable std::unordered_map<NodeId, PortAddRemoveWidget *> _nodeWidgets;

    /// A convenience variable needed for generating unique node ids.
    NodeId _nextNodeId;
};
