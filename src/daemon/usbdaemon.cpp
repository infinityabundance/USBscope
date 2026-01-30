#include "usbdaemon.h"

#include "dbus_adaptor.h"

UsbDaemon::UsbDaemon(QObject *parent)
    : QObject(parent) {
}

void UsbDaemon::setAdaptor(UsbscopeDBusAdaptor *adaptor) {
    m_adaptor = adaptor;
}

void UsbDaemon::appendEvent(const UsbEvent &event) {
    m_events.append(event);
    if (m_events.size() > m_maxEvents) {
        m_events.erase(m_events.begin(), m_events.begin() + (m_events.size() - m_maxEvents));
    }

    if (m_adaptor) {
        m_adaptor->emitLogEvent(event);
    }

    if (event.isError) {
        recordErrorBurst(event);
    }
}

void UsbDaemon::setDevices(const QList<UsbDeviceInfo> &devices) {
    m_devices = devices;
    if (m_adaptor) {
        m_adaptor->emitDevicesChanged();
    }
}

QList<QVariantList> UsbDaemon::recentEventsVariant(int limit) const {
    QList<QVariantList> data;
    if (limit <= 0) {
        return data;
    }
    int start = qMax(0, m_events.size() - limit);
    for (int i = start; i < m_events.size(); ++i) {
        data.append(toVariant(m_events.at(i)));
    }
    return data;
}

QList<QVariantList> UsbDaemon::currentDevicesVariant() const {
    QList<QVariantList> data;
    for (const UsbDeviceInfo &device : m_devices) {
        data.append(toVariant(device));
    }
    return data;
}

QVariantList UsbDaemon::stateSummary() const {
    QVariantList summary;
    summary.append(m_events.size());
    summary.append(m_devices.size());
    return summary;
}

void UsbDaemon::recordErrorBurst(const UsbEvent &event) {
    QDateTime now = QDateTime::currentDateTimeUtc();
    m_errorTimes.append(now);
    const int windowSeconds = 5;
    const int threshold = 5;

    while (!m_errorTimes.isEmpty() && m_errorTimes.first().secsTo(now) > windowSeconds) {
        m_errorTimes.removeFirst();
    }

    if (m_errorTimes.size() >= threshold && m_adaptor) {
        m_adaptor->emitErrorBurst(m_errorTimes.size(), event.message);
    }
}
