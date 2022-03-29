// LayerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CartoTypeDemo.h"
#include "LayerDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLayerDialog dialog


CLayerDialog::CLayerDialog(CartoType::CFramework& aFramework,CWnd* pParent /*=NULL*/) :
CDialog(CLayerDialog::IDD,pParent),
iFramework(aFramework)
    {
    EnableAutomation();

    //{{AFX_DATA_INIT(CLayerDialog)
    //}}AFX_DATA_INIT
    }


void CLayerDialog::OnFinalRelease()
    {
    // When the last reference for an automation object is released
    // OnFinalRelease is called.  The base class will automatically
    // deletes the object.  Add additional cleanup required for your
    // object before calling the base class.

    CDialog::OnFinalRelease();
    }

void CLayerDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CLayerDialog)
    //}}AFX_DATA_MAP
    DDX_Control(pDX,IDC_CHECK1,iCheck[0]);
    DDX_Control(pDX,IDC_CHECK2,iCheck[1]);
    DDX_Control(pDX,IDC_CHECK3,iCheck[2]);
    DDX_Control(pDX,IDC_CHECK4,iCheck[3]);
    DDX_Control(pDX,IDC_CHECK5,iCheck[4]);
    DDX_Control(pDX,IDC_CHECK6,iCheck[5]);
    DDX_Control(pDX,IDC_CHECK7,iCheck[6]);
    DDX_Control(pDX,IDC_CHECK8,iCheck[7]);
    DDX_Control(pDX,IDC_CHECK9,iCheck[8]);
    DDX_Control(pDX,IDC_CHECK10,iCheck[9]);
    DDX_Control(pDX,IDC_CHECK11,iCheck[10]);
    DDX_Control(pDX,IDC_CHECK12,iCheck[11]);
    DDX_Control(pDX,IDC_CHECK13,iCheck[12]);
    DDX_Control(pDX,IDC_CHECK14,iCheck[13]);
    DDX_Control(pDX,IDC_CHECK15,iCheck[14]);
    DDX_Control(pDX,IDC_CHECK16,iCheck[15]);
    DDX_Control(pDX,IDC_CHECK17,iCheck[16]);
    DDX_Control(pDX,IDC_CHECK18,iCheck[17]);
    DDX_Control(pDX,IDC_CHECK19,iCheck[18]);
    DDX_Control(pDX,IDC_CHECK20,iCheck[19]);
    DDX_Control(pDX,IDC_CHECK21,iCheck[20]);
    DDX_Control(pDX,IDC_CHECK22,iCheck[21]);
    DDX_Control(pDX,IDC_CHECK23,iCheck[22]);
    DDX_Control(pDX,IDC_CHECK24,iCheck[23]);
    DDX_Control(pDX,IDC_CHECK25,iCheck[24]);
    DDX_Control(pDX,IDC_CHECK26,iCheck[25]);
    DDX_Control(pDX,IDC_CHECK27,iCheck[26]);
    DDX_Control(pDX,IDC_CHECK28,iCheck[27]);
    DDX_Control(pDX,IDC_CHECK29,iCheck[28]);
    DDX_Control(pDX,IDC_CHECK30,iCheck[29]);
    DDX_Control(pDX,IDC_CHECK31,iCheck[30]);
    DDX_Control(pDX,IDC_CHECK32,iCheck[31]);
    }


BEGIN_MESSAGE_MAP(CLayerDialog,CDialog)
    //{{AFX_MSG_MAP(CLayerDialog)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CLayerDialog,CDialog)
    //{{AFX_DISPATCH_MAP(CLayerDialog)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_ILayerDialog to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {35690769-F374-4198-9935-52A3E770ADFC}
static const IID IID_ILayerDialog =
    { 0x35690769,0xf374,0x4198,{ 0x99,0x35,0x52,0xa3,0xe7,0x70,0xad,0xfc } };

BEGIN_INTERFACE_MAP(CLayerDialog,CDialog)
    INTERFACE_PART(CLayerDialog,IID_ILayerDialog,Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLayerDialog message handlers

BOOL CLayerDialog::OnInitDialog()
    {
    CDialog::OnInitDialog();

    // Set the buttons to the layer names.
    iLayerNames = iFramework.LayerNames();
    size_t layers = iLayerNames.size();
    size_t i = 0;
    while (i < layers && i < 32)
        {
        const CartoType::MString& layer_name = iLayerNames[i];
#ifdef _UNICODE
        CartoType::CString text;
        text.Set(layer_name);
        text.Append((uint16_t)0);
        iCheck[i].SetWindowText((LPCTSTR)text.Text());
#else
        CartoType::CUtf8String text(layer_name);
        iCheck[i].SetWindowText(text);
#endif
        iCheck[i].SetCheck(iFramework.LayerIsEnabled(layer_name));
        i++;
        }
    while (i < 32)
        {
        iCheck[i].ShowWindow(SW_HIDE);
        i++;
        }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
    }

void CLayerDialog::OnOK()
    {
    // Set the layer parameters according to the check boxes.
    size_t layers = iLayerNames.size();
    for (size_t i = 0; i < layers && i < 32; i++)
        iFramework.EnableLayer(iLayerNames[i],iCheck[i].GetCheck() == 1);

    CDialog::OnOK();
    }
