// CStrmSr.h : CCStrmSr 的声明

#pragma once
#include "resource.h"       // 主符号

#include "VideoSourcex_i.h"
#include "opencvref.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台(如不提供完全 DCOM 支持的 Windows Mobile 平台)上无法正确支持单线程 COM 对象。定义 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 可强制 ATL 支持创建单线程 COM 对象实现并允许使用其单线程 COM 对象实现。rgs 文件中的线程模型已被设置为“Free”，原因是该模型是非 DCOM Windows CE 平台支持的唯一线程模型。"
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
	//逆时针旋转图像degree角度（原尺寸）
	void RotateImage(IplImage* src, IplImage *dst, int degree);
};

OBJECT_ENTRY_AUTO(__uuidof(CStrmSr), CCStrmSr)
