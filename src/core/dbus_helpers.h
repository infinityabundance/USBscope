#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QObject>

#include "usbtypes.h"

class UsbDaemon;

class UsbscopeDBusAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.cachyos.USBscope1")
public:
    explicit UsbscopeDBusAdaptor(UsbDaemon *daemon);

public slots:
    QString GetVersion();
    QList<QVariantList> GetRecentEvents(int limit);
    QList<QVariantList> GetCurrentDevices();
    QVariantList GetStateSummary();

signals:
    void LogEvent(const QVariantList &event);
    void DevicesChanged();
    void ErrorBurst(int count, const QString &lastMessage);

public:
    void emitLogEvent(const UsbEvent &event);
    void emitDevicesChanged();
    void emitErrorBurst(int count, const QString &lastMessage);

private:
    UsbDaemon *m_daemon;
};

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
