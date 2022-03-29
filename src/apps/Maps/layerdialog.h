#ifndef LAYERDIALOG_H
#define LAYERDIALOG_H

#include <QDialog>

namespace Ui
    {
    class LayerDialog;
    }

class LayerDialog: public QDialog
    {
    Q_OBJECT

    public:
    explicit LayerDialog(QWidget* aParent = 0);
    ~LayerDialog();

    Ui::LayerDialog* m_ui;
    };

#endif // LAYERDIALOG_H
