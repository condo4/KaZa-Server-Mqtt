#include "mqtttopic.h"
#include "mqttclient.h"
#include <QJsonDocument>
#include <QDebug>

MqttTopic::MqttTopic(QObject *parent)
    : QObject{parent}
{
#ifdef DEBUG
    qDebug() << "Create MqttTopic";
#endif
}

MqttTopic::~MqttTopic()
{

}

MqttClient* MqttTopic::server() const
{
    return m_server;
}

void MqttTopic::setServer(MqttClient* server)
{
    if(m_server != server)
    {
        if(m_server != nullptr)
        {
            m_server->unregisterTopic(this);
        }
        m_server = server;
        if(not m_topic.isEmpty())
        {
            m_server->registerTopic(this);
        }
        emit serverChanged();
    }
}

QString MqttTopic::topic() const
{
    return m_topic;
}

void MqttTopic::setTopic(const QString &newTopic)
{
    if(m_topic != newTopic)
    {
        if(m_server != nullptr)
        {
            m_server->unregisterTopic(this);
            m_topic = newTopic;
            m_server->registerTopic(this);
        }
        else
        {
            m_topic = newTopic;
        }
        emit topicChanged();
    }
}

void MqttTopic::reciveMessage(const QString &payload)
{
    QVariant newvalue = m_value;

    if(payload.startsWith("{"))
    {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(payload.toUtf8());
        newvalue = jsonResponse["value"];
    }
    else
    {

        if(payload.startsWith("false") || payload.startsWith("true"))
        {
            newvalue.setValue(payload.startsWith("true"));
        }
        else
        {
            double raw;
            bool ok;
            raw = payload.toDouble(&ok);
            if(ok)
            {
                newvalue.setValue(raw);
            }
        }
    }
    setValue(newvalue);
}

QVariant MqttTopic::value() const
{
    return m_value;
}

void MqttTopic::setValue(const QVariant &newValue)
{
    if (m_value == newValue)
        return;
    m_value = newValue;
    emit valueChanged();
}
