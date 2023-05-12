
// ComponiumDoc.cpp : implementation of the CComponiumDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Componium.h"
#endif

#include "ComponiumDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CComponiumDoc

IMPLEMENT_DYNCREATE(CComponiumDoc, CDocument)

BEGIN_MESSAGE_MAP(CComponiumDoc, CDocument)
END_MESSAGE_MAP()


// CComponiumDoc construction/destruction

CComponiumDoc::CComponiumDoc()
{
  compo_= std::unique_ptr<Pisen>(NovaPisen());
  if( compo_ ) NovaSekv(compo_.get(),MainMel);
  compo_->Zmena=False;
}

CComponiumDoc::~CComponiumDoc()
{
}

BOOL CComponiumDoc::OnNewDocument()
{
  if (!CDocument::OnNewDocument())
    return FALSE;

  // TODO: add reinitialization code here
  // (SDI documents will reuse this document)

  compo_= std::unique_ptr<Pisen>(NovaPisen());
  if( compo_ ) NovaSekv(compo_.get(),MainMel);
  compo_->Zmena=False;

  return TRUE;
}




class FileMem: public FILEEmul
{
  void *mem_;
  size_t size_;
  size_t pos_;

  public:
  FileMem(void *mem, size_t size):mem_(mem),size_(size),pos_(0){}

  size_t fread(void *buffer, size_t size, size_t count) override;
  int fgetc() override;
};

size_t FileMem::fread(void *buffer, size_t size, size_t count)
{
  size_t left = size_-pos_;
  size_t toRead = (std::min)(left/size,count);
  memcpy(buffer,(char *)mem_+pos_,toRead*size);
  pos_ += toRead*size;
  return toRead;
}

int FileMem::fgetc()
{
  if (pos_>=size_) return EOF;
  return ((unsigned char *)mem_)[pos_++];
}


// CComponiumDoc serialization

void CComponiumDoc::Serialize(CArchive& ar)
{
  if (ar.IsStoring())
  {
    // TODO: add storing code here
  }
  else
  {
    // TODO: add loading code here
    CFile *file = ar.GetFile();
    if (!file) AfxThrowArchiveException(CArchiveException::genericException);
    size_t size = (size_t)ar.GetFile()->GetLength();
    std::unique_ptr<char[]> buffer(new char[size]);
    if (file->Read(buffer.get(),size)!=size) AfxThrowArchiveException(CArchiveException::genericException);;
    //AfxThrowArchiveException(CArchiveException::genericException);
    std::unique_ptr<FileMem> mem(new FileMem(buffer.get(),size));
    CtiPisen(mem.get());
  }
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CComponiumDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
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
void CComponiumDoc::InitializeSearchContent()
{
  CString strSearchContent;
  // Set search contents from document's data. 
  // The content parts should be separated by ";"

  // For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
  SetSearchContent(strSearchContent);
}

void CComponiumDoc::SetSearchContent(const CString& value)
{
  if (value.IsEmpty())
  {
    RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
  }
  else
  {
    CMFCFilterChunkValueImpl *pChunk = NULL;
    ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
    if (pChunk != NULL)
    {
      pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
      SetChunkValue(pChunk);
    }
  }
}

#endif // SHARED_HANDLERS

// CComponiumDoc diagnostics

#ifdef _DEBUG
void CComponiumDoc::AssertValid() const
{
  CDocument::AssertValid();
}

void CComponiumDoc::Dump(CDumpContext& dc) const
{
  CDocument::Dump(dc);
}
#endif //_DEBUG


// CComponiumDoc commands
