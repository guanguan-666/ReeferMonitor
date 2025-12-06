#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void onLoginClicked();
    void onCancelClicked();

private:
    void setupUI();
    void setupStyle();

    QLineEdit *txtUser;
    QLineEdit *txtPass;
    QPushButton *btnLogin;
    QPushButton *btnExit;
};

#endif // LOGINDIALOG_H
