#ifndef FSMCLIENT_HPP
#define FSMCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug> // For qInfo, qWarning, etc.

class FsmClient : public QObject
{
    Q_OBJECT
public:
    explicit FsmClient(QObject *parent = nullptr);
    ~FsmClient();

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();

    // Methods to send commands to the FSM
    void sendSetVariable(const QString &variableName, const QJsonValue &value);
    void sendStopFsm();

signals:
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject &message); // Emits the parsed JSON object
    void fsmError(const QString &errorMessage);

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onReadyRead();

private:
    QTcpSocket *m_socket;
    QByteArray m_buffer; // Buffer for incoming data

    void sendMessage(const QJsonObject &message);
};

#endif // FSMCLIENT_HPP
