
// ComponiumView.h : interface of the CComponiumView class
//

#pragma once

#include "COMPO/UTLCOMPO.H"

class CComponiumDoc;

class CComponiumView : public CView
{
  enum {NOff=3}; /* pocet vrstev ve stope (horni, osnova, dolni) */

  struct OffInfo
  {
    int xpos; // last unit pos
    int zpos;
    CasT zcas; // last unit time
    CasT xcas;
    bool synchro;
  };

  enum tramsmer {TJedno,TDolu,TNahoru,TRadDolu,TRadNahoru} ; // note leg direction

  struct GrfStatus
  {
    OffInfo i[NOff];
    CasT xcasspol; /* nejvyssi ze vsech kanalu */
    Err stat;
    int PredTon; // key before this unit
    int ZBlok,KBlok; // selection graphical coordinates
    byte PoslBlok;
    PosT RelPos; // number of items rendered so far - offset
    bool PridanTram; // beam flag
    int TramX,TramY; // beam coordinates, <0 negative means beam
    int NTram; // how many bars
    tramsmer TramSmer;
  };

  enum {MaxVec=16};
  struct VNota
  {
    JednotkaBuf t; /* vsechny param. */
    WindRect *W; /* kde se nakreslily */
    Flag JeKursor; /* zda na ni je kursor */
  };

  struct Vector  /* buffer na vektory not */
  {
    VNota v[MaxVec];
    int n; /* pocet not ve vektoru */
  };

  struct OknoInfo
  {
    SekvSoubor Soub[NMaxHlasu]; /* nastaven na zacatek okna */
    /* ze Soub[0] lze cist hlavni sekvenci! */
    //int NSoub; /* pocet zobraz. hlasu */
    int KKan; /* hlas, ve kterem je kursor */
    //SekvSoubor Kurs; /* pozice kursoru */
    WindRect KR; /* pozice kurzoru (rel. v okne ) */
    //CasT ZacCas,KonCas; /* historie - kazdy vznika jinak */
    //CasT CasZac,CasKon;
    int IStopa,NStop; /* prvni zobr. hlas, pocet zobr. hlasu */
    int ZBStopa,KBStopa; /* prvni hlas v bloku, posl hl. v bl. (<0 = odp. soub. uzavren) */
    SekvSoubor ZBlok,KBlok; /* zac. bloku v 1. hlase, konec v posl. hlase */
    //Flag ZmenaObrazu; /* nucene prekreslovani celeho pri lib. casti */
    //Flag ZobrazKursor; /* vynucene zobrazeni kursoru */
    //Flag VNazvuZmena;
    //Flag Rozvoj; /* indikace - tento rezim by mel byt ve vsech souborech */
  } OI;

  Vector TonVec[NMaxStop];

  GrfStatus KIStat[NMaxStop];

protected: // create from serialization only
  CComponiumView();
  DECLARE_DYNCREATE(CComponiumView)

// Attributes
public:
  CComponiumDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void OnDraw(CDC* pDC) override;
protected:
  virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
  virtual ~CComponiumView();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:

  void FKresliNoty(CDC *dc);
  void ZacniKresl(SekvSoubor *ss);
  void KresliJedn( SekvSoubor *s, int xpos, int Kan, int offs );
  void KresliTramH( int x1, int y1, int x2, int y2, int Cet );
  void KresliTramD( int x1, int y1, int x2, int y2, int Cet );

  bool JeVBloku( int Kan, const StrPosT *pos, CasT cas );
  void UzavriTram( GrfStatus *KS );
  void ZavedTram( GrfStatus *KS, int x, int y, int Cet, enum tramsmer Smer );
  int FKresliTonD( GrfStatus *KS, WindRect *W, JednItem *B, int xpos, int ypos, Flag Prapor, Flag Musi );
  int FKresliTonH( GrfStatus *KS, WindRect *W, JednItem *B, int xpos, int ypos, Flag Prapor, Flag Musi );
  int FKresliTon( GrfStatus *KS, WindRect *W, JednItem *B, int xpos, int ypos );
  int FKresliPau( WindRect *W, JednItem *B, int xpos, int ypos, int off );
  int KresliVec( GrfStatus *KS, Vector *V, int xpos, int ypos);
  int KresliTon( GrfStatus *KS, WindRect *W, SekvSoubor *s, int xpos, int ypos, int Kan );

  static void SetridVec( Vector *V );

// Generated message map functions
protected:
  afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
  afx_msg void OnFilePrintPreview();
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ComponiumView.cpp
inline CComponiumDoc* CComponiumView::GetDocument() const
   { return reinterpret_cast<CComponiumDoc*>(m_pDocument); }
#endif

