#include "aboutdialog.h"

#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("About USBscope");
    setWindowIcon(QIcon::fromTheme("usb"));
    setMinimumWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(this);

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
