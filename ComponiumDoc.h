
// ComponiumDoc.h : interface of the CComponiumDoc class
//


#pragma once

#include "COMPO/UTLCOMPO.H"
#include <memory>

class CComponiumDoc : public CDocument
{
protected: // create from serialization only
	CComponiumDoc();
	DECLARE_DYNCREATE(CComponiumDoc)

  std::unique_ptr<Pisen> compo_;

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CComponiumDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
