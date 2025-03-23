#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QtQml/qqmlregistration.h>
#include <QObject>

class MqttClientPrivate;
class MqttTopic;

class MqttClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString hostname READ hostname WRITE setHostname NOTIFY hostnameChanged FINAL)
    Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged FINAL)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged FINAL)
    Q_PROPERTY(bool debug READ debug WRITE setDebug NOTIFY debugChanged FINAL)


    Q_DECLARE_PRIVATE(MqttClient)
    QML_ELEMENT
    Q_DISABLE_COPY(MqttClient)
    QScopedPointer<MqttClientPrivate> const d_ptr;

public:
    explicit MqttClient(QObject *parent = nullptr);
    ~MqttClient() override;

    QString hostname() const;
    void setHostname(const QString &newHostname);

    QString clientId() const;
    void setClientId(const QString &newClientId);

    bool connected() const;
    void setConnected(bool newConnected);

    // Paho interface
    void reciveMessage(QString topictitle, QString payload);

    // Topic interface
    void unregisterTopic(MqttTopic *topic);
    void registerTopic(MqttTopic *topic);

    Q_INVOKABLE void publish(const QString &topic, const QString &message, int qos, bool retain);

    bool debug() const;
    void setDebug(bool newDebug);

signals:
    void hostnameChanged();
    void clientIdChanged();
    void connectedChanged();
    void message(QString topic, QString payload);

    void debugChanged();

private:

    bool _connect();
    bool m_debug;
};

#endif // MQTTCLIENT_H
