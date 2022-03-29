#include "finddialog.h"
#include "ui_finddialog.h"
#include <QKeyEvent>

FindDialog::FindDialog(QWidget* aParent):
    QDialog(aParent),
    m_ui(new Ui::FindDialog)
    {
    m_ui->setupUi(this);
    m_ui->findText->setFocus();
    m_find_param.iMaxObjectCount = 64;
    m_find_param.iAttributes = "$,name:*,ref,alt_name,int_name,addr:housename,pco";
    m_find_param.iCondition = "OsmType!='bsp'"; // exclude bus stops
    m_find_param.iStringMatchMethod = CartoType::TStringMatchMethod(CartoType::TStringMatchMethodFlag::Prefix,
                                                                    CartoType::TStringMatchMethodFlag::FoldAccents,
                                                                    CartoType::TStringMatchMethodFlag::FoldCase,
                                                                    CartoType::TStringMatchMethodFlag::IgnoreWhitespace,
                                                                    CartoType::TStringMatchMethodFlag::IgnoreSymbols);
    m_find_param.iTimeOut = 1.0; // search for up to one second
    }

void FindDialog::Set(CartoType::CFramework& aFramework)
    {
    m_framework = &aFramework;
    m_list_box_index = -1;
    m_lock = 0;

    // Find items in or near the view by preference.
    CartoType::TRectFP view;
    m_framework->GetView(view,CartoType::TCoordType::Map);
    m_find_param.iLocation = CartoType::CGeometry(view,CartoType::TCoordType::Map);

    // Install an event filter to intercept up and down arrow events and use them to move between the line editor and the list box.
    m_ui->findText->installEventFilter(this);
    m_ui->findList->installEventFilter(this);

    PopulateList(m_ui->findText->text());
    }

FindDialog::~FindDialog()
    {
    delete m_ui;
    }

CartoType::CMapObjectArray FindDialog::FoundObjectArray()
    {
    if (size_t(m_list_box_index) < m_map_object_group_array->size())
        return std::move((*m_map_object_group_array)[m_list_box_index].iMapObjectArray);

    CartoType::CMapObjectArray a;
    for (auto& p : *m_map_object_group_array)
        {
        for (auto& o : p.iMapObjectArray)
            a.push_back(std::move(o));
        }
    return a;
    }

void FindDialog::on_findText_textChanged(const QString& aText)
    {
    if (m_lock)
        return;
    m_lock++;
    PopulateList(aText);
    UpdateMatch();
    m_lock--;
    }

bool FindDialog::eventFilter(QObject* aWatched,QEvent* aEvent)
    {
    if ((aWatched == m_ui->findText || aWatched == m_ui->findList) &&
        aEvent->type() == QEvent::KeyPress)
        {
        auto key_event = static_cast<const QKeyEvent*>(aEvent);
        if (aWatched == m_ui->findText)
            {
            if (key_event->key() == Qt::Key_Down && m_ui->findList->count() > 0)
                {
                m_ui->findList->setFocus();
                return true;
                }
            }
        else
            {
            if (key_event->key() == Qt::Key_Up)
                {
                QModelIndexList index_list { m_ui->findList->selectionModel()->selectedIndexes() };
                if (!index_list.size() || index_list.cbegin()->row() == 0)
                    {
                    m_ui->findText->setFocus();
                    return true;
                    }
                }
            }
        }
    return false;
    }

void FindDialog::PopulateList(const QString& aText)
    {
    if (aText.isEmpty())
        {
        std::unique_lock<std::mutex> lock(m_map_object_group_array_mutex);
        m_map_object_group_array->clear();
        lock.unlock();
        SetListStrings();
        return;
        }

    CartoType::CString text;
    text.Set(aText.utf16());

    auto find_callback = [this](std::unique_ptr<CartoType::CMapObjectGroupArray> aMapObjectGroupArray)
        {
        std::unique_lock<std::mutex> lock(m_map_object_group_array_mutex);
        m_map_object_group_array = std::move(aMapObjectGroupArray);
        lock.unlock();
        SetListStrings();
        };

    // Find up to 64 items starting with the current text.
    CartoType::TFindParam param { m_find_param };
    param.iText = text;
    if (m_ui->fuzzyMatch->isChecked())
        param.iStringMatchMethod = CartoType::TStringMatchMethod::Fuzzy;
    if (m_ui->symbols->isChecked())
        param.iStringMatchMethod -= CartoType::TStringMatchMethodFlag::IgnoreSymbols;

    if (text.Length())
        m_framework->FindAsync(find_callback,param,true);
    else
        m_map_object_group_array->clear();

    // Put them in the list.
    SetListStrings();
    }

void FindDialog::SetListStrings()
    {
    /*
    Note on locks: not trying the lock, but setting it unconditionally, causes a deadlock; I don't know why.
    It may be because the find thread is blocked in a wait().
    */
    std::unique_lock<std::mutex> lock(m_map_object_group_array_mutex,std::defer_lock);
    if (!lock.try_lock())
        return;

    m_ui->findList->clear();
    for (const auto& cur_group : *m_map_object_group_array)
        {
        QString qs;
        qs.setUtf16(cur_group.iName.Text(),(int)cur_group.iName.Length());
        m_ui->findList->addItem(qs);
        }
    }

void FindDialog::UpdateMatch()
    {
    m_list_box_index = m_ui->findList->currentRow();
    }

void FindDialog::on_findList_currentTextChanged(const QString& aCurrentText)
    {
    if (m_lock)
        return;
    m_lock++;
    m_ui->findText->setText(aCurrentText);
    UpdateMatch();
    m_lock--;
    }

void FindDialog::on_fuzzyMatch_stateChanged(int /*aCheckState*/)
    {
    PopulateList(m_ui->findText->text());
    }

void FindDialog::on_symbols_stateChanged(int /*aCheckState*/)
    {
    PopulateList(m_ui->findText->text());
    }

void FindDialog::on_findList_itemDoubleClicked(QListWidgetItem* /*aItem*/)
    {
    accept();
    }
