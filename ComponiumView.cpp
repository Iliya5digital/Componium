
// ComponiumView.cpp : implementation of the CComponiumView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Componium.h"
#endif

#include "ComponiumDoc.h"
#include "ComponiumView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CComponiumView

IMPLEMENT_DYNCREATE(CComponiumView, CView)

BEGIN_MESSAGE_MAP(CComponiumView, CView)
  ON_WM_STYLECHANGED()
  ON_WM_CONTEXTMENU()
  ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CComponiumView construction/destruction

CComponiumView::CComponiumView()
{
  // TODO: add construction code here

}

CComponiumView::~CComponiumView()
{
}

BOOL CComponiumView::PreCreateWindow(CREATESTRUCT& cs)
{
  // TODO: Modify the Window class or styles here by modifying
  //  the CREATESTRUCT cs

  return CView::PreCreateWindow(cs);
}




void CComponiumView::OnDraw(CDC* pDC)
{
  SekvSoubor ss[NMaxStop];
  ZacniKresl(ss);
}


void CComponiumView::OnInitialUpdate()
{
  CView::OnInitialUpdate();


  // TODO: You may populate your ListView with items by directly accessing
  //  its list control through a call to GetListCtrl().
}

void CComponiumView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
  ClientToScreen(&point);
  OnContextMenu(this, point);
}

void CComponiumView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
  theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CComponiumView diagnostics

#ifdef _DEBUG
void CComponiumView::AssertValid() const
{
  CView::AssertValid();
}

void CComponiumView::Dump(CDumpContext& dc) const
{
  CView::Dump(dc);
}

CComponiumDoc* CComponiumView::GetDocument() const // non-debug version is inline
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CComponiumDoc)));
  return (CComponiumDoc*)m_pDocument;
}
#endif //_DEBUG


// CComponiumView message handlers
void CComponiumView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
  //TODO: add code to react to the user changing the view style of your window	
  CView::OnStyleChanged(nStyleType,lpStyleStruct);	
}

const char *SterLS = "L" ,*SterPS = "R"; // TODO: localize

const char *TempoText( int Tempo )
{
  // TODO: 
  static char buf[64];
  sprintf(buf,"Temp %d",Tempo); // TODO: use lente, andante ... ?
  return buf;
}

const static char HlasS[][4]={"ppp","pp","p","mp","mf","f","ff","fff"};
const static int HlasUr[]=   { 1*2,  16*2,32*2,52*2,80*2,96*2,112*2,127*2};

const char *HlasitText( int Hlas )
{
  // TODO: use HlasS / HlasUr conversion
  static char buf[64];
  sprintf(buf,"Vol %d",Tempo);
  return buf;
}

#include "COMPO/SPRITEM.H"

void  Draw( int x, int y, const Sprite *Def ) {}
int TiskS( int y, const char *Str, int x ) {return 0;}
int TiskSG( int y, const char *Str, int x ) /* sedy */ {return 0;}
int TiskS8( int y, const char *Str, int x ){return 0;}
int TiskS8T( int y, const char *Str, int x ) /* tucne */ {return 0;}
void HCara( int y, int xz, int xk ) {}
void HBlok( int y, int xz, int xk, int hb ){} 
int PropS( int ye, const char *Str, int x ) {return 0;}

void Chyba( Err err )
{
  assert(false);
}
