#ifndef MQTTTOPIC_H
#define MQTTTOPIC_H

#include <kazaobject.h>

class MqttClient;

class MqttTopic : public KaZaObject
{
    Q_OBJECT
    Q_PROPERTY(MqttClient* server READ server WRITE setServer NOTIFY serverChanged FINAL)
    Q_PROPERTY(QString topic READ topic WRITE setTopic NOTIFY topicChanged FINAL)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged FINAL)

    MqttClient *m_server {nullptr};
    QString m_topic;

public:
    explicit MqttTopic(QObject *parent = nullptr);
    ~MqttTopic();

    MqttClient* server() const;
    void setServer(MqttClient* server);

    QString topic() const;
    void setTopic(const QString &newTopic);

    void reciveMessage(const QString &payload);

signals:
    void serverChanged();
    void topicChanged();
};

#endif // MQTTTOPIC_H
