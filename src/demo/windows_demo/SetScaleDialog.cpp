// SetScaleDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CartoTypeDemo.h"
#include "SetScaleDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetScaleDialog dialog


CSetScaleDialog::CSetScaleDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CSetScaleDialog::IDD,pParent)
    {
    EnableAutomation();

    //{{AFX_DATA_INIT(CSetScaleDialog)
    iScale = _T("");
    //}}AFX_DATA_INIT
    }


void CSetScaleDialog::OnFinalRelease()
    {
    // When the last reference for an automation object is released
    // OnFinalRelease is called.  The base class will automatically
    // deletes the object.  Add additional cleanup required for your
    // object before calling the base class.

    CDialog::OnFinalRelease();
    }

void CSetScaleDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSetScaleDialog)
    DDX_Text(pDX,IDC_SCALE,iScale);
    //}}AFX_DATA_MAP
    }


BEGIN_MESSAGE_MAP(CSetScaleDialog,CDialog)
    //{{AFX_MSG_MAP(CSetScaleDialog)
    // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CSetScaleDialog,CDialog)
    //{{AFX_DISPATCH_MAP(CSetScaleDialog)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_ISetScaleDialog to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {3840AE79-ACFF-47F4-8213-CBFE661DEEFE}
static const IID IID_ISetScaleDialog =
    { 0x3840ae79,0xacff,0x47f4,{ 0x82,0x13,0xcb,0xfe,0x66,0x1d,0xee,0xfe } };

BEGIN_INTERFACE_MAP(CSetScaleDialog,CDialog)
    INTERFACE_PART(CSetScaleDialog,IID_ISetScaleDialog,Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetScaleDialog message handlers
