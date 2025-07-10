#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QFile>
#include <QPainter>
#include <QStyleOption>
#include <QFont>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setStyleSheet("QMainWindow { background: #f9fafb; border-radius: 16px; } ");
    resize(400, 260);
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet("background: #f9fafb; border-radius: 16px;");

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(18);
    mainLayout->addSpacing(24);

    // Controller info row
    QHBoxLayout* controllerLayout = new QHBoxLayout();
    controllerLayout->setSpacing(14);
    iconLabel = new QLabel();
    QPixmap iconPixmap;
    if (!iconPixmap.load(":/icons/gamepad.svg")) {
        iconLabel->setText("?");
    } else {
        iconLabel->setPixmap(iconPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    iconLabel->setFixedSize(48, 48);
    controllerLayout->addWidget(iconLabel);

    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    controllerNameLabel = new QLabel("Controller");
    QFont nameFont;
    nameFont.setPointSize(15);
    nameFont.setBold(true);
    controllerNameLabel->setFont(nameFont);
    controllerNameLabel->setStyleSheet("color: #222;");
    statusLabel = new QLabel("Disconnected");
    QFont statusFont;
    statusFont.setPointSize(12);
    statusLabel->setFont(statusFont);
    statusLabel->setStyleSheet("color: #555;");
    infoLayout->addWidget(controllerNameLabel);
    infoLayout->addWidget(statusLabel);
    controllerLayout->addLayout(infoLayout);
    controllerLayout->addStretch();
    mainLayout->addLayout(controllerLayout);

    mainLayout->addStretch();

    // Profile box (only outer box has border/background)
    profileBox = new QFrame();
    profileBox->setFrameShape(QFrame::StyledPanel);
    profileBox->setStyleSheet("QFrame { border: 1px solid #e0e0e0; border-radius: 10px; background: #fff; }");
    profileBox->setMinimumHeight(50);
    QHBoxLayout* profileLayout = new QHBoxLayout(profileBox);
    profileLayout->setContentsMargins(18, 8, 18, 8);
    profileLayout->setSpacing(8);
    // Use DotWidget for a perfect green dot
    profileStatusDot = new DotWidget();
    profileStatusDot->setFixedSize(16, 16);
    profileLayout->addWidget(profileStatusDot);
    // Profile name (no border, no background)
    profileNameLabel = new QLabel("Profile");
    QFont profileFont;
    profileFont.setPointSize(13);
    profileNameLabel->setFont(profileFont);
    profileNameLabel->setStyleSheet("color: #222; margin-left: 4px; background: transparent; border: none;");
    profileLayout->addWidget(profileNameLabel);
    profileLayout->addStretch();
    mainLayout->addWidget(profileBox);

    // Action links (anchored to bottom)
    configureButton = new QPushButton("Configure Controller");
    configureButton->setStyleSheet("QPushButton { color: #1565c0; background: transparent; border: none; text-align: left; font-size: 15px; padding: 0; } QPushButton:hover { text-decoration: underline; }");
    configureButton->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(configureButton, 0, Qt::AlignLeft);

    manageButton = new QPushButton("Manage All Controllers");
    manageButton->setStyleSheet("QPushButton { color: #1565c0; background: transparent; border: none; text-align: left; font-size: 15px; padding: 0; } QPushButton:hover { text-decoration: underline; }");
    manageButton->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(manageButton, 0, Qt::AlignLeft);
}

MainWindow::~MainWindow() = default; 