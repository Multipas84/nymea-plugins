/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "integrationpluginowlet.h"
#include "plugininfo.h"
#include "owletclient.h"
#include "owletserialclient.h"

#include "hardwaremanager.h"
#include "platform/platformzeroconfcontroller.h"
#include "network/zeroconf/zeroconfservicebrowser.h"
#include "network/zeroconf/zeroconfserviceentry.h"

#include "owlettcptransport.h"
#include "owletserialtransport.h"

#include <QColor>
#include <QTimer>

IntegrationPluginOwlet::IntegrationPluginOwlet()
{
}

void IntegrationPluginOwlet::init()
{
    m_owletIdParamTypeMap.insert(digitalOutputThingClassId, digitalOutputThingOwletIdParamTypeId);
    m_owletIdParamTypeMap.insert(digitalInputThingClassId, digitalInputThingOwletIdParamTypeId);
    m_owletIdParamTypeMap.insert(ws2812ThingClassId, ws2812ThingOwletIdParamTypeId);

    m_owletSerialPortParamTypeMap.insert(arduinoMiniProThingClassId, arduinoMiniProThingSerialPortParamTypeId);

    m_zeroConfBrowser = hardwareManager()->zeroConfController()->createServiceBrowser("_nymea-owlet._tcp");

    // Pin params of thins
    m_owletSerialPinParamTypeMap.insert(digitalOutputSerialThingClassId, digitalOutputSerialThingPinParamTypeId);
    m_owletSerialPinParamTypeMap.insert(digitalInputSerialThingClassId, digitalInputSerialThingPinParamTypeId);
    m_owletSerialPinParamTypeMap.insert(analogOutputSerialThingClassId, analogOutputSerialThingPinParamTypeId);
    m_owletSerialPinParamTypeMap.insert(analogInputSerialThingClassId, analogInputSerialThingPinParamTypeId);
    m_owletSerialPinParamTypeMap.insert(servoSerialThingClassId, servoSerialThingPinParamTypeId);

    // Arduino Mini Pro mapping
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin2ParamTypeId, 2);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin3ParamTypeId, 3);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin4ParamTypeId, 4);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin5ParamTypeId, 5);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin6ParamTypeId, 6);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin7ParamTypeId, 7);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin8ParamTypeId, 8);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin9ParamTypeId, 9);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin10ParamTypeId, 10);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin11ParamTypeId, 11);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin12ParamTypeId, 12);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPin13ParamTypeId, 13);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPinA1ParamTypeId, 15);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPinA2ParamTypeId, 16);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPinA3ParamTypeId, 17);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPinA4ParamTypeId, 18);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPinA5ParamTypeId, 19);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPinA6ParamTypeId, 20);
    m_arduinoMiniProPinMapping.insert(arduinoMiniProSettingsPinA7ParamTypeId, 21);
}

void IntegrationPluginOwlet::discoverThings(ThingDiscoveryInfo *info)
{
    if (info->thingClassId() == arduinoMiniProThingClassId) {
        // Discover serial ports for arduino bords
        foreach(const QSerialPortInfo &port, QSerialPortInfo::availablePorts()) {
            qCDebug(dcOwlet()) << "Found serial port" << port.systemLocation();
            QString description = port.systemLocation() + " " + port.manufacturer() + " " + port.description();
            ThingDescriptor thingDescriptor(info->thingClassId(), "Arduino Pro Mini Owlet", description);
            ParamList parameters;

            foreach (Thing *existingThing, myThings()) {
                if (existingThing->paramValue(m_owletSerialPortParamTypeMap.value(info->thingClassId())).toString() == port.systemLocation()) {
                    thingDescriptor.setThingId(existingThing->id());
                    break;
                }
            }

            parameters.append(Param(m_owletSerialPortParamTypeMap.value(info->thingClassId()), port.systemLocation()));
            thingDescriptor.setParams(parameters);
            info->addThingDescriptor(thingDescriptor);
        }
        info->finish(Thing::ThingErrorNoError);
    } else {
        foreach (const ZeroConfServiceEntry &entry, m_zeroConfBrowser->serviceEntries()) {
            qCDebug(dcOwlet()) << "Found owlet:" << entry;
            ThingDescriptor descriptor(info->thingClassId(), entry.name(), entry.txt("platform"));
            descriptor.setParams(ParamList() << Param(m_owletIdParamTypeMap.value(info->thingClassId()), entry.txt("id")));
            foreach (Thing *existingThing, myThings().filterByParam(m_owletIdParamTypeMap.value(info->thingClassId()), entry.txt("id"))) {
                descriptor.setThingId(existingThing->id());
                break;
            }
            info->addThingDescriptor(descriptor);
        }
        info->finish(Thing::ThingErrorNoError);
    }
}


void IntegrationPluginOwlet::setupThing(ThingSetupInfo *info)
{
    Thing *thing = info->thing();

    if (thing->thingClassId() == arduinoMiniProThingClassId) {
        QString serialPort = thing->paramValue(arduinoMiniProThingSerialPortParamTypeId).toString();
        qCDebug(dcOwlet()) << "Setup arduino mini pro owlet on" << serialPort;
        OwletTransport *transport = new OwletSerialTransport(serialPort, 115200, this);
        OwletSerialClient *client = new OwletSerialClient(transport, transport);

        // During setup
        connect(client, &OwletSerialClient::connected, info, [=](){
            qCDebug(dcOwlet()) << "Connected to serial owlet" << client->firmwareVersion();
            thing->setStateValue("connected", true);
        });

        connect(client, &OwletSerialClient::error, info, [=](){
            //info->finish(Thing::ThingErrorHardwareFailure);
            transport->deleteLater();
        });

        // Runtime
        connect(client, &OwletSerialClient::connected, thing, [=](){
            thing->setStateValue("connected", true);
            thing->setStateValue(arduinoMiniProCurrentVersionStateTypeId, client->firmwareVersion());
            foreach (Thing *childThing, myThings().filterByParentId(thing->id())) {
                childThing->setStateValue("connected", true);
            }
        });

        connect(client, &OwletSerialClient::disconnected, thing, [=](){
            thing->setStateValue("connected", false);
            foreach (Thing *childThing, myThings().filterByParentId(thing->id())) {
                childThing->setStateValue("connected", false);
            }
        });

        connect(thing, &Thing::settingChanged, thing, [=](const ParamTypeId &paramTypeId, const QVariant &value){
            qCDebug(dcOwlet()) << "Arduino Mini Pro settings changed" << paramTypeId << value;
            quint8 pinId = m_arduinoMiniProPinMapping.value(paramTypeId);
            OwletSerialClient::PinMode pinMode = getPinModeFromSettingsValue(value.toString());

            // Check if we have a thing for this pin and if we need to remove it before setting up
            Thing *existingThing = nullptr;
            foreach (Thing *childThing, myThings().filterByParentId(thing->id())) {
                int existingPinId = childThing->paramValue(m_owletSerialPinParamTypeMap.value(childThing->thingClassId())).toUInt();
                if (existingPinId == pinId) {
                    qCDebug(dcOwlet()) << "Found already configured thing for pin" << pinId;
                    existingThing = childThing;
                    break;
                }
            }

            if (existingThing) {
                if ((pinMode == OwletSerialClient::PinModeDigitalOutput && existingThing->thingClassId() == digitalOutputSerialThingClassId) ||
                        (pinMode == OwletSerialClient::PinModeDigitalInput && existingThing->thingClassId() == digitalInputSerialThingClassId) ||
                        (pinMode == OwletSerialClient::PinModeAnalogOutput && existingThing->thingClassId() == analogOutputSerialThingClassId) ||
                        (pinMode == OwletSerialClient::PinModeAnalogInput && existingThing->thingClassId() == analogInputSerialThingClassId) ||
                        (pinMode == OwletSerialClient::PinModeServo && existingThing->thingClassId() == servoSerialThingClassId)) {

                    qCDebug(dcOwlet()) << "Thing for pin" << pinId << "is already configured as" << pinMode;
                    return;
                } else {
                    qCDebug(dcOwlet()) << "Have thing for pin" << pinId << "but should be configured as" << pinMode;
                    qCDebug(dcOwlet()) << "Remove existing thing before setup a new one";
                    emit autoThingDisappeared(existingThing->id());
                }
            }

            setupArduinoChildThing(client, pinId, pinMode);
        });

        m_serialClients.insert(thing, client);
        info->finish(Thing::ThingErrorNoError);

        client->transport()->connectTransport();
        return;
    }

    if (thing->thingClassId() == digitalOutputSerialThingClassId) {
        info->finish(Thing::ThingErrorNoError);

        // Update states
        Thing *parent = myThings().findById(thing->parentId());
        bool parentConnected = false;
        if (parent)
            parentConnected = parent->stateValue("connected").toBool();

        thing->setStateValue("connected", parentConnected);

        OwletSerialClient *client = m_serialClients.value(parent);

        quint8 pinId = thing->paramValue(digitalOutputSerialThingPinParamTypeId).toUInt();
        OwletSerialClient::PinMode pinMode = OwletSerialClient::PinModeDigitalOutput;

        connect(client, &OwletSerialClient::connected, thing, [=](){
            configurePin(client, pinId, pinMode);
        });

        if (parentConnected) {
            configurePin(client, pinId, pinMode);
        }
    }

    if (thing->thingClassId() == digitalInputSerialThingClassId) {
        info->finish(Thing::ThingErrorNoError);

        // Update states
        Thing *parent = myThings().findById(thing->parentId());
        bool parentConnected = false;
        if (parent)
            parentConnected = parent->stateValue("connected").toBool();

        thing->setStateValue("connected", parentConnected);

        quint8 pinId = thing->paramValue(digitalInputSerialThingPinParamTypeId).toUInt();
        OwletSerialClient::PinMode pinMode = OwletSerialClient::PinModeDigitalInput;

        OwletSerialClient *client = m_serialClients.value(parent);
        connect(client, &OwletSerialClient::connected, thing, [=](){
            thing->setStateValue("connected", true);

            configurePin(client, pinId, pinMode);

            // Read current value
            quint8 pinId = thing->paramValue(digitalInputSerialThingPinParamTypeId).toUInt();
            OwletSerialClientReply *reply = client->readDigitalValue(pinId);
            connect(reply, &OwletSerialClientReply::finished, this, [=](){
                if (reply->status() != OwletSerialClient::StatusSuccess) {
                    qCWarning(dcOwlet()) << "Failed to read digital pin value from" << thing << reply->status();
                    return;
                }

                if (reply->responsePayload().count() < 2) {
                    qCWarning(dcOwlet()) << "Invalid response payload size from request" << pinId;
                    return;
                }

                OwletSerialClient::GPIOError configurationError = static_cast<OwletSerialClient::GPIOError>(reply->responsePayload().at(0));
                quint8 value = static_cast<quint8>(reply->responsePayload().at(1));
                if (configurationError != OwletSerialClient::GPIOErrorNoError) {
                    qCWarning(dcOwlet()) << "Configure pin request finished with error" << configurationError;
                    return;
                }

                thing->setStateValue(digitalInputSerialPowerStateTypeId, value != 0x00);
            });
        });

        connect(client, &OwletSerialClient::pinValueChanged, this, [=](quint8 id, bool power){
            if (id != pinId)
                return;

            thing->setStateValue(digitalInputSerialPowerStateTypeId, power);
        });

        if (parentConnected) {
            configurePin(client, pinId, pinMode);
        }

        return;
    }

    if (thing->thingClassId() == analogInputSerialThingClassId) {
        info->finish(Thing::ThingErrorNoError);

        // Update states
        Thing *parent = myThings().findById(thing->parentId());
        bool parentConnected = false;
        if (parent)
            parentConnected = parent->stateValue("connected").toBool();

        thing->setStateValue("connected", parentConnected);

        OwletSerialClient *client = m_serialClients.value(parent);

        QTimer *refreshTimer = new QTimer(thing);
        refreshTimer->setSingleShot(false);
        refreshTimer->setInterval(thing->setting(analogInputSerialSettingsRefreshRateParamTypeId).toUInt());
        connect(refreshTimer, &QTimer::timeout, this, [=](){
            // Read current value
            quint8 pinId = thing->paramValue(analogInputSerialThingPinParamTypeId).toUInt();
            OwletSerialClientReply *reply = client->readAnalogValue(pinId);
            connect(reply, &OwletSerialClientReply::finished, this, [=](){
                if (reply->status() != OwletSerialClient::StatusSuccess) {
                    qCWarning(dcOwlet()) << "Failed to read analog pin value from" << thing << reply->status();
                    return;
                }

                if (reply->responsePayload().count() < 3) {
                    qCWarning(dcOwlet()) << "Invalid response payload size from request" << pinId;
                    return;
                }

                QDataStream stream(reply->responsePayload());
                quint8 errorCode;
                stream >> errorCode;
                OwletSerialClient::GPIOError configurationError = static_cast<OwletSerialClient::GPIOError>(errorCode);
                if (configurationError != OwletSerialClient::GPIOErrorNoError) {
                    qCWarning(dcOwlet()) << "Configure pin request finished with error" << configurationError;
                    return;
                }
                quint32 value;
                stream >> value;
                qCDebug(dcOwlet()) << "Analog value of" << thing << value;
                thing->setStateValue(analogInputSerialAnalogValueStateTypeId, value);
            });
        });

        quint8 pinId = thing->paramValue(analogInputSerialThingPinParamTypeId).toUInt();
        OwletSerialClient::PinMode pinMode = OwletSerialClient::PinModeAnalogInput;

        connect(client, &OwletSerialClient::connected, thing, [=](){
            qCDebug(dcOwlet()) << "Starting refresh timer for" << thing;
            configurePin(client, pinId, pinMode);
            refreshTimer->start();
        });

        connect(client, &OwletSerialClient::disconnected, thing, [=](){
            qCDebug(dcOwlet()) << "Stopping refresh timer for" << thing;
            refreshTimer->stop();
        });

        // Start reading if already connected
        if (parentConnected) {
            qCDebug(dcOwlet()) << "Starting refresh timer for" << thing;
            configurePin(client, pinId, pinMode);
            refreshTimer->start();
        }

        connect(thing, &Thing::settingChanged, refreshTimer, [=](const ParamTypeId &paramTypeId, const QVariant &value){
            if (paramTypeId == analogInputSerialSettingsRefreshRateParamTypeId) {
                refreshTimer->setInterval(value.toUInt());
                refreshTimer->start();
            }
        });

        return;
    }


    if (thing->thingClassId() == analogOutputSerialThingClassId) {
        info->finish(Thing::ThingErrorNoError);

        // Update states
        Thing *parent = myThings().findById(thing->parentId());
        bool parentConnected = false;
        if (parent)
            parentConnected = parent->stateValue("connected").toBool();

        thing->setStateValue("connected", parentConnected);

        OwletSerialClient *client = m_serialClients.value(parent);

        quint8 pinId = thing->paramValue(analogInputSerialThingPinParamTypeId).toUInt();
        OwletSerialClient::PinMode pinMode = OwletSerialClient::PinModeAnalogInput;

        connect(client, &OwletSerialClient::connected, thing, [=](){
            configurePin(client, pinId, pinMode);
        });

        // Start reading if already connected
        if (parentConnected) {
            configurePin(client, pinId, pinMode);
        }

        return;
    }


    if (thing->thingClassId() == servoSerialThingClassId) {
        info->finish(Thing::ThingErrorNoError);

        // Update states
        Thing *parent = myThings().findById(thing->parentId());
        bool parentConnected = false;
        if (parent)
            parentConnected = parent->stateValue("connected").toBool();

        thing->setStateValue("connected", parentConnected);

        OwletSerialClient *client = m_serialClients.value(parent);

        quint8 pinId = thing->paramValue(servoSerialThingPinParamTypeId).toUInt();
        OwletSerialClient::PinMode pinMode = OwletSerialClient::PinModeServo;

        connect(client, &OwletSerialClient::connected, thing, [=](){
            configurePin(client, pinId, pinMode);
        });

        // Start reading if already connected
        if (parentConnected) {
            configurePin(client, pinId, pinMode);
        }

        return;
    }


    QHostAddress ip;
    int port = 5555;

    foreach (const ZeroConfServiceEntry &entry, m_zeroConfBrowser->serviceEntries()) {
        if (entry.txt("id") == info->thing()->paramValue(m_owletIdParamTypeMap.value(info->thing()->thingClassId()))) {
            ip = entry.hostAddress();
            port = entry.port();
            break;
        }
    }
    // Try cached ip
    if (ip.isNull()) {
        pluginStorage()->beginGroup(thing->id().toString());
        ip = QHostAddress(pluginStorage()->value("cachedIP").toString());
        pluginStorage()->endGroup();
    }

    if (ip.isNull()) {
        qCWarning(dcOwlet()) << "Can't find owlet in the local network.";
        info->finish(Thing::ThingErrorHardwareNotAvailable);
        return;
    }

    OwletTransport *transport = new OwletTcpTransport(ip, port, this);

    OwletClient *client = new OwletClient(transport, this);
    connect(client, &OwletClient::connected, info, [=](){
        qCDebug(dcOwlet()) << "Connected to owlet";
        m_clients.insert(thing, client);

        if (thing->thingClassId() == digitalOutputThingClassId) {
            QVariantMap params;
            params.insert("id", thing->paramValue(digitalOutputThingPinParamTypeId).toInt());
            params.insert("mode", "GPIOOutput");
            client->sendCommand("GPIO.ConfigurePin", params);
        }
        if (thing->thingClassId() == digitalInputThingClassId) {
            QVariantMap params;
            params.insert("id", thing->paramValue(digitalInputThingPinParamTypeId).toInt());
            params.insert("mode", "GPIOInput");
            client->sendCommand("GPIO.ConfigurePin", params);
        }
        if (thing->thingClassId() == ws2812ThingClassId) {
            QVariantMap params;
            params.insert("id", thing->paramValue(ws2812ThingPinParamTypeId).toInt());
            params.insert("mode", "WS2812");
            params.insert("ledCount", thing->paramValue(ws2812ThingLedCountParamTypeId).toUInt());
            params.insert("ledMode", "WS2812Mode" + thing->paramValue(ws2812ThingLedModeParamTypeId).toString());
            params.insert("ledClock", "WS2812Clock" + thing->paramValue(ws2812ThingLedClockParamTypeId).toString());
            client->sendCommand("GPIO.ConfigurePin", params);
        }

        info->finish(Thing::ThingErrorNoError);
    });

    connect(client, &OwletClient::error, info, [=](){
        info->finish(Thing::ThingErrorHardwareFailure);
    });

    connect(client, &OwletClient::connected, thing, [=](){
        thing->setStateValue("connected", true);

        pluginStorage()->beginGroup(thing->id().toString());
        pluginStorage()->setValue("cachedIP", ip.toString());
        pluginStorage()->endGroup();

        qCDebug(dcOwlet()) << "Sending get platform information request...";
        int id = client->sendCommand("Platform.GetInformation");
        connect(client, &OwletClient::replyReceived, thing, [=](int commandId, const QVariantMap &params){
            if (id != commandId)
                return;

            qCDebug(dcOwlet()) << "Reply from owlet platform information:" << params;
        });
    });

    connect(client, &OwletClient::disconnected, thing, [=](){
        thing->setStateValue("connected", false);
    });

    connect(client, &OwletClient::notificationReceived, this, [=](const QString &name, const QVariantMap &params){
        qCDebug(dcOwlet()) << "***Notif" << name << params;
        if (thing->thingClassId() == digitalInputThingClassId) {
            if (params.value("id").toInt() == thing->paramValue(digitalInputThingPinParamTypeId)) {
                thing->setStateValue(digitalInputPowerStateTypeId, params.value("power").toBool());
            }
        }
        if (thing->thingClassId() == digitalOutputThingClassId) {
            if (params.value("id").toInt() == thing->paramValue(digitalOutputThingPinParamTypeId)) {
                thing->setStateValue(digitalOutputPowerStateTypeId, params.value("power").toBool());
            }
        }
        if (thing->thingClassId() == ws2812ThingClassId) {
            if (name == "GPIO.PinChanged") {
                if (params.contains("power")) {
                    thing->setStateValue(ws2812PowerStateTypeId, params.value("power").toBool());
                }
                if (params.contains("brightness")) {
                    thing->setStateValue(ws2812BrightnessStateTypeId, params.value("brightness").toInt());
                }
                if (params.contains("color")) {
                    thing->setStateValue(ws2812ColorStateTypeId, params.value("color").value<QColor>());
                }
                if (params.contains("effect")) {
                    thing->setStateValue(ws2812EffectStateTypeId, params.value("effect").toInt());
                }
            }
        }
    });

    client->transport()->connectTransport();
}


void IntegrationPluginOwlet::executeAction(ThingActionInfo *info)
{
    if (info->thing()->thingClassId() == digitalOutputThingClassId) {
        OwletClient *client = m_clients.value(info->thing());
        QVariantMap params;
        params.insert("id", info->thing()->paramValue(digitalOutputThingPinParamTypeId).toInt());
        params.insert("power", info->action().paramValue(digitalOutputPowerActionPowerParamTypeId).toBool());
        qCDebug(dcOwlet()) << "Sending ControlPin" << params;
        int id = client->sendCommand("GPIO.ControlPin", params);
        connect(client, &OwletClient::replyReceived, info, [=](int commandId, const QVariantMap &params){
            if (id != commandId) {
                return;
            }
            qCDebug(dcOwlet()) << "reply from owlet:" << params;
            QString error = params.value("error").toString();
            if (error == "GPIOErrorNoError") {
                info->thing()->setStateValue(digitalOutputPowerStateTypeId, info->action().paramValue(digitalOutputPowerActionPowerParamTypeId).toBool());
                info->finish(Thing::ThingErrorNoError);
            } else {
                info->finish(Thing::ThingErrorHardwareFailure);
            }
        });

        return;
    }

    if (info->thing()->thingClassId() == ws2812ThingClassId) {
        OwletClient *client = m_clients.value(info->thing());
        QVariantMap params;
        params.insert("id", info->thing()->paramValue(ws2812ThingPinParamTypeId).toUInt());

        if (info->action().actionTypeId() == ws2812PowerActionTypeId) {
            params.insert("power", info->action().paramValue(ws2812PowerActionPowerParamTypeId).toBool());
        }
        if (info->action().actionTypeId() == ws2812BrightnessActionTypeId) {
            params.insert("brightness", info->action().paramValue(ws2812BrightnessActionBrightnessParamTypeId).toInt());
        }
        if (info->action().actionTypeId() == ws2812ColorActionTypeId) {
            QColor color = info->action().paramValue(ws2812ColorActionColorParamTypeId).value<QColor>();
            params.insert("color", (color.rgb() & 0xFFFFFF));
        }
        if (info->action().actionTypeId() == ws2812EffectActionTypeId) {
            int effect = info->action().paramValue(ws2812EffectActionEffectParamTypeId).toInt();
            params.insert("effect", effect);
        }

        int id = client->sendCommand("GPIO.ControlPin", params);
        connect(client, &OwletClient::replyReceived, info, [=](int commandId, const QVariantMap &params){
            if (id != commandId) {
                return;
            }
            qCDebug(dcOwlet()) << "reply from owlet:" << params;
            QString error = params.value("error").toString();
            if (error == "GPIOErrorNoError") {
                info->finish(Thing::ThingErrorNoError);
            } else {
                info->finish(Thing::ThingErrorHardwareFailure);
            }
        });
        return;
    }

    if (info->thing()->thingClassId() == arduinoMiniProThingClassId) {
        OwletSerialClient *client = m_serialClients.value(info->thing());
        if (!client) {
            qCWarning(dcOwlet()) << "Could not execute action. There is no client available for this thing";
            info->finish(Thing::ThingErrorHardwareFailure);
            return;
        }

        if (!client->ready()) {
            qCWarning(dcOwlet()) << "Could not execute action. The serial client is not ready or connected.";
            info->finish(Thing::ThingErrorHardwareNotAvailable);
            return;
        }
        if (info->action().actionTypeId() == arduinoMiniProPerformUpdateActionTypeId) {
            qCDebug(dcOwlet()) << "Perform firmware update on" << info->thing();
            //if (client->firmwareVersion() != )

            // TODO: run upgrade process using avrdude

            info->finish(Thing::ThingErrorNoError);
            return;
        }
    }

    if (info->thing()->thingClassId() == analogOutputSerialThingClassId) {
        OwletSerialClient *client = m_serialClients.value(myThings().findById(info->thing()->parentId()));
        if (!client) {
            qCWarning(dcOwlet()) << "Could not execute action. There is no client available for this thing";
            info->finish(Thing::ThingErrorHardwareFailure);
            return;
        }

        if (!client->ready()) {
            qCWarning(dcOwlet()) << "Could not execute action. The serial client is not ready or connected.";
            info->finish(Thing::ThingErrorHardwareNotAvailable);
            return;
        }

        if (info->action().actionTypeId() == analogOutputSerialDutyCycleActionTypeId) {
            quint8 dutyCycle = info->action().paramValue(analogOutputSerialDutyCycleActionDutyCycleParamTypeId).toUInt();
            qCDebug(dcOwlet()) << "Set PWM duty cycle of" << info->thing() << "to" << dutyCycle;
            quint8 pinId = info->thing()->paramValue(analogOutputSerialThingPinParamTypeId).toUInt();

            OwletSerialClientReply *reply = client->writeAnalogValue(pinId, dutyCycle);
            connect(reply, &OwletSerialClientReply::finished, this, [=](){
                if (reply->status() != OwletSerialClient::StatusSuccess) {
                    qCWarning(dcOwlet()) << "Failed to set power on pin" << pinId << dutyCycle << reply->status();
                    info->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                if (reply->responsePayload().count() < 1) {
                    qCWarning(dcOwlet()) << "Failed to set power on pin" << pinId << dutyCycle << "Invalid response payload size from request";
                    info->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                OwletSerialClient::GPIOError gpioError = static_cast<OwletSerialClient::GPIOError>(reply->responsePayload().at(0));
                if (gpioError != OwletSerialClient::GPIOErrorNoError) {
                    qCWarning(dcOwlet()) << "Set power on pin" << pinId << dutyCycle << "request finished with error" << gpioError;
                    info->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                qCDebug(dcOwlet()) << "Set PWM duty cycle finished successfully" << pinId << dutyCycle;
                info->thing()->setStateValue(analogOutputSerialDutyCycleStateTypeId, dutyCycle);
                info->finish(Thing::ThingErrorNoError);
                return;
            });

            return;
        }
    }


    if (info->thing()->thingClassId() == digitalOutputSerialThingClassId) {
        OwletSerialClient *client = m_serialClients.value(myThings().findById(info->thing()->parentId()));
        if (!client) {
            qCWarning(dcOwlet()) << "Could not execute action. There is no client available for this thing";
            info->finish(Thing::ThingErrorHardwareFailure);
            return;
        }

        if (!client->ready()) {
            qCWarning(dcOwlet()) << "Could not execute action. The serial client is not ready or connected.";
            info->finish(Thing::ThingErrorHardwareNotAvailable);
            return;
        }

        if (info->action().actionTypeId() == digitalOutputSerialPowerActionTypeId) {
            quint8 pinId = info->thing()->paramValue(digitalOutputSerialThingPinParamTypeId).toUInt();
            bool power = info->action().paramValue(digitalOutputSerialPowerActionPowerParamTypeId).toBool();
            OwletSerialClientReply *reply = client->writeDigitalValue(pinId, power);
            connect(reply, &OwletSerialClientReply::finished, this, [=](){
                if (reply->status() != OwletSerialClient::StatusSuccess) {
                    qCWarning(dcOwlet()) << "Failed to set power on pin" << pinId << power << reply->status();
                    info->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                if (reply->responsePayload().count() < 1) {
                    qCWarning(dcOwlet()) << "Failed to set power on pin" << pinId << power << "Invalid response payload size from request";
                    info->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                OwletSerialClient::GPIOError gpioError = static_cast<OwletSerialClient::GPIOError>(reply->responsePayload().at(0));
                if (gpioError != OwletSerialClient::GPIOErrorNoError) {
                    qCWarning(dcOwlet()) << "Set power on pin" << pinId << power << "request finished with error" << gpioError;
                    info->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                qCDebug(dcOwlet()) << "Set power finished successfully" << pinId << power;
                info->thing()->setStateValue(digitalOutputSerialPowerStateTypeId, power);
                info->finish(Thing::ThingErrorNoError);
                return;
            });

            return;
        }

        info->finish(Thing::ThingErrorUnsupportedFeature);
        return;
    }


    if (info->thing()->thingClassId() == servoSerialThingClassId) {
        OwletSerialClient *client = m_serialClients.value(myThings().findById(info->thing()->parentId()));
        if (!client) {
            qCWarning(dcOwlet()) << "Could not execute action. There is no client available for this thing";
            info->finish(Thing::ThingErrorHardwareFailure);
            return;
        }

        if (!client->ready()) {
            qCWarning(dcOwlet()) << "Could not execute action. The serial client is not ready or connected.";
            info->finish(Thing::ThingErrorHardwareNotAvailable);
            return;
        }

        if (info->action().actionTypeId() == servoSerialAngleActionTypeId) {
            quint8 pinId = info->thing()->paramValue(servoSerialThingPinParamTypeId).toUInt();
            quint8 angle = info->action().paramValue(servoSerialAngleActionAngleParamTypeId).toUInt();
            OwletSerialClientReply *reply = client->writeServoValue(pinId, angle);
            connect(reply, &OwletSerialClientReply::finished, this, [=](){
                if (reply->status() != OwletSerialClient::StatusSuccess) {
                    qCWarning(dcOwlet()) << "Failed to set servo angle on pin" << pinId << angle << reply->status();
                    info->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                if (reply->responsePayload().count() < 1) {
                    qCWarning(dcOwlet()) << "Failed to set servo angle on pin" << pinId << angle << "Invalid response payload size from request";
                    info->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                OwletSerialClient::GPIOError gpioError = static_cast<OwletSerialClient::GPIOError>(reply->responsePayload().at(0));
                if (gpioError != OwletSerialClient::GPIOErrorNoError) {
                    qCWarning(dcOwlet()) << "Set angle on servo pin" << pinId << angle << "request finished with error" << gpioError;
                    info->finish(Thing::ThingErrorHardwareFailure);
                    return;
                }

                qCDebug(dcOwlet()) << "Set servo angle finished successfully" << pinId << angle;
                info->thing()->setStateValue(servoSerialAngleStateTypeId, angle);
                info->finish(Thing::ThingErrorNoError);
                return;
            });

            return;
        }

        info->finish(Thing::ThingErrorUnsupportedFeature);
        return;
    }



    Q_ASSERT_X(false, "IntegrationPluginOwlet", "Not implemented");
    info->finish(Thing::ThingErrorUnsupportedFeature);
}

void IntegrationPluginOwlet::thingRemoved(Thing *thing)
{
    if (thing->thingClassId() == arduinoMiniProThingClassId && m_serialClients.contains(thing)) {
        m_serialClients.take(thing)->deleteLater();
    }
}

OwletSerialClient::PinMode IntegrationPluginOwlet::getPinModeFromSettingsValue(const QString &settingsValue)
{
    if (settingsValue == "Output") {
        return OwletSerialClient::PinModeDigitalOutput;
    } else if (settingsValue == "Input") {
        return OwletSerialClient::PinModeDigitalInput;
    } else if (settingsValue == "PWM") {
        return OwletSerialClient::PinModeAnalogOutput;
    } else if (settingsValue == "Analog Input") {
        return OwletSerialClient::PinModeAnalogInput;
    } else if (settingsValue == "Servo") {
        return OwletSerialClient::PinModeServo;
    } else {
        return OwletSerialClient::PinModeUnconfigured;
    }
}

void IntegrationPluginOwlet::setupArduinoChildThing(OwletSerialClient *client, quint8 pinId, OwletSerialClient::PinMode pinMode)
{
    Thing *parentThing = m_serialClients.key(client);
    if (!parentThing) {
        qCWarning(dcOwlet()) << "Could not setup child thing because the parent thing seems not to be available";
        return;
    }

    // Note: pin has no thing here any more, we can set up a new one if required
    OwletSerialClientReply *reply = client->configurePin(pinId, pinMode);
    connect(reply, &OwletSerialClientReply::finished, this, [=](){
        if (reply->status() != OwletSerialClient::StatusSuccess) {
            qCWarning(dcOwlet()) << "Failed to configure pin" << pinId << pinMode << reply->status();
            return;
        }

        if (reply->responsePayload().count() < 1) {
            qCWarning(dcOwlet()) << "Invalid configure pin response payload size from request" << pinId << pinMode;
            return;
        }

        OwletSerialClient::GPIOError configurationError = static_cast<OwletSerialClient::GPIOError>(reply->responsePayload().at(0));
        if (configurationError != OwletSerialClient::GPIOErrorNoError) {
            qCWarning(dcOwlet()) << "Configure pin request finished with error" << configurationError;
            return;
        }

        qCDebug(dcOwlet()) << "Configure pin request finished successfully" << pinId << pinMode;

        // Setup child devices
        switch (pinMode) {
        case OwletSerialClient::PinModeDigitalOutput: {
            qCDebug(dcOwlet()) << "Setting up digital output on serial owlet for pin" << pinId;
            ThingDescriptor descriptor(digitalOutputSerialThingClassId, thingClass(digitalOutputSerialThingClassId).displayName() + " (" + QString::number(pinId) + ")", QString(), parentThing->id());
            ParamList params;
            params.append(Param(digitalOutputSerialThingPinParamTypeId, pinId));
            descriptor.setParams(params);
            emit autoThingsAppeared(ThingDescriptors() << descriptor);
            break;
        }
        case OwletSerialClient::PinModeDigitalInput: {
            qCDebug(dcOwlet()) << "Setting up digital input on serial owlet for pin" << pinId;
            ThingDescriptor descriptor(digitalInputSerialThingClassId, thingClass(digitalInputSerialThingClassId).displayName() + " (" + QString::number(pinId) + ")", QString(), parentThing->id());
            ParamList params;
            params.append(Param(digitalInputSerialThingPinParamTypeId, pinId));
            descriptor.setParams(params);
            emit autoThingsAppeared(ThingDescriptors() << descriptor);
            break;
        }
        case OwletSerialClient::PinModeAnalogOutput: {
            qCDebug(dcOwlet()) << "Setting up digital output on serial owlet for pin" << pinId;
            ThingDescriptor descriptor(analogOutputSerialThingClassId, thingClass(analogOutputSerialThingClassId).displayName() + " (" + QString::number(pinId) + ")", QString(), parentThing->id());
            ParamList params;
            params.append(Param(analogOutputSerialThingPinParamTypeId, pinId));
            descriptor.setParams(params);
            emit autoThingsAppeared(ThingDescriptors() << descriptor);
            break;
        }
        case OwletSerialClient::PinModeAnalogInput: {
            qCDebug(dcOwlet()) << "Setting up analog input on serial owlet for pin" << pinId;
            ThingDescriptor descriptor(analogInputSerialThingClassId, thingClass(analogInputSerialThingClassId).displayName() + " (" + QString::number(pinId) + ")", QString(), parentThing->id());
            ParamList params;
            params.append(Param(analogInputSerialThingPinParamTypeId, pinId));
            descriptor.setParams(params);
            emit autoThingsAppeared(ThingDescriptors() << descriptor);
            break;
        }
        case OwletSerialClient::PinModeServo: {
            qCDebug(dcOwlet()) << "Setting up servo on serial owlet for pin" << pinId;
            ThingDescriptor descriptor(servoSerialThingClassId, thingClass(servoSerialThingClassId).displayName() + " (" + QString::number(pinId) + ")", QString(), parentThing->id());
            ParamList params;
            params.append(Param(servoSerialThingPinParamTypeId, pinId));
            descriptor.setParams(params);
            emit autoThingsAppeared(ThingDescriptors() << descriptor);
            break;
        }
        case OwletSerialClient::PinModeUnconfigured:

            break;
        }
    });
}

void IntegrationPluginOwlet::configurePin(OwletSerialClient *client, quint8 pinId, OwletSerialClient::PinMode pinMode)
{
    OwletSerialClientReply *reply = client->configurePin(pinId, pinMode);
    connect(reply, &OwletSerialClientReply::finished, this, [=](){
        if (reply->status() != OwletSerialClient::StatusSuccess) {
            qCWarning(dcOwlet()) << "Failed to set pin mode on pin" << pinId << pinMode << reply->status();
            return;
        }

        if (reply->responsePayload().count() < 1) {
            qCWarning(dcOwlet()) << "Failed to set pin mode on pin" << pinId << pinMode << "Invalid response payload size from request";
            return;
        }

        OwletSerialClient::GPIOError gpioError = static_cast<OwletSerialClient::GPIOError>(reply->responsePayload().at(0));
        if (gpioError != OwletSerialClient::GPIOErrorNoError) {
            qCWarning(dcOwlet()) << "Set pin mode on pin" << pinId << pinMode << "request finished with error" << gpioError;
            return;
        }

        qCDebug(dcOwlet()) << "Set pin mode finished successfully" << pinId << pinMode;
    });
}

