#pragma once

#ifdef _MSC_VER
	#define MIO2IT_NOINLINE __declspec(noinline)
#else
	#define MIO2IT_NOINLINE __attribute__((noinline))
#endif
