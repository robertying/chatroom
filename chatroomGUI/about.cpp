#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    //hide the qustion mark beside close button
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::ApplicationModal);
    ui->setupUi(this);
}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
    accept();
}
