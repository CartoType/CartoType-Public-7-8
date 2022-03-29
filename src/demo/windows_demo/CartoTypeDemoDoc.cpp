
// CartoTypeDemoDoc.cpp : implementation of the CCartoTypeDemoDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "CartoTypeDemo.h"
#endif

#include "CartoTypeDemoDoc.h"

#include <propkey.h>

#ifdef CARTOTYPE_VERIFY_ENCRYPTED_FILE
#include <cartotype_encrypted_stream.h>
#include <cartotype_twofish_encryption.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCartoTypeDemoDoc

IMPLEMENT_DYNCREATE(CCartoTypeDemoDoc, CDocument)

BEGIN_MESSAGE_MAP(CCartoTypeDemoDoc, CDocument)
END_MESSAGE_MAP()


// CCartoTypeDemoDoc construction/destruction

CCartoTypeDemoDoc::CCartoTypeDemoDoc()
{
	// TODO: add one-time construction code here

}

CCartoTypeDemoDoc::~CCartoTypeDemoDoc()
    {
    }

// CCartoTypeDemoDoc serialization

void CCartoTypeDemoDoc::Serialize(CArchive& ar)
    {
    if (ar.IsStoring())
        {
        // TODO: add storing code here
        }
    else
        {
        // TODO: add loading code here
        }
    }

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CCartoTypeDemoDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CCartoTypeDemoDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CCartoTypeDemoDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CCartoTypeDemoDoc diagnostics

#ifdef _DEBUG
void CCartoTypeDemoDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCartoTypeDemoDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCartoTypeDemoDoc commands

BOOL CCartoTypeDemoDoc::OnOpenDocument(LPCTSTR lpszPathName)
    {
    iMapDataSet.reset();
    CartoType::TResult error = 0;
    CCartoTypeDemoApp* app = (CCartoTypeDemoApp*)AfxGetApp();
    CartoType::CString filename;
    SetString(filename,lpszPathName);

    if (filename.Last(10) == ".ctm1_ctci")
        {

#ifdef CARTOTYPE_VERIFY_ENCRYPTED_FILE
        CartoType::CString plain_filename(filename.First(filename.Length() - 5));
        CartoType::CFileInputStream* f0 = CartoType::CFileInputStream::New(error,plain_filename);
        CartoType::CEncryptedFileInputStream* f1 = CartoType::CEncryptedFileInputStream::New(error,filename,"password");
        size_t length0 = f0->Length(error);
        size_t length1 = f1->Length(error);
        const uint8_t* p0;
        const uint8_t* p1;
        for (;;)
            {
            error = f0->Read(p0,length0);
            if (error)
                break;
            error = f1->Read(p1,length1);
            if (error)
                break;

            if (length0 != length1 || memcmp(p0,p1,length0))
                {
                int x = 999;
                }
            }
        delete f0;
        delete f1;
#endif

        std::string key("password");
        iMapDataSet = CartoType::CFrameworkMapDataSet::New(error,app->Engine(),filename,&key);
        }
    else
        iMapDataSet = CartoType::CFrameworkMapDataSet::New(error,app->Engine(),filename);

    if (error)
        {
        ::CString text("Error opening map ");
        text += lpszPathName;
        app->ShowError((LPCTSTR)text,error);
        return FALSE;
        }
    return TRUE;
    }


BOOL CCartoTypeDemoDoc::OnNewDocument()
    {
    // TODO: Add your specialized code here and/or call the base class

    return CDocument::OnNewDocument();
    }
