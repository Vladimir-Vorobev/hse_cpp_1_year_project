#ifndef __SKIP_INTERNAL_FATBINARY_HEADERS
#include "fatbinary_section.h"
#endif
#define __CUDAFATBINSECTION  ".nvFatBinSegment"
#define __CUDAFATBINDATASECTION  ".nv_fatbin"
#ifdef __cplusplus
extern "C" {
#endif

#pragma const_seg(__CUDAFATBINDATASECTION)
static const __declspec(allocate(__CUDAFATBINDATASECTION)) unsigned long long fatbinData[]= {0x00100001ba55ed50ull,0x00000000000003b0ull,0x0000004801010002ull,0x0000000000000368ull,
0x0000000000000000ull,0x0000003400010007ull,0x0000000000000000ull,0x0000000000000041ull,
0x0000000000000000ull,0x0000000000000000ull,0x0000000000000000ull,0x33010102464c457full,
0x0000000000000007ull,0x0000007c00be0002ull,0x0000000000000000ull,0x00000000000002f8ull,
0x0000000000000178ull,0x0038004000340534ull,0x0001000600400002ull,0x7472747368732e00ull,
0x747274732e006261ull,0x746d79732e006261ull,0x746d79732e006261ull,0x78646e68735f6261ull,
0x666e692e766e2e00ull,0x61632e766e2e006full,0x0068706172676c6cull,0x746f72702e766e2eull,
0x6e2e00657079746full,0x63612e6c65722e76ull,0x732e00006e6f6974ull,0x0062617472747368ull,
0x006261747274732eull,0x006261746d79732eull,0x5f6261746d79732eull,0x6e2e0078646e6873ull,
0x2e006f666e692e76ull,0x676c6c61632e766eull,0x766e2e0068706172ull,0x79746f746f72702eull,
0x722e766e2e006570ull,0x6f697463612e6c65ull,0x000000000000006eull,0x0000000000000000ull,
0x0000000000000000ull,0x0000000000000000ull,0x0004000300000032ull,0x0000000000000000ull,
0x0000000000000000ull,0x000500030000004eull,0x0000000000000000ull,0x0000000000000000ull,
0xffffffff00000000ull,0xfffffffe00000000ull,0xfffffffd00000000ull,0xfffffffc00000000ull,
0x0000000000000073ull,0x3605002511000000ull,0x0000000000000000ull,0x0000000000000000ull,
0x0000000000000000ull,0x0000000000000000ull,0x0000000000000000ull,0x0000000000000000ull,
0x0000000000000000ull,0x0000000000000000ull,0x0000000300000001ull,0x0000000000000000ull,
0x0000000000000000ull,0x0000000000000040ull,0x000000000000005dull,0x0000000000000000ull,
0x0000000000000001ull,0x0000000000000000ull,0x000000030000000bull,0x0000000000000000ull,
0x0000000000000000ull,0x000000000000009dull,0x000000000000005dull,0x0000000000000000ull,
0x0000000000000001ull,0x0000000000000000ull,0x0000000200000013ull,0x0000000000000000ull,
0x0000000000000000ull,0x0000000000000100ull,0x0000000000000048ull,0x0000000300000002ull,
0x0000000000000008ull,0x0000000000000018ull,0x7000000100000032ull,0x0000000000000000ull,
0x0000000000000000ull,0x0000000000000148ull,0x0000000000000020ull,0x0000000000000003ull,
0x0000000000000004ull,0x0000000000000008ull,0x7000000b0000004eull,0x0000000000000000ull,
0x0000000000000000ull,0x0000000000000168ull,0x0000000000000010ull,0x0000000000000000ull,
0x0000000000000008ull,0x0000000000000008ull,0x0000000500000006ull,0x00000000000002f8ull,
0x0000000000000000ull,0x0000000000000000ull,0x0000000000000070ull,0x0000000000000070ull,
0x0000000000000008ull,0x0000000500000001ull,0x00000000000002f8ull,0x0000000000000000ull,
0x0000000000000000ull,0x0000000000000070ull,0x0000000000000070ull,0x0000000000000008ull,

};
#pragma const_seg()

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
#pragma const_seg(".nvFatBinSegment")
__declspec(allocate(__CUDAFATBINSECTION)) __declspec(align(8)) static const __fatBinC_Wrapper_t __fatDeviceText= 
	{ 0x466243b1, 2, fatbinData, (void**)__cudaPrelinkedFatbins };
#pragma const_seg()
#ifdef __cplusplus
}
#endif