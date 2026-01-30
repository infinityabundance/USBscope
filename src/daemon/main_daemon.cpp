#include <QCoreApplication>
#include <QDBusConnection>

#include "dbus_adaptor.h"
#include "journaltail.h"
#include "usbdaemon.h"
#include "usbmonitor.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    UsbDaemon daemon;
    UsbscopeDBusAdaptor adaptor(&daemon);
    daemon.setAdaptor(&adaptor);

    QDBusConnection connection = QDBusConnection::systemBus();
    connection.registerService("org.cachyos.USBscope");
    connection.registerObject("/org/cachyos/USBscope/Daemon", &daemon, QDBusConnection::ExportAdaptors);

    JournalTail tail;
    UsbMonitor monitor;

    QObject::connect(&tail, &JournalTail::eventParsed, &daemon, &UsbDaemon::appendEvent);
    QObject::connect(&monitor, &UsbMonitor::devicesChanged, &daemon, &UsbDaemon::setDevices);

    tail.start();
    monitor.start();

    return app.exec();
}
