#pragma once

#ifndef _HEADER_FILE_
#define _HEADER_FILE_

// link
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// header 
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <directxmath.h>
#include <windows.h>

//#include <wrl.h> //for ComPtr
#include <wrl/client.h> //아마 최적화때문? 

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>


#endif

