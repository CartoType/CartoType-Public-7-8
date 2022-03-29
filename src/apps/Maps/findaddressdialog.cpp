#include "findaddressdialog.h"
#include "ui_findaddressdialog.h"

FindAddressDialog::FindAddressDialog(QWidget* aParent):
    QDialog(aParent),
    m_ui(new Ui::FindAddressDialog)
    {
    m_ui->setupUi(this);
    }

FindAddressDialog::~FindAddressDialog()
    {
    delete m_ui;
    }
