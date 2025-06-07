/**
 * @file DynamicPortsModel.hpp
 * @brief Declaration of the DynamicPortsModel class for managing dynamic nodes and connections in the FSM editor.
 *
 * This file contains the declaration of the DynamicPortsModel class, which provides functionality
 * for creating, modifying, and serializing nodes and connections in the graphical FSM editor.
 * It supports adding/removing nodes and ports, managing node and connection data, and converting
 * the internal model to and from persistent representations (such as JSON or custom file formats).
 *
 * Key responsibilities:
 * - Managing the set of nodes and connections in the FSM editor.
 * - Providing methods to add, remove, and query nodes, ports, and connections.
 * - Handling serialization and deserialization of the FSM graph to/from JSON and custom formats.
 * - Supporting the conversion of the graphical model to an Automaton object for code generation.
 * - Emitting signals for UI updates when the model changes.
 *
 * Notable mentions:
 *  - DynamicPortsModel class has been inspired by the example provided in the
 *    github repository of the nodeeditor library "dynamic_ports"
 *    link: https://github.com/paceholder/nodeeditor/tree/master/examples
 * 
 * @author Jakub Kovařík, Josef Ambruz
 * @date 2025-5-11
 */

#pragma once


#include "PortAddRemoveWidget.hpp"

#include <QtNodes/ConnectionIdUtils>

#include <QJsonArray>

#include <iterator>
#include <fstream>
#include <iostream>

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
        QSize size;      ///< The size of the node.
        QPointF pos;     ///< The position of the node.
    };

public:
    /**
     * @brief Constructs a new DynamicPortsModel object.
     */
    DynamicPortsModel();

    /**
     * @brief Destructor for DynamicPortsModel.
     */
    ~DynamicPortsModel() override;

    /**
     * @brief Returns a set of all node IDs in the model.
     * @return Set of NodeId.
     */
    std::unordered_set<NodeId> allNodeIds() const override;

    /**
     * @brief Returns all connection IDs associated with a given node.
     * @param nodeId The node ID.
     * @return Set of ConnectionId.
     */
    std::unordered_set<ConnectionId> allConnectionIds(NodeId const nodeId) const override;

    /**
     * @brief Returns all connections for a given node, port type, and port index.
     * @param nodeId The node ID.
     * @param portType The port type (input/output).
     * @param portIndex The port index.
     * @return Set of ConnectionId.
     */
    std::unordered_set<ConnectionId> connections(NodeId nodeId,
                                                 PortType portType,
                                                 PortIndex portIndex) const override;

    /**
     * @brief Returns the caption for the model.
     * @return The caption string.
     */
    QString caption() const  { return QStringLiteral("Result"); }

    /**
     * @brief Gets the name of a node.
     * @param nodeId The node ID.
     * @return The node name.
     */
    QString GetNodeName(NodeId const nodeId) { return _nodeNames[nodeId]; }

    /**
     * @brief Sets the name of a node.
     * @param nodeId The node ID.
     * @param name The new name.
     */
    void SetNodeName(NodeId const nodeId, QString name) {
        _nodeNames[nodeId] = name;
        setNodeData(nodeId, QtNodes::NodeRole::Caption, QVariant::fromValue(name));
    }

    /**
     * @brief Sets the action code for a node.
     * @param nodeId The node ID.
     * @param code The action code.
     */
    void SetNodeActionCode(NodeId const nodeId, QString code) { _nodeActionCodes[nodeId] = code; }

    /**
     * @brief Gets the action code for a node.
     * @param nodeId The node ID.
     * @return The action code.
     */
    QString GetNodeActionCode(NodeId const nodeId) { return _nodeActionCodes[nodeId]; }

    /**
     * @brief Sets the condition code for a connection.
     * @param connId The connection ID.
     * @param code The condition code.
     */
    void SetConnectionCode(ConnectionId const connId, QString code){ _connectionCodes[connId] = code; }
    
    /**
     * @brief Gets the condition code for a connection.
     * @param connId The connection ID.
     * @return The condition code.
     */
    QString GetConnectionCode(ConnectionId const connId) { return _connectionCodes[connId]; }

    /**
     * @brief Sets whether a node is a final state.
     * @param nodeId The node ID.
     * @param value True if final state, false otherwise.
     */
    void SetNodeFinalState(NodeId const nodeId, bool value){ _nodeFinalStates[nodeId] = value; }
    
    /**
     * @brief Gets whether a node is a final state.
     * @param nodeId The node ID.
     * @return True if final state, false otherwise.
     */
    bool GetNodeFinalState(NodeId const nodeId) { return _nodeFinalStates[nodeId]; }

    /**
     * @brief Sets the start node.
     * @param nodeId The node ID to set as start.
     */
    void SetStartNode(NodeId const nodeId) { _startStateId = nodeId; }
    
    /**
     * @brief Checks if a node is the start node.
     * @param nodeId The node ID.
     * @return True if start node, false otherwise.
     */    
    bool IsStartNode(NodeId const nodeId) { return nodeId == _startStateId; }
    
    /**
     * @brief Sets the delay (in ms) for a connection.
     * @param connId The connection ID.
     * @param value The delay in milliseconds.
     */
    void SetConnectionDelay(ConnectionId const connId, int value) { _connectionDelays[connId] = value; }
    
    /**
     * @brief Gets the delay (in ms) for a connection.
     * @param connId The connection ID.
     * @return The delay in milliseconds.
     */
    int  GetConnectionDelay(ConnectionId const connId) { return _connectionDelays[connId]; }

    /**
     * @brief Sets the FSM name.
     * @param name The FSM name.
     */
    void SetFsmName(QString const name){ _fsmName = name; }

    /**
     * @brief Saves the model to a file.
     * @param filename The file name.
     */
    void ToFile(std::string const filename) const;

    /**
     * @brief Checks if a connection exists.
     * @param connectionId The connection ID.
     * @return True if exists, false otherwise.
     */
    bool connectionExists(ConnectionId const connectionId) const override;

    /**
     * @brief Converts the model to an Automaton object.
     * @return Pointer to the Automaton.
     */
    Automaton* ToAutomaton() const;

    /**
     * @brief Adds a new node to the model.
     * @param nodeType The type of node (optional).
     * @return The new node ID.
     */
    NodeId addNode(QString const nodeType = QString()) override;

    void forceNodeUiUpdate(NodeId const id);

    /**
     * @brief Checks if a connection is possible (no duplicate or reverse).
     * @param connectionId The connection ID.
     * @return True if possible, false otherwise.
     */
    bool connectionPossible(ConnectionId const connectionId) const override;

    /**
     * @brief Adds a connection to the model.
     * @param connectionId The connection ID.
     */
    void addConnection(ConnectionId const connectionId) override;

/**
     * @brief Checks if a node exists.
     * @param nodeId The node ID.
     * @return True if exists, false otherwise.
     */
    bool nodeExists(NodeId const nodeId) const override;

    /**
     * @brief Finds a node by its name.
     * @param nodeName The node name.
     * @return The node ID, or InvalidNodeId if not found.
     */
    NodeId findNodeByName(QString const nodeName);

    /**
     * @brief Gets data for a node.
     * @param nodeId The node ID.
     * @param role The node role.
     * @return The data as QVariant.
     */
    QVariant nodeData(NodeId nodeId, NodeRole role) const override;

    /**
     * @brief Sets data for a node.
     * @param nodeId The node ID.
     * @param role The node role.
     * @param value The value to set.
     * @return True if successful, false otherwise.
     */
    bool setNodeData(NodeId nodeId, NodeRole role, QVariant value) override;

    /**
     * @brief Gets data for a port.
     * @param nodeId The node ID.
     * @param portType The port type.
     * @param portIndex The port index.
     * @param role The port role.
     * @return The data as QVariant.
     */
    QVariant portData(NodeId nodeId,
                      PortType portType,
                      PortIndex portIndex,
                      PortRole role) const override;
    /**
     * @brief Sets data for a port.
     * @param nodeId The node ID.
     * @param portType The port type.
     * @param portIndex The port index.
     * @param value The value to set.
     * @param role The port role (default: Data).
     * @return True if successful, false otherwise.
     */
    bool setPortData(NodeId nodeId,
                     PortType portType,
                     PortIndex portIndex,
                     QVariant const &value,
                     PortRole role = PortRole::Data) override;
    /**
     * @brief Deletes a connection from the model.
     * @param connectionId The connection ID.
     * @return True if successful, false otherwise.
     */
    bool deleteConnection(ConnectionId const connectionId) override;

    /**
     * @brief Deletes a node from the model.
     * @param nodeId The node ID.
     * @return True if successful, false otherwise.
     */
    bool deleteNode(NodeId const nodeId) override;
    /**
     * @brief Saves a node to a QJsonObject.
     * @param nodeId The node ID.
     * @return The QJsonObject representing the node.
     */
    QJsonObject saveNode(NodeId const) const override;
    /**
     * @brief Saves the entire model to a QJsonObject.
     * @return The QJsonObject representing the model.
     */
    QJsonObject save() const;


    /**
     * @brief Loads a node from a QJsonObject.
     * @param nodeJson The QJsonObject containing node data.
     */
    void loadNode(QJsonObject const &nodeJson) override;
    /**
     * @brief Loads the model from a QJsonObject.
     * @param jsonDocument The QJsonObject containing model data.
     */
    void load(QJsonObject const &jsonDocument);
    /**
     * @brief Adds a port to a node.
     * @param nodeId The node ID.
     * @param portType The port type.
     * @param portIndex The port index.
     */
    void addPort(NodeId nodeId, PortType portType, PortIndex portIndex);
    /**
     * @brief Removes a port from a node.
     * @param nodeId The node ID.
     * @param portType The port type.
     * @param first The port index.
     */
    void removePort(NodeId nodeId, PortType portType, PortIndex first);
    /**
     * @brief Generates a new unique node ID.
     * @return The new node ID.
     */
    NodeId newNodeId() override { return _nextNodeId++; }
    /**
     * @brief List of variable name/value pairs for the FSM.
     */
    std::vector<std::pair<std::string, std::string>> variables;

private:
    std::unordered_set<NodeId> _nodeIds;
    std::unordered_map<NodeId, QString> _nodeNames;
    std::unordered_map<NodeId, QString> _nodeActionCodes;
    std::unordered_map<ConnectionId, QString> _connectionCodes;
    std::unordered_map<ConnectionId, int> _connectionDelays;
    std::unordered_map<NodeId, bool> _nodeFinalStates;
    NodeId _startStateId = 0;
    QString _fsmName = "my_fsm";
    void writeNodeData(ofstream& os, NodeId const nodeId) const;

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
