#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setupStyle();

    // 去掉标题栏，设置为无边框窗口
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    this->setAttribute(Qt::WA_TranslucentBackground); // 允许圆角透明
    this->resize(460, 340); // 稍微加宽一点，防止字排不下
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::setupUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 背景容器
    QFrame *bgFrame = new QFrame;
    bgFrame->setObjectName("LoginFrame");

    // 阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 150));
    shadow->setOffset(0, 0);
    bgFrame->setGraphicsEffect(shadow);

    QVBoxLayout *contentLayout = new QVBoxLayout(bgFrame);
    contentLayout->setContentsMargins(40, 40, 40, 40);
    contentLayout->setSpacing(20);

    // --- 1. 标题部分 (这里修改了) ---
    QLabel *lblTitle = new QLabel("🔐 冷藏集装箱监测系统登录");
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setObjectName("LoginTitle");

    QLabel *lblSub = new QLabel("REEFER CONTAINER MONITORING SYSTEM");
    lblSub->setAlignment(Qt::AlignCenter);
    // 稍微调小一点英文字体，显得精致
    lblSub->setStyleSheet("color: #888; font-size: 9px; font-weight: bold; letter-spacing: 1px;");

    contentLayout->addWidget(lblTitle);
    contentLayout->addWidget(lblSub);
    contentLayout->addSpacing(15);

    // --- 2. 输入框 ---
    txtUser = new QLineEdit();
    txtUser->setPlaceholderText("请输入用户名 (root)");
    txtUser->setObjectName("LoginInput");

    txtPass = new QLineEdit();
    txtPass->setPlaceholderText("请输入密码 (123456)");
    txtPass->setEchoMode(QLineEdit::Password);
    txtPass->setObjectName("LoginInput");

    contentLayout->addWidget(txtUser);
    contentLayout->addWidget(txtPass);
    contentLayout->addSpacing(10);

    // --- 3. 按钮 ---
    btnLogin = new QPushButton("立即登录");
    btnLogin->setObjectName("BtnLogin");
    btnLogin->setCursor(Qt::PointingHandCursor);
    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);

    btnExit = new QPushButton("退出系统");
    btnExit->setObjectName("BtnExit");
    btnExit->setCursor(Qt::PointingHandCursor);
    connect(btnExit, &QPushButton::clicked, this, &LoginDialog::onCancelClicked);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(btnExit);
    btnLayout->addWidget(btnLogin);

    contentLayout->addLayout(btnLayout);
    mainLayout->addWidget(bgFrame);
}

void LoginDialog::onLoginClicked()
{
    QString user = txtUser->text().trimmed();
    QString pass = txtPass->text().trimmed();

    if (user == "root" && pass == "123456") {
        this->accept();
    } else {
        QMessageBox msg;
        msg.setWindowTitle("错误");
        msg.setText("账号或密码错误！");
        msg.setIcon(QMessageBox::Warning);
        // 给这个弹窗也加点样式，保持风格统一
        msg.setStyleSheet("QMessageBox { background-color: #1f2a3a; color: white; } QPushButton { color: white; background-color: #ff4d4f; padding: 5px 15px; border:none; border-radius:3px; }");
        msg.exec();

        txtPass->clear();
        txtPass->setFocus();
    }
}

void LoginDialog::onCancelClicked()
{
    this->reject();
}

void LoginDialog::setupStyle()
{
    // 样式表 (稍微调整了标题字号，适应更长的文字)
    this->setStyleSheet(R"(
        #LoginFrame {
            background-color: #161b22;
            border-radius: 12px;
            border: 1px solid #30363d;
        }
        #LoginTitle {
            color: #ffffff;
            font-size: 22px; /* 稍微调小一点，防止文字太长换行 */
            font-weight: bold;
            font-family: "Microsoft YaHei", "Segoe UI";
        }
        #LoginInput {
            background-color: #0d1117;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 10px;
            color: #c9d1d9;
            font-size: 14px;
        }
        #LoginInput:focus {
            border: 1px solid #1890ff;
            background-color: #1c2128;
        }
        #BtnLogin {
            background-color: #1f6feb;
            color: white;
            border: none;
            padding: 10px;
            border-radius: 6px;
            font-weight: bold;
            font-size: 14px;
        }
        #BtnLogin:hover { background-color: #388bfd; }
        #BtnLogin:pressed { background-color: #1557b0; }

        #BtnExit {
            background-color: transparent;
            color: #8b949e;
            border: 1px solid #30363d;
            padding: 10px;
            border-radius: 6px;
        }
        #BtnExit:hover { color: #ff4d4f; border-color: #ff4d4f; }
    )");
}
