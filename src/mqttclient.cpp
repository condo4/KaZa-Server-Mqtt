#include "mqttclient.h"
#include "mqtttopic.h"
#include <QDebug>

extern "C"
{
#include <MQTTAsync.h>
}

const int MQTT_PORT = 1883;

void connection_lost(void *context, char *cause)
{
    qWarning() << "Connection lost";
}

int message_arrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    MqttClient * const mq = static_cast<MqttClient*>(context);
    const QString topic {QString::fromUtf8(QByteArray(topicName, topicLen))};
    const QString msg(QString::fromUtf8(QByteArray(static_cast<char*>(message->payload), message->payloadlen)));
    mq->reciveMessage(topic, msg);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void delivery_complete(void* context, MQTTAsync_token token)
{
    qDebug() << "delivery_complete";

}

void on_connect_failure(void*, MQTTAsync_failureData*)
{
    qDebug() << "on_connect_failure";
}

void on_connect_success(void* context, MQTTAsync_successData*)
{
    MqttClient * const mq = static_cast<MqttClient*>(context);
    mq->setConnected(true);
    if(mq->debug())
        qDebug() << "on_connect_success";
}

void on_subscribe_failure(void* context, MQTTAsync_failureData*)
{
    MqttTopic * const topic = static_cast<MqttTopic*>(context);
    if(topic->server()->debug())
        qDebug() << "on_subscribe_failure " << topic->topic();
}

void on_subscribe_success(void* context, MQTTAsync_successData*)
{
    MqttTopic * const topic = static_cast<MqttTopic*>(context);

    if(topic->server()->debug())
        qDebug() << "on_subscribe_success " << topic->topic();
}

void on_unsubscribe_failure(void* context, MQTTAsync_failureData*)
{
    MqttTopic * const topic = static_cast<MqttTopic*>(context);
    if(topic->server()->debug())
        qDebug() << "on_unsubscribe_failure " << topic->topic();
}

void on_unsubscribe_success(void* context, MQTTAsync_successData*)
{
    MqttTopic * const topic = static_cast<MqttTopic*>(context);
    if(topic->server()->debug())
        qDebug() << "on_unsubscribe_success " << topic->topic();
}

void on_publish_failure(void* context, MQTTAsync_failureData*)
{
    MqttClient * const mq = static_cast<MqttClient*>(context);
    if(mq->debug())
        qDebug() << "on_publish_failure";
}

void on_publish_success(void* context, MQTTAsync_successData*)
{
    MqttClient * const mq = static_cast<MqttClient*>(context);
    if(mq->debug())
        qDebug() << "on_publish_success";
}

class MqttClientPrivate {
    Q_DISABLE_COPY(MqttClientPrivate)
    Q_DECLARE_PUBLIC(MqttClient)
    MqttClient * const q_ptr;

    QString m_hostname;
    int m_port {MQTT_PORT};
    QString m_clientId;
    MQTTAsync m_client;
    bool m_connected {false};
    QMap<QString, QList<MqttTopic*>> m_topics;

    MqttClientPrivate(MqttClient * mqttclient):
        q_ptr(mqttclient) {}
};


MqttClient::MqttClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new MqttClientPrivate(this))
{
    qDebug() << "MQTT integration loaded";
}

bool MqttClient::_connect()
{
    Q_D(MqttClient);
    if(!d->m_hostname.isEmpty() && d->m_port != 0 && !d->m_clientId.isEmpty()) {
        MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
        conn_opts.context = this;

        const QString address  = QString("tcp://%1:%2").arg(d->m_hostname).arg(d->m_port);

        if(m_debug)
            qDebug() << "_connect MqttClient to " << address;

        MQTTAsync_create(&d->m_client, qPrintable(address), qPrintable(d->m_clientId), MQTTCLIENT_PERSISTENCE_NONE, nullptr);
        MQTTAsync_setCallbacks(d->m_client, this, connection_lost, message_arrived, delivery_complete);

        conn_opts.keepAliveInterval = 60;
        conn_opts.cleansession = 1;
        conn_opts.onSuccess = on_connect_success;
        conn_opts.onFailure = on_connect_failure;
        conn_opts.context = static_cast<void*>(this);

        return MQTTASYNC_SUCCESS == MQTTAsync_connect(d->m_client, &conn_opts);
    }

    return false;
}

MqttClient::~MqttClient()
{

}

QString MqttClient::hostname() const
{
    Q_D(const MqttClient);
    return d->m_hostname;
}

void MqttClient::setHostname(const QString &newHostname)
{
    Q_D(MqttClient);
    int port  = d->m_port;
    QString hostname = newHostname;
    if(hostname.contains(":"))
    {
        QStringList h = hostname.split(":");
        hostname = h[0];
        port = h[1].toInt();
    }

    if (d->m_hostname == hostname && d->m_port == port)
        return;
    d->m_hostname = hostname;
    d->m_port = port;
    _connect();
    emit hostnameChanged();
}

QString MqttClient::clientId() const
{
    Q_D(const MqttClient);
    return d->m_clientId;
}

void MqttClient::setClientId(const QString &newClientId)
{
    Q_D(MqttClient);
    if (d->m_clientId == newClientId)
        return;
    d->m_clientId = newClientId;
    _connect();
    emit clientIdChanged();
}


bool MqttClient::connected() const
{
    Q_D(const MqttClient);
    return d->m_connected;
}

void MqttClient::setConnected(bool newConnected)
{
    Q_D(MqttClient);
    if(newConnected != d->m_connected)
    {
        d->m_connected = newConnected;
        for(const QString &key: d->m_topics.keys())
        {
            MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

            opts.onSuccess = on_subscribe_success;
            opts.onFailure = on_subscribe_failure;
            opts.context = static_cast<void*>(d->m_topics[key].first());

            const int rc = MQTTAsync_subscribe(d->m_client, qPrintable(key), 1, &opts);
            if (rc != MQTTASYNC_SUCCESS)
            {
                qWarning() << "MQTTAsync_subscribe failed for " << key;
                return;
            }
        }
        emit connectedChanged();
    }
}

void MqttClient::reciveMessage(QString topictitle, QString payload)
{
    Q_D(MqttClient);
    emit message(topictitle, payload);
    if(d->m_topics.contains(topictitle))
    {
        if(m_debug)
        {
            qDebug() << "RX " << topictitle;
        }
        for(MqttTopic *topic: std::as_const(d->m_topics[topictitle]))
        {
            topic->reciveMessage(payload);
        }
    }
}

void MqttClient::unregisterTopic(MqttTopic *topic)
{
    Q_D(MqttClient);
    if(d->m_topics.contains(topic->topic()))
    {
        d->m_topics[topic->topic()].removeAll(topic);
        if(d->m_topics[topic->topic()].empty())
        {
            // Last topic is removed, can unsubscribe
            MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

            opts.onSuccess = on_unsubscribe_success;
            opts.onFailure =  on_unsubscribe_failure;
            opts.context = static_cast<void*>(topic);

            const auto& rc = MQTTAsync_unsubscribe(d->m_client, qPrintable(topic->topic()), &opts);
            if (rc != MQTTASYNC_SUCCESS)
            {
                qWarning() << "MQTTAsync_unsubscribe failed for " << topic->topic();
                return;
            }
        }
    }
}

void MqttClient::registerTopic(MqttTopic *topic)
{
    Q_D(MqttClient);
    if(!d->m_topics.contains(topic->topic()))
    {
        // First MqttTopic for this topic
        d->m_topics[topic->topic()] = QList<MqttTopic*>();

        if(d->m_connected)
        {
            MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

            opts.onSuccess = on_subscribe_success;
            opts.onFailure = on_subscribe_failure;
            opts.context = static_cast<void*>(topic);

            const int rc = MQTTAsync_subscribe(d->m_client, qPrintable(topic->topic()), 1, &opts);
            if (rc != MQTTASYNC_SUCCESS)
            {
                qWarning() << "MQTTAsync_subscribe failed for " << topic->topic();
                return;
            }
        }
        d->m_topics[topic->topic()].append(topic);
    }
}

void MqttClient::publish(const QString &topic, const QString &message, int qos, bool retain)
{
    Q_D(MqttClient);

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

    opts.onSuccess = on_publish_success;
    opts.onFailure = on_publish_failure;
    opts.context = static_cast<void*>(this);

    QByteArray latin1Msg {message.toLatin1()};
    pubmsg.payload = static_cast<void*>(latin1Msg.data());
    pubmsg.payloadlen = latin1Msg.size();
    pubmsg.qos = qos;
    pubmsg.retained = static_cast<char>(retain);

    const int rc = MQTTAsync_sendMessage(d->m_client, qPrintable(topic), &pubmsg, &opts);
    if (rc != MQTTASYNC_SUCCESS) {
        qWarning() << "Failed to start sendMessage, return code" << rc;
    }
}

bool MqttClient::debug() const
{
    return m_debug;
}

void MqttClient::setDebug(bool newDebug)
{
    if (m_debug == newDebug)
        return;
    m_debug = newDebug;
    emit debugChanged();
}
