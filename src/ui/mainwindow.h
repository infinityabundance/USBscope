#pragma once

#include <QAbstractTableModel>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
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
    enum FilterPreset {
        AllEvents,
        UsbEventsOnly,
        ErrorsOnly,
        UsbErrors,
        Custom
    };

    explicit UsbLogFilterProxyModel(QObject *parent = nullptr);

    void setUsbOnly(bool enabled);
    void setErrorsOnly(bool enabled);
    void setFilterPreset(FilterPreset preset);
    void setDateRange(const QDateTime &start, const QDateTime &end);
    void clearDateRange();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool m_usbOnly = false;
    bool m_errorsOnly = false;
    bool m_useDateFilter = false;
    QDateTime m_startDate;
    QDateTime m_endDate;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void handleLogEvent(const UsbEvent &event);
    void refreshDevices();
    void updateFilters();
    void onFilterPresetChanged(int index);
    void onDateRangeChanged();
    void showAboutDialog();
    void exportToCsv();
    void copySelection();
    void showContextMenu(const QPoint &pos);

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupActions();
    void loadInitialData();

    UsbscopeDBusClient m_client;
    UsbLogModel m_model;
    UsbLogFilterProxyModel m_filterModel;

    // UI Components
    QTableView *m_logView = nullptr;
    QListWidget *m_deviceList = nullptr;
    QLineEdit *m_textFilter = nullptr;
    QComboBox *m_filterPreset = nullptr;
    QDateTimeEdit *m_startDate = nullptr;
    QDateTimeEdit *m_endDate = nullptr;
    QCheckBox *m_enableDateFilter = nullptr;

    // Actions
    QAction *m_exportCsvAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_quitAction = nullptr;
    QAction *m_refreshAction = nullptr;
    QAction *m_aboutAction = nullptr;
};
