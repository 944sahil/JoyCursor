#include "CoreWorker.h"
#include <QDebug>
#include <QThread>

CoreWorker::CoreWorker(QObject* parent)
    : QObject(parent), m_core(std::make_unique<JoyCursorCore>()), m_timer(nullptr) {
    
    // Set up core callbacks
    m_core->setControllerConnectedCallback(
        [this](const std::string& guid, const std::string& name) {
            onControllerConnected(guid, name);
        }
    );
    
    m_core->setControllerDisconnectedCallback(
        [this](const std::string& guid) {
            onControllerDisconnected(guid);
        }
    );
    
    m_core->setButtonEventCallback(
        [this](const std::string& guid, const std::string& button, bool pressed) {
            onButtonEvent(guid, button, pressed);
        }
    );
    
    m_core->setStickEventCallback(
        [this](const std::string& guid, const std::string& stick, float x, float y) {
            onStickEvent(guid, stick, x, y);
        }
    );
    
    m_core->setTriggerEventCallback(
        [this](const std::string& guid, const std::string& trigger, float value) {
            onTriggerEvent(guid, trigger, value);
        }
    );
    
    // Initialize the core
    if (!m_core->initialize()) {
        qWarning() << "Failed to initialize JoyCursorCore";
    }
    
    // Set up timer for polling
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CoreWorker::poll);
}

CoreWorker::~CoreWorker() {
    stop();
}

void CoreWorker::start() {
    if (m_timer && !m_timer->isActive()) {
        m_timer->start(5); // Poll every 5ms
        qDebug() << "CoreWorker started";
    }
}

void CoreWorker::stop() {
    if (m_timer && m_timer->isActive()) {
        m_timer->stop();
        qDebug() << "CoreWorker stopped";
    }
}

void CoreWorker::poll() {
    if (m_core) {
        m_core->pollEvents();
    }
}

void CoreWorker::onControllerConnected(const std::string& guid, const std::string& name) {
    QString qGuid = QString::fromStdString(guid);
    QString qName = QString::fromStdString(name);
    
    qDebug() << "Controller connected:" << qName << "(" << qGuid << ")";
    emit controllerConnected(qGuid, qName);
}

void CoreWorker::onControllerDisconnected(const std::string& guid) {
    QString qGuid = QString::fromStdString(guid);
    
    qDebug() << "Controller disconnected:" << qGuid;
    emit controllerDisconnected(qGuid);
}

void CoreWorker::onButtonEvent(const std::string& guid, const std::string& button, bool pressed) {
    QString qGuid = QString::fromStdString(guid);
    QString qButton = QString::fromStdString(button);
    
    if (pressed) {
        emit buttonPressed(qGuid, qButton);
    } else {
        emit buttonReleased(qGuid, qButton);
    }
}

void CoreWorker::onStickEvent(const std::string& guid, const std::string& stick, float x, float y) {
    QString qGuid = QString::fromStdString(guid);
    QString qStick = QString::fromStdString(stick);
    
    emit stickMoved(qGuid, qStick, x, y);
}

void CoreWorker::onTriggerEvent(const std::string& guid, const std::string& trigger, float value) {
    QString qGuid = QString::fromStdString(guid);
    QString qTrigger = QString::fromStdString(trigger);
    
    emit triggerMoved(qGuid, qTrigger, value);
} 