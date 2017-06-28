// dllmain.h : 模块类的声明。

class CVideoSourcexModule : public CAtlDllModuleT< CVideoSourcexModule >
{
public :
	DECLARE_LIBID(LIBID_VideoSourcexLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_VIDEOSOURCEX, "{B1DD313E-55FD-4FB3-94CC-B851FF0D1757}")
};

extern class CVideoSourcexModule _AtlModule;
