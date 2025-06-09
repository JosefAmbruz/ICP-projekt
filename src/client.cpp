
/**
 * @file client.cpp
 * @brief Implementation of the FsmClient class for managing communication with an FSM server.
 * 
 * This file contains the implementation of the FsmClient class, which provides functionality
 * to connect to an FSM server, send messages, and handle incoming messages. The communication
 * is based on JSON messages sent over a QTcpSocket.
 *
 * @author Josef Ambruz
 * @date 2025-5-11
 */

 /**
 * @class FsmClient
 * @brief A client class for communicating with an FSM server using JSON over TCP.
 * 
 * The FsmClient class provides methods to connect to an FSM server, send commands, and
 * handle incoming messages. It uses Qt's QTcpSocket for network communication and emits
 * signals for various events such as connection, disconnection, and errors.
 * 
 * @note This class assumes that JSON messages are newline-delimited.
 * 
 * @signal connected() Emitted when the client successfully connects to the server.
 * @signal disconnected() Emitted when the client disconnects from the server.
 * @signal fsmError(const QString &error) Emitted when a socket error occurs.
 * @signal messageReceived(const QJsonObject &message) Emitted when a valid JSON message is received.
 */
#include "client.hpp"

FsmClient::FsmClient(QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &FsmClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &FsmClient::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &FsmClient::onErrorOccurred);
    connect(m_socket, &QTcpSocket::readyRead, this, &FsmClient::onReadyRead);
}

FsmClient::~FsmClient()
{
    // m_socket is a child of FsmClient, so it will be deleted automatically
    // if it's still connected, it will be closed upon deletion.
}

void FsmClient::connectToServer(const QString &host, quint16 port)
{
    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        qInfo() << "[Client] Attempting to connect to" << host << ":" << port;
        m_socket->connectToHost(host, port);
    } else {
        qWarning() << "[Client] Already connected or connecting.";
    }
}

bool FsmClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void FsmClient::disconnectFromServer()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        qInfo() << "[Client] Disconnecting from server.";
        m_socket->disconnectFromHost();
        if (m_socket->state() == QAbstractSocket::UnconnectedState) {
            // If already disconnected (e.g. server closed connection), ensure our signal fires.
            // onDisconnected will be called by the socket if it was connected.
        }
    }
}

void FsmClient::sendSetVariable(const QString &variableName, const QJsonValue &value)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "[Client] Cannot send message: Not connected.";
        return;
    }

    QJsonObject payload;
    payload["name"] = variableName;
    payload["value"] = value;

    QJsonObject message;
    message["type"] = "SET_VARIABLE";
    message["payload"] = payload;

    sendMessage(message);
}

void FsmClient::sendStopFsm()
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "[Client] Cannot send message: Not connected.";
        return;
    }

    QJsonObject message;
    message["type"] = "STOP_FSM";
    message["payload"] = QJsonObject(); // Empty payload

    sendMessage(message);
}


void FsmClient::sendMessage(const QJsonObject &message)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "[Client] Error sending: Not connected.";
        return;
    }
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n"; // Add newline delimiter
    qint64 bytesWritten = m_socket->write(data);
    if (bytesWritten == -1) {
        qWarning() << "[Client] Error writing to socket:" << m_socket->errorString();
    } else if (bytesWritten < data.size()) {
        qWarning() << "[Client] Not all data written to socket. Wrote" << bytesWritten << "of" << data.size();
        // Handle partial writes if necessary, though for small JSON it's less common
    } else {
        qInfo() << "[Client -> FSM] Sent:" << message; // Can be verbose
    }
    m_socket->flush(); // Ensure data is sent immediately
}


void FsmClient::onConnected()
{
    qInfo() << "[Client] Successfully connected to FSM server.";
    m_buffer.clear(); // Clear buffer on new connection
    emit connected();
}

void FsmClient::onDisconnected()
{
    qInfo() << "[Client] Disconnected from FSM server.";
    emit disconnected();
}

void FsmClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
     // Only emit fsmError for real errors, not for normal disconnects
    if (socketError == QAbstractSocket::RemoteHostClosedError) {
        qInfo() << "[Client] Remote host closed the connection (normal disconnect).";
        return;
    }

    Q_UNUSED(socketError); // socketError parameter is not directly used here
    qWarning() << "[Client] Socket error:" << m_socket->errorString();
    emit fsmError(m_socket->errorString());
    // The disconnected() signal will usually follow an error that causes disconnection.
}

void FsmClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    // Process all complete JSON messages in the buffer
    // Messages are expected to be newline-terminated
    while (true) {
        int newlinePos = m_buffer.indexOf('\n');
        if (newlinePos == -1) {
            break; // No complete message yet
        }

        QByteArray jsonData = m_buffer.left(newlinePos);
        m_buffer = m_buffer.mid(newlinePos + 1); // Remove processed message (and the newline)

        if (jsonData.trimmed().isEmpty()) { // Skip if it was just a newline or whitespace
            continue;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "[Client] Failed to parse JSON:" << parseError.errorString();
            qWarning() << "[Client] Corrupted JSON data:" << QString::fromUtf8(jsonData);
            emit fsmError("Received corrupted JSON data from FSM.");
            continue;
        }

        if (doc.isObject()) {
            // qInfo() << "[FSM -> Client] Received:" << doc.object(); // Can be verbose
            emit messageReceived(doc.object());
        } else {
            qWarning() << "[Client] Received JSON is not an object:" << QString::fromUtf8(jsonData);
        }
    }
}
