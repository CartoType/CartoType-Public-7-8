// FindDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CartoTypeDemo.h"
#include "FindDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFindTextDialog::CFindTextDialog():
    CDialog(CFindTextDialog::IDD,nullptr),
    iMapObjectGroupArray(std::make_unique<CartoType::CMapObjectGroupArray>())
    {
    iFindParam.iMaxObjectCount = 64;
    iFindParam.iAttributes = "$,name:*,ref,alt_name,int_name,addr:housename,pco";
    iFindParam.iCondition = "OsmType!='bsp'"; // exclude bus stops
    iFindParam.iStringMatchMethod = CartoType::TStringMatchMethod(CartoType::TStringMatchMethodFlag::Prefix,
                                                                  CartoType::TStringMatchMethodFlag::FoldAccents,
                                                                  CartoType::TStringMatchMethodFlag::FoldCase,
                                                                  CartoType::TStringMatchMethodFlag::IgnoreSymbols,
                                                                  CartoType::TStringMatchMethodFlag::IgnoreWhitespace);
    iFindParam.iTimeOut = 2;
    }

void CFindTextDialog::Set(CartoType::CFramework* aFramework)
    {
    std::lock_guard<std::mutex> lock(iMapObjectGroupArrayMutex);
    iFramework = aFramework;
    iMapObjectGroupArray->clear();
    CartoType::TRectFP view;
    iFramework->GetView(view,CartoType::TCoordType::Map);
    iFindParam.iLocation = CartoType::CGeometry(view,CartoType::TCoordType::Map);
    }

CartoType::CMapObjectArray CFindTextDialog::FoundObjectArray()
    {
    CartoType::CMapObjectArray a;

    std::lock_guard<std::mutex> lock(iMapObjectGroupArrayMutex);
    if (iListBoxIndex >= 0 && size_t(iListBoxIndex) < iMapObjectGroupArray->size())
        a = std::move((*iMapObjectGroupArray)[iListBoxIndex].iMapObjectArray);

    else
        {
        for (auto& p : *iMapObjectGroupArray)
            {
            for (auto& o : p.iMapObjectArray)
                a.push_back(std::move(o));
            }
        }

    iMapObjectGroupArray->clear();
    return a;
    }

void CFindTextDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFindTextDialog)
    DDX_Text(pDX,IDC_FIND_TEXT,iFindText);
    DDX_Check(pDX,IDC_FIND_SYMBOLS,iSymbols);
    DDX_Check(pDX,IDC_FIND_FUZZY,iFuzzy);
    //}}AFX_DATA_MAP
    }

BEGIN_MESSAGE_MAP(CFindTextDialog,CDialog)
    ON_CBN_EDITCHANGE(IDC_FIND_TEXT,OnEditChange)
    ON_CBN_DBLCLK(IDC_FIND_TEXT,OnComboBoxDoubleClick)
    ON_CBN_SELCHANGE(IDC_FIND_TEXT,OnComboBoxSelChange)
END_MESSAGE_MAP()

BOOL CFindTextDialog::OnInitDialog()
    {
    CComboBox* cb = (CComboBox*)GetDlgItem(IDC_FIND_TEXT);
    cb->SetHorizontalExtent(400);
    UpdateData(0);
    PopulateComboBox();
    return true;
    }

void CFindTextDialog::OnEditChange()
    {
    PopulateComboBox();
    }

void CFindTextDialog::OnComboBoxDoubleClick()
    {
    UpdateMatch();
    EndDialog(IDOK);
    }

void CFindTextDialog::OnComboBoxSelChange()
    {
    UpdateMatch();
    }

void CFindTextDialog::PopulateComboBox()
    {
    // Copy the text and button states into the member variables.
    UpdateData(1);

    if (iFindText.IsEmpty())
        {
        std::unique_lock<std::mutex> lock(iMapObjectGroupArrayMutex);
        iMapObjectGroupArray->clear();
        lock.unlock();
        SetComboboxStrings();
        return;
        }

    // Get the current text.
    CartoType::CString text;
    SetString(text,iFindText);

    // Find up to 64 items starting with the current text.
    CartoType::TPointOfInterestType poi = CartoType::TPointOfInterestType::None;
    if (text == "airport") poi = CartoType::TPointOfInterestType::Airport;
    else if (text == "bar") poi = CartoType::TPointOfInterestType::Bar;
    else if (text == "beach") poi = CartoType::TPointOfInterestType::Beach;
    else if (text == "bus") poi = CartoType::TPointOfInterestType::BusStation;
    else if (text == "cafe") poi = CartoType::TPointOfInterestType::Cafe;
    else if (text == "camping") poi = CartoType::TPointOfInterestType::Camping;
    else if (text == "fastfood") poi = CartoType::TPointOfInterestType::FastFood;
    else if (text == "fuel") poi = CartoType::TPointOfInterestType::Fuel;
    else if (text == "golf") poi = CartoType::TPointOfInterestType::GolfCourse;
    else if (text == "hospital") poi = CartoType::TPointOfInterestType::Hospital;
    else if (text == "hotel") poi = CartoType::TPointOfInterestType::Hotel;
    else if (text == "pharmacy") poi = CartoType::TPointOfInterestType::Pharmacy;
    else if (text == "police") poi = CartoType::TPointOfInterestType::Police;
    else if (text == "restaurant") poi = CartoType::TPointOfInterestType::Restaurant;
    else if (text == "shops") poi = CartoType::TPointOfInterestType::Shops;
    else if (text == "sport") poi = CartoType::TPointOfInterestType::SportsCenter;
    else if (text == "supermarket") poi = CartoType::TPointOfInterestType::Supermarket;
    else if (text == "swim") poi = CartoType::TPointOfInterestType::SwimmingPool;
    else if (text == "tourism") poi = CartoType::TPointOfInterestType::Tourism;
    else if (text == "train") poi = CartoType::TPointOfInterestType::TrainStation;

    auto find_callback = [this](std::unique_ptr<CartoType::CMapObjectGroupArray> aMapObjectGroupArray)
        {
        std::unique_lock<std::mutex> lock(iMapObjectGroupArrayMutex);
        iMapObjectGroupArray = std::move(aMapObjectGroupArray);
        lock.unlock();
        SetComboboxStrings();
        };

    if (poi != CartoType::TPointOfInterestType::None)
        {
        CartoType::TFindNearbyParam param;
        param.iType = poi;
        param.iLocation = iFindParam.iLocation;
        iFramework->FindAsync(find_callback,param,true);
        }
    else
        {
        CartoType::TFindParam param(iFindParam);
        param.iText = text;
        if (iFuzzy)
            param.iStringMatchMethod = CartoType::TStringMatchMethod::Fuzzy;
        if (iSymbols)
            param.iStringMatchMethod -= CartoType::TStringMatchMethodFlag::IgnoreSymbols;
        iFramework->FindAsync(find_callback,param,true);
        }

    // Put them in the combo box.
    SetComboboxStrings();
    }

void CFindTextDialog::SetComboboxStrings()
    {
    // The window handle is null if this is called from the find callback after the dialog window has closed.
    if (!m_hWnd)
        return;

    /*
    Note on locks: not trying the lock, but setting it unconditionally, causes a deadlock; I don't know why.
    It may be because the find thread is blocked in a wait().
    */
    std::unique_lock<std::mutex> lock(iMapObjectGroupArrayMutex,std::defer_lock);
    if (!lock.try_lock())
        return;

    CComboBox* cb = (CComboBox*)GetDlgItem(IDC_FIND_TEXT);
    for (int i = cb->GetCount(); i >= 0; i--)
        cb->DeleteString(i);
    for (const auto& cur_group : *iMapObjectGroupArray)
        {
        CString s;
        SetString(s,cur_group.iName);
        cb->AddString(s);
        }
    }

void CFindTextDialog::UpdateMatch()
    {
    CComboBox* cb = (CComboBox*)GetDlgItem(IDC_FIND_TEXT);
    iListBoxIndex = cb->GetCurSel();
    }

CFindAddressDialog::CFindAddressDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CFindAddressDialog::IDD,pParent)
    {
    //{{AFX_DATA_INIT(CFindAddressDialog)
    iBuilding = _T("");
    iFeature = _T("");
    iStreet = _T("");
    iSubLocality = _T("");
    iLocality = _T("");
    iSubAdminArea = _T("");
    iAdminArea = _T("");
    iCountry = _T("");
    iPostCode = _T("");
    //}}AFX_DATA_INIT
    }

void CFindAddressDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFindAddressDialog)
    DDX_Text(pDX,IDC_FIND_BUILDING,iBuilding);
    DDX_Text(pDX,IDC_FIND_FEATURE,iFeature);
    DDX_Text(pDX,IDC_FIND_STREET,iStreet);
    DDX_Text(pDX,IDC_FIND_SUBLOCALITY,iSubLocality);
    DDX_Text(pDX,IDC_FIND_LOCALITY,iLocality);
    DDX_Text(pDX,IDC_FIND_SUBADMINAREA,iSubAdminArea);
    DDX_Text(pDX,IDC_FIND_ADMINAREA,iAdminArea);
    DDX_Text(pDX,IDC_FIND_COUNTRY,iCountry);
    DDX_Text(pDX,IDC_FIND_POSTCODE,iPostCode);
    //}}AFX_DATA_MAP
    }

BEGIN_MESSAGE_MAP(CFindAddressDialog,CDialog)
    //{{AFX_MSG_MAP(CFindAddressDialog)
    // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
