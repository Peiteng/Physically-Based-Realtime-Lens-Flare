#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdexcept>
#include "../include/PBLensFlare.h"

#include "imgui.h"
#include "examples/imgui_impl_win32.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)//描画までに呼ばれるコールバック関数
{
  Application* appPtr = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
  static POINT lastPoint;

  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
    return TRUE;

  switch (msg)
  {
  case WM_PAINT:
    if (appPtr)
    {
    }
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  case WM_SIZE:
    if (appPtr)
    {
      RECT rect{};
      GetClientRect(hWnd, &rect);
      bool isMinimized = wp == SIZE_MINIMIZED;
      appPtr->onSizeChanged(rect.right - rect.left, rect.bottom - rect.top, isMinimized);
    }
    return 0;

  case WM_SYSKEYDOWN:
    if ((wp == VK_RETURN) && (lp & (1 << 29)))
    {
      if (appPtr) {
          appPtr->toggleFullscreen();
      }
    }
    break;

  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN:
    SetCapture(hWnd);
    if (appPtr)
    {
      auto io = ImGui::GetIO();
      if (io.WantCaptureMouse )
        break;

      GetCursorPos(&lastPoint);
      ScreenToClient(hWnd, &lastPoint);
      UINT buttonType = 0;
      if (msg == WM_LBUTTONDOWN) { buttonType = 0; }
      if (msg == WM_RBUTTONDOWN) { buttonType = 1; }
      if (msg == WM_MBUTTONDOWN) { buttonType = 2; }
      appPtr->onMouseButtonDown(buttonType);
    }
    break;
  
  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MBUTTONUP:
    if (appPtr)
    {
      ReleaseCapture();

      UINT buttonType = 0;
      if (msg == WM_LBUTTONDOWN) { buttonType = 0; }
      if (msg == WM_MBUTTONDOWN) { buttonType = 1; }
      if (msg == WM_MBUTTONDOWN) { buttonType = 2; }

      appPtr->onMouseButtonUp(buttonType);
    }
    break;

  case WM_MOUSEMOVE:
    if (appPtr)
    {
      auto io = ImGui::GetIO();
      if (io.WantCaptureMouse)
        break;

      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(hWnd, &pt);
      int dx = pt.x - lastPoint.x;
      int dy = pt.y - lastPoint.y;
      appPtr->onMouseMove(msg, dx, dy);
      lastPoint = pt;
    }
    break;
  }
  return DefWindowProc(hWnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  PBLensFlare app{};

  CoInitializeEx(NULL, COINIT_MULTITHREADED);

  WNDCLASSEX wc{};
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;//CallBack
  wc.hInstance = hInstance;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = L"D3D12Book3";
  RegisterClassEx(&wc);

  DWORD dwStyle = WS_OVERLAPPEDWINDOW;// &~WS_SIZEBOX;
  RECT rect = { 0,0, WINDOW_WIDTH, WINDOW_HEIGHT };
  AdjustWindowRect(&rect, dwStyle, FALSE);

  auto hwnd = CreateWindow(wc.lpszClassName, L"D3D12Book3",
    dwStyle,
    CW_USEDEFAULT, CW_USEDEFAULT,
    rect.right - rect.left, rect.bottom - rect.top,
    nullptr,
    nullptr,
    hInstance,
    &app
  );

  {
      app.initializeApp(hwnd, DXGI_FORMAT_R8G8B8A8_UNORM, false);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&app));
    ShowWindow(hwnd, nCmdShow);

    MSG msg{};
    while (msg.message != WM_QUIT)
    {
      if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      else
      {
          app.render();
      }
    }

    app.terminate();
    return static_cast<int>(msg.wParam);
  }
  {
    OutputDebugStringA("\n");
  }
  return 0;
}
