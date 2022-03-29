#include "scaledialog.h"
#include "ui_scaledialog.h"

ScaleDialog::ScaleDialog(QWidget* aParent):
    QDialog(aParent),
    m_ui(new Ui::ScaleDialog)
    {
    m_ui->setupUi(this);
    }

ScaleDialog::~ScaleDialog()
    {
    delete m_ui;
    }
