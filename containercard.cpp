#include "containercard.h"
#include <QStyle>

ContainerCard::ContainerCard(QString id, QWidget *parent) : QFrame(parent)
{
    setupUI();
    lblID->setText(id);
    this->setCursor(Qt::PointingHandCursor);
}

void ContainerCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(lblID->text());
    }
    QFrame::mousePressEvent(event);
}

void ContainerCard::setupUI()
{
    this->setObjectName("ContainerCard");
    this->setFixedSize(320, 200); // 保持之前的大卡片尺寸

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(5);

    // 1. Top
    QHBoxLayout *topLayout = new QHBoxLayout();
    lblID = new QLabel("CNTR-XXXX");
    lblID->setObjectName("CardID");

    lblSignalIcon = new QLabel("📶");
    lblSignalVal = new QLabel("- dBm");
    lblSignalVal->setObjectName("CardSignal");

    topLayout->addWidget(lblID);
    topLayout->addStretch();
    topLayout->addWidget(lblSignalIcon);
    topLayout->addWidget(lblSignalVal);

    // 2. Middle
    QHBoxLayout *midLayout = new QHBoxLayout();
    QVBoxLayout *tempLayout = new QVBoxLayout();
    lblTempTitle = new QLabel("回风温度");
    lblTempTitle->setObjectName("CardLabel");
    lblTempVal = new QLabel("--.-- °C");
    lblTempVal->setObjectName("CardTempRed");
    tempLayout->addWidget(lblTempTitle);
    tempLayout->addWidget(lblTempVal);

    QVBoxLayout *heatLayout = new QVBoxLayout();
    lblHeatTitle = new QLabel("过热度");
    lblHeatTitle->setObjectName("CardLabel");
    lblHeatTitle->setAlignment(Qt::AlignRight);
    lblHeatVal = new QLabel("--.-- K");
    lblHeatVal->setObjectName("CardHeatGreen");
    lblHeatVal->setAlignment(Qt::AlignRight);
    heatLayout->addWidget(lblHeatTitle);
    heatLayout->addWidget(lblHeatVal);

    midLayout->addLayout(tempLayout);
    midLayout->addStretch();
    midLayout->addLayout(heatLayout);

    // 3. Bottom
    QHBoxLayout *botLayout = new QHBoxLayout();
    lblPidStatus = new QLabel("❄️ PID: 运行");
    lblPidStatus->setObjectName("CardStatus");
    lblValveVal = new QLabel("⚡ 45%");
    lblValveVal->setObjectName("CardStatus");

    botLayout->addWidget(lblPidStatus);
    botLayout->addStretch();
    botLayout->addWidget(lblValveVal);

    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(midLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(new QLabel("_______________________________________"));
    mainLayout->addLayout(botLayout);
}

void ContainerCard::updateData(double temp, double superheat, int rssi, bool isOnline, bool pidRunning)
{
    Q_UNUSED(pidRunning);
    lblTempVal->setText(QString::number(temp, 'f', 1) + " °C");

    if (temp > -18.0) lblTempVal->setObjectName("CardTempRed");
    else lblTempVal->setObjectName("CardTempGreen");

    lblTempVal->style()->unpolish(lblTempVal);
    lblTempVal->style()->polish(lblTempVal);

    lblHeatVal->setText(QString::number(superheat, 'f', 1) + " K");

    lblSignalVal->setText(QString::number(rssi) + "dBm");
    if (rssi > -80) {
        lblSignalVal->setStyleSheet("color: #52c41a;");
        lblSignalIcon->setText("📶");
    } else if (rssi > -95) {
        lblSignalVal->setStyleSheet("color: #faad14;");
        lblSignalIcon->setText("📶");
    } else {
        lblSignalVal->setStyleSheet("color: #ff4d4f;");
        lblSignalIcon->setText("⚠️");
    }

    if (!isOnline) {
        this->setEnabled(false);
        lblID->setText(lblID->text() + " (离线)");
    } else {
        this->setEnabled(true);
    }

    // ==========================================
    // 【新增】根据紧急程度设置卡片边框颜色
    // ==========================================
    QString state = "normal"; // 默认为正常

    if (!isOnline) {
        state = "offline"; // 离线 (灰色/暗红)
    }
    else if (temp > -15.0 || rssi < -100) {
        state = "alarm";   // 严重报警 (红色框): 温度过高 或 信号极差
    }
    else if (temp > -18.0 || rssi < -90) {
        state = "warning"; // 警告 (橙色框): 温度偏高 或 信号稍差
    }

    // 只有当状态发生变化时才刷新样式，避免闪烁和性能浪费
    if (this->property("urgency").toString() != state) {
        this->setProperty("urgency", state);
        this->style()->unpolish(this); // 强制刷新样式表
        this->style()->polish(this);
    }
}
