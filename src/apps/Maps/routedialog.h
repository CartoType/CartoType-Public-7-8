#ifndef ROUTEDIALOG_H
#define ROUTEDIALOG_H

#include <QDialog>
#include <cartotype_framework.h>

class QListWidgetItem;

namespace Ui
{
class RouteDialog;
}

class RouteDialog: public QDialog
    {
    Q_OBJECT

    public:
    explicit RouteDialog(QWidget& aParent,CartoType::CFramework& aFramework,const std::string& aPushPinColor,uint16_t aPushPinGlyph);
    ~RouteDialog();
    void EnablePushPinEditing();
    const std::string& PushPinColor() const { return m_pushpin_color; }
    uint16_t PushPinGlyph() const { return m_pushpin_glyph; }

    Ui::RouteDialog* m_ui;

    private slots:
    void on_setStartOfRoute_clicked();
    void on_setEndOfRoute_clicked();
    void on_addPushPin_clicked();
    void on_cutPushPin_clicked();
    void on_pushPinColor_clicked();
    void on_pushPinGlyph_currentItemChanged(QListWidgetItem* aCurrent,QListWidgetItem* aPrevious);

    private:
    void PopulateGlyphList();

    CartoType::CFramework& m_framework;
    std::string m_pushpin_color;
    uint16_t m_pushpin_glyph = 0;
    };

#endif // ROUTEDIALOG_H
