#include "app.h"
#include <cstring>
#include <set>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){

  CWindows::App *app = new CWindows::App;
  app->Run();
  return 0;
}
