#if !defined(AFX_LAYERDIALOG_H__81E915CF_628A_4A75_A6C7_FD4D42D1BA6B__INCLUDED_)
#define AFX_LAYERDIALOG_H__81E915CF_628A_4A75_A6C7_FD4D42D1BA6B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>
#include "Resource.h"
#include <cartotype_framework.h>

/////////////////////////////////////////////////////////////////////////////
// CLayerDialog dialog

#include "cartotype_framework.h"

class CLayerDialog : public CDialog
    {
    // Construction
    public:
        CLayerDialog(CartoType::CFramework& aFramework,CWnd* pParent = NULL);   // standard constructor

        // Dialog Data
        //{{AFX_DATA(CLayerDialog)
        enum { IDD = IDD_LAYER_DIALOG };
        //}}AFX_DATA

        CButton iCheck[32];

        // Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CLayerDialog)
    public:
        virtual void OnFinalRelease();
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

        // Implementation
    protected:

        // Generated message map functions
        //{{AFX_MSG(CLayerDialog)
        virtual BOOL OnInitDialog();
        virtual void OnOK();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
        // Generated OLE dispatch map functions
        //{{AFX_DISPATCH(CLayerDialog)
        // NOTE - the ClassWizard will add and remove member functions here.
        //}}AFX_DISPATCH
        DECLARE_DISPATCH_MAP()
        DECLARE_INTERFACE_MAP()

    private:
        CartoType::CFramework& iFramework;
        std::vector<CartoType::CString> iLayerNames;
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LAYERDIALOG_H__81E915CF_628A_4A75_A6C7_FD4D42D1BA6B__INCLUDED_)
