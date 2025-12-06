#ifndef CONTAINERDETAILDIALOG_H
#define CONTAINERDETAILDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QtCharts>

using namespace QtCharts;

class ContainerDetailDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ContainerDetailDialog(QString containerId, QWidget *parent = nullptr);

// 【修改点1】新增接收数据的槽函数
public slots:
    void updateRealtimeData(QString id, double temp, double pressure, double superheat, int valve, int rssi);

private:
    void setupUI(QString id);
    void setupStyle();

    QFrame* createDetailCard(QString title, QLabel* &valLabel, QString unit, QString colorHex);

    // 【修改点2】将需要更新的 Label 变为成员变量
    QLabel *valTemp;
    QLabel *valPress;
    QLabel *valSuperheat;
    QLabel *valValve;
    QLabel *valRssi;

    QChart *chart;
    QLineSeries *seriesTemp;
    QLineSeries *seriesSuperheat;

    QString currentId; // 记录当前弹窗的ID，用于过滤信号
    int timeCounter;   // 曲线图的时间轴计数
};

#endif // CONTAINERDETAILDIALOG_H
