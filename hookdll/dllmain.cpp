// -*- mode:C++; coding:utf-8-ws-dos; -*-
// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

//CRITICAL_SECTION lock;

BOOL APIENTRY
DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    //InitializeCriticalSection(&lock);
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    //DeleteCriticalSection(&lock);
    break;
  }
  return TRUE;
}


LONG shift_pressed = 0;
LONG ctrl_pressed = 0;
LONG mouse_stopped = FALSE;
LONG mouse_play = 32;
POINT pressed_pos;

LRESULT CALLBACK
KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode < 0)
    return CallNextHookEx(NULL, nCode, wParam, lParam);

  bool bPrevDown = lParam & 0x40000000;
  bool bCurDown = ! (lParam & 0x80000000);
  if (wParam==VK_SHIFT || wParam==VK_CONTROL) {
    if (bPrevDown != bCurDown) {
      bool both_pressed = shift_pressed && ctrl_pressed;
      if (wParam == VK_SHIFT)
	shift_pressed = bCurDown;
      if (wParam == VK_CONTROL)
	ctrl_pressed = bCurDown;
      if (!both_pressed && shift_pressed && ctrl_pressed) {
	GetCursorPos(&pressed_pos);
	mouse_stopped = TRUE;
      }else if (!shift_pressed || !ctrl_pressed) {
	mouse_stopped = FALSE;
      }
    }
  }

  if (wParam=='A' && ctrl_pressed && !shift_pressed && !bPrevDown && bCurDown) {
    INPUT input[2] = {};
    input[0].type = INPUT_KEYBOARD;
    input[0].ki = {
      VK_SUBTRACT, (WORD)MapVirtualKey(VK_SUBTRACT, MAPVK_VK_TO_VSC), 0, 0,
      (ULONG_PTR)GetMessageExtraInfo() };
    input[1] = input[0];
    input[1].ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput(2, input, sizeof(INPUT));
  }

  return CallNextHookEx(NULL, nCode, wParam, lParam);
}


LRESULT CALLBACK
MouseProc(int nCode,WPARAM wParam, LPARAM lParam)
{
  if (nCode < 0)
    return CallNextHookEx(NULL, nCode, wParam, lParam);

  if (mouse_stopped && wParam==WM_MOUSEMOVE) {
    POINT &pt = reinterpret_cast<MOUSEHOOKSTRUCT *>(lParam)->pt;
    int dx = pressed_pos.x - pt.x;
    int dy = pressed_pos.y - pt.y;
    if (dx<-mouse_play || dx>mouse_play || dy<-mouse_play || dy>mouse_play)
      mouse_stopped = FALSE;
    else
      return 1;
  }
  
  return CallNextHookEx(NULL, nCode, wParam, lParam);
}
