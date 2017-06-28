// CStrmSr.h : CCStrmSr ������

#pragma once
#include "resource.h"       // ������

#include "VideoSourcex_i.h"
#include "opencvref.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif



// CCStrmSr

class ATL_NO_VTABLE CCStrmSr :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCStrmSr, &CLSID_CStrmSr>,
	public IDispatchImpl<ICStrmSr, &IID_ICStrmSr, &LIBID_VideoSourcexLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CCStrmSr()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CSTRMSR)


BEGIN_COM_MAP(CCStrmSr)
	COM_INTERFACE_ENTRY(ICStrmSr)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	STDMETHOD(PlayVideo)(void);
	STDMETHOD(PlayVideoOnWnd)(OLE_HANDLE hwnd);

private:
	//��ʱ����תͼ��degree�Ƕȣ�ԭ�ߴ磩
	void RotateImage(IplImage* src, IplImage *dst, int degree);
};

OBJECT_ENTRY_AUTO(__uuidof(CStrmSr), CCStrmSr)
