#include "mainwindow.h"

#include "aboutdialog.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QTextStream>
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

void UsbLogFilterProxyModel::setFilterPreset(FilterPreset preset) {
    switch (preset) {
    case AllEvents:
        m_usbOnly = false;
        m_errorsOnly = false;
        break;
    case UsbEventsOnly:
        m_usbOnly = true;
        m_errorsOnly = false;
        break;
    case ErrorsOnly:
        m_usbOnly = false;
        m_errorsOnly = true;
        break;
    case UsbErrors:
        m_usbOnly = true;
        m_errorsOnly = true;
        break;
    case Custom:
        break;
    }
    invalidateFilter();
}

void UsbLogFilterProxyModel::setDateRange(const QDateTime &start, const QDateTime &end) {
    m_startDate = start;
    m_endDate = end;
    m_useDateFilter = true;
    invalidateFilter();
}

void UsbLogFilterProxyModel::clearDateRange() {
    m_useDateFilter = false;
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

    if (m_useDateFilter) {
        QDateTime eventTime = QDateTime::fromString(event.timestamp, Qt::ISODate);
        if (!eventTime.isValid()) {
            eventTime = QDateTime::fromString(event.timestamp, "MMM dd hh:mm:ss");
        }
        if (eventTime.isValid()) {
            if (eventTime < m_startDate || eventTime > m_endDate) {
                return false;
            }
        }
    }

    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupActions();
    setupMenuBar();
    setupUi();
    loadInitialData();

    connect(&m_client, &UsbscopeDBusClient::LogEvent, this, &MainWindow::handleLogEvent);
    connect(&m_client, &UsbscopeDBusClient::DevicesChanged, this, &MainWindow::refreshDevices);
}

void MainWindow::setupActions() {
    m_exportCsvAction = new QAction(QIcon::fromTheme("document-export"), "Export to CSV...", this);
    m_exportCsvAction->setShortcut(QKeySequence::SaveAs);
    connect(m_exportCsvAction, &QAction::triggered, this, &MainWindow::exportToCsv);

    m_copyAction = new QAction(QIcon::fromTheme("edit-copy"), "Copy Selection", this);
    m_copyAction->setShortcut(QKeySequence::Copy);
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::copySelection);

    m_refreshAction = new QAction(QIcon::fromTheme("view-refresh"), "Refresh", this);
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshDevices);

    m_aboutAction = new QAction(QIcon::fromTheme("help-about"), "About USBscope", this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    m_quitAction = new QAction(QIcon::fromTheme("application-exit"), "Quit", this);
    m_quitAction->setShortcut(QKeySequence::Quit);
    connect(m_quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

void MainWindow::setupMenuBar() {
    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(m_exportCsvAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_quitAction);

    QMenu *editMenu = menuBar->addMenu("&Edit");
    editMenu->addAction(m_copyAction);

    QMenu *viewMenu = menuBar->addMenu("&View");
    viewMenu->addAction(m_refreshAction);

    QMenu *helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction(m_aboutAction);

    setMenuBar(menuBar);
}

void MainWindow::setupUi() {
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QHBoxLayout *filterLayout = new QHBoxLayout();

    m_textFilter = new QLineEdit(this);
    m_textFilter->setPlaceholderText("Filter logs...");

    m_filterPreset = new QComboBox(this);
    m_filterPreset->addItem("All Events", UsbLogFilterProxyModel::AllEvents);
    m_filterPreset->addItem("USB Events Only", UsbLogFilterProxyModel::UsbEventsOnly);
    m_filterPreset->addItem("Errors Only", UsbLogFilterProxyModel::ErrorsOnly);
    m_filterPreset->addItem("USB Errors", UsbLogFilterProxyModel::UsbErrors);

    m_enableDateFilter = new QCheckBox("Date Range:", this);
    m_startDate = new QDateTimeEdit(this);
    m_startDate->setDateTime(QDateTime::currentDateTime().addDays(-1));
    m_startDate->setCalendarPopup(true);
    m_startDate->setEnabled(false);

    m_endDate = new QDateTimeEdit(this);
    m_endDate->setDateTime(QDateTime::currentDateTime());
    m_endDate->setCalendarPopup(true);
    m_endDate->setEnabled(false);

    filterLayout->addWidget(new QLabel("Search:"));
    filterLayout->addWidget(m_textFilter);
    filterLayout->addWidget(new QLabel("Filter:"));
    filterLayout->addWidget(m_filterPreset);
    filterLayout->addWidget(m_enableDateFilter);
    filterLayout->addWidget(m_startDate);
    filterLayout->addWidget(new QLabel("to"));
    filterLayout->addWidget(m_endDate);
    filterLayout->addStretch();

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
    resize(1200, 700);

    m_filterModel.setSourceModel(&m_model);
    m_filterModel.setFilterKeyColumn(4);
    m_logView->setModel(&m_filterModel);

    connect(m_textFilter, &QLineEdit::textChanged, &m_filterModel, &QSortFilterProxyModel::setFilterFixedString);
    connect(m_filterPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterPresetChanged);
    connect(m_enableDateFilter, &QCheckBox::toggled, this, [this](bool enabled) {
        m_startDate->setEnabled(enabled);
        m_endDate->setEnabled(enabled);
        onDateRangeChanged();
    });
    connect(m_startDate, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::onDateRangeChanged);
    connect(m_endDate, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::onDateRangeChanged);
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
    // Filters are now handled by preset combo box
}

void MainWindow::onFilterPresetChanged(int index) {
    auto preset = static_cast<UsbLogFilterProxyModel::FilterPreset>(m_filterPreset->itemData(index).toInt());
    m_filterModel.setFilterPreset(preset);
}

void MainWindow::onDateRangeChanged() {
    if (m_enableDateFilter->isChecked()) {
        m_filterModel.setDateRange(m_startDate->dateTime(), m_endDate->dateTime());
    } else {
        m_filterModel.clearDateRange();
    }
}

void MainWindow::showAboutDialog() {
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::exportToCsv() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export to CSV",
        "usbscope_log.csv",
        "CSV Files (*.csv);;All Files (*)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Failed", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    out << "\"Timestamp\",\"Level\",\"Subsystem\",\"Source\",\"Message\"\n";

    for (int row = 0; row < m_filterModel.rowCount(); ++row) {
        QStringList fields;
        for (int col = 0; col < 5; ++col) {
            QModelIndex index = m_filterModel.index(row, col);
            QString value = index.data().toString();
            value.replace("\"", "\"\"");
            fields << QString("\"%1\"").arg(value);
        }
        out << fields.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Complete",
        QString("Exported %1 events to %2").arg(m_filterModel.rowCount()).arg(fileName));
}

void MainWindow::copySelection() {
    QModelIndexList indexes = m_logView->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return;
    }

    std::sort(indexes.begin(), indexes.end());

    QString text;
    for (const QModelIndex &index : indexes) {
        QStringList fields;
        for (int col = 0; col < 5; ++col) {
            QModelIndex cellIndex = m_filterModel.index(index.row(), col);
            fields << cellIndex.data().toString();
        }
        text += fields.join("\t") + "\n";
    }

    QApplication::clipboard()->setText(text);
}
