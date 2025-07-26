#include "ControllerCustomizationWindow.h"
#include <QFont>
#include <QHeaderView>
#include <QFormLayout>
#include <QStackedWidget>
#include <QScrollArea>
#include <QDoubleSpinBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <fstream>
#include <nlohmann/json.hpp>
#include <QMessageBox>
#include <QDebug> // Added for debug output
#include "../utils/logging.h"

// Helper functions for mapping action_type string to UI index
int mouseActionIndexFromType(const std::string& action_type) {
    if (action_type == "mouse_left_click") return 0;
    if (action_type == "mouse_right_click") return 1;
    if (action_type == "mouse_middle_click") return 2;
    return 0;
}
int keyboardActionIndexFromType(const std::string& action_type) {
    if (action_type == "keyboard_enter") return 0;
    if (action_type == "keyboard_escape") return 1;
    if (action_type == "keyboard_tab") return 2;
    if (action_type == "keyboard_space") return 3;
    if (action_type == "keyboard_up") return 4;
    if (action_type == "keyboard_down") return 5;
    if (action_type == "keyboard_left") return 6;
    if (action_type == "keyboard_right") return 7;
    if (action_type == "keyboard_alt") return 8;
    if (action_type == "keyboard_ctrl") return 9;
    if (action_type == "keyboard_shift") return 10;
    if (action_type == "keyboard_f1") return 11;
    if (action_type == "keyboard_f2") return 12;
    if (action_type == "keyboard_f3") return 13;
    if (action_type == "keyboard_f4") return 14;
    if (action_type == "keyboard_f5") return 15;
    if (action_type == "keyboard_f6") return 16;
    if (action_type == "keyboard_f7") return 17;
    if (action_type == "keyboard_f8") return 18;
    if (action_type == "keyboard_f9") return 19;
    if (action_type == "keyboard_f10") return 20;
    if (action_type == "keyboard_f11") return 21;
    if (action_type == "keyboard_f12") return 22;
    return 0;
}

namespace {
// Button names and display labels
const QList<QString> buttonKeys = {
    "button_a", "button_b", "button_x", "button_y",
    "left_shoulder", "right_shoulder", "start", "back",
    /*"guide",*/
    "dpad_up", "dpad_down", "dpad_left", "dpad_right"
};
const QList<QString> buttonLabels = {
    "A", "B", "X", "Y",
    "L1", "R1", "Start", "Select",
    /*"Guide",*/
    "D-Up", "D-Down", "D-Left", "D-Right"
};
// Define mouse and keyboard actions
const QStringList mouseActions = {"Left Click", "Right Click", "Middle Click"};
const QStringList keyboardActions = {"Enter", "Escape", "Tab", "Space", "Up", "Down", "Left", "Right", "Alt", "Ctrl", "Shift", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"};
}

QMap<QString, ControllerCustomizationWindow*> ControllerCustomizationWindow::s_openWindows;

ControllerCustomizationWindow* ControllerCustomizationWindow::openForController(const QString& guid, const QString& name, bool connected, CoreWorker* coreWorker, QWidget* parent) {
    if (s_openWindows.contains(guid)) {
        ControllerCustomizationWindow* win = s_openWindows[guid];
        if (win) {
            win->setCoreWorker(coreWorker);
            win->raise();
            win->activateWindow();
            win->show();
            return win;
        }
    }
    ControllerCustomizationWindow* win = new ControllerCustomizationWindow(guid, name, connected, coreWorker, parent);
    win->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(win, &QWidget::destroyed, [guid]() { s_openWindows.remove(guid); });
    s_openWindows[guid] = win;
    win->show();
    win->raise();
    win->activateWindow();
    return win;
}

ControllerCustomizationWindow* ControllerCustomizationWindow::getOpenWindow(const QString& guid) {
    return s_openWindows.value(guid, nullptr);
}

ControllerCustomizationWindow::ControllerCustomizationWindow(QWidget* parent)
    : ControllerCustomizationWindow("", "Xbox Series Controller", true, nullptr, parent) {}

ControllerCustomizationWindow::ControllerCustomizationWindow(const QString& guid, const QString& name, bool connected, CoreWorker* coreWorker, QWidget* parent)
    : QWidget(parent), m_guid(guid), m_name(name), m_connected(connected), m_coreWorker(coreWorker) {
    setWindowTitle("Controller Customization");
    setFixedSize(940, 700); 
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(14);

    // --- Main content widget for scrolling ---
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(14);

    // Title and status
    titleLabel = new QLabel(name);
    QFont titleFont; titleFont.setPointSize(16); titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    scrollLayout->addWidget(titleLabel, 0, Qt::AlignLeft);
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLabel = new QLabel("Status:");
    QFont labelFont; labelFont.setPointSize(11);
    statusLabel->setFont(labelFont);
    statusValueLabel = new QLabel(connected ? "Connected" : "Not Connected");
    QFont statusValueFont; statusValueFont.setPointSize(11); statusValueFont.setBold(true);
    statusValueLabel->setFont(statusValueFont);
    statusValueLabel->setStyleSheet(connected ? "color: #21c521;" : "color: #555;");
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(statusValueLabel);
    statusLayout->addStretch();
    scrollLayout->addLayout(statusLayout);

    // --- Sticks Row (Left + Right) ---
    QHBoxLayout* sticksRow = new QHBoxLayout();
    sticksRow->setSpacing(18);

    // --- Left Stick ---
    leftStickGroup = new QGroupBox("Left Stick");
    QVBoxLayout* leftStickLayout = new QVBoxLayout(leftStickGroup);
    QHBoxLayout* leftStickTopRow = new QHBoxLayout();
    leftStickEnabled = new QCheckBox("Enabled");
    leftStickEnabled->setChecked(true);
    leftStickTopRow->addWidget(leftStickEnabled);
    leftStickTopRow->addSpacing(8);
    leftStickTopRow->addWidget(new QLabel("Action Type:"));
    leftStickActionType = new QComboBox();
    leftStickActionType->addItems({"Cursor", "Scroll"});
    leftStickTopRow->addWidget(leftStickActionType);
    leftStickTopRow->addSpacing(8);
    leftStickTopRow->addWidget(new QLabel("Deadzone:"));
    leftStickDeadzoneSpin = new QSpinBox();
    leftStickDeadzoneSpin->setMinimum(0); leftStickDeadzoneSpin->setMaximum(32767); leftStickDeadzoneSpin->setValue(8000);
    leftStickTopRow->addWidget(leftStickDeadzoneSpin);
    leftStickTopRow->addStretch();
    leftStickLayout->addLayout(leftStickTopRow);
    // Stacked advanced settings
    leftStickStack = new QStackedWidget();
    // Cursor params widget
    leftStickCursorWidget = new QWidget();
    QFormLayout* leftCursorForm = new QFormLayout(leftStickCursorWidget);
    int stickSliderWidth = 180;
    // Sensitivity (Cursor)
    leftStickCursorSensi = new QSlider(Qt::Horizontal); leftStickCursorSensi->setMinimum(1); leftStickCursorSensi->setMaximum(1000); leftStickCursorSensi->setValue(15); leftStickCursorSensi->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 0.15
    QDoubleSpinBox* leftStickCursorSensiSpin = new QDoubleSpinBox(); leftStickCursorSensiSpin->setRange(0.01, 10.0); leftStickCursorSensiSpin->setSingleStep(0.01); leftStickCursorSensiSpin->setValue(0.15);
    QObject::connect(leftStickCursorSensi, &QSlider::valueChanged, [leftStickCursorSensiSpin](int v){ leftStickCursorSensiSpin->setValue(v/100.0); });
    QObject::connect(leftStickCursorSensiSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ leftStickCursorSensi->setValue(static_cast<int>(v*100)); });
    QWidget* leftSensiRow = new QWidget(); QHBoxLayout* leftSensiLayout = new QHBoxLayout(leftSensiRow); leftSensiLayout->setContentsMargins(0,0,0,0); leftSensiLayout->addWidget(leftStickCursorSensi); leftSensiLayout->addWidget(leftStickCursorSensiSpin);
    leftCursorForm->addRow("Sensitivity:", leftSensiRow);
    // Boosted Sensitivity (Cursor)
    leftStickCursorBoosted = new QSlider(Qt::Horizontal); leftStickCursorBoosted->setMinimum(1); leftStickCursorBoosted->setMaximum(1000); leftStickCursorBoosted->setValue(60); leftStickCursorBoosted->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 0.6
    QDoubleSpinBox* leftStickCursorBoostedSpin = new QDoubleSpinBox(); leftStickCursorBoostedSpin->setRange(0.01, 10.0); leftStickCursorBoostedSpin->setSingleStep(0.01); leftStickCursorBoostedSpin->setValue(0.6);
    QObject::connect(leftStickCursorBoosted, &QSlider::valueChanged, [leftStickCursorBoostedSpin](int v){ leftStickCursorBoostedSpin->setValue(v/100.0); });
    QObject::connect(leftStickCursorBoostedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ leftStickCursorBoosted->setValue(static_cast<int>(v*100)); });
    QWidget* leftBoostedRow = new QWidget(); QHBoxLayout* leftBoostedLayout = new QHBoxLayout(leftBoostedRow); leftBoostedLayout->setContentsMargins(0,0,0,0); leftBoostedLayout->addWidget(leftStickCursorBoosted); leftBoostedLayout->addWidget(leftStickCursorBoostedSpin);
    leftCursorForm->addRow("Boosted Sensitivity:", leftBoostedRow);
    // Smoothing (Cursor)
    leftStickCursorSmoothing = new QSlider(Qt::Horizontal); leftStickCursorSmoothing->setMinimum(0); leftStickCursorSmoothing->setMaximum(100); leftStickCursorSmoothing->setValue(20); leftStickCursorSmoothing->setFixedWidth(stickSliderWidth); // 0.00-1.00, default 0.2
    QDoubleSpinBox* leftStickCursorSmoothingSpin = new QDoubleSpinBox(); leftStickCursorSmoothingSpin->setRange(0.00, 1.00); leftStickCursorSmoothingSpin->setSingleStep(0.01); leftStickCursorSmoothingSpin->setValue(0.2);
    QObject::connect(leftStickCursorSmoothing, &QSlider::valueChanged, [leftStickCursorSmoothingSpin](int v){ leftStickCursorSmoothingSpin->setValue(v/100.0); });
    QObject::connect(leftStickCursorSmoothingSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ leftStickCursorSmoothing->setValue(static_cast<int>(v*100)); });
    QWidget* leftSmoothingRow = new QWidget(); QHBoxLayout* leftSmoothingLayout = new QHBoxLayout(leftSmoothingRow); leftSmoothingLayout->setContentsMargins(0,0,0,0); leftSmoothingLayout->addWidget(leftStickCursorSmoothing); leftSmoothingLayout->addWidget(leftStickCursorSmoothingSpin);
    leftCursorForm->addRow("Smoothing:", leftSmoothingRow);
    // Scroll params widget
    leftStickScrollWidget = new QWidget();
    QFormLayout* leftScrollForm = new QFormLayout(leftStickScrollWidget);
    leftStickScrollVSensi = new QSlider(Qt::Horizontal); leftStickScrollVSensi->setMinimum(1); leftStickScrollVSensi->setMaximum(1000); leftStickScrollVSensi->setValue(100); leftStickScrollVSensi->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 1.0
    QDoubleSpinBox* leftStickScrollVSensiSpin = new QDoubleSpinBox(); leftStickScrollVSensiSpin->setRange(0.01, 10.0); leftStickScrollVSensiSpin->setSingleStep(0.01); leftStickScrollVSensiSpin->setValue(1.0);
    QObject::connect(leftStickScrollVSensi, &QSlider::valueChanged, [leftStickScrollVSensiSpin](int v){ leftStickScrollVSensiSpin->setValue(v/100.0); });
    QObject::connect(leftStickScrollVSensiSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ leftStickScrollVSensi->setValue(static_cast<int>(v*100)); });
    QWidget* leftVSensiRow = new QWidget(); QHBoxLayout* leftVSensiLayout = new QHBoxLayout(leftVSensiRow); leftVSensiLayout->setContentsMargins(0,0,0,0); leftVSensiLayout->addWidget(leftStickScrollVSensi); leftVSensiLayout->addWidget(leftStickScrollVSensiSpin);
    leftScrollForm->addRow("Vertical Sensitivity:", leftVSensiRow);
    leftStickScrollHSensi = new QSlider(Qt::Horizontal); leftStickScrollHSensi->setMinimum(1); leftStickScrollHSensi->setMaximum(1000); leftStickScrollHSensi->setValue(50); leftStickScrollHSensi->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 0.5
    QDoubleSpinBox* leftStickScrollHSensiSpin = new QDoubleSpinBox(); leftStickScrollHSensiSpin->setRange(0.01, 10.0); leftStickScrollHSensiSpin->setSingleStep(0.01); leftStickScrollHSensiSpin->setValue(0.5);
    QObject::connect(leftStickScrollHSensi, &QSlider::valueChanged, [leftStickScrollHSensiSpin](int v){ leftStickScrollHSensiSpin->setValue(v/100.0); });
    QObject::connect(leftStickScrollHSensiSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ leftStickScrollHSensi->setValue(static_cast<int>(v*100)); });
    QWidget* leftHSensiRow = new QWidget(); QHBoxLayout* leftHSensiLayout = new QHBoxLayout(leftHSensiRow); leftHSensiLayout->setContentsMargins(0,0,0,0); leftHSensiLayout->addWidget(leftStickScrollHSensi); leftHSensiLayout->addWidget(leftStickScrollHSensiSpin);
    leftScrollForm->addRow("Horizontal Sensitivity:", leftHSensiRow);
    leftStickScrollVMax = new QSlider(Qt::Horizontal); leftStickScrollVMax->setMinimum(1); leftStickScrollVMax->setMaximum(50); leftStickScrollVMax->setValue(20); leftStickScrollVMax->setFixedWidth(stickSliderWidth); // 1-50, default 20
    QDoubleSpinBox* leftStickScrollVMaxSpin = new QDoubleSpinBox(); leftStickScrollVMaxSpin->setRange(1.0, 50.0); leftStickScrollVMaxSpin->setSingleStep(1.0); leftStickScrollVMaxSpin->setValue(20.0);
    QObject::connect(leftStickScrollVMax, &QSlider::valueChanged, [leftStickScrollVMaxSpin](int v){ leftStickScrollVMaxSpin->setValue(v); });
    QObject::connect(leftStickScrollVMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ leftStickScrollVMax->setValue(static_cast<int>(v)); });
    QWidget* leftVMaxRow = new QWidget(); QHBoxLayout* leftVMaxLayout = new QHBoxLayout(leftVMaxRow); leftVMaxLayout->setContentsMargins(0,0,0,0); leftVMaxLayout->addWidget(leftStickScrollVMax); leftVMaxLayout->addWidget(leftStickScrollVMaxSpin);
    leftScrollForm->addRow("Vertical Max Speed:", leftVMaxRow);
    leftStickScrollHMax = new QSlider(Qt::Horizontal); leftStickScrollHMax->setMinimum(1); leftStickScrollHMax->setMaximum(50); leftStickScrollHMax->setValue(10); leftStickScrollHMax->setFixedWidth(stickSliderWidth); // 1-50, default 10
    QDoubleSpinBox* leftStickScrollHMaxSpin = new QDoubleSpinBox(); leftStickScrollHMaxSpin->setRange(1.0, 50.0); leftStickScrollHMaxSpin->setSingleStep(1.0); leftStickScrollHMaxSpin->setValue(10.0);
    QObject::connect(leftStickScrollHMax, &QSlider::valueChanged, [leftStickScrollHMaxSpin](int v){ leftStickScrollHMaxSpin->setValue(v); });
    QObject::connect(leftStickScrollHMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ leftStickScrollHMax->setValue(static_cast<int>(v)); });
    QWidget* leftHMaxRow = new QWidget(); QHBoxLayout* leftHMaxLayout = new QHBoxLayout(leftHMaxRow); leftHMaxLayout->setContentsMargins(0,0,0,0); leftHMaxLayout->addWidget(leftStickScrollHMax); leftHMaxLayout->addWidget(leftStickScrollHMaxSpin);
    leftScrollForm->addRow("Horizontal Max Speed:", leftHMaxRow);
    // Add to stacked widget
    leftStickStack->addWidget(new QWidget()); // index 0: none (empty)
    leftStickStack->addWidget(leftStickCursorWidget); // index 1: cursor
    leftStickStack->addWidget(leftStickScrollWidget); // index 2: scroll
    leftStickLayout->addWidget(leftStickStack);
    sticksRow->addWidget(leftStickGroup, 1);

    // --- Right Stick ---
    rightStickGroup = new QGroupBox("Right Stick");
    QVBoxLayout* rightStickLayout = new QVBoxLayout(rightStickGroup);
    QHBoxLayout* rightStickTopRow = new QHBoxLayout();
    rightStickEnabled = new QCheckBox("Enabled");
    rightStickEnabled->setChecked(true);
    rightStickTopRow->addWidget(rightStickEnabled);
    rightStickTopRow->addSpacing(8);
    rightStickTopRow->addWidget(new QLabel("Action Type:"));
    rightStickActionType = new QComboBox();
    rightStickActionType->addItems({"Cursor", "Scroll"});
    rightStickTopRow->addWidget(rightStickActionType);
    rightStickTopRow->addSpacing(8);
    rightStickTopRow->addWidget(new QLabel("Deadzone:"));
    rightStickDeadzoneSpin = new QSpinBox();
    rightStickDeadzoneSpin->setMinimum(0); rightStickDeadzoneSpin->setMaximum(32767); rightStickDeadzoneSpin->setValue(8000);
    rightStickTopRow->addWidget(rightStickDeadzoneSpin);
    rightStickTopRow->addStretch();
    rightStickLayout->addLayout(rightStickTopRow);
    // Stacked advanced settings
    rightStickStack = new QStackedWidget();
    // Cursor params widget
    rightStickCursorWidget = new QWidget();
    QFormLayout* rightCursorForm = new QFormLayout(rightStickCursorWidget);
    // Sensitivity (Cursor)
    rightStickCursorSensi = new QSlider(Qt::Horizontal); rightStickCursorSensi->setMinimum(1); rightStickCursorSensi->setMaximum(1000); rightStickCursorSensi->setValue(40); rightStickCursorSensi->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 0.4
    QDoubleSpinBox* rightStickCursorSensiSpin = new QDoubleSpinBox(); rightStickCursorSensiSpin->setRange(0.01, 10.0); rightStickCursorSensiSpin->setSingleStep(0.01); rightStickCursorSensiSpin->setValue(0.4);
    QObject::connect(rightStickCursorSensi, &QSlider::valueChanged, [rightStickCursorSensiSpin](int v){ rightStickCursorSensiSpin->setValue(v/100.0); });
    QObject::connect(rightStickCursorSensiSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ rightStickCursorSensi->setValue(static_cast<int>(v*100)); });
    QWidget* rightSensiRow = new QWidget(); QHBoxLayout* rightSensiLayout = new QHBoxLayout(rightSensiRow); rightSensiLayout->setContentsMargins(0,0,0,0); rightSensiLayout->addWidget(rightStickCursorSensi); rightSensiLayout->addWidget(rightStickCursorSensiSpin);
    rightCursorForm->addRow("Sensitivity:", rightSensiRow);
    // Boosted Sensitivity (Cursor)
    rightStickCursorBoosted = new QSlider(Qt::Horizontal); rightStickCursorBoosted->setMinimum(1); rightStickCursorBoosted->setMaximum(1000); rightStickCursorBoosted->setValue(60); rightStickCursorBoosted->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 0.8
    QDoubleSpinBox* rightStickCursorBoostedSpin = new QDoubleSpinBox(); rightStickCursorBoostedSpin->setRange(0.01, 10.0); rightStickCursorBoostedSpin->setSingleStep(0.01); rightStickCursorBoostedSpin->setValue(0.8);
    QObject::connect(rightStickCursorBoosted, &QSlider::valueChanged, [rightStickCursorBoostedSpin](int v){ rightStickCursorBoostedSpin->setValue(v/100.0); });
    QObject::connect(rightStickCursorBoostedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ rightStickCursorBoosted->setValue(static_cast<int>(v*100)); });
    QWidget* rightBoostedRow = new QWidget(); QHBoxLayout* rightBoostedLayout = new QHBoxLayout(rightBoostedRow); rightBoostedLayout->setContentsMargins(0,0,0,0); rightBoostedLayout->addWidget(rightStickCursorBoosted); rightBoostedLayout->addWidget(rightStickCursorBoostedSpin);
    rightCursorForm->addRow("Boosted Sensitivity:", rightBoostedRow);
    // Smoothing (Cursor)
    rightStickCursorSmoothing = new QSlider(Qt::Horizontal); rightStickCursorSmoothing->setMinimum(0); rightStickCursorSmoothing->setMaximum(100); rightStickCursorSmoothing->setValue(20); rightStickCursorSmoothing->setFixedWidth(stickSliderWidth); // 0.00-1.00, default 0.2
    QDoubleSpinBox* rightStickCursorSmoothingSpin = new QDoubleSpinBox(); rightStickCursorSmoothingSpin->setRange(0.00, 1.00); rightStickCursorSmoothingSpin->setSingleStep(0.01); rightStickCursorSmoothingSpin->setValue(0.2);
    QObject::connect(rightStickCursorSmoothing, &QSlider::valueChanged, [rightStickCursorSmoothingSpin](int v){ rightStickCursorSmoothingSpin->setValue(v/100.0); });
    QObject::connect(rightStickCursorSmoothingSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ rightStickCursorSmoothing->setValue(static_cast<int>(v*100)); });
    QWidget* rightSmoothingRow = new QWidget(); QHBoxLayout* rightSmoothingLayout = new QHBoxLayout(rightSmoothingRow); rightSmoothingLayout->setContentsMargins(0,0,0,0); rightSmoothingLayout->addWidget(rightStickCursorSmoothing); rightSmoothingLayout->addWidget(rightStickCursorSmoothingSpin);
    rightCursorForm->addRow("Smoothing:", rightSmoothingRow);
    // Scroll params widget
    rightStickScrollWidget = new QWidget();
    QFormLayout* rightScrollForm = new QFormLayout(rightStickScrollWidget);
    // Vertical Sensitivity
    rightStickScrollVSensi = new QSlider(Qt::Horizontal); rightStickScrollVSensi->setMinimum(1); rightStickScrollVSensi->setMaximum(1000); rightStickScrollVSensi->setValue(100); leftStickScrollVSensi->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 1.0
    QDoubleSpinBox* rightStickScrollVSensiSpin = new QDoubleSpinBox(); rightStickScrollVSensiSpin->setRange(0.01, 10.0); rightStickScrollVSensiSpin->setSingleStep(0.01); rightStickScrollVSensiSpin->setValue(1.0);
    QObject::connect(rightStickScrollVSensi, &QSlider::valueChanged, [rightStickScrollVSensiSpin](int v){ rightStickScrollVSensiSpin->setValue(v/100.0); });
    QObject::connect(rightStickScrollVSensiSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ rightStickScrollVSensi->setValue(static_cast<int>(v*100)); });
    QWidget* rightVSensiRow = new QWidget(); QHBoxLayout* rightVSensiLayout = new QHBoxLayout(rightVSensiRow); rightVSensiLayout->setContentsMargins(0,0,0,0); rightVSensiLayout->addWidget(rightStickScrollVSensi); rightVSensiLayout->addWidget(rightStickScrollVSensiSpin);
    rightScrollForm->addRow("Vertical Sensitivity:", rightVSensiRow);
    // Horizontal Sensitivity
    rightStickScrollHSensi = new QSlider(Qt::Horizontal); rightStickScrollHSensi->setMinimum(1); rightStickScrollHSensi->setMaximum(1000); rightStickScrollHSensi->setValue(50); leftStickScrollHSensi->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 0.5
    QDoubleSpinBox* rightStickScrollHSensiSpin = new QDoubleSpinBox(); rightStickScrollHSensiSpin->setRange(0.01, 10.0); rightStickScrollHSensiSpin->setSingleStep(0.01); rightStickScrollHSensiSpin->setValue(0.5);
    QObject::connect(rightStickScrollHSensi, &QSlider::valueChanged, [rightStickScrollHSensiSpin](int v){ rightStickScrollHSensiSpin->setValue(v/100.0); });
    QObject::connect(rightStickScrollHSensiSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ rightStickScrollHSensi->setValue(static_cast<int>(v*100)); });
    QWidget* rightHSensiRow = new QWidget(); QHBoxLayout* rightHSensiLayout = new QHBoxLayout(rightHSensiRow); rightHSensiLayout->setContentsMargins(0,0,0,0); rightHSensiLayout->addWidget(rightStickScrollHSensi); rightHSensiLayout->addWidget(rightStickScrollHSensiSpin);
    rightScrollForm->addRow("Horizontal Sensitivity:", rightHSensiRow);
    // Vertical Max Speed
    rightStickScrollVMax = new QSlider(Qt::Horizontal); rightStickScrollVMax->setMinimum(1); rightStickScrollVMax->setMaximum(50); rightStickScrollVMax->setValue(20); leftStickScrollVMax->setFixedWidth(stickSliderWidth); // 1-50, default 20
    QDoubleSpinBox* rightStickScrollVMaxSpin = new QDoubleSpinBox(); rightStickScrollVMaxSpin->setRange(1.0, 50.0); rightStickScrollVMaxSpin->setSingleStep(1.0); rightStickScrollVMaxSpin->setValue(20.0);
    QObject::connect(rightStickScrollVMax, &QSlider::valueChanged, [rightStickScrollVMaxSpin](int v){ rightStickScrollVMaxSpin->setValue(v); });
    QObject::connect(rightStickScrollVMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ rightStickScrollVMax->setValue(static_cast<int>(v)); });
    QWidget* rightVMaxRow = new QWidget(); QHBoxLayout* rightVMaxLayout = new QHBoxLayout(rightVMaxRow); rightVMaxLayout->setContentsMargins(0,0,0,0); rightVMaxLayout->addWidget(rightStickScrollVMax); rightVMaxLayout->addWidget(rightStickScrollVMaxSpin);
    rightScrollForm->addRow("Vertical Max Speed:", rightVMaxRow);
    // Horizontal Max Speed
    rightStickScrollHMax = new QSlider(Qt::Horizontal); rightStickScrollHMax->setMinimum(1); rightStickScrollHMax->setMaximum(50); rightStickScrollHMax->setValue(10); leftStickScrollHMax->setFixedWidth(stickSliderWidth); // 1-50, default 10
    QDoubleSpinBox* rightStickScrollHMaxSpin = new QDoubleSpinBox(); rightStickScrollHMaxSpin->setRange(1.0, 50.0); rightStickScrollHMaxSpin->setSingleStep(1.0); rightStickScrollHMaxSpin->setValue(10.0);
    QObject::connect(rightStickScrollHMax, &QSlider::valueChanged, [rightStickScrollHMaxSpin](int v){ rightStickScrollHMaxSpin->setValue(v); });
    QObject::connect(rightStickScrollHMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ rightStickScrollHMax->setValue(static_cast<int>(v)); });
    QWidget* rightHMaxRow = new QWidget(); QHBoxLayout* rightHMaxLayout = new QHBoxLayout(rightHMaxRow); rightHMaxLayout->setContentsMargins(0,0,0,0); rightHMaxLayout->addWidget(rightStickScrollHMax); rightHMaxLayout->addWidget(rightStickScrollHMaxSpin);
    rightScrollForm->addRow("Horizontal Max Speed:", rightHMaxRow);
    // Add to stacked widget
    rightStickStack->addWidget(new QWidget()); // index 0: none (empty)
    rightStickStack->addWidget(rightStickCursorWidget); // index 1: cursor
    rightStickStack->addWidget(rightStickScrollWidget); // index 2: scroll
    rightStickLayout->addWidget(rightStickStack);
    sticksRow->addWidget(rightStickGroup, 1);

    scrollLayout->addLayout(sticksRow);

    // Connect action type changes to stacked widget
    connect(leftStickActionType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        // 0: Cursor, 1: Scroll
        leftStickStack->setCurrentIndex(idx + 1); // 1 for Cursor, 2 for Scroll
    });
    connect(rightStickActionType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        rightStickStack->setCurrentIndex(idx + 1);
    });
    // Set initial state
    leftStickStack->setCurrentIndex(1); // Cursor by default
    rightStickStack->setCurrentIndex(1);

    // --- Buttons Group/Table Construction ---
    buttonsGroup = new QGroupBox("Buttons");
    QVBoxLayout* buttonsLayout = new QVBoxLayout(buttonsGroup);
    // Single table for all buttons
    QTableWidget* buttonTable = new QTableWidget(buttonKeys.size(), 4, this);
    buttonTable->setHorizontalHeaderLabels({"Button", "Enabled", "Action Type", "Action"});
    buttonTable->verticalHeader()->setVisible(false);
    buttonTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    buttonTable->setShowGrid(false);
    buttonTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    buttonTable->setSelectionMode(QAbstractItemView::NoSelection);
    buttonTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    buttonTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    int totalHeight = buttonTable->horizontalHeader()->height();
    buttonRowToName.clear();
    for (int i = 0; i < buttonKeys.size(); ++i) {
        QString btnName = buttonKeys[i];
        QString label = buttonLabels[i];
        if (label == "Select") btnName = "back";
        buttonRowToName[i] = btnName;
        QLabel* btnLabel = new QLabel(label);
        btnLabel->setAlignment(Qt::AlignCenter);
        buttonTable->setCellWidget(i, 0, btnLabel);
        QCheckBox* enabledBox = new QCheckBox(); enabledBox->setChecked(true);
        buttonTable->setCellWidget(i, 1, enabledBox);
        QComboBox* actionTypeBox = new QComboBox(); actionTypeBox->addItems({"None", "Mouse", "Keyboard"});
        buttonTable->setCellWidget(i, 2, actionTypeBox);
        QComboBox* actionBox = new QComboBox();
        actionBox->addItem("None");
        buttonTable->setCellWidget(i, 3, actionBox);
        QObject::connect(actionTypeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [actionBox](int idx) {
            actionBox->clear();
            if (idx == 0) { actionBox->addItem("None"); }
            else if (idx == 1) { actionBox->addItems(mouseActions); }
            else if (idx == 2) { actionBox->addItems(keyboardActions); }
        });
        buttonEnabledMap[i] = enabledBox;
        buttonActionTypeMap[i] = actionTypeBox;
        buttonKeyMap[i] = actionBox;
        totalHeight += buttonTable->rowHeight(i);
    }
    buttonTable->setMinimumHeight(totalHeight);
    buttonTable->setMaximumHeight(totalHeight);
    buttonsLayout->addWidget(buttonTable);
    scrollLayout->addWidget(buttonsGroup);

    // --- Triggers Row (Left + Right) ---
    QHBoxLayout* triggersRow = new QHBoxLayout();
    triggersRow->setSpacing(18);

    // --- Left Trigger ---
    leftTriggerGroup = new QGroupBox("Left Trigger");
    QVBoxLayout* leftTrigLayout = new QVBoxLayout(leftTriggerGroup);
    leftTriggerEnabled = new QCheckBox("Enabled"); leftTriggerEnabled->setChecked(true);
    leftTrigLayout->addWidget(leftTriggerEnabled);
    QHBoxLayout* leftTrigTypeLayout = new QHBoxLayout();
    leftTrigTypeLayout->addWidget(new QLabel("Action Type:"));
    leftTriggerActionType = new QComboBox(); leftTriggerActionType->addItems({"Scroll", "Button"});
    leftTrigTypeLayout->addWidget(leftTriggerActionType);
    leftTrigTypeLayout->addStretch();
    leftTrigLayout->addLayout(leftTrigTypeLayout);
    QHBoxLayout* leftTrigThreshLayout = new QHBoxLayout();
    leftTrigThreshLayout->addWidget(new QLabel("Threshold:"));
    leftTriggerThreshold = nullptr; // Remove QSlider
    leftTriggerThresholdSpin = new QSpinBox();
    leftTriggerThresholdSpin->setMinimum(0); leftTriggerThresholdSpin->setMaximum(32767); leftTriggerThresholdSpin->setValue(8000);
    leftTrigThreshLayout->addWidget(leftTriggerThresholdSpin);
    leftTrigLayout->addLayout(leftTrigThreshLayout);
    // Scroll params
    leftTriggerScrollWidget = new QWidget();
    QFormLayout* leftTrigScrollForm = new QFormLayout(leftTriggerScrollWidget);
    leftTriggerScrollDir = new QComboBox(); leftTriggerScrollDir->addItems({"Up", "Down"});
    leftTriggerScrollVSensi = new QSlider(Qt::Horizontal); leftTriggerScrollVSensi->setMinimum(1); leftTriggerScrollVSensi->setMaximum(1000); leftTriggerScrollVSensi->setValue(10); leftTriggerScrollVSensi->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 1.0
    QDoubleSpinBox* leftTriggerScrollVSensiSpin = new QDoubleSpinBox(); leftTriggerScrollVSensiSpin->setRange(0.01, 10.0); leftTriggerScrollVSensiSpin->setSingleStep(0.01); leftTriggerScrollVSensiSpin->setValue(1.0);
    QObject::connect(leftTriggerScrollVSensi, &QSlider::valueChanged, [leftTriggerScrollVSensiSpin](int v){ leftTriggerScrollVSensiSpin->setValue(v/100.0); });
    QObject::connect(leftTriggerScrollVSensiSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ leftTriggerScrollVSensi->setValue(static_cast<int>(v*100)); });
    QWidget* leftTrigVSensiRow = new QWidget(); QHBoxLayout* leftTrigVSensiLayout = new QHBoxLayout(leftTrigVSensiRow); leftTrigVSensiLayout->setContentsMargins(0,0,0,0); leftTrigVSensiLayout->addWidget(leftTriggerScrollVSensi); leftTrigVSensiLayout->addWidget(leftTriggerScrollVSensiSpin);
    leftTrigScrollForm->addRow("Vertical Sensitivity:", leftTrigVSensiRow);
    leftTriggerScrollVMax = new QSlider(Qt::Horizontal); leftTriggerScrollVMax->setMinimum(1); leftTriggerScrollVMax->setMaximum(1000); leftTriggerScrollVMax->setValue(40); leftTriggerScrollVMax->setFixedWidth(stickSliderWidth); // 1-100, default 40
    QDoubleSpinBox* leftTriggerScrollVMaxSpin = new QDoubleSpinBox(); leftTriggerScrollVMaxSpin->setRange(1.0, 100.0); leftTriggerScrollVMaxSpin->setSingleStep(1.0); leftTriggerScrollVMaxSpin->setValue(40.0);
    QObject::connect(leftTriggerScrollVMax, &QSlider::valueChanged, [leftTriggerScrollVMaxSpin](int v){ leftTriggerScrollVMaxSpin->setValue(v); });
    QObject::connect(leftTriggerScrollVMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ leftTriggerScrollVMax->setValue(static_cast<int>(v)); });
    QWidget* leftTrigVMaxRow = new QWidget(); QHBoxLayout* leftTrigVMaxLayout = new QHBoxLayout(leftTrigVMaxRow); leftTrigVMaxLayout->setContentsMargins(0,0,0,0); leftTrigVMaxLayout->addWidget(leftTriggerScrollVMax); leftTrigVMaxLayout->addWidget(leftTriggerScrollVMaxSpin);
    leftTrigScrollForm->addRow("Vertical Max Speed:", leftTrigVMaxRow);
    leftTrigLayout->addWidget(leftTriggerScrollWidget);
    // Button params
    leftTriggerButtonWidget = new QWidget();
    QFormLayout* leftTrigBtnForm = new QFormLayout(leftTriggerButtonWidget);
    leftTriggerButtonAction = new QComboBox(); leftTriggerButtonAction->addItems(mouseActions + keyboardActions);
    leftTrigBtnForm->addRow("Button Action:", leftTriggerButtonAction);
    leftTrigLayout->addWidget(leftTriggerButtonWidget);
    triggersRow->addWidget(leftTriggerGroup, 1);

    // --- Right Trigger ---
    rightTriggerGroup = new QGroupBox("Right Trigger");
    QVBoxLayout* rightTrigLayout = new QVBoxLayout(rightTriggerGroup);
    rightTriggerEnabled = new QCheckBox("Enabled"); rightTriggerEnabled->setChecked(true);
    rightTrigLayout->addWidget(rightTriggerEnabled);
    QHBoxLayout* rightTrigTypeLayout = new QHBoxLayout();
    rightTrigTypeLayout->addWidget(new QLabel("Action Type:"));
    rightTriggerActionType = new QComboBox(); rightTriggerActionType->addItems({"Scroll", "Button"});
    rightTrigTypeLayout->addWidget(rightTriggerActionType);
    rightTrigTypeLayout->addStretch();
    rightTrigLayout->addLayout(rightTrigTypeLayout);
    QHBoxLayout* rightTrigThreshLayout = new QHBoxLayout();
    rightTrigThreshLayout->addWidget(new QLabel("Threshold:"));
    rightTriggerThreshold = nullptr; // Remove QSlider
    rightTriggerThresholdSpin = new QSpinBox();
    rightTriggerThresholdSpin->setMinimum(0); rightTriggerThresholdSpin->setMaximum(32767); rightTriggerThresholdSpin->setValue(8000);
    rightTrigThreshLayout->addWidget(rightTriggerThresholdSpin);
    rightTrigLayout->addLayout(rightTrigThreshLayout);
    // Scroll params
    rightTriggerScrollWidget = new QWidget();
    QFormLayout* rightTrigScrollForm = new QFormLayout(rightTriggerScrollWidget);
    rightTriggerScrollDir = new QComboBox(); rightTriggerScrollDir->addItems({"Up", "Down"});
    rightTriggerScrollVSensi = new QSlider(Qt::Horizontal); rightTriggerScrollVSensi->setMinimum(1); rightTriggerScrollVSensi->setMaximum(1000); rightTriggerScrollVSensi->setValue(10); rightTriggerScrollVSensi->setFixedWidth(stickSliderWidth); // 0.01-10.0, default 1.0
    QDoubleSpinBox* rightTriggerScrollVSensiSpin = new QDoubleSpinBox(); rightTriggerScrollVSensiSpin->setRange(0.01, 10.0); rightTriggerScrollVSensiSpin->setSingleStep(0.01); rightTriggerScrollVSensiSpin->setValue(1.0);
    QObject::connect(rightTriggerScrollVSensi, &QSlider::valueChanged, [rightTriggerScrollVSensiSpin](int v){ rightTriggerScrollVSensiSpin->setValue(v/100.0); });
    QObject::connect(rightTriggerScrollVSensiSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ rightTriggerScrollVSensi->setValue(static_cast<int>(v*100)); });
    QWidget* rightTrigVSensiRow = new QWidget(); QHBoxLayout* rightTrigVSensiLayout = new QHBoxLayout(rightTrigVSensiRow); rightTrigVSensiLayout->setContentsMargins(0,0,0,0); rightTrigVSensiLayout->addWidget(rightTriggerScrollVSensi); rightTrigVSensiLayout->addWidget(rightTriggerScrollVSensiSpin);
    rightTrigScrollForm->addRow("Vertical Sensitivity:", rightTrigVSensiRow);
    rightTriggerScrollVMax = new QSlider(Qt::Horizontal); rightTriggerScrollVMax->setMinimum(1); rightTriggerScrollVMax->setMaximum(1000); rightTriggerScrollVMax->setValue(40); rightTriggerScrollVMax->setFixedWidth(stickSliderWidth); // 1-100, default 40
    QDoubleSpinBox* rightTriggerScrollVMaxSpin = new QDoubleSpinBox(); rightTriggerScrollVMaxSpin->setRange(1.0, 100.0); rightTriggerScrollVMaxSpin->setSingleStep(1.0); rightTriggerScrollVMaxSpin->setValue(40.0);
    QObject::connect(rightTriggerScrollVMax, &QSlider::valueChanged, [rightTriggerScrollVMaxSpin](int v){ rightTriggerScrollVMaxSpin->setValue(v); });
    QObject::connect(rightTriggerScrollVMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ rightTriggerScrollVMax->setValue(static_cast<int>(v)); });
    QWidget* rightTrigVMaxRow = new QWidget(); QHBoxLayout* rightTrigVMaxLayout = new QHBoxLayout(rightTrigVMaxRow); rightTrigVMaxLayout->setContentsMargins(0,0,0,0); rightTrigVMaxLayout->addWidget(rightTriggerScrollVMax); rightTrigVMaxLayout->addWidget(rightTriggerScrollVMaxSpin);
    rightTrigScrollForm->addRow("Vertical Max Speed:", rightTrigVMaxRow);
    rightTrigLayout->addWidget(rightTriggerScrollWidget);
    // Button params
    rightTriggerButtonWidget = new QWidget();
    QFormLayout* rightTrigBtnForm = new QFormLayout(rightTriggerButtonWidget);
    rightTriggerButtonAction = new QComboBox(); rightTriggerButtonAction->addItems(mouseActions + keyboardActions);
    rightTrigBtnForm->addRow("Button Action:", rightTriggerButtonAction);
    rightTrigLayout->addWidget(rightTriggerButtonWidget);
    triggersRow->addWidget(rightTriggerGroup, 1);

    scrollLayout->addLayout(triggersRow);

    // --- Scroll area for all content ---
    QScrollArea* mainScrollArea = new QScrollArea();
    mainScrollArea->setWidgetResizable(true);
    mainScrollArea->setFrameShape(QFrame::NoFrame);
    mainScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mainScrollArea->setWidget(scrollContent);
    mainLayout->addWidget(mainScrollArea);

    // --- Buttons ---
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    resetButton = new QPushButton("Reset to Default");
    okButton = new QPushButton("OK");
    okButton->setDefault(true);
    okButton->setStyleSheet("background: #1565c0; color: white; font-weight: bold; padding: 4px 18px; border-radius: 6px;");
    btnLayout->addWidget(resetButton);
    btnLayout->addWidget(okButton);
    mainLayout->addLayout(btnLayout);

    connect(resetButton, &QPushButton::clicked, this, &ControllerCustomizationWindow::resetToDefault);
    connect(okButton, &QPushButton::clicked, this, &ControllerCustomizationWindow::saveMappingsToCore);

    // Load current mappings from core
    loadMappingsFromCore();

    // Connect to coreWorker signals for live status updates
    if (m_coreWorker) {
        connect(m_coreWorker, &CoreWorker::controllerConnected, this, [this](const QString& guid, const QString&) {
            if (guid == m_guid) setControllerInfo(m_guid, m_name, true);
        });
        connect(m_coreWorker, &CoreWorker::controllerDisconnected, this, [this](const QString& guid) {
            if (guid == m_guid) setControllerInfo(m_guid, m_name, false);
        });
    }
}

void ControllerCustomizationWindow::setControllerInfo(const QString& guid, const QString& name, bool connected) {
    m_guid = guid;
    m_name = name;
    m_connected = connected;
    if (titleLabel) titleLabel->setText(name);
    if (statusValueLabel) {
        statusValueLabel->setText(connected ? "Connected" : "Not Connected");
        statusValueLabel->setStyleSheet(connected ? "color: #21c521;" : "color: #555;");
    }
} 

void ControllerCustomizationWindow::setCoreWorker(CoreWorker* coreWorker) {
    if (m_coreWorker == coreWorker || !coreWorker)
        return;
    m_coreWorker = coreWorker;
    // Disconnect any previous connections
    this->disconnect(m_coreWorker);
    // Connect to coreWorker signals for live status updates
    connect(m_coreWorker, &CoreWorker::controllerConnected, this, [this](const QString& guid, const QString&) {
        if (guid == m_guid) setControllerInfo(m_guid, m_name, true);
    });
    connect(m_coreWorker, &CoreWorker::controllerDisconnected, this, [this](const QString& guid) {
        if (guid == m_guid) setControllerInfo(m_guid, m_name, false);
    });
} 

void ControllerCustomizationWindow::resetToDefault() {
    logInfo("Resetting UI to default settings");
    
    // --- Reset Left Stick UI to Default ---
    leftStickEnabled->setChecked(true);
    leftStickActionType->setCurrentIndex(0); // Cursor
    leftStickDeadzoneSpin->setValue(8000);
    leftStickCursorSensi->setValue(15); // 0.15 * 100
    leftStickCursorBoosted->setValue(60); // 0.6 * 100
    leftStickCursorSmoothing->setValue(20); // 0.2 * 100
    leftStickScrollVSensi->setValue(100); // 1.0 * 100
    leftStickScrollHSensi->setValue(50); // 0.5 * 100
    leftStickScrollVMax->setValue(20);
    leftStickScrollHMax->setValue(10);
    
    // --- Reset Right Stick UI to Default ---
    rightStickEnabled->setChecked(true);
    rightStickActionType->setCurrentIndex(1); // Scroll
    rightStickDeadzoneSpin->setValue(8000);
    rightStickCursorSensi->setValue(40); // 0.4 * 100
    rightStickCursorBoosted->setValue(80); // 0.8 * 100
    rightStickCursorSmoothing->setValue(20); // 0.2 * 100
    rightStickScrollVSensi->setValue(100); // 1.0 * 100
    rightStickScrollHSensi->setValue(50); // 0.5 * 100
    rightStickScrollVMax->setValue(20);
    rightStickScrollHMax->setValue(10);
    
    // --- Reset Buttons UI to Default ---
    // A button - Left Click
    if (buttonEnabledMap.contains(0)) buttonEnabledMap[0]->setChecked(true);
    if (buttonActionTypeMap.contains(0)) buttonActionTypeMap[0]->setCurrentIndex(1); // Mouse
    if (buttonKeyMap.contains(0)) {
        buttonKeyMap[0]->clear();
        buttonKeyMap[0]->addItems(mouseActions);
        buttonKeyMap[0]->setCurrentIndex(0); // Left Click
    }
    
    // B button - Escape
    if (buttonEnabledMap.contains(1)) buttonEnabledMap[1]->setChecked(true);
    if (buttonActionTypeMap.contains(1)) buttonActionTypeMap[1]->setCurrentIndex(2); // Keyboard
    if (buttonKeyMap.contains(1)) {
        buttonKeyMap[1]->clear();
        buttonKeyMap[1]->addItems(keyboardActions);
        buttonKeyMap[1]->setCurrentIndex(1); // Escape
    }
    
    // X button - Enter
    if (buttonEnabledMap.contains(2)) buttonEnabledMap[2]->setChecked(true);
    if (buttonActionTypeMap.contains(2)) buttonActionTypeMap[2]->setCurrentIndex(2); // Keyboard
    if (buttonKeyMap.contains(2)) {
        buttonKeyMap[2]->clear();
        buttonKeyMap[2]->addItems(keyboardActions);
        buttonKeyMap[2]->setCurrentIndex(0); // Enter
    }
    
    // Y button - Disabled
    if (buttonEnabledMap.contains(3)) buttonEnabledMap[3]->setChecked(false);
    if (buttonActionTypeMap.contains(3)) buttonActionTypeMap[3]->setCurrentIndex(0); // None
    if (buttonKeyMap.contains(3)) {
        buttonKeyMap[3]->clear();
        buttonKeyMap[3]->addItem("None");
        buttonKeyMap[3]->setCurrentIndex(0);
    }
    
    // Left Shoulder - Disabled
    if (buttonEnabledMap.contains(4)) buttonEnabledMap[4]->setChecked(false);
    if (buttonActionTypeMap.contains(4)) buttonActionTypeMap[4]->setCurrentIndex(0); // None
    if (buttonKeyMap.contains(4)) {
        buttonKeyMap[4]->clear();
        buttonKeyMap[4]->addItem("None");
        buttonKeyMap[4]->setCurrentIndex(0);
    }
    
    // Right Shoulder - Right Click
    if (buttonEnabledMap.contains(5)) buttonEnabledMap[5]->setChecked(true);
    if (buttonActionTypeMap.contains(5)) buttonActionTypeMap[5]->setCurrentIndex(1); // Mouse
    if (buttonKeyMap.contains(5)) {
        buttonKeyMap[5]->clear();
        buttonKeyMap[5]->addItems(mouseActions);
        buttonKeyMap[5]->setCurrentIndex(1); // Right Click
    }
    
    // Start - Tab
    if (buttonEnabledMap.contains(6)) buttonEnabledMap[6]->setChecked(true);
    if (buttonActionTypeMap.contains(6)) buttonActionTypeMap[6]->setCurrentIndex(2); // Keyboard
    if (buttonKeyMap.contains(6)) {
        buttonKeyMap[6]->clear();
        buttonKeyMap[6]->addItems(keyboardActions);
        buttonKeyMap[6]->setCurrentIndex(2); // Tab
    }
    
    // Back - Alt
    if (buttonEnabledMap.contains(7)) buttonEnabledMap[7]->setChecked(true);
    if (buttonActionTypeMap.contains(7)) buttonActionTypeMap[7]->setCurrentIndex(2); // Keyboard
    if (buttonKeyMap.contains(7)) {
        buttonKeyMap[7]->clear();
        buttonKeyMap[7]->addItems(keyboardActions);
        buttonKeyMap[7]->setCurrentIndex(8); // Alt
    }
    
    // D-Pad Up - Up Arrow
    if (buttonEnabledMap.contains(8)) buttonEnabledMap[8]->setChecked(true);
    if (buttonActionTypeMap.contains(8)) buttonActionTypeMap[8]->setCurrentIndex(2); // Keyboard
    if (buttonKeyMap.contains(8)) {
        buttonKeyMap[8]->clear();
        buttonKeyMap[8]->addItems(keyboardActions);
        buttonKeyMap[8]->setCurrentIndex(4); // Up
    }
    
    // D-Pad Down - Down Arrow
    if (buttonEnabledMap.contains(9)) buttonEnabledMap[9]->setChecked(true);
    if (buttonActionTypeMap.contains(9)) buttonActionTypeMap[9]->setCurrentIndex(2); // Keyboard
    if (buttonKeyMap.contains(9)) {
        buttonKeyMap[9]->clear();
        buttonKeyMap[9]->addItems(keyboardActions);
        buttonKeyMap[9]->setCurrentIndex(5); // Down
    }
    
    // D-Pad Left - Left Arrow
    if (buttonEnabledMap.contains(10)) buttonEnabledMap[10]->setChecked(true);
    if (buttonActionTypeMap.contains(10)) buttonActionTypeMap[10]->setCurrentIndex(2); // Keyboard
    if (buttonKeyMap.contains(10)) {
        buttonKeyMap[10]->clear();
        buttonKeyMap[10]->addItems(keyboardActions);
        buttonKeyMap[10]->setCurrentIndex(6); // Left
    }
    
    // D-Pad Right - Right Arrow
    if (buttonEnabledMap.contains(11)) buttonEnabledMap[11]->setChecked(true);
    if (buttonActionTypeMap.contains(11)) buttonActionTypeMap[11]->setCurrentIndex(2); // Keyboard
    if (buttonKeyMap.contains(11)) {
        buttonKeyMap[11]->clear();
        buttonKeyMap[11]->addItems(keyboardActions);
        buttonKeyMap[11]->setCurrentIndex(7); // Right
    }
    
    // --- Reset Triggers UI to Default ---
    // Left Trigger - Scroll Up
    leftTriggerEnabled->setChecked(true);
    leftTriggerActionType->setCurrentIndex(0); // Scroll
    leftTriggerThresholdSpin->setValue(8000);
    leftTriggerScrollDir->setCurrentIndex(0); // Up
    leftTriggerScrollVSensi->setValue(100); // 1.0 * 100
    leftTriggerScrollVMax->setValue(40);
    leftTriggerButtonAction->setCurrentIndex(0); // None (disabled)
    
    // Right Trigger - Scroll Down
    rightTriggerEnabled->setChecked(true);
    rightTriggerActionType->setCurrentIndex(0); // Scroll
    rightTriggerThresholdSpin->setValue(8000);
    rightTriggerScrollDir->setCurrentIndex(1); // Down
    rightTriggerScrollVSensi->setValue(100); // 1.0 * 100
    rightTriggerScrollVMax->setValue(40);
    rightTriggerButtonAction->setCurrentIndex(0); // None (disabled)
    
    logInfo("UI reset to default settings - click OK to save");
} 

void ControllerCustomizationWindow::loadMappingsFromCore() {
    if (!m_coreWorker || !m_coreWorker->getCore()) return;
    auto* core = m_coreWorker->getCore();
    std::string guid = m_guid.toStdString();
    logInfo("Loading mappings from core for current controller");
    // --- Sticks ---
    StickMapping leftStick = core->getLeftStickMapping(guid);
    leftStickEnabled->setChecked(leftStick.enabled);
    leftStickActionType->setCurrentIndex(leftStick.action_type == StickActionType::CURSOR ? 0 : 1);
    leftStickDeadzoneSpin->setValue(leftStick.deadzone);
    // Cursor
    leftStickCursorSensi->setValue(int(leftStick.cursor_action.sensitivity * 100));
    leftStickCursorBoosted->setValue(int(leftStick.cursor_action.boosted_sensitivity * 100));
    leftStickCursorSmoothing->setValue(int(leftStick.cursor_action.smoothing * 100));
    // Scroll
    leftStickScrollVSensi->setValue(int(leftStick.scroll_action.vertical_sensitivity * 100));
    leftStickScrollHSensi->setValue(int(leftStick.scroll_action.horizontal_sensitivity * 100));
    leftStickScrollVMax->setValue(leftStick.scroll_action.vertical_max_speed);
    leftStickScrollHMax->setValue(leftStick.scroll_action.horizontal_max_speed);

    StickMapping rightStick = core->getRightStickMapping(guid);
    rightStickEnabled->setChecked(rightStick.enabled);
    rightStickActionType->setCurrentIndex(rightStick.action_type == StickActionType::CURSOR ? 0 : 1);
    rightStickDeadzoneSpin->setValue(rightStick.deadzone);
    rightStickCursorSensi->setValue(int(rightStick.cursor_action.sensitivity * 100));
    rightStickCursorBoosted->setValue(int(rightStick.cursor_action.boosted_sensitivity * 100));
    rightStickCursorSmoothing->setValue(int(rightStick.cursor_action.smoothing * 100));
    rightStickScrollVSensi->setValue(int(rightStick.scroll_action.vertical_sensitivity * 100));
    rightStickScrollHSensi->setValue(int(rightStick.scroll_action.horizontal_sensitivity * 100));
    rightStickScrollVMax->setValue(rightStick.scroll_action.vertical_max_speed);
    rightStickScrollHMax->setValue(rightStick.scroll_action.horizontal_max_speed);

    // --- Buttons ---
    for (auto it = buttonRowToName.begin(); it != buttonRowToName.end(); ++it) {
        int row = it.key();
        QString btnName = it.value();
        ButtonMapping mapping = core->getButtonMapping(guid, btnName.toStdString());
        std::string msg = std::string("Loaded mapping for button: ") + btnName.toStdString();
        logInfo(msg.c_str());
        bool enabled = mapping.enabled && !mapping.actions.empty() && mapping.actions[0].enabled;
        if (buttonEnabledMap.contains(row)) buttonEnabledMap[row]->setChecked(enabled);
        if (buttonActionTypeMap.contains(row) && buttonKeyMap.contains(row)) {
            QComboBox* typeBox = buttonActionTypeMap[row];
            QComboBox* keyBox = buttonKeyMap[row];
            if (!enabled || mapping.actions.empty()) {
                typeBox->setCurrentIndex(0);
                keyBox->clear();
                keyBox->addItem("None");
                keyBox->setCurrentIndex(0);
            } else {
                std::string action_type = "none";
                if (mapping.actions[0].click_type == MouseClickType::LEFT_CLICK) action_type = "mouse_left_click";
                else if (mapping.actions[0].click_type == MouseClickType::RIGHT_CLICK) action_type = "mouse_right_click";
                else if (mapping.actions[0].click_type == MouseClickType::MIDDLE_CLICK) action_type = "mouse_middle_click";
                else if (mapping.actions[0].key_type == KeyboardKeyType::ENTER) action_type = "keyboard_enter";
                else if (mapping.actions[0].key_type == KeyboardKeyType::ESCAPE) action_type = "keyboard_escape";
                else if (mapping.actions[0].key_type == KeyboardKeyType::TAB) action_type = "keyboard_tab";
                else if (mapping.actions[0].key_type == KeyboardKeyType::SPACE) action_type = "keyboard_space";
                else if (mapping.actions[0].key_type == KeyboardKeyType::UP) action_type = "keyboard_up";
                else if (mapping.actions[0].key_type == KeyboardKeyType::DOWN) action_type = "keyboard_down";
                else if (mapping.actions[0].key_type == KeyboardKeyType::LEFT) action_type = "keyboard_left";
                else if (mapping.actions[0].key_type == KeyboardKeyType::RIGHT) action_type = "keyboard_right";
                else if (mapping.actions[0].key_type == KeyboardKeyType::ALT) action_type = "keyboard_alt";
                else if (mapping.actions[0].key_type == KeyboardKeyType::CTRL) action_type = "keyboard_ctrl";
                else if (mapping.actions[0].key_type == KeyboardKeyType::SHIFT) action_type = "keyboard_shift";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F1) action_type = "keyboard_f1";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F2) action_type = "keyboard_f2";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F3) action_type = "keyboard_f3";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F4) action_type = "keyboard_f4";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F5) action_type = "keyboard_f5";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F6) action_type = "keyboard_f6";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F7) action_type = "keyboard_f7";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F8) action_type = "keyboard_f8";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F9) action_type = "keyboard_f9";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F10) action_type = "keyboard_f10";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F11) action_type = "keyboard_f11";
                else if (mapping.actions[0].key_type == KeyboardKeyType::F12) action_type = "keyboard_f12";
                if (action_type.rfind("mouse_", 0) == 0) {
                    typeBox->setCurrentIndex(1);
                    keyBox->clear();
                    keyBox->addItems(mouseActions);
                    keyBox->setCurrentIndex(mouseActionIndexFromType(action_type));
                } else if (action_type.rfind("keyboard_", 0) == 0) {
                    typeBox->setCurrentIndex(2);
                    keyBox->clear();
                    keyBox->addItems(keyboardActions);
                    keyBox->setCurrentIndex(keyboardActionIndexFromType(action_type));
                } else {
                    typeBox->setCurrentIndex(0);
                    keyBox->clear();
                    keyBox->addItem("None");
                    keyBox->setCurrentIndex(0);
                }
                qDebug() << "Button" << btnName << "enabled:" << enabled << "type:" << typeBox->currentText() << "action:" << keyBox->currentText() << "(from action_type:" << QString::fromStdString(action_type) << ")";
            }
        }
    }

    // --- Triggers ---
    TriggerMapping leftTrig = core->getTriggerMapping(guid, "left_trigger");
    leftTriggerEnabled->setChecked(leftTrig.enabled);
    leftTriggerActionType->setCurrentIndex(leftTrig.action_type == TriggerActionType::SCROLL ? 0 : 1);
    leftTriggerThresholdSpin->setValue(leftTrig.threshold);
    // Scroll
    leftTriggerScrollDir->setCurrentIndex(leftTrig.scroll_direction == "up" ? 0 : 1);
    leftTriggerScrollVSensi->setValue(int(leftTrig.trigger_scroll_action.vertical_sensitivity * 100));
    leftTriggerScrollVMax->setValue(leftTrig.trigger_scroll_action.vertical_max_speed);
    // Button
    if (!leftTrig.button_action.actions.empty()) {
        const auto& action = leftTrig.button_action.actions[0];
        if (action.click_type != MouseClickType::NONE) {
            // Mouse action - map to the correct index in the mixed combo box
            leftTriggerButtonAction->setCurrentIndex(static_cast<int>(action.click_type));
        } else if (action.key_type != KeyboardKeyType::NONE) {
            // Keyboard action - map to the correct index in the mixed combo box
            int keyIdx = static_cast<int>(action.key_type) - static_cast<int>(KeyboardKeyType::ENTER);
            leftTriggerButtonAction->setCurrentIndex(mouseActions.size() + keyIdx);
        } else {
            leftTriggerButtonAction->setCurrentIndex(0);
        }
    } else {
        leftTriggerButtonAction->setCurrentIndex(0);
    }

    TriggerMapping rightTrig = core->getTriggerMapping(guid, "right_trigger");
    rightTriggerEnabled->setChecked(rightTrig.enabled);
    rightTriggerActionType->setCurrentIndex(rightTrig.action_type == TriggerActionType::SCROLL ? 0 : 1);
    rightTriggerThresholdSpin->setValue(rightTrig.threshold);
    rightTriggerScrollDir->setCurrentIndex(rightTrig.scroll_direction == "up" ? 0 : 1);
    rightTriggerScrollVSensi->setValue(int(rightTrig.trigger_scroll_action.vertical_sensitivity * 100));
    rightTriggerScrollVMax->setValue(rightTrig.trigger_scroll_action.vertical_max_speed);
    if (!rightTrig.button_action.actions.empty()) {
        const auto& action = rightTrig.button_action.actions[0];
        if (action.click_type != MouseClickType::NONE) {
            // Mouse action - map to the correct index in the mixed combo box
            rightTriggerButtonAction->setCurrentIndex(static_cast<int>(action.click_type));
        } else if (action.key_type != KeyboardKeyType::NONE) {
            // Keyboard action - map to the correct index in the mixed combo box
            int keyIdx = static_cast<int>(action.key_type) - static_cast<int>(KeyboardKeyType::ENTER);
            rightTriggerButtonAction->setCurrentIndex(mouseActions.size() + keyIdx);
        } else {
            rightTriggerButtonAction->setCurrentIndex(0);
        }
    } else {
        rightTriggerButtonAction->setCurrentIndex(0);
    }
}

void ControllerCustomizationWindow::saveMappingsToCore() {
    if (!m_coreWorker || !m_coreWorker->getCore()) return;
    QMetaObject::invokeMethod(m_coreWorker, "stop", Qt::BlockingQueuedConnection);
    auto* core = m_coreWorker->getCore();
    std::string guid = m_guid.toStdString();
    // --- Sticks ---
    StickMapping leftStick;
    leftStick.enabled = leftStickEnabled->isChecked();
    leftStick.action_type = leftStickActionType->currentIndex() == 0 ? StickActionType::CURSOR : StickActionType::SCROLL;
    leftStick.deadzone = leftStickDeadzoneSpin->value();
    leftStick.cursor_action.sensitivity = leftStickCursorSensi->value() / 100.0f;
    leftStick.cursor_action.boosted_sensitivity = leftStickCursorBoosted->value() / 100.0f;
    leftStick.cursor_action.smoothing = leftStickCursorSmoothing->value() / 100.0f;
    leftStick.scroll_action.vertical_sensitivity = leftStickScrollVSensi->value() / 100.0f;
    leftStick.scroll_action.horizontal_sensitivity = leftStickScrollHSensi->value() / 100.0f;
    leftStick.scroll_action.vertical_max_speed = leftStickScrollVMax->value();
    leftStick.scroll_action.horizontal_max_speed = leftStickScrollHMax->value();
    core->setLeftStickMapping(guid, leftStick);

    StickMapping rightStick;
    rightStick.enabled = rightStickEnabled->isChecked();
    rightStick.action_type = rightStickActionType->currentIndex() == 0 ? StickActionType::CURSOR : StickActionType::SCROLL;
    rightStick.deadzone = rightStickDeadzoneSpin->value();
    rightStick.cursor_action.sensitivity = rightStickCursorSensi->value() / 100.0f;
    rightStick.cursor_action.boosted_sensitivity = rightStickCursorBoosted->value() / 100.0f;
    rightStick.cursor_action.smoothing = rightStickCursorSmoothing->value() / 100.0f;
    rightStick.scroll_action.vertical_sensitivity = rightStickScrollVSensi->value() / 100.0f;
    rightStick.scroll_action.horizontal_sensitivity = rightStickScrollHSensi->value() / 100.0f;
    rightStick.scroll_action.vertical_max_speed = rightStickScrollVMax->value();
    rightStick.scroll_action.horizontal_max_speed = rightStickScrollHMax->value();
    core->setRightStickMapping(guid, rightStick);

    // --- Buttons ---
    for (auto it = buttonRowToName.begin(); it != buttonRowToName.end(); ++it) {
        int row = it.key();
        QString btnName = it.value();
        ButtonMapping mapping;
        mapping.enabled = buttonEnabledMap.contains(row) ? buttonEnabledMap[row]->isChecked() : false;
        ButtonAction action;
        QComboBox* typeBox = buttonActionTypeMap.value(row, nullptr);
        QComboBox* keyBox = buttonKeyMap.value(row, nullptr);
        if (typeBox && keyBox) {
            int typeIdx = typeBox->currentIndex();
            if (typeIdx == 1) { // Mouse action
                action.click_type = MouseClickType(keyBox->currentIndex());
                action.key_type = KeyboardKeyType::NONE;
                action.enabled = mapping.enabled;
            }
            else if (typeIdx == 2) { // Keyboard action
                action.click_type = MouseClickType::NONE;
                // Map keyboard combo box index to KeyboardKeyType enum
                int keyIdx = keyBox->currentIndex();
                switch (keyIdx) {
                    case 0: action.key_type = KeyboardKeyType::ENTER; break;
                    case 1: action.key_type = KeyboardKeyType::ESCAPE; break;
                    case 2: action.key_type = KeyboardKeyType::TAB; break;
                    case 3: action.key_type = KeyboardKeyType::SPACE; break;
                    case 4: action.key_type = KeyboardKeyType::UP; break;
                    case 5: action.key_type = KeyboardKeyType::DOWN; break;
                    case 6: action.key_type = KeyboardKeyType::LEFT; break;
                    case 7: action.key_type = KeyboardKeyType::RIGHT; break;
                    case 8: action.key_type = KeyboardKeyType::ALT; break;
                    case 9: action.key_type = KeyboardKeyType::CTRL; break;
                    case 10: action.key_type = KeyboardKeyType::SHIFT; break;
                    case 11: action.key_type = KeyboardKeyType::F1; break;
                    case 12: action.key_type = KeyboardKeyType::F2; break;
                    case 13: action.key_type = KeyboardKeyType::F3; break;
                    case 14: action.key_type = KeyboardKeyType::F4; break;
                    case 15: action.key_type = KeyboardKeyType::F5; break;
                    case 16: action.key_type = KeyboardKeyType::F6; break;
                    case 17: action.key_type = KeyboardKeyType::F7; break;
                    case 18: action.key_type = KeyboardKeyType::F8; break;
                    case 19: action.key_type = KeyboardKeyType::F9; break;
                    case 20: action.key_type = KeyboardKeyType::F10; break;
                    case 21: action.key_type = KeyboardKeyType::F11; break;
                    case 22: action.key_type = KeyboardKeyType::F12; break;
                    default: action.key_type = KeyboardKeyType::NONE; break;
                }
                action.enabled = mapping.enabled;
            }
            else { // None
                action.click_type = MouseClickType::NONE;
                action.key_type = KeyboardKeyType::NONE;
                action.enabled = false;
            }
        }
        mapping.actions.push_back(action);
        core->setButtonMapping(guid, btnName.toStdString(), mapping);
    }

    // --- Triggers ---
    TriggerMapping leftTrig;
    leftTrig.enabled = leftTriggerEnabled->isChecked();
    leftTrig.action_type = leftTriggerActionType->currentIndex() == 0 ? TriggerActionType::SCROLL : TriggerActionType::BUTTON;
    leftTrig.threshold = leftTriggerThresholdSpin->value();
    leftTrig.scroll_direction = leftTriggerScrollDir->currentIndex() == 0 ? "up" : "down";
    leftTrig.trigger_scroll_action.vertical_sensitivity = leftTriggerScrollVSensi->value() / 100.0f;
    leftTrig.trigger_scroll_action.vertical_max_speed = leftTriggerScrollVMax->value();
    ButtonMapping leftTrigBtn;
    leftTrigBtn.enabled = leftTrig.enabled && leftTrig.action_type == TriggerActionType::BUTTON;
    ButtonAction leftTrigAction;
    // Map the mixed mouse+keyboard actions combo box index to the correct enum
    int actionIdx = leftTriggerButtonAction->currentIndex();
    if (actionIdx < mouseActions.size()) {
        // Mouse action
        leftTrigAction.click_type = MouseClickType(actionIdx);
        leftTrigAction.key_type = KeyboardKeyType::NONE;
    } else {
        // Keyboard action
        leftTrigAction.click_type = MouseClickType::NONE;
        int keyIdx = actionIdx - mouseActions.size();
        switch (keyIdx) {
            case 0: leftTrigAction.key_type = KeyboardKeyType::ENTER; break;
            case 1: leftTrigAction.key_type = KeyboardKeyType::ESCAPE; break;
            case 2: leftTrigAction.key_type = KeyboardKeyType::TAB; break;
            case 3: leftTrigAction.key_type = KeyboardKeyType::SPACE; break;
            case 4: leftTrigAction.key_type = KeyboardKeyType::UP; break;
            case 5: leftTrigAction.key_type = KeyboardKeyType::DOWN; break;
            case 6: leftTrigAction.key_type = KeyboardKeyType::LEFT; break;
            case 7: leftTrigAction.key_type = KeyboardKeyType::RIGHT; break;
            case 8: leftTrigAction.key_type = KeyboardKeyType::ALT; break;
            case 9: leftTrigAction.key_type = KeyboardKeyType::CTRL; break;
            case 10: leftTrigAction.key_type = KeyboardKeyType::SHIFT; break;
            case 11: leftTrigAction.key_type = KeyboardKeyType::F1; break;
            case 12: leftTrigAction.key_type = KeyboardKeyType::F2; break;
            case 13: leftTrigAction.key_type = KeyboardKeyType::F3; break;
            case 14: leftTrigAction.key_type = KeyboardKeyType::F4; break;
            case 15: leftTrigAction.key_type = KeyboardKeyType::F5; break;
            case 16: leftTrigAction.key_type = KeyboardKeyType::F6; break;
            case 17: leftTrigAction.key_type = KeyboardKeyType::F7; break;
            case 18: leftTrigAction.key_type = KeyboardKeyType::F8; break;
            case 19: leftTrigAction.key_type = KeyboardKeyType::F9; break;
            case 20: leftTrigAction.key_type = KeyboardKeyType::F10; break;
            case 21: leftTrigAction.key_type = KeyboardKeyType::F11; break;
            case 22: leftTrigAction.key_type = KeyboardKeyType::F12; break;
            default: leftTrigAction.key_type = KeyboardKeyType::NONE; break;
        }
    }
    leftTrigAction.enabled = leftTrigBtn.enabled;
    leftTrigBtn.actions.push_back(leftTrigAction);
    leftTrig.button_action = leftTrigBtn;
    core->setTriggerMapping(guid, "left_trigger", leftTrig);

    TriggerMapping rightTrig;
    rightTrig.enabled = rightTriggerEnabled->isChecked();
    rightTrig.action_type = rightTriggerActionType->currentIndex() == 0 ? TriggerActionType::SCROLL : TriggerActionType::BUTTON;
    rightTrig.threshold = rightTriggerThresholdSpin->value();
    rightTrig.scroll_direction = rightTriggerScrollDir->currentIndex() == 0 ? "up" : "down";
    rightTrig.trigger_scroll_action.vertical_sensitivity = rightTriggerScrollVSensi->value() / 100.0f;
    rightTrig.trigger_scroll_action.vertical_max_speed = rightTriggerScrollVMax->value();
    ButtonMapping rightTrigBtn;
    rightTrigBtn.enabled = rightTrig.enabled && rightTrig.action_type == TriggerActionType::BUTTON;
    ButtonAction rightTrigAction;
    // Map the mixed mouse+keyboard actions combo box index to the correct enum
    actionIdx = rightTriggerButtonAction->currentIndex();
    if (actionIdx < mouseActions.size()) {
        // Mouse action
        rightTrigAction.click_type = MouseClickType(actionIdx);
        rightTrigAction.key_type = KeyboardKeyType::NONE;
    } else {
        // Keyboard action
        rightTrigAction.click_type = MouseClickType::NONE;
        int keyIdx = actionIdx - mouseActions.size();
        switch (keyIdx) {
            case 0: rightTrigAction.key_type = KeyboardKeyType::ENTER; break;
            case 1: rightTrigAction.key_type = KeyboardKeyType::ESCAPE; break;
            case 2: rightTrigAction.key_type = KeyboardKeyType::TAB; break;
            case 3: rightTrigAction.key_type = KeyboardKeyType::SPACE; break;
            case 4: rightTrigAction.key_type = KeyboardKeyType::UP; break;
            case 5: rightTrigAction.key_type = KeyboardKeyType::DOWN; break;
            case 6: rightTrigAction.key_type = KeyboardKeyType::LEFT; break;
            case 7: rightTrigAction.key_type = KeyboardKeyType::RIGHT; break;
            case 8: rightTrigAction.key_type = KeyboardKeyType::ALT; break;
            case 9: rightTrigAction.key_type = KeyboardKeyType::CTRL; break;
            case 10: rightTrigAction.key_type = KeyboardKeyType::SHIFT; break;
            case 11: rightTrigAction.key_type = KeyboardKeyType::F1; break;
            case 12: rightTrigAction.key_type = KeyboardKeyType::F2; break;
            case 13: rightTrigAction.key_type = KeyboardKeyType::F3; break;
            case 14: rightTrigAction.key_type = KeyboardKeyType::F4; break;
            case 15: rightTrigAction.key_type = KeyboardKeyType::F5; break;
            case 16: rightTrigAction.key_type = KeyboardKeyType::F6; break;
            case 17: rightTrigAction.key_type = KeyboardKeyType::F7; break;
            case 18: rightTrigAction.key_type = KeyboardKeyType::F8; break;
            case 19: rightTrigAction.key_type = KeyboardKeyType::F9; break;
            case 20: rightTrigAction.key_type = KeyboardKeyType::F10; break;
            case 21: rightTrigAction.key_type = KeyboardKeyType::F11; break;
            case 22: rightTrigAction.key_type = KeyboardKeyType::F12; break;
            default: rightTrigAction.key_type = KeyboardKeyType::NONE; break;
        }
    }
    rightTrigAction.enabled = rightTrigBtn.enabled;
    rightTrigBtn.actions.push_back(rightTrigAction);
    rightTrig.button_action = rightTrigBtn;
    core->setTriggerMapping(guid, "right_trigger", rightTrig);

    // Save to JSON
    core->saveConfiguration();
    // Clear cache to ensure fresh mappings are loaded
    core->clearMappingCache();
    // Reload configuration to pick up the new mappings
    core->loadConfiguration();
    // Reload controller manager mappings
    core->reloadControllerMappings();
    // Restart the core after saving
    QMetaObject::invokeMethod(m_coreWorker, "start", Qt::QueuedConnection);
    // Close the window instead of showing confirmation
    close();
} 