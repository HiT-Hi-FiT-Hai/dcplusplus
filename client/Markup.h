// Markup.h: interface for the CMarkup class.
//

#if !defined(AFX_MARKUP_H__948A2705_9E68_11D2_A0BF_00105A27C570__INCLUDED_)
#define AFX_MARKUP_H__948A2705_9E68_11D2_A0BF_00105A27C570__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMarkup  
{
public:
	CMarkup() { SetDoc( NULL ); };
	CMarkup( LPCTSTR szDoc ) { SetDoc( szDoc ); };
	CMarkup( const CMarkup& markup ) { *this = markup; };
	void operator=( const CMarkup& markup );
	virtual ~CMarkup() {};

	// Parse and Navigate
	bool SetDoc( LPCTSTR szDoc );
	bool FindElem( LPCTSTR szName=NULL );
	bool FindChildElem( LPCTSTR szName=NULL );
	bool IntoElem();
	bool OutOfElem();
	void ResetChildPos() { m_iPosChild = 0; };
	void ResetPos();
	CString GetTagName() const { return x_GetTagName(m_iPos); };
	CString GetChildTagName() const { return x_GetTagName(m_iPosChild); };
	CString GetData() const { return x_GetData(m_iPos); };
	CString GetChildData() const { return x_GetData(m_iPosChild); };
	CString GetAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib(m_iPos,szAttrib); };
	int GetIntAttrib( LPCTSTR szAttrib) const { return _ttoi(x_GetAttrib(m_iPos, szAttrib)); };
	CString GetChildAttrib( LPCTSTR szAttrib ) const { return x_GetAttrib(m_iPosChild,szAttrib); };
	int GetChildIntAttrib( LPCTSTR szAttrib) const { return _ttoi(x_GetAttrib(m_iPosChild, szAttrib)); };
	CString GetAttribName( int n ) const;
	bool SavePos( LPCTSTR szPosName=_T("") );
	bool RestorePos( LPCTSTR szPosName=_T("") );
	bool GetOffsets( int& nStart, int& nEnd ) const;
	CString GetError() const { return m_csError; };

	// Create
	bool AddElem( LPCTSTR szName, LPCTSTR szData=NULL );
	bool AddChildElem( LPCTSTR szName, LPCTSTR szData=NULL );
	bool InsertChildElem( LPCTSTR szName, LPCTSTR szData=NULL );
	bool AddAttrib( LPCTSTR szAttrib, LPCTSTR szValue );
	bool AddChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue );
	bool AddAttrib( LPCTSTR szAttrib, int nValue );
	bool AddChildAttrib( LPCTSTR szAttrib, int nValue );
	CString GetDoc() const { return m_csDoc; };
	bool AddChildSubDoc( LPCTSTR szSubDoc );
	bool InsertChildSubDoc( LPCTSTR szSubDoc );
	CString GetChildSubDoc() const;

	// Modify
	bool RemoveElem();
	bool RemoveChildElem();
	bool SetAttrib( LPCTSTR szAttrib, LPCTSTR szValue );
	bool SetChildAttrib( LPCTSTR szAttrib, LPCTSTR szValue );
	bool SetAttrib( LPCTSTR szAttrib, int nValue );
	bool SetChildAttrib( LPCTSTR szAttrib, int nValue );
	bool SetData( LPCTSTR szData, int nCDATA=0 ) { return x_SetData(m_iPos,szData,nCDATA); };
	bool SetChildData( LPCTSTR szData, int nCDATA=0 ) { return x_SetData(m_iPosChild,szData,nCDATA); };

protected:
	CString m_csDoc;
	int m_nLevel;
	CString m_csError;

	struct ElemPos
	{
		ElemPos() { Clear(); };
		ElemPos( const ElemPos& pos ) { *this = pos; };
		bool IsEmptyElement() const { return (nStartR == nEndL + 1); };
		void Clear()
		{
			nStartL=0; nStartR=0; nEndL=0; nEndR=0; nNext=0;
			iElemParent=0; iElemChild=0; iElemNext=0;
		};
		int nStartL;
		int nStartR;
		int nEndL;
		int nEndR;
		int nNext;
		int iElemParent;
		int iElemChild;
		int iElemNext;
	};

	CArray< ElemPos, ElemPos& > m_aPos;
	int m_iPos;
	int m_iPosChild;
	int m_iPosFree;

	int x_GetFreePos();
	int x_ReleasePos();

	struct TokenPos
	{
		TokenPos() { Clear(); };
		bool IsValid() const { return (nL <= nR); };
		void Clear() { nL=0; nR=-1; bIsString=false; };
		int nL;
		int nR;
		int nNext;
		bool bIsString;
	};

	struct SavedPos
	{
		int iPos;
		int iPosChild;
		int iLevel;
	};
	CMap<CString,LPCTSTR,SavedPos,SavedPos> m_aSavedPos;

	int x_ParseElem( int iPos );
	int x_ParseError( LPCTSTR szError, LPCTSTR szTag = NULL );
	bool x_FindChar( int&n, _TCHAR c ) const;
	bool x_FindToken( TokenPos& token ) const;
	CString x_GetToken( const TokenPos& token ) const;
	CString x_GetTagName( int iPos ) const;
	CString x_GetData( int iPos ) const;
	CString x_GetAttrib( int iPos, LPCTSTR szAttrib ) const;
	int x_Add( int iPosParent, int iPosBefore, LPCTSTR szName, LPCTSTR szValue );
	int x_AddSubDoc( int iPosParent, int iPosBefore, LPCTSTR szSubDoc );
	bool x_FindAttrib( TokenPos& token, LPCTSTR szAttrib=NULL ) const;
	int x_AddAttrib( int iPos, LPCTSTR szAttrib, LPCTSTR szValue );
	int x_SetAttrib( int iPos, LPCTSTR szAttrib, LPCTSTR szValue );
	bool x_SetData( int iPos, LPCTSTR szData, int nCDATA );
	int x_Remove( int iPos );
	void x_DocChange( int nLeft, int nReplace, const CString& csInsert );
	void x_PosInsert( int iPos, int nInsertLength );
	void x_Adjust( int iPos, int nShift );
	CString x_TextToDoc( LPCTSTR szText, bool bAttrib = false ) const;
	CString x_TextFromDoc( int nLeft, int nRight ) const;
};

#endif // !defined(AFX_MARKUP_H__948A2705_9E68_11D2_A0BF_00105A27C570__INCLUDED_)
