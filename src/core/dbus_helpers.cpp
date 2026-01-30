#include "dbus_helpers.h"

#include <QCoreApplication>
#include <QDBusConnectionInterface>
#include <QDBusError>
#include <QDBusMetaType>
#include <QFileInfo>
#include <QProcess>
#include <QVariant>

namespace {
const char *kServiceName = "org.cachyos.USBscope";
const char *kObjectPath = "/org/cachyos/USBscope/Daemon";
const char *kInterfaceName = "org.cachyos.USBscope1";
}

UsbscopeDBusClient::UsbscopeDBusClient(QObject *parent)
    : QObject(parent)
    , m_interface(kServiceName, kObjectPath, kInterfaceName, usbscopeBus()) {
    QDBusConnection bus = usbscopeBus();
    bus.connect(
        kServiceName,
        kObjectPath,
        kInterfaceName,
        "LogEvent",
        this,
        SLOT(handleLogEvent(QVariantList)));

    bus.connect(
        kServiceName,
        kObjectPath,
        kInterfaceName,
        "DevicesChanged",
        this,
        SLOT(handleDevicesChanged()));

    bus.connect(
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

QDBusConnection usbscopeBus() {
    const QByteArray scope = qgetenv("USBSCOPE_BUS");
    if (scope == "system") {
        return QDBusConnection::systemBus();
    }
    QDBusConnection session = QDBusConnection::sessionBus();
    if (session.isConnected()) {
        return session;
    }
    return QDBusConnection::systemBus();
}

void registerUsbDbusTypes() {
    qRegisterMetaType<QList<QVariantList>>("QList<QVariantList>");
    qDBusRegisterMetaType<QList<QVariantList>>();
}

namespace {
QString executablePath(const QString &name) {
    const QString candidate = QCoreApplication::applicationDirPath() + "/" + name;
    QFileInfo info(candidate);
    if (info.exists() && info.isExecutable()) {
        return candidate;
    }
    return name;
}

bool isProcessRunning(const QString &name) {
    return QProcess::execute("pgrep", {"-x", name}) == 0;
}
}

bool isUsbScopeRunning() {
    QDBusConnection bus = usbscopeBus();
    if (!bus.isConnected()) {
        return false;
    }
    QDBusConnectionInterface *iface = bus.interface();
    if (!iface) {
        return false;
    }
    return iface->isServiceRegistered("org.cachyos.USBscope");
}

bool startUsbScopeDaemon() {
    if (isUsbScopeRunning()) {
        return true;
    }
    return QProcess::startDetached(executablePath("usbscoped"));
}

bool stopUsbScopeDaemon() {
    return QProcess::startDetached("pkill", {"-x", "usbscoped"});
}

bool startUsbScopeTray() {
    if (isProcessRunning("usbscope-tray")) {
        return true;
    }
    return QProcess::startDetached(executablePath("usbscope-tray"));
}

bool startUsbScopeUi() {
    if (isProcessRunning("usbscope-ui")) {
        return true;
    }
    return QProcess::startDetached(executablePath("usbscope-ui"));
}
