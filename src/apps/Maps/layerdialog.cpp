#include "layerdialog.h"
#include "ui_layerdialog.h"

LayerDialog::LayerDialog(QWidget *aParent) :
    QDialog(aParent),
    m_ui(new Ui::LayerDialog)
    {
    m_ui->setupUi(this);
    }

LayerDialog::~LayerDialog()
    {
    delete m_ui;
    }
