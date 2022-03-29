#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <cartotype_framework.h>
#include <mutex>

namespace Ui
    {
    class FindDialog;
    }
class QListWidgetItem;

class FindDialog: public QDialog
    {
    Q_OBJECT

    public:
    explicit FindDialog(QWidget* aParent);
    ~FindDialog();
    void Set(CartoType::CFramework& aFramework);
    CartoType::CMapObjectArray FoundObjectArray();

    Ui::FindDialog* m_ui;

    private slots:
    void on_findText_textChanged(const QString& aText);
    void on_findList_currentTextChanged(const QString& aCurrentText);
    void on_fuzzyMatch_stateChanged(int aCheckState);
    void on_symbols_stateChanged(int aCheckState);
    void on_findList_itemDoubleClicked(QListWidgetItem* aItem);

    private:
    void PopulateList(const QString& aText);
    void SetListStrings();
    void UpdateMatch();
    bool eventFilter(QObject* aWatched,QEvent* aEvent) override;

    CartoType::CFramework* m_framework = nullptr;
    CartoType::TFindParam m_find_param;
    std::unique_ptr<CartoType::CMapObjectGroupArray> m_map_object_group_array = std::make_unique<CartoType::CMapObjectGroupArray>();
    std::mutex m_map_object_group_array_mutex;
    int m_list_box_index = -1;
    int m_lock = 0;
    };

#endif // FINDDIALOG_H
