#include "util.hpp"
#include <stdio.h>
#include <iostream>

#define CASEFOR(x) case (x): return (#x)

namespace util {
	std::string nameForD3DFeatureLevel(D3D_FEATURE_LEVEL lv) {
		switch (lv) {
			CASEFOR(D3D_FEATURE_LEVEL_1_0_GENERIC);
			CASEFOR(D3D_FEATURE_LEVEL_1_0_CORE);
			CASEFOR(D3D_FEATURE_LEVEL_9_1);
			CASEFOR(D3D_FEATURE_LEVEL_9_2);
			CASEFOR(D3D_FEATURE_LEVEL_9_3);
			CASEFOR(D3D_FEATURE_LEVEL_10_0);
			CASEFOR(D3D_FEATURE_LEVEL_10_1);
			CASEFOR(D3D_FEATURE_LEVEL_11_0);
			CASEFOR(D3D_FEATURE_LEVEL_11_1);
			CASEFOR(D3D_FEATURE_LEVEL_12_0);
			CASEFOR(D3D_FEATURE_LEVEL_12_1);
			CASEFOR(D3D_FEATURE_LEVEL_12_2);
		}

		return "Unknown feature level";
	}
	int printWinErr(HRESULT hr, std::string preamble, bool onlyIfFailed) {
		char *errmsg = 0;

		DWORD res = FormatMessageA(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
				NULL, hr, 0, (char*)&errmsg, 8, NULL
				);

		std::cout << preamble;
		printf(" (%08lx): %s", hr, errmsg ? errmsg : "(getting string failed ");
		if (!errmsg)
			printf("%lu)", res);

		putchar('\n');

		LocalFree(errmsg);

		return hr ? 2 : 0;
	}
}
