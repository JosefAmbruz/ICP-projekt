/**
 * @file PortAddRemoveWidget.hpp
 * @brief Declaration of the PortAddRemoveWidget class for dynamic port management in the FSM editor.
 *
 * This file contains the declaration of the PortAddRemoveWidget class, which provides a QWidget-based
 * interface for adding and removing ports on nodes in the graphical FSM editor. The widget displays
 * groups of [+] and [-] buttons for each port, allowing users to dynamically modify the number of
 * input and output ports on a node.
 *
 * The widget is composed of two main vertical layouts (left and right), each containing groups of
 * buttons for adding and removing ports. Each group is managed using a QHBoxLayout within the
 * corresponding QVBoxLayout.
 *
 * Key responsibilities:
 * - Displaying and managing [+] and [-] button groups for each port.
 * - Handling user interactions to add or remove ports.
 * - Communicating port changes to the associated DynamicPortsModel.
 *
 * Example layout:
 * ```
 *       _left                         _right
 *       layout                        layout
 *     ----------------------------------------
 *     |         |                  |         |
 *     | [+] [-] |                  | [+] [-] |
 *     |         |                  |         |
 *     | [+] [-] |                  | [+] [-] |
 *     |         |                  |         |
 *     | [+] [-] |                  | [+] [-] |
 *     |         |                  |         |
 *     | [+] [-] |                  | [+] [-] |
 *     |         |                  |         |
 *     |_________|__________________|_________|
 * ```
 *
 * @author Jakub Kovařík
 * @date 2025-5-11
 */

#pragma once

#include <QPushButton>
#include <QWidget>

#include <QtNodes/Definitions>

#include <QHBoxLayout>
#include <QVBoxLayout>

using QtNodes::NodeId;
using QtNodes::PortIndex;
using QtNodes::PortType;

class DynamicPortsModel;

/**
 * @class PortAddRemoveWidget
 * @brief Widget for dynamically adding and removing ports on a node.
 *
 * This widget provides a user interface for managing the number of input and output ports
 * on a node in the FSM editor. It displays groups of [+] and [-] buttons for each port,
 * allowing users to add or remove ports interactively.
 */
class PortAddRemoveWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a PortAddRemoveWidget.
     * @param nInPorts Number of initial input ports.
     * @param nOutPorts Number of initial output ports.
     * @param nodeId The ID of the node this widget is associated with.
     * @param model Reference to the DynamicPortsModel.
     * @param parent The parent QWidget.
     */
    PortAddRemoveWidget(unsigned int nInPorts,
                        unsigned int nOutPorts,
                        NodeId nodeId,
                        DynamicPortsModel &model,
                        QWidget *parent = nullptr);
    /**
     * @brief Destructor for PortAddRemoveWidget.
     */
    ~PortAddRemoveWidget();

    /**
     * @brief Populates the widget with button groups according to the number of ports.
     * @param portType The type of port (input/output).
     * @param nPorts The number of ports to create button groups for.
     */
    void populateButtons(PortType portType, unsigned int nPorts);

    /**
     * @brief Adds a single [+][-] button group to a given layout.
     * @param vbl The QVBoxLayout to add the button group to.
     * @param portIndex The index of the port.
     * @return Pointer to the created QHBoxLayout containing the button group.
     */
    QHBoxLayout *addButtonGroupToLayout(QVBoxLayout *vbl, unsigned int portIndex);

    /**
     * @brief Removes a single [+][-] button group from a given layout.
     * @param vbl The QVBoxLayout to remove the button group from.
     * @param portIndex The index of the port.
     */
    void removeButtonGroupFromLayout(QVBoxLayout *vbl, unsigned int portIndex);

private Q_SLOTS:
    /**
     * @brief Slot called when a plus button is clicked to add a port.
     */
    void onPlusClicked();

    /**
     * @brief Slot called when a minus button is clicked to remove a port.
     */
    void onMinusClicked();
private:
    /**
     * @brief Determines which port was clicked based on the sender and button index.
     * @param sender The QObject that sent the signal.
     * @param buttonIndex The index of the button (0 for plus, 1 for minus).
     * @return Pair of PortType and PortIndex indicating the port.
     */
    std::pair<PortType, PortIndex> findWhichPortWasClicked(QObject *sender, int const buttonIndex);

private:
NodeId const _nodeId;              ///< The node ID this widget is associated with.
DynamicPortsModel &_model;         ///< Reference to the model for port management.
QVBoxLayout *_left;                ///< Layout for input port button groups.
QVBoxLayout *_right;               ///< Layout for output port button groups.
};
