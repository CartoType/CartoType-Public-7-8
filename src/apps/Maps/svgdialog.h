#ifndef SVGDIALOG_H
#define SVGDIALOG_H

#include <QDialog>
#include "styleitem.h"
#include <cartotype_framework.h>

namespace Ui
{
class SvgDialog;
}

class SvgDialog: public QDialog
    {
    Q_OBJECT

    public:
    explicit SvgDialog(QWidget& aParent,CartoType::CFramework& aFramework,CStyleItem& aStyleItem);
    ~SvgDialog();
    std::string SvgText();

    private slots:
    void on_updateButton_clicked();
    void on_pickColorButton_clicked();
    void on_svgText_selectionChanged();
    void on_undoButton_clicked();
    void on_redoButton_clicked();

    private:
    void DrawSvgImage();

    CartoType::CFramework& m_framework;
    Ui::SvgDialog* m_ui;
    CStyleItem& m_style_item;
    };

#endif // SVGDIALOG_H
