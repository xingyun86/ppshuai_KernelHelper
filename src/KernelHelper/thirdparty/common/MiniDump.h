#ifndef __MINIDUMP_H_
#define __MINIDUMP_H_

#include <windows.h>

class CMiniDumper
{
public:

    CMiniDumper(bool bPromptUserForMiniDump = true);
    ~CMiniDumper(void);

private:

    static LONG WINAPI unhandledExceptionHandler(struct _EXCEPTION_POINTERS *pExceptionInfo);
    void setMiniDumpFileName(void);
    bool getImpersonationToken(HANDLE* phToken);
    BOOL enablePrivilege(LPCTSTR pszPriv, HANDLE hToken, TOKEN_PRIVILEGES* ptpOld);
    BOOL restorePrivilege(HANDLE hToken, TOKEN_PRIVILEGES* ptpOld);
    LONG writeMiniDump(_EXCEPTION_POINTERS *pExceptionInfo );

    _EXCEPTION_POINTERS *m_pExceptionInfo;
    _TCHAR m_szMiniDumpPath[MAX_PATH];
    _TCHAR m_szAppPath[MAX_PATH];
    _TCHAR m_szAppBaseName[MAX_PATH];
    bool m_bPromptUserForMiniDump;

    static CMiniDumper* G_pMiniDumper;
    static LPCRITICAL_SECTION G_pCriticalSection;
};

#endif // __MINIDUMP_H_
