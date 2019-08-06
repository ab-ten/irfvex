// -*- mode:C++; coding:utf-8-ws-dos; -*-
// irfvex.cpp : アプリケーションのクラス動作を定義します。
//
#include "stdafx.h"
#include "irfvex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Pathcch.lib")

// CirfvexApp

BEGIN_MESSAGE_MAP(CirfvexApp, CWinApp)
  ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CirfvexApp の構築

CirfvexApp::CirfvexApp()
{
  // TODO: この位置に構築用コードを追加してください。
  // ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}


// 唯一の CirfvexApp オブジェクト

CirfvexApp theApp;

enum retry_allocator_func_return_t {
  RETRY_ALLOCATOR_RETRY = 0,
  RETRY_ALLOCATOR_SUCCESS,
  RETRY_ALLOCATOR_FAIL,
};
template<typename Ty, typename LenTy=DWORD>
class retry_allocator {
  typedef Ty buffer_type;
  typedef LenTy length_type;
  typedef std::function<retry_allocator_func_return_t (buffer_type *p, length_type &len)> func_type;
  //
  std::unique_ptr<buffer_type> m_buf;
  length_type m_len;
  int m_max_retry;
public:
  retry_allocator(length_type initlen=1, int max_retry=16)
  : m_len(initlen), m_max_retry(max_retry) {
  }
  length_type operator() (func_type f) {
    for (int retry=0; retry<m_max_retry; retry++) {
      m_buf.reset(new buffer_type[m_len + 16]);
      retry_allocator_func_return_t r = f(m_buf.get(), m_len);
      if (r == RETRY_ALLOCATOR_SUCCESS)
	return m_len;
      if (r == RETRY_ALLOCATOR_FAIL)
	break;
    }
    m_buf.reset();
    m_len = 0;
    return 0;
  }
  buffer_type *get() {
    return m_buf.get();
  }
  length_type len() {
    return m_len;
  }
};


// CirfvexApp の初期化

BOOL CirfvexApp::InitInstance()
{
  // アプリケーション マニフェストが visual スタイルを有効にするために、
  // ComCtl32.dll Version 6 以降の使用を指定する場合は、
  // Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
  INITCOMMONCONTROLSEX InitCtrls;
  InitCtrls.dwSize = sizeof(InitCtrls);
  // アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
  // これを設定します。
  InitCtrls.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&InitCtrls);

  CWinApp::InitInstance();


  AfxEnableControlContainer();

  // ダイアログにシェル ツリー ビューまたはシェル リスト ビュー コントロールが
  // 含まれている場合にシェル マネージャーを作成します。
  //CShellManager *pShellManager = new CShellManager;

  // MFC コントロールでテーマを有効にするために、"Windows ネイティブ" のビジュアル マネージャーをアクティブ化
  CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

  // 標準初期化
  // これらの機能を使わずに最終的な実行可能ファイルの
  // サイズを縮小したい場合は、以下から不要な初期化
  // ルーチンを削除してください。
  // 設定が格納されているレジストリ キーを変更します。
  // TODO: 会社名または組織名などの適切な文字列に
  // この文字列を変更してください。
  //SetRegistryKey(_T("アプリケーション ウィザードで生成されたローカル アプリケーション"));

  retry_allocator<wchar_t> modulefn(256);
  DWORD r = modulefn([](wchar_t *p, DWORD &len) {
      DWORD r = GetModuleFileName(NULL, p, len);
      if (r <= 0) {
	len = 0;
	return RETRY_ALLOCATOR_FAIL;
      }
      if (r >= len) {
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
	  len = r*2 + 256;
	  return RETRY_ALLOCATOR_RETRY;
	}
	len = 0;
	return RETRY_ALLOCATOR_FAIL;
      }
      return RETRY_ALLOCATOR_SUCCESS;
    });
  if (r <= 0) {
    AfxMessageBox(L"GetModuleFileName(NULL) failed.");
    return FALSE;
  }
  PathCchRemoveFileSpec(modulefn.get(), r+1);
  std::wstring exedir(modulefn.get());
  std::wstring ivexe = exedir + L"\\i_view64.exe";
  #ifdef _DEBUG
//SHADOWCOMMENT//
//SHADOWCOMMENT//
//SHADOWCOMMENT//
//SHADOWCOMMENT//
  #endif

  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi;
  BOOL br = CreateProcess(
      ivexe.c_str(),
      GetCommandLine(),
      NULL, NULL, FALSE,
      GetPriorityClass(GetCurrentProcess()),
      NULL, NULL, &si, &pi);
  if (! br) {
    AfxMessageBox(L"CreateProcess() failed.");
    return FALSE;
  }
  WaitForInputIdle(pi.hProcess, INFINITE);
  CloseHandle(pi.hThread);

  std::wstring dllname = exedir + L"\\irfvex.dll";
  HMODULE hModuleDll = LoadLibrary(dllname.c_str());
  if (hModuleDll == NULL) {
    AfxMessageBox(L"LoadLibrary() failed.");
    return FALSE;
  }

  HOOKPROC mouseproc = (HOOKPROC)GetProcAddress(hModuleDll, "MouseProc");
  HOOKPROC kbdproc = (HOOKPROC)GetProcAddress(hModuleDll, "KeyboardProc");
  if (mouseproc==NULL || kbdproc==NULL) {
    AfxMessageBox(L"GetProcAddress() failed.");
    return FALSE;
  }
  HHOOK hhMouse = SetWindowsHookEx(WH_MOUSE, mouseproc, hModuleDll, pi.dwThreadId);
  HHOOK hhKbd = SetWindowsHookEx(WH_KEYBOARD, kbdproc, hModuleDll, pi.dwThreadId);

  WaitForSingleObject(pi.hProcess, INFINITE);
  UnhookWindowsHookEx(hhKbd);
  UnhookWindowsHookEx(hhMouse);

  // 上で作成されたシェル マネージャーを削除します。
  //if (pShellManager != nullptr) {
  //  delete pShellManager;
  //}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
  ControlBarCleanUp();
#endif

  // ダイアログは閉じられました。アプリケーションのメッセージ ポンプを開始しないで
  //  アプリケーションを終了するために FALSE を返してください。
  return FALSE;
}
