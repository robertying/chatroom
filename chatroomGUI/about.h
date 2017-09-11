//about window
#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = 0);
    ~About();

private slots:
    //click to show
    void on_pushButton_clicked();

private:
    Ui::About *ui;
};

#endif // ABOUT_H
