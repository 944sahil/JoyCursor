#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QIcon>
#include <QApplication>
#include <QDebug>
#include <QPixmap>
#include <QFile>
#include <QPainter>
#include <QStyleOption>
#include <QFont>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setStyleSheet("QMainWindow { background: #f9fafb; border-radius: 16px; } ");
    // resize(400, 260);
    setFixedSize(400,260);
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
    configureButton->setEnabled(false);
    mainLayout->addWidget(configureButton, 0, Qt::AlignLeft);
    connect(configureButton, &QPushButton::clicked, this, &MainWindow::onConfigureControllerClicked);

    manageButton = new QPushButton("Manage All Controllers");
    manageButton->setStyleSheet("QPushButton { color: #1565c0; background: transparent; border: none; text-align: left; font-size: 15px; padding: 0; } QPushButton:hover { text-decoration: underline; }");
    manageButton->setCursor(Qt::PointingHandCursor);
    connect(manageButton, &QPushButton::clicked, this, &MainWindow::onManageControllersClicked);
    mainLayout->addWidget(manageButton, 0, Qt::AlignLeft);

    // --- Worker thread setup ---
    workerThread = new QThread(this);
    coreWorker = new CoreWorker();
    coreWorker->moveToThread(workerThread);
    
    connect(workerThread, &QThread::started, coreWorker, &CoreWorker::start);
    connect(workerThread, &QThread::finished, coreWorker, &CoreWorker::stop);
    connect(coreWorker, &CoreWorker::controllerConnected, this, &MainWindow::onControllerConnected);
    connect(coreWorker, &CoreWorker::controllerDisconnected, this, &MainWindow::onControllerDisconnected);
    
    workerThread->start();
    
    // Initialize controller library window pointer
    controllerLibraryWindow = nullptr;
    // customizationWindow = nullptr;
}

MainWindow::~MainWindow() {
    if (coreWorker) {
        QMetaObject::invokeMethod(coreWorker, "stop", Qt::BlockingQueuedConnection);
    }
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
    }
}

void MainWindow::onControllerConnected(const QString& guid, const QString& name) {
    m_currentControllerGuid = guid;
    m_currentControllerName = name;
    m_currentControllerConnected = true;
    controllerNameLabel->setText(name);
    statusLabel->setText("Connected");
    if (profileStatusDot) static_cast<DotWidget*>(profileStatusDot)->setColor(QColor("#21c521")); // Green
    configureButton->setEnabled(true);
}

void MainWindow::onControllerDisconnected(const QString& guid) {
    m_currentControllerGuid.clear();
    m_currentControllerName.clear();
    m_currentControllerConnected = false;
    controllerNameLabel->setText("Controller");
    statusLabel->setText("Disconnected");
    statusLabel->setStyleSheet("color: #555;");
    static_cast<DotWidget*>(profileStatusDot)->setColor(QColor("#FF3B30"));
    profileNameLabel->setText("Profile");
    configureButton->setEnabled(false);
}

void MainWindow::onManageControllersClicked() {
    if (!controllerLibraryWindow) {
        controllerLibraryWindow = new ControllerLibraryWindow(coreWorker, nullptr); // Pass the existing coreWorker
        connect(controllerLibraryWindow, &ControllerLibraryWindow::windowClosed, 
                this, &MainWindow::onControllerLibraryClosed);
    }
    if (controllerLibraryWindow->isVisible()) {
        controllerLibraryWindow->raise();
        controllerLibraryWindow->activateWindow();
    } else {
        controllerLibraryWindow->show();
    }
}

void MainWindow::onControllerLibraryClosed() {
    // The window is closed but we keep the pointer for reuse
    // The window will be deleted when MainWindow is destroyed
} 

void MainWindow::onConfigureControllerClicked() {
    if (!m_currentControllerConnected)
        return;
    ControllerCustomizationWindow::openForController(m_currentControllerGuid, m_currentControllerName, m_currentControllerConnected, coreWorker);
} 