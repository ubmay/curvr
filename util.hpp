#ifndef m_util_hpp
#define m_util_hpp
#include <Windows.h>
#include <string>
#include <d3d11.h>

namespace util {
	std::string nameForD3DFeatureLevel(D3D_FEATURE_LEVEL lv);
	int printWinErr(HRESULT hr, const std::string preamble, bool onlyIfFailed = false);
}

#endif
