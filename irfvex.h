// -*- mode:C++; coding:utf-8-ws-dos; -*-
// irfvex.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CirfvexApp:
// このクラスの実装については、irfvex.cpp を参照してください
//

class CirfvexApp : public CWinApp
{
public:
  CirfvexApp();

// オーバーライド
public:
  virtual BOOL InitInstance();

// 実装
  DECLARE_MESSAGE_MAP()
};

extern CirfvexApp theApp;
