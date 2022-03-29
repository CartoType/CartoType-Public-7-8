#ifndef POLYGONSTYLEDIALOG_H
#define POLYGONSTYLEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <cartotype_framework.h>
#include "styleitem.h"

namespace Ui
{
class PolygonStyleDialog;
}

class PolygonStyleDialog: public QDialog
    {
    Q_OBJECT

    public:
    PolygonStyleDialog(QWidget& aParent,CStyleItem& aStyleItem,CartoType::CLegend& aLegend);
    ~PolygonStyleDialog();
    void DrawPolygonSample();

    Ui::PolygonStyleDialog* m_ui;
    CStyleItem& m_style_item;
    CartoType::CLegend& m_legend;

    private slots:
    void on_fillColorText_editingFinished();
    void on_borderColorText_editingFinished();
    void on_borderWidthText_editingFinished();
    void on_opacityText_editingFinished();
    void on_dashArrayText_editingFinished();
    void on_drawHeightCheckBox_stateChanged(int aState);
    void on_fillColorButton_clicked();
    void on_borderColorButton_clicked();

    private:
    void EditingFinishedHelper(const char* aKey,QLineEdit& aLineEdit);
    void ColorButtonHelper(const char* aKey,QLineEdit& aLineEdit);
    };

#endif // POLYGONSTYLEDIALOG_H
