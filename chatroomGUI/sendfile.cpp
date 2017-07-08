#include "sendfile.h"
#include "ui_sendfile.h"
#include <QMessageBox>

SendFile::SendFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendFile)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
}

SendFile::~SendFile()
{
 //   QMessageBox::warning(NULL, "warning", "Content", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    delete ui;
}

void SendFile::mySlot(QString qstr)
{
    ui->filename->setText(qstr);
    ui->filename->adjustSize();
}
