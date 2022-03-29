#include "locationdialog.h"
#include "ui_locationdialog.h"

LocationDialog::LocationDialog(QWidget* aParent):
    QDialog(aParent),
    m_ui(new Ui::LocationDialog)
    {
    m_ui->setupUi(this);
    }

LocationDialog::~LocationDialog()
    {
    delete m_ui;
    }
