#ifndef LINESTYLEDIALOG_H
#define LINESTYLEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <cartotype_framework.h>
#include "styleitem.h"

namespace Ui
{
class LineStyleDialog;
}

class LineStyleDialog : public QDialog
    {
    Q_OBJECT

    public:
    LineStyleDialog(QWidget& aParent,CStyleItem& aStyleItem,CartoType::CLegend& aLegend);
    ~LineStyleDialog();
    void DrawLineSamples();
    void DrawLineSample(QLabel& aText,QLabel& aImage,int aScale);

    Ui::LineStyleDialog* m_ui;
    CStyleItem& m_style_item;
    CartoType::CLegend& m_legend;

    private slots:
    void on_lineColorText_editingFinished();
    void on_lineWidthText_editingFinished();
    void on_borderColorText_editingFinished();
    void on_borderWidthText_editingFinished();
    void on_centerLineColorText_editingFinished();
    void on_centerLineWidthText_editingFinished();
    void on_lineDashText_editingFinished();
    void on_opacityText_editingFinished();
    void on_truncationAtStartText_editingFinished();
    void on_truncationAtEndText_editingFinished();
    void on_distanceText_editingFinished();
    void on_lineColorButton_clicked();
    void on_borderColorButton_clicked();
    void on_centerLineColorButton_clicked();

    private:
    void EditingFinishedHelper(const char* aKey,QLineEdit& aLineEdit);
    void ColorButtonHelper(const char* aKey,QLineEdit& aLineEdit);
    };

#endif // LINESTYLEDIALOG_H
