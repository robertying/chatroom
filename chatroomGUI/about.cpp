#include "stdafx.h"
#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
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
