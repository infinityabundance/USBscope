#include "dbus_helpers.h"

#include <QDBusConnectionInterface>
#include <QDBusError>
#include <QVariant>

namespace {
const char *kServiceName = "org.cachyos.USBscope";
const char *kObjectPath = "/org/cachyos/USBscope/Daemon";
const char *kInterfaceName = "org.cachyos.USBscope1";
}

UsbscopeDBusClient::UsbscopeDBusClient(QObject *parent)
    : QObject(parent)
    , m_interface(kServiceName, kObjectPath, kInterfaceName, QDBusConnection::systemBus()) {
    QDBusConnection::systemBus().connect(
        kServiceName,
        kObjectPath,
        kInterfaceName,
        "LogEvent",
        this,
        SLOT(handleLogEvent(QVariantList)));

    QDBusConnection::systemBus().connect(
        kServiceName,
        kObjectPath,
        kInterfaceName,
        "DevicesChanged",
        this,
        SLOT(handleDevicesChanged()));

    QDBusConnection::systemBus().connect(
        kServiceName,
        kObjectPath,
        kInterfaceName,
        "ErrorBurst",
        this,
        SLOT(handleErrorBurst(int,QString)));
}

QList<UsbEvent> UsbscopeDBusClient::getRecentEvents(int limit) {
    QDBusReply<QList<QVariantList>> reply = m_interface.call("GetRecentEvents", limit);
    QList<UsbEvent> events;
    if (!reply.isValid()) {
        return events;
    }
    for (const QVariantList &item : reply.value()) {
        events.append(fromVariant(item));
    }
    return events;
}

QList<UsbDeviceInfo> UsbscopeDBusClient::getCurrentDevices() {
    QDBusReply<QList<QVariantList>> reply = m_interface.call("GetCurrentDevices");
    QList<UsbDeviceInfo> devices;
    if (!reply.isValid()) {
        return devices;
    }
    for (const QVariantList &item : reply.value()) {
        devices.append(deviceFromVariant(item));
    }
    return devices;
}

QVariantList UsbscopeDBusClient::getStateSummary() {
    QDBusReply<QVariantList> reply = m_interface.call("GetStateSummary");
    return reply.isValid() ? reply.value() : QVariantList{};
}

void UsbscopeDBusClient::handleLogEvent(const QVariantList &event) {
    emit LogEvent(fromVariant(event));
}

void UsbscopeDBusClient::handleDevicesChanged() {
    emit DevicesChanged();
}

void UsbscopeDBusClient::handleErrorBurst(int count, const QString &lastMessage) {
    emit ErrorBurst(count, lastMessage);
}
