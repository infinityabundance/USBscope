#include "mainwindow.h"

#include "aboutdialog.h"
#include "eventmarker.h"
#include "timelinescene.h"
#include "timelineview.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QTextStream>
#include <QToolBar>
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
    beginFilterChange();
    endFilterChange();
}

void UsbLogFilterProxyModel::setErrorsOnly(bool enabled) {
    m_errorsOnly = enabled;
    beginFilterChange();
    endFilterChange();
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
    beginFilterChange();
    endFilterChange();
}

void UsbLogFilterProxyModel::setDateRange(const QDateTime &start, const QDateTime &end) {
    m_startDate = start;
    m_endDate = end;
    m_useDateFilter = true;
    beginFilterChange();
    endFilterChange();
}

void UsbLogFilterProxyModel::clearDateRange() {
    m_useDateFilter = false;
    beginFilterChange();
    endFilterChange();
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
    setupToolBar();
    setupUi();
    loadInitialData();

    connect(&m_client, &UsbscopeDBusClient::LogEvent, this, &MainWindow::handleLogEvent);
    connect(&m_client, &UsbscopeDBusClient::DevicesChanged, this, &MainWindow::refreshDevices);
}

void MainWindow::setupActions() {
    m_exportCsvAction = new QAction(QIcon::fromTheme("document-export"), "Export to CSV...", this);
    m_exportCsvAction->setShortcut(QKeySequence::SaveAs);
    connect(m_exportCsvAction, &QAction::triggered, this, &MainWindow::exportToCsv);

    m_exportDevicesAction = new QAction(QIcon::fromTheme("document-export"), "Export Devices to CSV...", this);
    connect(m_exportDevicesAction, &QAction::triggered, this, &MainWindow::exportDevicesToCsv);

    m_copyAction = new QAction(QIcon::fromTheme("edit-copy"), "Copy Selection", this);
    m_copyAction->setShortcut(QKeySequence::Copy);
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::copySelection);

    m_refreshAction = new QAction(QIcon::fromTheme("view-refresh"), "Refresh", this);
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshDevices);

    m_aboutAction = new QAction(QIcon::fromTheme("help-about"), "About USBscope", this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    m_startDaemonAction = new QAction(QIcon::fromTheme("media-playback-start"), "Start Daemon", this);
    connect(m_startDaemonAction, &QAction::triggered, this, &MainWindow::startDaemon);

    m_stopDaemonAction = new QAction(QIcon::fromTheme("media-playback-stop"), "Stop Daemon", this);
    connect(m_stopDaemonAction, &QAction::triggered, this, &MainWindow::stopDaemon);

    m_openTrayAction = new QAction(QIcon::fromTheme("preferences-system-tray"), "Open Tray", this);
    connect(m_openTrayAction, &QAction::triggered, this, &MainWindow::openTray);

    m_quitAction = new QAction(QIcon::fromTheme("application-exit"), "Quit", this);
    m_quitAction->setShortcut(QKeySequence::Quit);
    connect(m_quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

void MainWindow::setupMenuBar() {
    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(m_exportCsvAction);
    fileMenu->addAction(m_exportDevicesAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_quitAction);

    QMenu *editMenu = menuBar->addMenu("&Edit");
    editMenu->addAction(m_copyAction);

    QMenu *viewMenu = menuBar->addMenu("&View");
    viewMenu->addAction(m_refreshAction);

    QMenu *toolsMenu = menuBar->addMenu("&Tools");
    toolsMenu->addAction(m_startDaemonAction);
    toolsMenu->addAction(m_stopDaemonAction);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_openTrayAction);

    QMenu *helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction(m_aboutAction);

    setMenuBar(menuBar);
}

void MainWindow::setupToolBar() {
    QToolBar *toolBar = new QToolBar("Main Toolbar", this);
    toolBar->setMovable(false);

    toolBar->addAction(m_exportCsvAction);
    toolBar->addAction(m_copyAction);
    toolBar->addSeparator();
    toolBar->addAction(m_refreshAction);
    toolBar->addSeparator();
    toolBar->addAction(m_openTrayAction);

    addToolBar(toolBar);
}

void MainWindow::setupUi() {
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Create tab widget
    m_tabWidget = new QTabWidget(this);

    // ===== Log View Tab =====
    QWidget *logTab = new QWidget();
    QVBoxLayout *logLayout = new QVBoxLayout(logTab);

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
    m_logView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_logView, &QTableView::customContextMenuRequested, this, &MainWindow::showContextMenu);

    m_deviceList = new QListWidget(this);
    m_deviceList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_deviceList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_deviceList, &QListWidget::customContextMenuRequested, this, &MainWindow::showDeviceContextMenu);

    QSplitter *splitter = new QSplitter(this);
    splitter->addWidget(m_logView);
    splitter->addWidget(m_deviceList);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    logLayout->addLayout(filterLayout);
    logLayout->addWidget(splitter);

    // ===== Timeline Tab =====
    QWidget *timelineTab = new QWidget();
    QVBoxLayout *timelineLayout = new QVBoxLayout(timelineTab);

    // Create timeline controls
    QHBoxLayout *timelineControls = new QHBoxLayout();
    QPushButton *zoomInBtn = new QPushButton(QIcon::fromTheme("zoom-in"), "Zoom In", this);
    QPushButton *zoomOutBtn = new QPushButton(QIcon::fromTheme("zoom-out"), "Zoom Out", this);
    QPushButton *resetZoomBtn = new QPushButton(QIcon::fromTheme("zoom-original"), "Reset Zoom", this);

    timelineControls->addWidget(new QLabel("Timeline Controls:"));
    timelineControls->addWidget(zoomInBtn);
    timelineControls->addWidget(zoomOutBtn);
    timelineControls->addWidget(resetZoomBtn);
    timelineControls->addStretch();

    QLabel *helpLabel = new QLabel("Ctrl+Wheel to zoom, Shift+Click or Middle-click to pan");
    helpLabel->setStyleSheet("color: #666;");
    timelineControls->addWidget(helpLabel);

    // Create timeline view and scene
    m_timelineScene = new TimelineScene(this);
    m_timelineView = new TimelineView(this);
    m_timelineView->setScene(m_timelineScene);

    timelineLayout->addLayout(timelineControls);
    timelineLayout->addWidget(m_timelineView);

    // Connect timeline controls
    connect(zoomInBtn, &QPushButton::clicked, m_timelineView, &TimelineView::zoomIn);
    connect(zoomOutBtn, &QPushButton::clicked, m_timelineView, &TimelineView::zoomOut);
    connect(resetZoomBtn, &QPushButton::clicked, m_timelineView, &TimelineView::resetZoom);
    connect(m_timelineScene, &TimelineScene::eventClicked, this, &MainWindow::onTimelineEventClicked);

    // Add tabs to tab widget
    m_tabWidget->addTab(logTab, QIcon::fromTheme("view-list-details"), "Log View");
    m_tabWidget->addTab(timelineTab, QIcon::fromTheme("view-time-schedule"), "Timeline");

    mainLayout->addWidget(m_tabWidget);

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
    QList<UsbEvent> events = m_client.getRecentEvents(500);
    m_model.setEvents(events);
    m_timelineScene->setEvents(events);
    m_timelineView->fitToView();
    refreshDevices();
}

void MainWindow::handleLogEvent(const UsbEvent &event) {
    m_model.appendEvent(event);
    m_timelineScene->addEvent(event);
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
        QListWidgetItem *item = new QListWidgetItem(label, m_deviceList);
        item->setData(Qt::UserRole, QVariant::fromValue(toVariant(device)));
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

void MainWindow::copyDevicesSelection() {
    QList<QListWidgetItem *> selected = m_deviceList->selectedItems();
    if (selected.isEmpty()) {
        return;
    }

    QString text;
    for (QListWidgetItem *item : selected) {
        const QVariantList data = item->data(Qt::UserRole).toList();
        const UsbDeviceInfo device = deviceFromVariant(data);
        QStringList fields;
        fields << device.busId
               << device.deviceId
               << device.vendorId
               << device.productId
               << device.summary
               << device.sysPath;
        text += fields.join("\t") + "\n";
    }

    QApplication::clipboard()->setText(text);
}

void MainWindow::showContextMenu(const QPoint &pos) {
    QMenu contextMenu(this);

    QModelIndexList selectedRows = m_logView->selectionModel()->selectedRows();
    bool hasSelection = !selectedRows.isEmpty();

    QAction *copyRowAction = contextMenu.addAction(QIcon::fromTheme("edit-copy"), "Copy Selected Row(s)");
    copyRowAction->setEnabled(hasSelection);
    connect(copyRowAction, &QAction::triggered, this, &MainWindow::copySelection);

    contextMenu.addSeparator();

    QAction *copyAllAction = contextMenu.addAction(QIcon::fromTheme("edit-copy-all"), "Copy All Visible as CSV");
    connect(copyAllAction, &QAction::triggered, this, [this]() {
        QString text;
        text += "\"Timestamp\",\"Level\",\"Subsystem\",\"Source\",\"Message\"\n";

        for (int row = 0; row < m_filterModel.rowCount(); ++row) {
            QStringList fields;
            for (int col = 0; col < 5; ++col) {
                QModelIndex index = m_filterModel.index(row, col);
                QString value = index.data().toString();
                value.replace("\"", "\"\"");
                fields << QString("\"%1\"").arg(value);
            }
            text += fields.join(",") + "\n";
        }

        QApplication::clipboard()->setText(text);
    });

    contextMenu.addSeparator();

    contextMenu.addAction(m_exportCsvAction);

    contextMenu.exec(m_logView->mapToGlobal(pos));
}

void MainWindow::showDeviceContextMenu(const QPoint &pos) {
    QMenu contextMenu(this);

    bool hasSelection = !m_deviceList->selectedItems().isEmpty();

    QAction *copyDevicesAction = contextMenu.addAction(QIcon::fromTheme("edit-copy"), "Copy Selected Device(s)");
    copyDevicesAction->setEnabled(hasSelection);
    connect(copyDevicesAction, &QAction::triggered, this, &MainWindow::copyDevicesSelection);

    contextMenu.addSeparator();
    contextMenu.addAction(m_exportDevicesAction);

    contextMenu.exec(m_deviceList->mapToGlobal(pos));
}

void MainWindow::exportDevicesToCsv() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Devices to CSV",
        "usbscope_devices.csv",
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
    out << "\"Bus ID\",\"Device ID\",\"Vendor ID\",\"Product ID\",\"Summary\",\"Sys Path\"\n";

    const QList<UsbDeviceInfo> devices = m_client.getCurrentDevices();
    for (const UsbDeviceInfo &device : devices) {
        QStringList fields;
        fields << device.busId
               << device.deviceId
               << device.vendorId
               << device.productId
               << device.summary
               << device.sysPath;
        for (QString &field : fields) {
            field.replace("\"", "\"\"");
            field = QString("\"%1\"").arg(field);
        }
        out << fields.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Complete",
        QString("Exported %1 devices to %2").arg(devices.size()).arg(fileName));
}

void MainWindow::onTimelineEventClicked(const UsbEvent &event) {
    // Switch to Log View tab
    m_tabWidget->setCurrentIndex(0);

    // Find the event in the model and select it
    for (int row = 0; row < m_filterModel.rowCount(); ++row) {
        QModelIndex proxyIndex = m_filterModel.index(row, 0);
        QModelIndex sourceIndex = m_filterModel.mapToSource(proxyIndex);

        const UsbEvent &rowEvent = m_model.eventAt(sourceIndex.row());

        // Match by timestamp and message (unique enough)
        if (rowEvent.timestamp == event.timestamp && rowEvent.message == event.message) {
            m_logView->selectRow(row);
            m_logView->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);
            break;
        }
    }
}

void MainWindow::startDaemon() {
    startUsbScopeDaemon();
}

void MainWindow::stopDaemon() {
    stopUsbScopeDaemon();
}

void MainWindow::openTray() {
    startUsbScopeTray();
}
