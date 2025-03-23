#include "plugin.h"
#include "mqttclient.h"
#include "mqtttopic.h"

#include <qqml.h>

void MqttPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<MqttClient>(uri, 1, 0, "MqttClient");
    qmlRegisterType<MqttTopic>(uri, 1, 0, "MqttTopic");
}
