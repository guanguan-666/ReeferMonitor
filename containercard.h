#ifndef CONTAINERCARD_H
#define CONTAINERCARD_H

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent> // 引入鼠标事件

class ContainerCard : public QFrame
{
    Q_OBJECT
public:
    explicit ContainerCard(QString id, QWidget *parent = nullptr);
    void updateData(double temp, double superheat, int rssi, bool isOnline, bool pidRunning);

signals:
    void clicked(QString id); // 新增点击信号

protected:
    // 覆写鼠标点击事件
    void mousePressEvent(QMouseEvent *event) override;

private:
    // UI 元素
    QLabel *lblID;
    QLabel *lblSignalIcon;
    QLabel *lblSignalVal;
    QLabel *lblTempTitle;
    QLabel *lblTempVal;
    QLabel *lblHeatTitle;
    QLabel *lblHeatVal;
    QLabel *lblPidStatus;
    QLabel *lblValveVal;

    void setupUI();
};

#endif // CONTAINERCARD_H
