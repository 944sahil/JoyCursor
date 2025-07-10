#include "ControllerWorker.h"
#include <QThread>
#include <QDebug>

ControllerWorker::ControllerWorker(QObject* parent)
    : QObject(parent), m_manager(nullptr), m_timer(nullptr), m_controllerPresent(false) {
    m_manager = createControllerManager();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ControllerWorker::poll);
}

ControllerWorker::~ControllerWorker() {
    stop();
    delete m_manager;
}

void ControllerWorker::start() {
    m_timer->start(5); // Poll every 5ms
}

void ControllerWorker::stop() {
    if (m_timer->isActive())
        m_timer->stop();
}

void ControllerWorker::poll() {
    if (!m_manager) return;
    m_manager->pollEvents();
    bool controllerNow = m_manager->hasActiveController();
    std::string controllerNameStd = m_manager->getActiveControllerName();
    QString controllerName = QString::fromStdString(controllerNameStd);
    if (controllerNow && !m_controllerPresent) {
        m_controllerPresent = true;
        m_lastControllerName = controllerName;
        emit controllerConnected(controllerName);
    } else if (!controllerNow && m_controllerPresent) {
        m_controllerPresent = false;
        emit controllerDisconnected();
    }
} 