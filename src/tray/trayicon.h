#pragma once

#include <QDateTime>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>

#include "dbus_helpers.h"

class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT
public:
    explicit TrayIcon(QObject *parent = nullptr);

private slots:
    void handleLogEvent(const UsbEvent &event);
    void handleErrorBurst(int count, const QString &lastMessage);
    void updateTooltip();
    void openUi();
    void openRepo();
    void copyLastError();
    void updateDaemonStatus();

private:
    void setupMenu();
    QString tooltipText() const;
    QString daemonStatusText() const;

    UsbscopeDBusClient m_client;
    QMenu m_menu;
    QAction *m_openAction = nullptr;
    QAction *m_statusAction = nullptr;
    QAction *m_aboutAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_quitAction = nullptr;

    QString m_lastErrorMessage;
    QDateTime m_lastErrorTime;
    QTimer m_tooltipTimer;
    QTimer m_statusTimer;
};
