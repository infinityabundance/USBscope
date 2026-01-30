#include <QApplication>

#include "dbus_helpers.h"
#include "trayicon.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    registerUsbDbusTypes();

    TrayIcon tray;
    tray.show();

    return app.exec();
}
