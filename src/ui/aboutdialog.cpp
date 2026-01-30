#include "aboutdialog.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("About USBscope");
    setWindowIcon(QIcon::fromTheme("usb"));
    setMinimumWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *iconLabel = new QLabel(this);
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString iconPath = appDir + "/../data/icons/usbscope.svg";
    QPixmap logo;
    if (QFileInfo::exists(iconPath)) {
        logo.load(iconPath);
    } else {
        logo = QIcon::fromTheme("usb").pixmap(96, 96);
    }
    if (!logo.isNull()) {
        iconLabel->setPixmap(logo.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        iconLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(iconLabel);
    }

    QLabel *titleLabel = new QLabel("<h2>USBscope</h2>", this);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *versionLabel = new QLabel("<b>Version:</b> 0.1.0", this);
    versionLabel->setAlignment(Qt::AlignCenter);

    QLabel *descLabel = new QLabel(
        "A live USB event dashboard for CachyOS and Linux<br><br>"
        "Monitor USB devices and events in real-time with "
        "timeline visualization, advanced filtering, and comprehensive device details.",
        this);
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);

    QPushButton *githubButton = new QPushButton("View on GitHub", this);
    connect(githubButton, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/infinityabundance/USBscope"));
    });

    QLabel *licenseLabel = new QLabel("<b>License:</b> Apache License 2.0", this);
    licenseLabel->setAlignment(Qt::AlignCenter);

    QPushButton *closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    layout->addWidget(titleLabel);
    layout->addWidget(versionLabel);
    layout->addSpacing(10);
    layout->addWidget(descLabel);
    layout->addSpacing(15);
    layout->addWidget(githubButton);
    layout->addSpacing(10);
    layout->addWidget(licenseLabel);
    layout->addStretch();
    layout->addWidget(closeButton);

    setLayout(layout);
}
