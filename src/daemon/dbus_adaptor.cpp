#include "dbus_adaptor.h"

#include "usbdaemon.h"

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
