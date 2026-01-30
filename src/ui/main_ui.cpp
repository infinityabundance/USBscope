#include <QApplication>
#include <QDBusConnectionInterface>
#include <QIcon>
#include <QProcess>

#include "dbus_helpers.h"
#include "mainwindow.h"

namespace {
void ensureDaemonAndTrayRunning() {
    QDBusConnection bus = usbscopeBus();
    if (!bus.isConnected()) {
        return;
    }
    QDBusConnectionInterface *iface = bus.interface();
    if (!iface) {
        return;
    }

    if (!iface->isServiceRegistered("org.cachyos.USBscope")) {
        QProcess::startDetached("usbscoped");
    }

    QProcess::startDetached("usbscope-tray");
}
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    registerUsbDbusTypes();
    ensureDaemonAndTrayRunning();

    MainWindow window;
    window.setWindowIcon(QIcon::fromTheme("usb"));
    window.show();

    return app.exec();
}
