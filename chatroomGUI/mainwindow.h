//main window
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
    //enable press enter to send messages
    bool eventFilter(QObject *target, QEvent *event);

    //pop-up confirm question box before closing
    void closeEvent(QCloseEvent* event);

private slots:
    //open about window
    void on_action_about_triggered();
    //send messages when release "send" button
    void on_pushButton_released();
    //used to limit the maximum of messages in the text editor
    void on_textEdit_textChanged();
    //open log file
    void on_action_L_triggered();

    //start client and deal with disconnection
    void ClientStart();

    //read local log to present messages
    void ReadFile();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
