#include "routeinstructionsdialog.h"
#include "ui_routeinstructionsdialog.h"

RouteInstructionsDialog::RouteInstructionsDialog(QWidget *aParent):
    QDialog(aParent),
    m_ui(new Ui::RouteInstructionsDialog)
    {
    m_ui->setupUi(this);
    }

RouteInstructionsDialog::~RouteInstructionsDialog()
    {
    delete m_ui;
    }
