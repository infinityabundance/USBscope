#pragma once

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QObject>

#include "usbtypes.h"

class UsbscopeDBusClient : public QObject {
    Q_OBJECT
public:
    explicit UsbscopeDBusClient(QObject *parent = nullptr);

    QList<UsbEvent> getRecentEvents(int limit);
    QList<UsbDeviceInfo> getCurrentDevices();
    QVariantList getStateSummary();

signals:
    void LogEvent(const UsbEvent &event);
    void DevicesChanged();
    void ErrorBurst(int count, const QString &lastMessage);

private slots:
    void handleLogEvent(const QVariantList &event);
    void handleDevicesChanged();
    void handleErrorBurst(int count, const QString &lastMessage);

private:
    QDBusInterface m_interface;
};

QDBusConnection usbscopeBus();
void registerUsbDbusTypes();
bool isUsbScopeRunning();
bool startUsbScopeDaemon();
bool stopUsbScopeDaemon();
bool startUsbScopeTray();
bool startUsbScopeUi();
