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
    void reciveMessage(QString topic, QString payload);

    // Topic interface
    void unregisterTopic(MqttTopic *topic);
    void registerTopic(MqttTopic *topic);


signals:
    void hostnameChanged();
    void clientIdChanged();
    void connectedChanged();

private:

    bool _connect();
};

#endif // MQTTCLIENT_H
