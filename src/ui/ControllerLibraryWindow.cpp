#include "ControllerLibraryWindow.h"
#include <QApplication>
#include <QStyle>
#include <QFont>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QScrollArea>

// ControllerListItem implementation
ControllerCard::ControllerCard(const QString& guid, const QString& name, bool connected, QWidget* parent)
    : QFrame(parent), m_guid(guid), m_name(name) {
    setupUI();
    updateStatus(connected);
    setCursor(Qt::PointingHandCursor);
    setFrameShape(QFrame::NoFrame);
    setStyleSheet("QFrame { background: #fff; border: 1px solid #e0e0e0; border-radius: 8px; } QLabel { background: transparent; border: none; }");
    setFixedHeight(48);
    installEventFilter(this);
}

void ControllerCard::setupUI() {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 4, 12, 4);
    layout->setSpacing(10);
    // Icon
    m_iconLabel = new QLabel();
    m_iconLabel->setStyleSheet("background: transparent; border: none;");
    QPixmap iconPixmap;
    if (iconPixmap.load(":/icons/gamepad.svg")) {
        m_iconLabel->setPixmap(iconPixmap.scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_iconLabel->setText("🎮");
        m_iconLabel->setStyleSheet("font-size: 24px; background: transparent; border: none;");
    }
    m_iconLabel->setFixedSize(36, 36);
    layout->addWidget(m_iconLabel);
    // Info
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(0);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    m_nameLabel = new QLabel(m_name);
    QFont nameFont;
    nameFont.setPointSize(13);
    nameFont.setBold(true);
    m_nameLabel->setFont(nameFont);
    m_nameLabel->setStyleSheet("color: #222; background: transparent; border: none;");
    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet("color: #555; background: transparent; border: none;");
    QFont statusFont;
    statusFont.setPointSize(11);
    m_statusLabel->setFont(statusFont);
    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_statusLabel);
    layout->addLayout(infoLayout);
    layout->addStretch();
}

void ControllerCard::setConnected(bool connected) {
    updateStatus(connected);
}

void ControllerCard::setName(const QString& name) {
    m_name = name;
    m_nameLabel->setText(name);
}

void ControllerCard::updateStatus(bool connected) {
    if (connected) {
        m_statusLabel->setText("Connected");
        m_statusLabel->setStyleSheet("color: #21c521; font-weight: bold;");
        m_iconLabel->setStyleSheet("opacity: 1.0;");
    } else {
        m_statusLabel->setText("Not Connected");
        m_statusLabel->setStyleSheet("color: #555;");
        m_iconLabel->setStyleSheet("opacity: 0.5;");
    }
}

void ControllerCard::setHover(bool hover) {
    m_hovered = hover;
    if (hover) {
        setStyleSheet(
            "QFrame { background: #f5faff; border: 1px solid #b3d4fc; border-radius: 8px; }"
            "QLabel { background: transparent; border: none; }"
        );
    } else {
        setStyleSheet(
            "QFrame { background: #fff; border: 1px solid #e0e0e0; border-radius: 8px; }"
            "QLabel { background: transparent; border: none; }"
        );
    }
}

bool ControllerCard::eventFilter(QObject* obj, QEvent* event) {
    if (obj == this) {
        if (event->type() == QEvent::MouseButtonRelease) {
            emit clicked(m_guid);
            return true;
        }
    }
    return QFrame::eventFilter(obj, event);
}

void ControllerCard::enterEvent(QEnterEvent* event) {
    setHover(true);
    QFrame::enterEvent(event);
}
void ControllerCard::leaveEvent(QEvent* event) {
    setHover(false);
    QFrame::leaveEvent(event);
}

// ControllerLibraryWindow implementation
ControllerLibraryWindow::ControllerLibraryWindow(CoreWorker* sharedCoreWorker, QWidget* parent)
    : QWidget(parent), m_coreWorker(sharedCoreWorker) {
    setWindowTitle("Controller Library");
    setFixedSize(400, 400); // Match main window width
    setStyleSheet("QWidget { background: #f9fafb; }");
    setupUI();
    // Connect signals to the shared coreWorker
    connect(m_coreWorker, &CoreWorker::controllerConnected, this, &ControllerLibraryWindow::onControllerConnected);
    connect(m_coreWorker, &CoreWorker::controllerDisconnected, this, &ControllerLibraryWindow::onControllerDisconnected);
    loadKnownControllers();
}

ControllerLibraryWindow::~ControllerLibraryWindow() {
    // No need to stop or delete m_coreWorker or any thread
}

void ControllerLibraryWindow::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(12);
    // Title
    m_titleLabel = new QLabel("Controller Library");
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: #222; margin-bottom: 8px;");
    m_mainLayout->addWidget(m_titleLabel);
    // Card list area
    m_cardListWidget = new QWidget();
    m_cardListLayout = new QVBoxLayout(m_cardListWidget);
    m_cardListLayout->setContentsMargins(0, 0, 0, 0);
    m_cardListLayout->setSpacing(8);
    // Make the card list scrollable
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Hide vertical scrollbar
    scrollArea->setWidget(m_cardListWidget);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; } QScrollBar { width: 0px; height: 0px; }");
    m_mainLayout->addWidget(scrollArea);
    m_mainLayout->addStretch();
}

void ControllerLibraryWindow::clearControllerCards() {
    QLayoutItem* item;
    while ((item = m_cardListLayout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }
    m_cardWidgets.clear();
}

void ControllerLibraryWindow::loadKnownControllers() {
    if (m_coreWorker && m_coreWorker->getCore()) {
        auto knownControllers = m_coreWorker->getCore()->getKnownControllers();
        m_knownControllers.clear();
        for (const auto& pair : knownControllers) {
            QString guid = QString::fromStdString(pair.first);
            QString name = QString::fromStdString(pair.second);
            m_knownControllers[guid] = name;
        }
        // Get currently connected controllers for initial status
        m_connectedGuids.clear();
        auto connectedControllers = m_coreWorker->getCore()->getConnectedControllers();
        for (const auto& pair : connectedControllers) {
            QString guid = QString::fromStdString(pair.first);
            m_connectedGuids.insert(guid);
        }
    }
    refreshControllerList();
}

void ControllerLibraryWindow::refreshControllerList() {
    clearControllerCards();
    for (auto it = m_knownControllers.begin(); it != m_knownControllers.end(); ++it) {
        QString guid = it.key();
        QString name = it.value();
        bool connected = m_connectedGuids.contains(guid);
        ControllerCard* card = new ControllerCard(guid, name, connected);
        connect(card, &ControllerCard::clicked, this, &ControllerLibraryWindow::onControllerCardClicked);
        m_cardListLayout->addWidget(card);
        m_cardWidgets[guid] = card;
    }
    m_cardListLayout->addStretch();
}

void ControllerLibraryWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    refreshControllerList();
}

void ControllerLibraryWindow::onControllerConnected(const QString& guid, const QString& name) {
    if (m_cardWidgets.contains(guid)) {
        m_cardWidgets[guid]->setConnected(true);
    }
}

void ControllerLibraryWindow::onControllerDisconnected(const QString& guid) {
    if (m_cardWidgets.contains(guid)) {
        m_cardWidgets[guid]->setConnected(false);
    }
}

void ControllerLibraryWindow::onControllerCardClicked(const QString& guid) {
    emit controllerSelected(guid);
    openCustomizationWindow(guid);
}

void ControllerLibraryWindow::openCustomizationWindow(const QString& guid) {
    QString name = m_knownControllers.value(guid, "Unknown Controller");
    bool connected = m_connectedGuids.contains(guid);
    ControllerCustomizationWindow::openForController(guid, name, connected, m_coreWorker);
}

void ControllerLibraryWindow::updateControllerStatus(const QString& guid, bool connected) {
    if (m_cardWidgets.contains(guid)) {
        m_cardWidgets[guid]->setConnected(connected);
    }
}

void ControllerLibraryWindow::saveControllerChanges() {
    QJsonObject root;
    QJsonArray controllers;
    
    for (auto it = m_knownControllers.begin(); it != m_knownControllers.end(); ++it) {
        QJsonObject controller;
        controller["guid"] = it.key();
        controller["name"] = it.value();
        controllers.append(controller);
    }
    
    root["controllers"] = controllers;
    
    QFile file("controllers.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson());
        file.close();
    }
} 