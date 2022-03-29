#ifndef ATTRIBUTEDIALOG_H
#define ATTRIBUTEDIALOG_H

#include <QDialog>
#include <QLineEdit>

namespace Ui
{
class AttributeDialog;
}

class AttributeDialog: public QDialog
    {
    Q_OBJECT

    public:
    explicit AttributeDialog(QWidget& aParent,const std::string& aElementName,const std::map<std::string,std::string>& aAttributes);
    ~AttributeDialog();
    std::map<std::string,std::string> Attributes();

    private slots:
    void on_text01_selectionChanged();
    void on_text02_selectionChanged();
    void on_text03_selectionChanged();
    void on_text04_selectionChanged();
    void on_text05_selectionChanged();
    void on_text06_selectionChanged();
    void on_text07_selectionChanged();
    void on_text08_selectionChanged();
    void on_text09_selectionChanged();
    void on_text10_selectionChanged();
    void on_text11_selectionChanged();
    void on_text12_selectionChanged();
    void on_text13_selectionChanged();
    void on_text14_selectionChanged();
    void on_text15_selectionChanged();
    void on_text16_selectionChanged();
    void on_pickColorButton_clicked();

    private:
    void OnSelectionChanged(QLineEdit* aLineEdit);

    Ui::AttributeDialog* m_ui;
    std::vector<std::pair<QLineEdit*,QLineEdit*>> m_form_row;
    QLineEdit* m_color_line_edit = nullptr; // the line edit field in use by the color picker
    };

#endif // ATTRIBUTEDIALOG_H
