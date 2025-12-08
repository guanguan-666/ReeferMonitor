#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QTimer>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QTableWidget> // 新增：表格控件
#include <QHeaderView>  // 新增：表头
#include <QComboBox>    // 新增：下拉框
#include <QtCharts>
#include <QVector>
#include "containercard.h"
#include "containerdetaildialog.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
using namespace QtCharts;

// 数据结构
struct NodeData {
    QString id;
    double temp;
    double superheat;
    double pressure;
    int valve;
    int rssi;
    bool pidRunning;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void broadcastData(QString id, double temp, double pressure, double superheat, int valve, int rssi);

private slots:
    void updateSimulation();
    void onNavButtonClicked(int id);
    void toggleTheme();
    void onCardClicked(QString id);

private:
    void setupUI();
    void setupStyle();
    void updateStyle(); // 负责刷新 QSS

    QWidget* createDashboardPage();
    QWidget* createHistoryPage();   // 【重点修改】
    QWidget* createTopologyPage();

    QWidget* createTopStatCard(QString title, QString value, QString icon);

    // --- 核心控件 ---
    QStackedWidget *mainStack;
    QButtonGroup *navGroup;
    QPushButton *btnTheme;

    // --- 仪表盘 ---
    QGridLayout *gridContainer;
    QVector<ContainerCard*> m_cards;
    QVector<NodeData> m_data;
    QLabel *lblAvgTemp;

    // --- 历史页组件 (新增成员变量以便通过 QSS 控制) ---
    QChartView *chartHistoryMain; // 左侧大图
    QChartView *chartPie;         // 右上饼图
    QTableWidget *tableEvents;    // 右下表格

    QTimer *timer;
    bool isDarkMode;

    QSqlDatabase m_db;
    void initDatabase(); // 用于初始化连接
};

#endif // MAINWINDOW_H
