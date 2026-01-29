#pragma once

#include <QDateTime>
#include <QObject>

#include "dbus_helpers.h"
#include "usbtypes.h"

class UsbDaemon : public QObject {
    Q_OBJECT
public:
    explicit UsbDaemon(QObject *parent = nullptr);

    void setAdaptor(UsbscopeDBusAdaptor *adaptor);

    void appendEvent(const UsbEvent &event);
    void setDevices(const QList<UsbDeviceInfo> &devices);

    QList<QVariantList> recentEventsVariant(int limit) const;
    QList<QVariantList> currentDevicesVariant() const;
    QVariantList stateSummary() const;

private:
    void recordErrorBurst(const UsbEvent &event);

    QList<UsbEvent> m_events;
    QList<UsbDeviceInfo> m_devices;
    QList<QDateTime> m_errorTimes;
    int m_maxEvents = 5000;
    UsbscopeDBusAdaptor *m_adaptor = nullptr;
};
