#include "mainwindow.h"
#include "logindialog.h" // 引用登录窗口
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 先创建登录窗口
    LoginDialog login;

    // 2. 以模态方式运行 (exec 会阻塞，直到用户点击登录或退出)
    if (login.exec() == QDialog::Accepted) {
        // 3. 如果验证通过 (Accept)，才显示主窗口
        MainWindow w;
        w.show();
        return a.exec(); // 进入主程序循环
    } else {
        // 4. 如果点击取消或关闭，直接退出程序
        return 0;
    }
}
