#include <QApplication>

#include "trayicon.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    TrayIcon tray;
    tray.show();

    return app.exec();
}
