#include "dbus_helpers.h"

#include <QDBusConnectionInterface>
#include <QDBusError>
#include <QVariant>

#include "usbdaemon.h"

namespace {
const char *kServiceName = "org.cachyos.USBscope";
const char *kObjectPath = "/org/cachyos/USBscope/Daemon";
const char *kInterfaceName = "org.cachyos.USBscope1";
}

UsbscopeDBusAdaptor::UsbscopeDBusAdaptor(UsbDaemon *daemon)
    : QDBusAbstractAdaptor(daemon), m_daemon(daemon) {
}

QString UsbscopeDBusAdaptor::GetVersion() {
    return QStringLiteral("0.1.0");
}

QList<QVariantList> UsbscopeDBusAdaptor::GetRecentEvents(int limit) {
    return m_daemon ? m_daemon->recentEventsVariant(limit) : QList<QVariantList>{};
}

QList<QVariantList> UsbscopeDBusAdaptor::GetCurrentDevices() {
    return m_daemon ? m_daemon->currentDevicesVariant() : QList<QVariantList>{};
}

QVariantList UsbscopeDBusAdaptor::GetStateSummary() {
    return m_daemon ? m_daemon->stateSummary() : QVariantList{};
}

void UsbscopeDBusAdaptor::emitLogEvent(const UsbEvent &event) {
    emit LogEvent(toVariant(event));
}

void UsbscopeDBusAdaptor::emitDevicesChanged() {
    emit DevicesChanged();
}

void UsbscopeDBusAdaptor::emitErrorBurst(int count, const QString &lastMessage) {
    emit ErrorBurst(count, lastMessage);
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
