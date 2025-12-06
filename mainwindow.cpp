#include "mainwindow.h"
#include <QRandomGenerator>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QDebug>
#include <QPieSeries>
#include <QPieSlice>
#include <QSplineSeries>
#include <QCategoryAxis>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isDarkMode(true)
{
    // 初始化 66 个节点
    int nodeCount = 66;
    for(int i=0; i<nodeCount; i++) {
        NodeData d;
        d.id = QString("CNTR-2023%1").arg(i+1, 3, 10, QChar('0'));
        d.temp = -18.0;
        d.superheat = 5.0;
        d.pressure = 0.33;
        d.valve = 45;
        d.rssi = -80;
        d.pidRunning = true;
        m_data.append(d);
    }

    setupUI();
    updateStyle(); // 应用初始样式

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    timer->start(1000);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    this->resize(1400, 900);
    this->setWindowTitle("冷链监控中心 (Cold Chain Monitor Center) v3.0 - Pro Analytics");

    QWidget *centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);

    // 1. Sidebar
    QWidget *sidebar = new QWidget;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(240);
    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(0, 30, 0, 20);
    sideLayout->setSpacing(10);

    QLabel *logo = new QLabel("🚢 冷链监控中心");
    logo->setObjectName("AppLogo");
    logo->setAlignment(Qt::AlignCenter);
    sideLayout->addWidget(logo);
    sideLayout->addSpacing(20);

    navGroup = new QButtonGroup(this);
    connect(navGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &MainWindow::onNavButtonClicked);

    QStringList menus = {"⊞ 全船概览", "📈 历史数据分析", "🕸️ LoRa网关拓扑", "⚠️ 告警日志"};
    for(int i=0; i<menus.size(); ++i) {
        QPushButton *btn = new QPushButton(menus[i]);
        btn->setObjectName("SideBtn");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        navGroup->addButton(btn, i);
        sideLayout->addWidget(btn);
        if(i==0) btn->setChecked(true);
    }
    sideLayout->addStretch();

    btnTheme = new QPushButton("🌙 夜间模式");
    btnTheme->setObjectName("ThemeBtn");
    connect(btnTheme, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    sideLayout->addWidget(btnTheme);

    QLabel *user = new QLabel("👤 值班员: ADMIN\n🟢 状态: 在线");
    user->setObjectName("UserInfo");
    sideLayout->addWidget(user);

    mainLayout->addWidget(sidebar);

    // 2. Main Content
    mainStack = new QStackedWidget;
    mainStack->addWidget(createDashboardPage()); // 0
    mainStack->addWidget(createHistoryPage());   // 1 (重写了这里)
    mainStack->addWidget(createTopologyPage());  // 2

    QLabel *placeholder = new QLabel("告警日志模块正在开发中...");
    placeholder->setAlignment(Qt::AlignCenter);
    mainStack->addWidget(placeholder);           // 3

    mainLayout->addWidget(mainStack);
}

// --- 页面 1: 仪表盘 ---
QWidget* MainWindow::createDashboardPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QHBoxLayout *topBar = new QHBoxLayout;
    QLabel *title = new QLabel("全船冷藏箱实时监控");
    title->setObjectName("PageTitle");
    topBar->addWidget(title);
    topBar->addStretch();
    QLabel *badge = new QLabel(QString("🟢 在线: %1/%1").arg(m_data.size()));
    badge->setObjectName("BadgeLabel");
    topBar->addWidget(badge);
    layout->addLayout(topBar);

    QHBoxLayout *statLayout = new QHBoxLayout;
    statLayout->setSpacing(20);
    lblAvgTemp = new QLabel("-18.0°C");
    statLayout->addWidget(createTopStatCard("平均箱温", "-18.0°C", "🌡️"));
    statLayout->addWidget(createTopStatCard("平均过热度", "5.0 K", "∿"));
    statLayout->addWidget(createTopStatCard("运行能耗", "482 kW", "⚡"));
    statLayout->addWidget(createTopStatCard("网关负载", "78%", "📶"));
    layout->addLayout(statLayout);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setObjectName("ContentScroll");
    QWidget *gridWidget = new QWidget;
    gridWidget->setAttribute(Qt::WA_StyledBackground);
    gridWidget->setObjectName("GridWidget");
    gridContainer = new QGridLayout(gridWidget);
    gridContainer->setSpacing(6);
    gridContainer->setContentsMargins(0,0,10,0);

    int cols = 4;
    for(int i=0; i<m_data.size(); ++i) {
        ContainerCard *card = new ContainerCard(m_data[i].id);
        m_cards.append(card);
        connect(card, &ContainerCard::clicked, this, &MainWindow::onCardClicked);
        gridContainer->addWidget(card, i/cols, i%cols);
    }
    scroll->setWidget(gridWidget);
    layout->addWidget(scroll);

    return page;
}

// ========================================================
// --- 页面 2: 历史数据分析 (已对接 66 个节点) ---
// ========================================================
QWidget* MainWindow::createHistoryPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // --- 1. 顶部控制栏 ---
    QHBoxLayout *topBar = new QHBoxLayout;
    QLabel *title = new QLabel("历史趋势分析");
    title->setObjectName("PageTitle");

    QComboBox *comboId = new QComboBox;
    // 【关键修改】不再是写死的 ID，而是从数据源动态加载所有 66 个节点
    for(const NodeData &node : m_data) {
        comboId->addItem(node.id);
    }
    comboId->setFixedSize(140, 32);

    // 模拟：当切换ID时，刷新一下图表（这里只是演示，实际应读取对应历史库）
    connect(comboId, &QComboBox::currentTextChanged, [this](const QString &text){
        // 这里可以添加逻辑：根据 text (ID) 去数据库查历史数据并 series->replace()
        qDebug() << "Switching history view to:" << text;
    });

    QComboBox *comboTime = new QComboBox;
    comboTime->addItems({"本次航程", "最近24小时", "最近7天"});
    comboTime->setFixedSize(120, 32);

    QPushButton *btnExport = new QPushButton("📥 导出报表");
    btnExport->setObjectName("BtnExport");
    btnExport->setFixedSize(100, 32);
    btnExport->setCursor(Qt::PointingHandCursor);

    topBar->addWidget(title);
    topBar->addStretch();
    topBar->addWidget(comboId);
    topBar->addSpacing(10);
    topBar->addWidget(comboTime);
    topBar->addSpacing(10);
    topBar->addWidget(btnExport);
    mainLayout->addLayout(topBar);

    // --- 2. 主体内容 (左图右表) ---
    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(20);

    // === 左侧：温控性能综合曲线 (双Y轴大图) ===
    QChart *chartMain = new QChart();
    chartMain->setBackgroundVisible(false);
    chartMain->setTitle("温控性能综合曲线 (温度 vs 阀门开度)");
    chartMain->legend()->setAlignment(Qt::AlignTop);

    // 曲线1：温度 (实线，左轴)
    QSplineSeries *seriesTempH = new QSplineSeries();
    seriesTempH->setName("温度 (°C)");
    QPen penTemp(QColor("#40a9ff")); penTemp.setWidth(3);
    seriesTempH->setPen(penTemp);

    // 曲线2：阀门 (虚线，右轴)
    QSplineSeries *seriesValveH = new QSplineSeries();
    seriesValveH->setName("阀门开度 (%)");
    QPen penValve(QColor("#9254de")); // 紫色
    penValve.setWidth(3); penValve.setStyle(Qt::DotLine);
    seriesValveH->setPen(penValve);

    // 模拟24小时数据
    for(int i=0; i<=24; i++) {
        double t = -18.0 + sin(i/3.0) * 2.0 + (QRandomGenerator::global()->bounded(10)/10.0);
        double v = 40 + cos(i/3.0) * 20 + (QRandomGenerator::global()->bounded(10));
        seriesTempH->append(i, t);
        seriesValveH->append(i, v);
    }

    chartMain->addSeries(seriesTempH);
    chartMain->addSeries(seriesValveH);

    // 坐标轴设置
    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0, 24);
    axisX->setLabelFormat("%d:00");
    axisX->setTickCount(13);

    QValueAxis *axisYLeft = new QValueAxis;
    axisYLeft->setRange(-22, -14);
    axisYLeft->setTitleText("温度 (°C)");
    axisYLeft->setLinePenColor(QColor("#40a9ff"));

    QValueAxis *axisYRight = new QValueAxis;
    axisYRight->setRange(0, 100);
    axisYRight->setTitleText("阀门 (%)");
    axisYRight->setLinePenColor(QColor("#9254de"));

    chartMain->addAxis(axisX, Qt::AlignBottom);
    chartMain->addAxis(axisYLeft, Qt::AlignLeft);
    chartMain->addAxis(axisYRight, Qt::AlignRight);

    seriesTempH->attachAxis(axisX);
    seriesTempH->attachAxis(axisYLeft);
    seriesValveH->attachAxis(axisX);
    seriesValveH->attachAxis(axisYRight);

    chartHistoryMain = new QChartView(chartMain);
    chartHistoryMain->setRenderHint(QPainter::Antialiasing);
    chartHistoryMain->setObjectName("ChartBox"); // 加上边框背景

    contentLayout->addWidget(chartHistoryMain, 2); // 左侧占 2/3 宽度

    // === 右侧：垂直布局 (上饼图，下表格) ===
    QVBoxLayout *rightPanel = new QVBoxLayout;
    rightPanel->setSpacing(20);

    // 1. 右上：过热度分布 (环形图)
    QChart *chartP = new QChart();
    chartP->setBackgroundVisible(false);
    chartP->setTitle("过热度分布 (PID稳定性)");
    chartP->legend()->setAlignment(Qt::AlignRight);

    QPieSeries *pieSeries = new QPieSeries();
    pieSeries->setHoleSize(0.45); // 变成环形
    QPieSlice *s1 = pieSeries->append("过冷 (<4K)", 15);
    QPieSlice *s2 = pieSeries->append("最佳 (4-8K)", 65);
    QPieSlice *s3 = pieSeries->append("过热 (>8K)", 20);

    s1->setColor(QColor("#40a9ff")); // 蓝
    s2->setColor(QColor("#52c41a")); // 绿
    s3->setColor(QColor("#ff4d4f")); // 红
    s2->setExploded(true); // 突出显示最佳区域

    chartP->addSeries(pieSeries);

    chartPie = new QChartView(chartP);
    chartPie->setRenderHint(QPainter::Antialiasing);
    chartPie->setObjectName("ChartBox");
    rightPanel->addWidget(chartPie, 1);

    // 2. 右下：异常事件记录 (表格)
    QVBoxLayout *tableWrap = new QVBoxLayout;
    QLabel *lblTableTitle = new QLabel("异常事件记录");
    lblTableTitle->setStyleSheet("font-weight: bold; color: #888; margin-bottom: 5px;");

    tableEvents = new QTableWidget(5, 3); // 5行3列
    tableEvents->setObjectName("EventTable");
    tableEvents->setHorizontalHeaderLabels({"时间", "事件", "值"});
    tableEvents->verticalHeader()->setVisible(false);
    tableEvents->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableEvents->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableEvents->setEditTriggers(QAbstractItemView::NoEditTriggers); // 不可编辑
    tableEvents->setFocusPolicy(Qt::NoFocus); // 去虚线框

    // 填入模拟数据
    QStringList times = {"08:30:12", "04:15:00", "02:00:00", "Yesterday", "Yesterday"};
    QStringList events = {"除霜启动", "温度偏差", "网关握手", "PID饱和", "压缩机启动"};
    QStringList values = {"Cycle Start", "-20.5°C", "OK", "100%", "ON"};

    for(int i=0; i<5; i++) {
        tableEvents->setItem(i, 0, new QTableWidgetItem(times[i]));
        tableEvents->setItem(i, 1, new QTableWidgetItem(events[i]));
        tableEvents->setItem(i, 2, new QTableWidgetItem(values[i]));
        // 居中
        tableEvents->item(i, 0)->setTextAlignment(Qt::AlignCenter);
        tableEvents->item(i, 1)->setTextAlignment(Qt::AlignCenter);
        tableEvents->item(i, 2)->setTextAlignment(Qt::AlignCenter);
    }

    // 给表格加个容器背景
    QFrame *tableFrame = new QFrame;
    tableFrame->setObjectName("ChartBox");
    QVBoxLayout *tfLayout = new QVBoxLayout(tableFrame);
    tfLayout->addWidget(lblTableTitle);
    tfLayout->addWidget(tableEvents);

    rightPanel->addWidget(tableFrame, 1);

    contentLayout->addLayout(rightPanel, 1); // 右侧占 1/3 宽度
    mainLayout->addLayout(contentLayout);

    return page;
}

// --- 页面 3: LoRa网关拓扑 ---
QWidget* MainWindow::createTopologyPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 30, 30, 30);
    QLabel *title = new QLabel("🕸️ LoRaMesh 网络拓扑结构");
    title->setObjectName("PageTitle");
    layout->addWidget(title);

    QGraphicsScene *scene = new QGraphicsScene();
    QGraphicsView *view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setObjectName("TopologyView");

    QGraphicsEllipseItem *gateway = scene->addEllipse(-40, -40, 80, 80, QPen(Qt::NoPen), QBrush(QColor("#1890ff")));
    QGraphicsTextItem *gwText = scene->addText("Master\nGateway");
    gwText->setPos(-30, -35);
    gwText->setDefaultTextColor(Qt::white);

    int radius = 250;
    for(int i=0; i<16; i++) {
        double angle = 2 * 3.14159 * i / 16;
        double x = radius * cos(angle);
        double y = radius * sin(angle);
        scene->addLine(0, 0, x, y, QPen(QColor("#888"), 1, Qt::DashLine));
        scene->addEllipse(x-15, y-15, 30, 30, QPen(Qt::NoPen), QBrush(QColor("#52c41a")));
        QGraphicsTextItem *txt = scene->addText(QString("N%1").arg(i+1));
        txt->setPos(x-12, y-10);
    }
    layout->addWidget(view);
    return page;
}

QWidget* MainWindow::createTopStatCard(QString title, QString value, QString icon) {
    QFrame *card = new QFrame;
    card->setObjectName("StatCard");
    card->setFixedHeight(100);
    QHBoxLayout *l = new QHBoxLayout(card);
    QVBoxLayout *vl = new QVBoxLayout;
    QLabel *t = new QLabel(title);
    t->setObjectName("CardLabel");
    QLabel *v = new QLabel(value);
    v->setObjectName("StatValue");
    if(title.contains("箱温")) lblAvgTemp = v;
    vl->addWidget(t);
    vl->addWidget(v);
    QLabel *ico = new QLabel(icon);
    ico->setStyleSheet("font-size: 30px; color: #1890ff;");
    l->addLayout(vl);
    l->addStretch();
    l->addWidget(ico);
    return card;
}

void MainWindow::updateSimulation()
{
    double total = 0;
    for(int i=0; i<m_cards.size(); i++) {
        double noise = (QRandomGenerator::global()->bounded(10) - 5) / 10.0;
        m_data[i].temp += noise;
        if(m_data[i].temp > -16.0) m_data[i].temp -= 0.3;
        if(m_data[i].temp < -20.0) m_data[i].temp += 0.3;
        m_data[i].pressure = 0.3 + (m_data[i].temp + 20) * 0.05;
        m_data[i].superheat = 5.0 - noise * 2;
        m_data[i].valve = 45 + (int)(noise * 10);
        m_data[i].rssi = -80 + QRandomGenerator::global()->bounded(10) - 5;

        m_cards[i]->updateData(m_data[i].temp, m_data[i].superheat, m_data[i].rssi, true, true);
        emit broadcastData(m_data[i].id, m_data[i].temp, m_data[i].pressure, m_data[i].superheat, m_data[i].valve, m_data[i].rssi);
        total += m_data[i].temp;
    }
    if(lblAvgTemp && m_cards.size() > 0) {
        lblAvgTemp->setText(QString::number(total/m_cards.size(), 'f', 1) + "°C");
    }
}

void MainWindow::onNavButtonClicked(int id) { mainStack->setCurrentIndex(id); }
void MainWindow::onCardClicked(QString id) {
    ContainerDetailDialog dlg(id, this);
    connect(this, &MainWindow::broadcastData, &dlg, &ContainerDetailDialog::updateRealtimeData);
    dlg.exec();
}
void MainWindow::toggleTheme() { isDarkMode = !isDarkMode; updateStyle(); btnTheme->setText(isDarkMode ? "🌙 夜间模式" : "☀️ 日间模式"); }

// ==========================================
// 核心：样式表 (增加了表格和图表容器样式)
// ==========================================
void MainWindow::updateStyle()
{
    QString bgColor, sideColor, contentColor, cardColor, textColor, borderColor, titleColor, tableHeaderBg, tableHover;

    if(isDarkMode) {
        bgColor = "#0d1117"; sideColor = "#161b22"; contentColor = "#0d1117";
        cardColor = "#161b22"; borderColor = "#30363d"; textColor = "#c9d1d9";
        titleColor = "#ffffff"; tableHeaderBg = "#21262d"; tableHover = "#30363d";
    } else {
        bgColor = "#f0f2f5"; sideColor = "#ffffff"; contentColor = "#f0f2f5";
        cardColor = "#ffffff"; borderColor = "#d9d9d9"; textColor = "#595959";
        titleColor = "#000000"; tableHeaderBg = "#fafafa"; tableHover = "#f5f5f5";
    }

    QString qss = QString(R"(
        QMainWindow { background-color: %1; }
        QWidget { font-family: "Segoe UI", "Microsoft YaHei"; }
        #Sidebar { background-color: %2; border-right: 1px solid %5; }
        #AppLogo { font-size: 20px; font-weight: bold; color: #1890ff; }
        #UserInfo { color: #888; padding-left: 20px; font-size: 12px; }
        #SideBtn { border: none; background: transparent; color: %6; text-align: left; padding: 15px 20px; font-size: 14px; }
        #SideBtn:checked { background-color: %4; color: #1890ff; border-left: 4px solid #1890ff; font-weight: bold; }
        #SideBtn:hover { background-color: rgba(128,128,128, 0.1); }
        #ThemeBtn { border: 1px solid %5; border-radius: 15px; background: transparent; color: %6; margin: 0 20px; padding: 5px; }
        #PageTitle { font-size: 20px; font-weight: bold; color: %7; margin-bottom: 10px; }
        #BadgeLabel { background-color: %4; color: #52c41a; padding: 5px 15px; border-radius: 14px; border: 1px solid %5; }
        #StatCard { background-color: %4; border-radius: 8px; border: 1px solid %5; }
        #StatValue { font-size: 24px; font-weight: bold; color: %7; }
        #CardLabel { font-size: 12px; color: #888; }

        #ContainerCard { background-color: %4; border-radius: 6px; border: 1px solid %5; }
        #ContainerCard[urgency="normal"] { border: 1px solid #52c41a; }
        #ContainerCard[urgency="warning"] { border: 2px solid #faad14; }
        #ContainerCard[urgency="alarm"] { border: 2px solid #ff4d4f; }

        #CardID { background-color: rgba(128,128,128, 0.1); color: #888; border-radius: 4px; padding: 2px; }
        #CardSignal { color: #faad14; font-weight: bold; }
        #CardStatus { color: #888; font-size: 12px; }
        #CardTempRed { color: #ff4d4f; font-size: 26px; font-weight: bold; }
        #CardTempGreen { color: #52c41a; font-size: 26px; font-weight: bold; }
        #CardHeatGreen { color: #52c41a; font-size: 14px; font-weight: bold; }

        QScrollArea { border: none; background: transparent; }
        #GridWidget { background: transparent; }
        #TopologyView { border: 1px solid %5; background-color: %4; border-radius: 8px; }

        /* --- 新增：历史页专用样式 --- */
        #ChartBox { background-color: %4; border: 1px solid %5; border-radius: 8px; }
        #BtnExport { background-color: #52c41a; color: white; border: none; border-radius: 4px; font-weight: bold; }
        #BtnExport:hover { background-color: #73d13d; }

        /* 下拉框样式 */
        QComboBox { background-color: %4; border: 1px solid %5; border-radius: 4px; padding: 5px; color: %6; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background-color: %4; color: %6; selection-background-color: #1890ff; }

        /* 表格样式 */
        #EventTable { background-color: transparent; border: none; color: %6; gridline-color: %5; }
        #EventTable::item { padding: 5px; }
        #EventTable::item:selected { background-color: rgba(24, 144, 255, 0.2); }
        QHeaderView::section { background-color: %8; color: %6; padding: 8px; border: none; font-weight: bold; }
    )")
    .arg(bgColor, sideColor, contentColor, cardColor, borderColor, textColor, titleColor, tableHeaderBg);

    this->setStyleSheet(qss);

    // 更新 Charts 的主题
    QChart::ChartTheme theme = isDarkMode ? QChart::ChartThemeDark : QChart::ChartThemeLight;
    QColor titleBrush = isDarkMode ? Qt::white : Qt::black;

    if(chartHistoryMain) { chartHistoryMain->chart()->setTheme(theme); chartHistoryMain->chart()->setTitleBrush(titleBrush); }
    if(chartPie) { chartPie->chart()->setTheme(theme); chartPie->chart()->setTitleBrush(titleBrush); }
}
