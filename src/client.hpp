/**
 * @file client.hpp
 * @brief Declaration of the FsmClient class for TCP communication with the Python FSM server.
 *
 * The FsmClient class provides methods to connect to the FSM server, send commands,
 * and receive messages using Qt's QTcpSocket. It emits signals for connection events,
 * received messages, and errors.
 * 
 * @author Josef Ambruz
 * @date 2025-5-11
 */

#ifndef FSMCLIENT_HPP
#define FSMCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug> // For qInfo, qWarning, etc.

/**
 * @class FsmClient
 * @brief TCP client for communicating with the Python FSM server.
 *
 * This class manages a TCP connection to the FSM server, allowing the application
 * to send commands (such as setting variables or stopping the FSM) and receive
 * JSON messages from the server. It emits signals for connection status, received
 * messages, and errors.
 */
class FsmClient : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the FsmClient object.
     * @param parent The parent QObject.
     */
    explicit FsmClient(QObject *parent = nullptr);

    /**
     * @brief Destructor for FsmClient.
     */
    ~FsmClient();

    /**
     * @brief Connects to the FSM server at the specified host and port.
     * @param host The server hostname or IP address.
     * @param port The server port.
     */
    void connectToServer(const QString &host, quint16 port);

    /**
     * @brief Checks if the client is currently connected to the server.
     * @return True if connected, false otherwise.
     */
    bool isConnected() const;

    /**
     * @brief Disconnects from the FSM server.
     */
    void disconnectFromServer();

    /**
     * @brief Sends a command to set a variable on the FSM server.
     * @param variableName The name of the variable.
     * @param value The value to set.
     */
    void sendSetVariable(const QString &variableName, const QJsonValue &value);

    /**
     * @brief Sends a command to stop the FSM on the server.
     */
    void sendStopFsm();

signals:
    /**
     * @brief Emitted when the client successfully connects to the server.
     */
    void connected();

    /**
     * @brief Emitted when the client disconnects from the server.
     */
    void disconnected();

    /**
     * @brief Emitted when a JSON message is received from the server.
     * @param message The received JSON object.
     */
    void messageReceived(const QJsonObject &message);

    /**
     * @brief Emitted when an error occurs in the client.
     * @param errorMessage The error message.
     */
    void fsmError(const QString &errorMessage);

private slots:
    /**
     * @brief Slot called when the socket successfully connects.
     */
    void onConnected();

    /**
     * @brief Slot called when the socket disconnects.
     */
    void onDisconnected();

    /**
     * @brief Slot called when a socket error occurs.
     * @param socketError The socket error code.
     */
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

    /**
     * @brief Slot called when data is available to read from the socket.
     */
    void onReadyRead();

private:
    QTcpSocket *m_socket;   ///< The TCP socket for communication.
    QByteArray m_buffer;    ///< Buffer for incoming data.

    /**
     * @brief Sends a JSON message to the server.
     * @param message The JSON object to send.
     */
    void sendMessage(const QJsonObject &message);
};

#endif // FSMCLIENT_HPP
