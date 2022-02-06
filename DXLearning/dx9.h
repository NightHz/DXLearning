#pragma once
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#include <d3d9caps.h>
//#include <d3d9helper.h>
#include "window.h"

IDirect3DDevice9* CreateSimpleDx9Device(SimpleWindow* window);
