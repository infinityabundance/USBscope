#include <QApplication>

#include "dbus_helpers.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    registerUsbDbusTypes();

    MainWindow window;
    window.show();

    return app.exec();
}
