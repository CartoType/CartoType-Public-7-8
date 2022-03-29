#if !defined(AFX_SETSCALEDIALOG_H__BE8D875F_8567_4CCF_AF50_27AE455FE376__INCLUDED_)
#define AFX_SETSCALEDIALOG_H__BE8D875F_8567_4CCF_AF50_27AE455FE376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>
#include "Resource.h"
#include <cartotype_framework.h>

/////////////////////////////////////////////////////////////////////////////
// CSetScaleDialog dialog

class CSetScaleDialog : public CDialog
    {
    // Construction
    public:
        CSetScaleDialog(CWnd* pParent = NULL);   // standard constructor

        // Dialog Data
        //{{AFX_DATA(CSetScaleDialog)
        enum { IDD = IDD_SET_SCALE_DIALOG };
        CString	iScale;
        //}}AFX_DATA


        // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CSetScaleDialog)
    public:
        virtual void OnFinalRelease();
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

        // Implementation
    protected:

        // Generated message map functions
        //{{AFX_MSG(CSetScaleDialog)
        // NOTE: the ClassWizard will add member functions here
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
        // Generated OLE dispatch map functions
        //{{AFX_DISPATCH(CSetScaleDialog)
        // NOTE - the ClassWizard will add and remove member functions here.
        //}}AFX_DISPATCH
        DECLARE_DISPATCH_MAP()
        DECLARE_INTERFACE_MAP()
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETSCALEDIALOG_H__BE8D875F_8567_4CCF_AF50_27AE455FE376__INCLUDED_)
