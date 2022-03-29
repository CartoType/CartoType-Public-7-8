
// CartoTypeDemo.h : main header file for the CartoTypeDemo application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include <memory>

#include "resource.h"       // main symbols


// CCartoTypeDemoApp:
// See CartoTypeDemo.cpp for the implementation of this class
//

#include <cartotype_framework.h>

class CCartoTypeDemoApp : public CWinAppEx
{
public:
	CCartoTypeDemoApp();
    ~CCartoTypeDemoApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

public:
    void ShowError(const _TCHAR* aMessage);
    void ShowError(const _TCHAR* aMessage,int aError);
    void ShowError(const std::string& aMessage,int aError);
    void ShowError(int aError);
    std::shared_ptr<CartoType::CFrameworkEngine> Engine() { return iEngine; }
    const CartoType::MString& DefaultStyleSheetName() const { return iDefaultStyleSheetName; }
    double ZoomFactor() const { return iZoomFactor; }

private:
    void GetFontPath(CartoType::CString& aPath,const ::CString& aAppPath,const char* aFontName);
    CartoType::TResult LoadFont(const ::CString& aAppPath,const char* aFontName);
    CartoType::TResult Configure(const ::CString& aAppPath,const char* aFileName);
    CartoType::TResult InitializeCartoType();

    std::shared_ptr<CartoType::CFrameworkEngine> iEngine;
    CartoType::CString iDefaultStyleSheetName;
    double iZoomFactor;
    };

static inline void SetString(CartoType::MString& aDest, LPCTSTR aSource)
    {
    aDest.Set((const uint16_t*)aSource);
    }

static inline void SetString(::CString& aDest,const char* aSource)
    {
    CartoType::CString s(aSource);
    aDest.SetString((LPCTSTR)s.Text(),(int)s.Length());
    }

static inline void SetString(::CString& aDest, const CartoType::MString& aSource)
    {
    aDest.SetString((LPCTSTR)aSource.Text(),(int)aSource.Length());
    }

extern CCartoTypeDemoApp theApp;
