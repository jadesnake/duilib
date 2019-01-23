#pragma once
#define UILIB_STATIC
#include "..\..\DuiLib\UIlib.h"

using namespace DuiLib;

#ifdef _DEBUG
#   pragma comment(lib, "..\\..\\Debug\\DuiLib.lib")
#else
#   pragma comment(lib, "..\\..\\Release\\DuiLib.lib")
#endif

#include "resource.h"

using namespace std;
//
#include "FrameWnd.h"