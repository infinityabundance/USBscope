#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

#include "dbus_adaptor.h"
#include "dbus_helpers.h"
#include "journaltail.h"
#include "usbdaemon.h"
#include "usbmonitor.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    UsbDaemon daemon;
    UsbscopeDBusAdaptor adaptor(&daemon);
    daemon.setAdaptor(&adaptor);

    QDBusConnection connection = usbscopeBus();
    if (!connection.isConnected()) {
        qWarning() << "USBscope: D-Bus connection failed:" << connection.lastError().message();
    } else {
        if (!connection.registerService("org.cachyos.USBscope")) {
            qWarning() << "USBscope: Failed to register D-Bus service:"
                       << connection.lastError().message();
        }
        if (!connection.registerObject("/org/cachyos/USBscope/Daemon", &daemon,
                                        QDBusConnection::ExportAdaptors)) {
            qWarning() << "USBscope: Failed to register D-Bus object:"
                       << connection.lastError().message();
        }
    }

    JournalTail tail;
    UsbMonitor monitor;

    QObject::connect(&tail, &JournalTail::eventParsed, &daemon, &UsbDaemon::appendEvent);
    QObject::connect(&monitor, &UsbMonitor::devicesChanged, &daemon, &UsbDaemon::setDevices);

    tail.start();
    monitor.start();

    return app.exec();
}
