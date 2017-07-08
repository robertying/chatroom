#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "sendfile.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool createBool;
    SendFile *dlg;

signals:
    void changeName(QString qstr);

protected:
    bool eventFilter(QObject *target, QEvent *event);
    void closeEvent(QCloseEvent* event);

private slots:
    void on_action_about_triggered();
    void ClientStart();
    void on_pushButton_released();
    void ReadFile();
    void on_textEdit_textChanged();

    void on_action_L_triggered();

    void on_fileButton_released();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
