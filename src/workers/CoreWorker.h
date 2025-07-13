#pragma once

#include <QObject>
#include <QTimer>
#include <memory>
#include "../core/joycursor_core.h"

class CoreWorker : public QObject {
    Q_OBJECT

public:
    explicit CoreWorker(QObject* parent = nullptr);
    ~CoreWorker();

    // Core access
    JoyCursorCore* getCore() { return m_core.get(); }
    const JoyCursorCore* getCore() const { return m_core.get(); }

signals:
    void controllerConnected(const QString& guid, const QString& name);
    void controllerDisconnected(const QString& guid);
    void buttonPressed(const QString& guid, const QString& button);
    void buttonReleased(const QString& guid, const QString& button);
    void stickMoved(const QString& guid, const QString& stick, float x, float y);
    void triggerMoved(const QString& guid, const QString& trigger, float value);

public slots:
    void start();
    void stop();
    void poll();

private:
    std::unique_ptr<JoyCursorCore> m_core;
    QTimer* m_timer;
    
    // Core event handlers
    void onControllerConnected(const std::string& guid, const std::string& name);
    void onControllerDisconnected(const std::string& guid);
    void onButtonEvent(const std::string& guid, const std::string& button, bool pressed);
    void onStickEvent(const std::string& guid, const std::string& stick, float x, float y);
    void onTriggerEvent(const std::string& guid, const std::string& trigger, float value);
}; 