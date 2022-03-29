#if !defined(AFX_LATLONGDIALOG_H__B94FC518_35C2_4AF7_95EE_8472AB6E0086__INCLUDED_)
#define AFX_LATLONGDIALOG_H__B94FC518_35C2_4AF7_95EE_8472AB6E0086__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>
#include "Resource.h"
#include <cartotype_framework.h>

/////////////////////////////////////////////////////////////////////////////
// CLatLongDialog dialog

class CLatLongDialog : public CDialog
    {
    // Construction
    public:
        CLatLongDialog(CWnd* pParent = NULL);   // standard constructor

        // Dialog Data
        //{{AFX_DATA(CFindDialog)
        enum { IDD = IDD_LATLONG_DIALOG };
        CString	iLongitude;
        CString	iLatitude;
        //}}AFX_DATA


        // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CLatLongDialog)
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

        // Implementation
    protected:

        // Generated message map functions
        //{{AFX_MSG(CLatLongDialog)
        // NOTE: the ClassWizard will add member functions here
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LATLONGDIALOG_H__B94FC518_35C2_4AF7_95EE_8472AB6E0086__INCLUDED_)
