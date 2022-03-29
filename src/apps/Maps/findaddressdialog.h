#ifndef FINDADDRESSDIALOG_H
#define FINDADDRESSDIALOG_H

#include <QDialog>

namespace Ui {
class FindAddressDialog;
}

class FindAddressDialog : public QDialog
    {
    Q_OBJECT

    public:
    explicit FindAddressDialog(QWidget* aParent = 0);
    ~FindAddressDialog();

    Ui::FindAddressDialog* m_ui;
    };

#endif // FINDADDRESSDIALOG_H
