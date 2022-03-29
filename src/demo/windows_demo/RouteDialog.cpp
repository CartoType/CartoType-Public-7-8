// RouteDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CartoTypeDemo.h"
#include "RouteDialog.h"

IMPLEMENT_DYNAMIC(CRouteDialog,CDialog)

CRouteDialog::CRouteDialog(bool aPushPinSelected,CWnd* pParent /*=NULL*/) :
CDialog(CRouteDialog::IDD,pParent),
iPushPinSelected(aPushPinSelected)
    {
    iAddress= _T("");
    iPushPinName = _T("");
    iPushPinDesc = _T("");
    iHeading = _T("");
    iButtonIndex = 0;
    iUseAsWayPoint = 0;
    iBestRoute = 0;
    }

CRouteDialog::~CRouteDialog()
    {
    }

BOOL CRouteDialog::OnInitDialog()
    {
    CDialog::OnInitDialog();

    if (iPushPinSelected)
        {
        CWnd* add_button = GetDlgItem(IDC_ADD_PUSHPIN);
        add_button->SetWindowText(_T("Edit pushpin"));
        }
    else
        {
        CWnd* cut_button = GetDlgItem(IDC_CUT_PUSHPIN);
        cut_button->ShowWindow(SW_HIDE);
        }
    ShowOrHideFields();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
    }

void CRouteDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX,IDC_ROUTE_ADDRESS,iAddress);
    DDX_Text(pDX,IDC_PUSHPIN_NAME,iPushPinName);
    DDX_Text(pDX,IDC_PUSHPIN_DESC,iPushPinDesc);
    DDX_Text(pDX,IDC_HEADING,iHeading);
    DDX_Radio(pDX,IDC_START_OF_ROUTE,iButtonIndex);
    DDX_Check(pDX,IDC_USE_AS_WAYPOINT,iUseAsWayPoint);
    DDX_Check(pDX,IDC_BEST_ROUTE,iBestRoute);
    }


BEGIN_MESSAGE_MAP(CRouteDialog,CDialog)
    ON_BN_CLICKED(IDC_START_OF_ROUTE,&CRouteDialog::OnRadioButtonClicked)
    ON_BN_CLICKED(IDC_END_OF_ROUTE,&CRouteDialog::OnRadioButtonClicked)
    ON_BN_CLICKED(IDC_ADD_PUSHPIN,&CRouteDialog::OnRadioButtonClicked)
    ON_BN_CLICKED(IDC_CUT_PUSHPIN,&CRouteDialog::OnRadioButtonClicked)
END_MESSAGE_MAP()

void CRouteDialog::OnRadioButtonClicked()
    {
    ShowOrHideFields();
    }

void CRouteDialog::ShowOrHideFields()
    {
    int b = GetCheckedRadioButton(IDC_START_OF_ROUTE,IDC_CUT_PUSHPIN);
    bool routing = b == IDC_START_OF_ROUTE || b == IDC_END_OF_ROUTE;
    GetDlgItem(IDC_PUSHPIN_NAME)->EnableWindow(routing ? false : true);
    GetDlgItem(IDC_PUSHPIN_DESC)->EnableWindow(routing ? false : true);
    GetDlgItem(IDC_STATIC_PUSHPIN_NAME)->EnableWindow(routing ? false : true);
    GetDlgItem(IDC_STATIC_PUSHPIN_DESC)->EnableWindow(routing ? false : true);
    GetDlgItem(IDC_USE_AS_WAYPOINT)->EnableWindow(routing ? false : true);
    }

