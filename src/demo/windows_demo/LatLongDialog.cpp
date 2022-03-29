// LatLongDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CartoTypeDemo.h"
#include "LatLongDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLatLongDialog dialog


CLatLongDialog::CLatLongDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CLatLongDialog::IDD,pParent)
    {
    //{{AFX_DATA_INIT(CLatLongDialog)
    iLongitude = _T("0");
    iLatitude = _T("0");
    //}}AFX_DATA_INIT
    }


void CLatLongDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CLatLongDialog)
    DDX_Text(pDX,IDC_LONGITUDE,iLongitude);
    DDX_Text(pDX,IDC_LATITUDE,iLatitude);
    //}}AFX_DATA_MAP
    }


BEGIN_MESSAGE_MAP(CLatLongDialog,CDialog)
    //{{AFX_MSG_MAP(CLatLongDialog)
    // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLatLongDialog message handlers
