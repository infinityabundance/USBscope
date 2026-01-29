#include "mainwindow.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>

UsbLogModel::UsbLogModel(QObject *parent)
    : QAbstractTableModel(parent) {
}

int UsbLogModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_events.size();
}

int UsbLogModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return 5;
}

QVariant UsbLogModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_events.size()) {
        return {};
    }
    const UsbEvent &event = m_events.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return event.timestamp;
        case 1:
            return event.level;
        case 2:
            return event.subsystem;
        case 3:
            return event.source;
        case 4:
            return event.message;
        default:
            return {};
        }
    }
    return {};
}

QVariant UsbLogModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return QStringLiteral("Timestamp");
        case 1:
            return QStringLiteral("Level");
        case 2:
            return QStringLiteral("Subsystem");
        case 3:
            return QStringLiteral("Source");
        case 4:
            return QStringLiteral("Message");
        default:
            return {};
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void UsbLogModel::setEvents(const QList<UsbEvent> &events) {
    beginResetModel();
    m_events = events;
    endResetModel();
}

void UsbLogModel::appendEvent(const UsbEvent &event) {
    beginInsertRows(QModelIndex(), m_events.size(), m_events.size());
    m_events.append(event);
    endInsertRows();
}

const UsbEvent &UsbLogModel::eventAt(int row) const {
    return m_events.at(row);
}

UsbLogFilterProxyModel::UsbLogFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void UsbLogFilterProxyModel::setUsbOnly(bool enabled) {
    m_usbOnly = enabled;
    invalidateFilter();
}

void UsbLogFilterProxyModel::setErrorsOnly(bool enabled) {
    m_errorsOnly = enabled;
    invalidateFilter();
}

bool UsbLogFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex idx = sourceModel()->index(sourceRow, 4, sourceParent);
    if (!idx.isValid()) {
        return false;
    }

    const UsbLogModel *model = qobject_cast<const UsbLogModel *>(sourceModel());
    if (!model) {
        return false;
    }
    const UsbEvent &event = model->eventAt(sourceRow);

    if (m_usbOnly && !event.isUsb) {
        return false;
    }
    if (m_errorsOnly && !event.isError) {
        return false;
    }

    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
    loadInitialData();

    connect(&m_client, &UsbscopeDBusClient::LogEvent, this, &MainWindow::handleLogEvent);
    connect(&m_client, &UsbscopeDBusClient::DevicesChanged, this, &MainWindow::refreshDevices);
}

void MainWindow::setupUi() {
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QHBoxLayout *filterLayout = new QHBoxLayout();
    m_textFilter = new QLineEdit(this);
    m_textFilter->setPlaceholderText("Filter logs...");
    m_usbOnly = new QCheckBox("USB only", this);
    m_errorsOnly = new QCheckBox("Errors only", this);

    filterLayout->addWidget(new QLabel("Search:"));
    filterLayout->addWidget(m_textFilter);
    filterLayout->addWidget(m_usbOnly);
    filterLayout->addWidget(m_errorsOnly);

    m_logView = new QTableView(this);
    m_logView->horizontalHeader()->setStretchLastSection(true);
    m_logView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_deviceList = new QListWidget(this);

    QSplitter *splitter = new QSplitter(this);
    splitter->addWidget(m_logView);
    splitter->addWidget(m_deviceList);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(splitter);

    setCentralWidget(central);
    setWindowTitle("USBscope");

    m_filterModel.setSourceModel(&m_model);
    m_filterModel.setFilterKeyColumn(4);
    m_logView->setModel(&m_filterModel);

    connect(m_textFilter, &QLineEdit::textChanged, &m_filterModel, &QSortFilterProxyModel::setFilterFixedString);
    connect(m_usbOnly, &QCheckBox::toggled, this, &MainWindow::updateFilters);
    connect(m_errorsOnly, &QCheckBox::toggled, this, &MainWindow::updateFilters);
}

void MainWindow::loadInitialData() {
    m_model.setEvents(m_client.getRecentEvents(500));
    refreshDevices();
}

void MainWindow::handleLogEvent(const UsbEvent &event) {
    m_model.appendEvent(event);
}

void MainWindow::refreshDevices() {
    m_deviceList->clear();
    const QList<UsbDeviceInfo> devices = m_client.getCurrentDevices();
    for (const UsbDeviceInfo &device : devices) {
        QString label = device.summary;
        if (label.isEmpty()) {
            label = device.deviceId;
        }
        if (!device.vendorId.isEmpty() && !device.productId.isEmpty()) {
            label += QStringLiteral(" (%1:%2)").arg(device.vendorId, device.productId);
        }
        m_deviceList->addItem(label);
    }
}

void MainWindow::updateFilters() {
    m_filterModel.setUsbOnly(m_usbOnly->isChecked());
    m_filterModel.setErrorsOnly(m_errorsOnly->isChecked());
}
