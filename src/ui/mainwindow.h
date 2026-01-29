#pragma once

#include <QAbstractTableModel>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>

#include "dbus_helpers.h"

class UsbLogModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit UsbLogModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setEvents(const QList<UsbEvent> &events);
    void appendEvent(const UsbEvent &event);
    const UsbEvent &eventAt(int row) const;

private:
    QList<UsbEvent> m_events;
};

class UsbLogFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit UsbLogFilterProxyModel(QObject *parent = nullptr);

    void setUsbOnly(bool enabled);
    void setErrorsOnly(bool enabled);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool m_usbOnly = false;
    bool m_errorsOnly = false;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void handleLogEvent(const UsbEvent &event);
    void refreshDevices();
    void updateFilters();

private:
    void setupUi();
    void loadInitialData();

    UsbscopeDBusClient m_client;
    UsbLogModel m_model;
    UsbLogFilterProxyModel m_filterModel;

    QTableView *m_logView = nullptr;
    QListWidget *m_deviceList = nullptr;
    QLineEdit *m_textFilter = nullptr;
    QCheckBox *m_usbOnly = nullptr;
    QCheckBox *m_errorsOnly = nullptr;
};
