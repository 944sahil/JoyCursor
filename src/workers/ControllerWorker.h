#pragma once
#include <QObject>
#include <QString>
#include <QTimer>
#include "../core/controller_manager.h"
#include <string>

class ControllerWorker : public QObject {
    Q_OBJECT
public:
    explicit ControllerWorker(QObject* parent = nullptr);
    ~ControllerWorker();

signals:
    void controllerConnected(const QString& name);
    void controllerDisconnected();

public slots:
    void start();
    void stop();

private slots:
    void poll();

private:
    ControllerManager* m_manager;
    QTimer* m_timer;
    bool m_controllerPresent;
    QString m_lastControllerName;
}; 