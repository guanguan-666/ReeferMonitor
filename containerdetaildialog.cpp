#include "containerdetaildialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QRandomGenerator>
#include <QDateTime>

ContainerDetailDialog::ContainerDetailDialog(QString containerId, QWidget *parent)
    : QDialog(parent)
    , currentId(containerId)
    , timeCounter(0) // 时间从0开始
{
    setupUI(containerId);
    setupStyle();

    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    this->setWindowTitle("集装箱详情: " + containerId);
    this->resize(1000, 650);
}

// 【修改点1】核心：接收实时数据并更新界面
void ContainerDetailDialog::updateRealtimeData(QString id, double temp, double pressure, double superheat, int valve, int rssi)
{
    // 如果收到的数据不是本弹窗的ID，直接忽略
    if(id != currentId) return;

    // 1. 更新文字
    if(valTemp) valTemp->setText(QString::number(temp, 'f', 1));
    if(valPress) valPress->setText(QString::number(pressure, 'f', 2));
    if(valSuperheat) valSuperheat->setText(QString::number(superheat, 'f', 1) + " °C");
    if(valValve) valValve->setText(QString::number(valve));
    if(valRssi) valRssi->setText(QString::number(rssi) + " dBm");

    // 2. 更新图表 (曲线滚动效果)
    timeCounter++;
    seriesTemp->append(timeCounter, temp);
    seriesSuperheat->append(timeCounter, superheat);

    // 让X轴随着时间滚动 (始终显示最近 20 个点)
    if (timeCounter > 20) {
        chart->axes(Qt::Horizontal).first()->setRange(timeCounter - 20, timeCounter);
        // 为了性能，移除太久远的数据 (可选)
        if(seriesTemp->count() > 100) seriesTemp->remove(0);
        if(seriesSuperheat->count() > 100) seriesSuperheat->remove(0);
    } else {
        chart->axes(Qt::Horizontal).first()->setRange(0, 20);
    }
}

void ContainerDetailDialog::setupUI(QString id)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // --- 1. Top ---
    QHBoxLayout *headerLayout = new QHBoxLayout;
    QLabel *lblIcon = new QLabel("🧊");
    lblIcon->setStyleSheet("font-size: 24px;");

    QLabel *lblTitle = new QLabel("集装箱 ID:" + id);
    lblTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: white;");

    QLabel *lblSub = new QLabel("货物类型: 疫苗/生鲜 | 设定温度: -18.0°C");
    lblSub->setStyleSheet("font-size: 14px; color: #888; margin-left: 10px;");

    headerLayout->addWidget(lblIcon);
    headerLayout->addWidget(lblTitle);
    headerLayout->addWidget(lblSub);
    headerLayout->addStretch();

    // --- 2. Center ---
    QHBoxLayout *centerLayout = new QHBoxLayout;
    centerLayout->setSpacing(20);

    // Left Panel
    QVBoxLayout *leftPanel = new QVBoxLayout;
    leftPanel->setSpacing(15);

    // 【修改点2】传递成员变量指针来创建卡片，以便后续更新
    leftPanel->addWidget(createDetailCard("当前回风温度 (Tin)", valTemp, "°C", "#ffffff"));
    leftPanel->addWidget(createDetailCard("蒸发压力 (Pe)", valPress, "MPa", "#40a9ff"));

    // 过热度特殊处理 (带进度条)
    QFrame *shCard = new QFrame;
    shCard->setObjectName("DetailCard");
    QVBoxLayout *shLayout = new QVBoxLayout(shCard);
    QLabel *shTitle = new QLabel("过热度 (Superheat)");
    shTitle->setStyleSheet("color: #888;");
    valSuperheat = new QLabel("5.0 °C"); // 初始化
    valSuperheat->setStyleSheet("color: #52c41a; font-size: 28px; font-weight: bold;");
    QProgressBar *bar = new QProgressBar;
    bar->setRange(0, 15);
    bar->setValue(5);
    bar->setFixedHeight(6);
    bar->setTextVisible(false);
    bar->setStyleSheet("QProgressBar::chunk { background-color: #52c41a; border-radius: 3px; } QProgressBar { background-color: #333; border-radius: 3px; border: none; }");
    shLayout->addWidget(shTitle);
    shLayout->addWidget(valSuperheat);
    shLayout->addWidget(bar);
    leftPanel->addWidget(shCard);

    leftPanel->addWidget(createDetailCard("电子膨胀阀 (EEV)", valValve, "%", "#d3adf7"));

    centerLayout->addLayout(leftPanel, 1);

    // Right Chart
    chart = new QChart();
    chart->setBackgroundVisible(false);
    chart->setTitle("温压协同实时监控曲线");
    chart->setTitleBrush(Qt::white);
    chart->legend()->setAlignment(Qt::AlignTop);
    chart->legend()->setLabelColor(Qt::white);

    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0, 20); // 初始范围
    axisX->setLabelsColor(Qt::gray);
    axisX->setTitleText("Time (s)");

    QValueAxis *axisYTemp = new QValueAxis;
    axisYTemp->setRange(-20, -10);
    axisYTemp->setLinePenColor(QColor("#40a9ff"));
    axisYTemp->setTitleText("温度 (°C)");
    axisYTemp->setTitleBrush(QColor("#40a9ff"));

    QValueAxis *axisYSH = new QValueAxis;
    axisYSH->setRange(0, 15);
    axisYSH->setLinePenColor(QColor("#52c41a"));
    axisYSH->setTitleText("过热度 (K)");
    axisYSH->setTitleBrush(QColor("#52c41a"));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYTemp, Qt::AlignLeft);
    chart->addAxis(axisYSH, Qt::AlignRight);

    seriesTemp = new QLineSeries();
    seriesTemp->setName("实际温度");
    seriesTemp->setColor(QColor("#40a9ff"));

    seriesSuperheat = new QLineSeries();
    seriesSuperheat->setName("过热度");
    seriesSuperheat->setColor(QColor("#52c41a"));

    // 初始化一些点，让图表不为空
    for(int i=0; i<5; i++) {
        seriesTemp->append(i, -18.0);
        seriesSuperheat->append(i, 5.0);
    }
    timeCounter = 5;

    chart->addSeries(seriesTemp);
    chart->addSeries(seriesSuperheat);

    seriesTemp->attachAxis(axisX);
    seriesTemp->attachAxis(axisYTemp);
    seriesSuperheat->attachAxis(axisX);
    seriesSuperheat->attachAxis(axisYSH);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setObjectName("DetailChart");
    centerLayout->addWidget(chartView, 2);

    // --- 3. Bottom ---
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(20);

    QFrame *healthFrame = new QFrame;
    healthFrame->setObjectName("DetailCard");
    QGridLayout *gLayout = new QGridLayout(healthFrame);
    gLayout->addWidget(new QLabel("电池电压:"), 0, 0);
    QLabel *vBat = new QLabel("24.2 V"); vBat->setStyleSheet("color: #52c41a;");
    gLayout->addWidget(vBat, 0, 1);

    gLayout->addWidget(new QLabel("RSSI 信号:"), 0, 2);
    valRssi = new QLabel("-96 dBm"); // 成员变量
    valRssi->setStyleSheet("color: #faad14;");
    gLayout->addWidget(valRssi, 0, 3);

    gLayout->addWidget(new QLabel("最后通信:"), 1, 0);
    gLayout->addWidget(new QLabel("Just now"), 1, 1);

    gLayout->addWidget(new QLabel("运行模式:"), 1, 2);
    gLayout->addWidget(new QLabel("模糊PID自动"), 1, 3);

    for(int i=0; i<gLayout->count(); i++) {
        QWidget *w = gLayout->itemAt(i)->widget();
        if(QLabel *l = qobject_cast<QLabel*>(w)) {
            if(l->styleSheet().isEmpty()) l->setStyleSheet("color: #aaa;");
        }
    }

    QFrame *cmdFrame = new QFrame;
    cmdFrame->setObjectName("DetailCard");
    QHBoxLayout *cmdLayout = new QHBoxLayout(cmdFrame);

    QPushButton *btnSet = new QPushButton("🌡️ 修改设定点");
    btnSet->setStyleSheet("background-color: #1890ff; color: white; border: none; padding: 10px; border-radius: 4px; font-weight: bold;");

    QPushButton *btnFrost = new QPushButton("❄️ 强制除霜");
    btnFrost->setStyleSheet("background-color: #faad14; color: white; border: none; padding: 10px; border-radius: 4px; font-weight: bold;");

    QPushButton *btnStop = new QPushButton("🛑 紧急停机");
    btnStop->setStyleSheet("background-color: #ff4d4f; color: white; border: none; padding: 10px; border-radius: 4px; font-weight: bold;");

    cmdLayout->addWidget(new QLabel("远程指令下发:"));
    cmdLayout->addWidget(btnSet);
    cmdLayout->addWidget(btnFrost);
    cmdLayout->addWidget(btnStop);

    bottomLayout->addWidget(healthFrame, 1);
    bottomLayout->addWidget(cmdFrame, 2);

    mainLayout->addLayout(headerLayout);
    mainLayout->addLayout(centerLayout, 1);
    mainLayout->addLayout(bottomLayout);
}

// 修改后的辅助函数，接收 QLabel 指针引用
QFrame* ContainerDetailDialog::createDetailCard(QString title, QLabel* &valLabel, QString unit, QString colorHex)
{
    QFrame *card = new QFrame;
    card->setObjectName("DetailCard");
    QVBoxLayout *l = new QVBoxLayout(card);

    QLabel *t = new QLabel(title);
    t->setStyleSheet("color: #888;");

    valLabel = new QLabel("--"); // 创建Label并赋值给传入的指针
    valLabel->setStyleSheet(QString("color: %1; font-size: 32px; font-weight: bold;").arg(colorHex));

    QLabel *u = new QLabel(unit);
    u->setStyleSheet("color: #666; alignment: right;");

    l->addWidget(t);
    l->addWidget(valLabel);
    l->addWidget(u, 0, Qt::AlignRight);
    return card;
}

void ContainerDetailDialog::setupStyle()
{
    this->setStyleSheet(R"(
        QDialog { background-color: #1f2a3a; }
        #DetailCard {
            background-color: #263345;
            border-radius: 8px;
            border: 1px solid #333;
        }
        #DetailChart {
            background-color: #263345;
            border-radius: 8px;
        }
    )");
}
