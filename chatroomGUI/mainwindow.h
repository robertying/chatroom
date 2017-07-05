#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
