#ifndef LOCATIONDIALOG_H
#define LOCATIONDIALOG_H

#include <QDialog>

namespace Ui
    {
    class LocationDialog;
    }

class LocationDialog: public QDialog
    {
    Q_OBJECT

    public:
    explicit LocationDialog(QWidget* aParent = 0);
    ~LocationDialog();

    Ui::LocationDialog* m_ui;
    };

#endif // LOCATIONDIALOG_H
