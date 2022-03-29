
// CartoTypeDemo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "CartoTypeDemo.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "CartoTypeDemoDoc.h"
#include "CartoTypeDemoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCartoTypeDemoApp

BEGIN_MESSAGE_MAP(CCartoTypeDemoApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CCartoTypeDemoApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CCartoTypeDemoApp construction

CCartoTypeDemoApp::CCartoTypeDemoApp():
    iZoomFactor(2.0)
    {
	m_bHiColorIcons = TRUE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("CartoType.CartoTypeDemo.VS2015.4"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
    }

CCartoTypeDemoApp::~CCartoTypeDemoApp()
    {
    }

// The one and only CCartoTypeDemoApp object

CCartoTypeDemoApp theApp;

BOOL CCartoTypeDemoApp::InitInstance()
    {
	CWinAppEx::InitInstance();
	EnableTaskbarInteraction();

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("CartoType"));
	LoadStdProfileSettings(8);  // Load standard INI file options (including MRU)

    if (InitializeCartoType() != CartoType::KErrorNone)
        return FALSE;

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_CartoTypeTYPE,
		RUNTIME_CLASS(CCartoTypeDemoDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CCartoTypeDemoView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	    {
		delete pMainFrame;
		return FALSE;
	    }
	m_pMainWnd = pMainFrame;

	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

    if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew ||
        cmdInfo.m_nShellCommand == CCommandLineInfo::FileNothing)
        {
        if (m_pRecentFileList)
            {
            ::CString mru = (*m_pRecentFileList)[0];
            if (!mru.IsEmpty())
                OpenDocumentFile(mru);
            }
        }
    else if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
        // Dispatch commands specified on the command line
        if (!ProcessShellCommand(cmdInfo))
            return FALSE;

    // Override saved accelerators with those from this EXE. I don't want accelerators to be customisable.
    theApp.GetKeyboardManager()->ResetAll();

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
    }

int CCartoTypeDemoApp::ExitInstance()
    {
	//TODO: handle additional resources you may have added
	return CWinAppEx::ExitInstance();
    }

void CCartoTypeDemoApp::ShowError(const _TCHAR* aMessage)
    {
    m_pMainWnd->MessageBox(aMessage, _TEXT("CartoType"), MB_OK | MB_ICONERROR);
    }

void CCartoTypeDemoApp::ShowError(const _TCHAR* aMessage, int aError)
    {
    CartoType::CString text;
    text.Set((const uint16_t*)aMessage);
    CartoType::CString s = CartoType::ErrorString(aError);
    if (!s.IsEmpty())
        {
        text.Append(": ");
        text.Append(s);
        }
    text.Append((uint16_t)0);
    ShowError((LPCTSTR)text.Text());
    }

void CCartoTypeDemoApp::ShowError(const std::string& aMessage,int aError)
    {
    CartoType::CString text = aMessage;
    if (aError)
        text += ": " + CartoType::ErrorString(aError);
    text.Append((uint16_t)0);
    ShowError((LPCTSTR)text.Text());
    }

void CCartoTypeDemoApp::ShowError(int aError)
    {
    ShowError(_TEXT("CartoType error"),aError);
    }

void CCartoTypeDemoApp::GetFontPath(CartoType::CString& aPath,const ::CString& aAppPath,const char* aFontName)
    {
    ::CString font_devel_path("..\\..\\..\\..\\font\\");
    ::CString font_release_path("font\\");
    ::CString font_path = aAppPath + aFontName;
    if (_taccess((LPCTSTR)font_path, 4) == -1)
        {
        font_path = aAppPath + font_devel_path + ::CString(aFontName);
        if (_taccess((LPCTSTR)font_path, 4) == -1)
            font_path = aAppPath + font_release_path + ::CString(aFontName);
        }
    aPath.Set((const uint16_t*)(LPCTSTR)font_path);
    }

/**
Search for a font file and load it. Paths are searched in this order:
the application directory; the development tree directory; the subdirectory 'font' of the application directory
*/
CartoType::TResult CCartoTypeDemoApp::LoadFont(const ::CString& aAppPath, const char* aFontName)
    {
    CartoType::CString filename;
    GetFontPath(filename,aAppPath,aFontName);
    CartoType::TResult error = iEngine->LoadFont(filename);
    if (error)
        {
        ::CString text("Error loading font file ");
        text += aFontName;
        ShowError(text, error);
        }
    return error;
    }

/**
Search for a configuration file and load it. Paths are searched in this order:
the application directory; the development tree directory; the subdirectory 'config' of the application directory
*/
CartoType::TResult CCartoTypeDemoApp::Configure(const ::CString& aAppPath, const char* aFileName)
    {
    CartoType::CString filename;
    ::CString devel_path("..\\..\\..\\..\\src\\config\\");
    ::CString release_path("config\\");
    ::CString path = aAppPath + aFileName;
    if (_taccess((LPCTSTR)path, 4) == -1)
        {
        path = aAppPath + devel_path + ::CString(aFileName);
        if (_taccess((LPCTSTR)path, 4) == -1)
            path = aAppPath + release_path + ::CString(aFileName);
        }
    SetString(filename, path);
    CartoType::TResult error = iEngine->Configure(filename);
    if (error)
        {
        ::CString text("Error loading configuration file ");
        text += path;
        ShowError(text, error);
        }
    return error;
    }

CartoType::TResult CCartoTypeDemoApp::InitializeCartoType()
    {
    // Determine path of executable.
    _TCHAR path[MAX_PATH];
    GetModuleFileName(AfxGetInstanceHandle(),path,MAX_PATH);
    _TCHAR drive[_MAX_DRIVE];
    _TCHAR dir[_MAX_DIR];
    _TCHAR fname[_MAX_FNAME];
    _TCHAR ext[_MAX_EXT];
    _tsplitpath_s(path,drive,dir,fname,ext);
    ::CString app_path = (::CString)drive + dir;

    // Get the path of the default font.
    CartoType::CString font_filename;
    GetFontPath(font_filename,app_path,"DejaVuSans.ttf");

    // Create the engine, loading the default font.
    CartoType::TResult error = 0;
    iEngine = CartoType::CFrameworkEngine::New(error,font_filename);
    if (!iEngine || error)
        {
        ShowError(_TEXT("Error creating CartoType engine"),error);
        return error;
        }

    // Load extra fonts.
    error = LoadFont(app_path,"DejaVuSans-Bold.ttf");
    if (!error)
        error = LoadFont(app_path,"DejaVuSerif.ttf");
    if (!error)
        error = LoadFont(app_path,"DejaVuSerif-Italic.ttf");
    if (!error)
        error = LoadFont(app_path,"DroidSansFallback.ttf");
    if (!error)
        error = LoadFont(app_path,"MapKeyIcons.ttf");

    /*
    Enable this statement to use the sample configuration file,
    It's not usually necessary because the built-in configuration
    is appropriate.
    */
    /***
    if (!error)
    error = Configure(app_path,"standard.cartotype_config");
    ***/

    /*
    Find the default style sheet.
    Paths are searched in this order:
    the application directory;
    the development tree directory;
    the subdirectory style_sheet of the application directory.
    */
    ::CString style_sheet_name("standard.ctstyle");
    ::CString style_sheet_devel_path("..\\..\\..\\..\\style\\");
    ::CString style_sheet_release_path("style_sheet\\");
    ::CString style_sheet_path = app_path + style_sheet_name;
    if (_taccess((LPCTSTR)style_sheet_path,4) == -1)
        {
        style_sheet_path = app_path + style_sheet_devel_path + style_sheet_name;
        if (_taccess((LPCTSTR)style_sheet_path,4) == -1)
            style_sheet_path = app_path + style_sheet_release_path + style_sheet_name;
        }
    if (!error)
        SetString(iDefaultStyleSheetName,style_sheet_path);

    return error;
    }

// CCartoTypeDemoApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
    // Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    CString	iTextLine1;
    CString	iTextLine2;
    //}}AFX_DATA

protected:
    //{{AFX_VIRTUAL(CLatLongDialog)
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CLatLongDialog)
    iTextLine1 = _T("0");
    iTextLine2 = _T("0");
    //}}AFX_DATA_INIT
    }

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CLatLongDialog)
    DDX_Text(pDX,IDC_ABOUT_TEXT_LINE_1,iTextLine1);
    DDX_Text(pDX,IDC_ABOUT_TEXT_LINE_2,iTextLine2);
    //}}AFX_DATA_MAP
    }

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CCartoTypeDemoApp::OnAppAbout()
{
	CAboutDlg about_dialog;
    SetString(about_dialog.iTextLine1,CartoType::CString("CartoType Windows demo"));
    CartoType::CString s = "built using CartoType ";
    s += CartoType::Version();
    s += ".";
    s += CartoType::Build();
    SetString(about_dialog.iTextLine2,s);

    about_dialog.DoModal();
}

// CCartoTypeDemoApp customization load/save methods

void CCartoTypeDemoApp::PreLoadState()
{
}

void CCartoTypeDemoApp::LoadCustomState()
{
}

void CCartoTypeDemoApp::SaveCustomState()
{
}

// CCartoTypeDemoApp message handlers



