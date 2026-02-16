#include <QApplication>

#include "dbus_helpers.h"
#include "trayicon.h"

// Entry point for the usbscope-tray process. Registers D-Bus types, creates
// the TrayIcon, and hands control to the Qt event loop.

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    registerUsbDbusTypes();

    TrayIcon tray;
    tray.show();

    return app.exec();
}
