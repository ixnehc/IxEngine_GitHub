#include "stdh.h"

#include <xmmintrin.h>
#include <emmintrin.h>
#include <sal.h>



typedef __m128 XMVECTOR;

typedef const XMVECTOR FXMVECTOR;


#define _DECLSPEC_ALIGN_16_   __declspec(align(16))

typedef _DECLSPEC_ALIGN_16_ struct XMVECTORF32 {
	union {
		float f[4];
		XMVECTOR v;
	};

#if defined(__cplusplus)
	inline operator XMVECTOR() const { return v; }
	inline operator const float*() const { return f; }
	inline operator __m128i() const { return reinterpret_cast<const __m128i *>(&v)[0]; }
	inline operator __m128d() const { return reinterpret_cast<const __m128d *>(&v)[0]; }
#endif // __cplusplus
} XMVECTORF32;

typedef _DECLSPEC_ALIGN_16_ struct XMVECTORI32 {
	union {
		INT i[4];
		XMVECTOR v;
	};
#if defined(__cplusplus)
	inline operator XMVECTOR() const { return v; }
	inline operator __m128i() const { return reinterpret_cast<const __m128i *>(&v)[0]; }
	inline operator __m128d() const { return reinterpret_cast<const __m128d *>(&v)[0]; }
#endif // __cplusplus
} XMVECTORI32;

#define XM_PI               3.141592654f
#define XM_2PI              6.283185307f
#define XM_1DIVPI           0.318309886f
#define XM_1DIV2PI          0.159154943f
#define XM_PIDIV2           1.570796327f
#define XM_PIDIV4           0.785398163f

#define XM_SELECT_0         0x00000000
#define XM_SELECT_1         0xFFFFFFFF

#define XM_PERMUTE_0X       0x00010203
#define XM_PERMUTE_0Y       0x04050607
#define XM_PERMUTE_0Z       0x08090A0B
#define XM_PERMUTE_0W       0x0C0D0E0F
#define XM_PERMUTE_1X       0x10111213
#define XM_PERMUTE_1Y       0x14151617
#define XM_PERMUTE_1Z       0x18191A1B
#define XM_PERMUTE_1W       0x1C1D1E1F

#define XM_CRMASK_CR6       0x000000F0
#define XM_CRMASK_CR6TRUE   0x00000080
#define XM_CRMASK_CR6FALSE  0x00000020
#define XM_CRMASK_CR6BOUNDS XM_CRMASK_CR6FALSE


#define XM_CACHE_LINE_SIZE  64


#define XMGLOBALCONST extern CONST __declspec(selectany)

XMGLOBALCONST XMVECTORF32 g_XMSinCoefficients0    = {1.0f, -0.166666667f, 8.333333333e-3f, -1.984126984e-4f};
XMGLOBALCONST XMVECTORF32 g_XMSinCoefficients1    = {2.755731922e-6f, -2.505210839e-8f, 1.605904384e-10f, -7.647163732e-13f};
XMGLOBALCONST XMVECTORF32 g_XMSinCoefficients2    = {2.811457254e-15f, -8.220635247e-18f, 1.957294106e-20f, -3.868170171e-23f};
XMGLOBALCONST XMVECTORF32 g_XMCosCoefficients0    = {1.0f, -0.5f, 4.166666667e-2f, -1.388888889e-3f};
XMGLOBALCONST XMVECTORF32 g_XMCosCoefficients1    = {2.480158730e-5f, -2.755731922e-7f, 2.087675699e-9f, -1.147074560e-11f};
XMGLOBALCONST XMVECTORF32 g_XMCosCoefficients2    = {4.779477332e-14f, -1.561920697e-16f, 4.110317623e-19f, -8.896791392e-22f};
XMGLOBALCONST XMVECTORF32 g_XMTanCoefficients0    = {1.0f, 0.333333333f, 0.133333333f, 5.396825397e-2f};
XMGLOBALCONST XMVECTORF32 g_XMTanCoefficients1    = {2.186948854e-2f, 8.863235530e-3f, 3.592128167e-3f, 1.455834485e-3f};
XMGLOBALCONST XMVECTORF32 g_XMTanCoefficients2    = {5.900274264e-4f, 2.391290764e-4f, 9.691537707e-5f, 3.927832950e-5f};
XMGLOBALCONST XMVECTORF32 g_XMASinCoefficients0   = {-0.05806367563904f, -0.41861972469416f, 0.22480114791621f, 2.17337241360606f};
XMGLOBALCONST XMVECTORF32 g_XMASinCoefficients1   = {0.61657275907170f, 4.29696498283455f, -1.18942822255452f, -6.53784832094831f};
XMGLOBALCONST XMVECTORF32 g_XMASinCoefficients2   = {-1.36926553863413f, -4.48179294237210f, 1.41810672941833f, 5.48179257935713f};
XMGLOBALCONST XMVECTORF32 g_XMATanCoefficients0   = {1.0f, 0.333333334f, 0.2f, 0.142857143f};
XMGLOBALCONST XMVECTORF32 g_XMATanCoefficients1   = {1.111111111e-1f, 9.090909091e-2f, 7.692307692e-2f, 6.666666667e-2f};
XMGLOBALCONST XMVECTORF32 g_XMATanCoefficients2   = {5.882352941e-2f, 5.263157895e-2f, 4.761904762e-2f, 4.347826087e-2f};
XMGLOBALCONST XMVECTORF32 g_XMSinEstCoefficients  = {1.0f, -1.66521856991541e-1f, 8.199913018755e-3f, -1.61475937228e-4f};
XMGLOBALCONST XMVECTORF32 g_XMCosEstCoefficients  = {1.0f, -4.95348008918096e-1f, 3.878259962881e-2f, -9.24587976263e-4f};
XMGLOBALCONST XMVECTORF32 g_XMTanEstCoefficients  = {2.484f, -1.954923183e-1f, 2.467401101f, XM_1DIVPI};
XMGLOBALCONST XMVECTORF32 g_XMATanEstCoefficients = {7.689891418951e-1f, 1.104742493348f, 8.661844266006e-1f, XM_PIDIV2};
XMGLOBALCONST XMVECTORF32 g_XMASinEstCoefficients = {-1.36178272886711f, 2.37949493464538f, -8.08228565650486e-1f, 2.78440142746736e-1f};
XMGLOBALCONST XMVECTORF32 g_XMASinEstConstants    = {1.00000011921f, XM_PIDIV2, 0.0f, 0.0f};
XMGLOBALCONST XMVECTORF32 g_XMPiConstants0        = {XM_PI, XM_2PI, XM_1DIVPI, XM_1DIV2PI};
XMGLOBALCONST XMVECTORF32 g_XMIdentityR0          = {1.0f, 0.0f, 0.0f, 0.0f};
XMGLOBALCONST XMVECTORF32 g_XMIdentityR1          = {0.0f, 1.0f, 0.0f, 0.0f};
XMGLOBALCONST XMVECTORF32 g_XMIdentityR2          = {0.0f, 0.0f, 1.0f, 0.0f};
XMGLOBALCONST XMVECTORF32 g_XMIdentityR3          = {0.0f, 0.0f, 0.0f, 1.0f};
XMGLOBALCONST XMVECTORF32 g_XMNegIdentityR0       = {-1.0f,0.0f, 0.0f, 0.0f};
XMGLOBALCONST XMVECTORF32 g_XMNegIdentityR1       = {0.0f,-1.0f, 0.0f, 0.0f};
XMGLOBALCONST XMVECTORF32 g_XMNegIdentityR2       = {0.0f, 0.0f,-1.0f, 0.0f};
XMGLOBALCONST XMVECTORF32 g_XMNegIdentityR3       = {0.0f, 0.0f, 0.0f,-1.0f};
XMGLOBALCONST XMVECTORI32 g_XMNegativeZero      = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
XMGLOBALCONST XMVECTORI32 g_XMNegate3           = {0x80000000, 0x80000000, 0x80000000, 0x00000000};
XMGLOBALCONST XMVECTORI32 g_XMMask3             = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
XMGLOBALCONST XMVECTORI32 g_XMMaskX             = {0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000};
XMGLOBALCONST XMVECTORI32 g_XMMaskY             = {0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000};
XMGLOBALCONST XMVECTORI32 g_XMMaskZ             = {0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000};
XMGLOBALCONST XMVECTORI32 g_XMMaskW             = {0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF};
XMGLOBALCONST XMVECTORF32 g_XMOne               = { 1.0f, 1.0f, 1.0f, 1.0f};
XMGLOBALCONST XMVECTORF32 g_XMOne3              = { 1.0f, 1.0f, 1.0f, 0.0f};
XMGLOBALCONST XMVECTORF32 g_XMZero              = { 0.0f, 0.0f, 0.0f, 0.0f};
XMGLOBALCONST XMVECTORF32 g_XMNegativeOne       = {-1.0f,-1.0f,-1.0f,-1.0f};
XMGLOBALCONST XMVECTORF32 g_XMOneHalf           = { 0.5f, 0.5f, 0.5f, 0.5f};
XMGLOBALCONST XMVECTORF32 g_XMNegativeOneHalf   = {-0.5f,-0.5f,-0.5f,-0.5f};
XMGLOBALCONST XMVECTORF32 g_XMNegativeTwoPi     = {-XM_2PI, -XM_2PI, -XM_2PI, -XM_2PI};
XMGLOBALCONST XMVECTORF32 g_XMNegativePi        = {-XM_PI, -XM_PI, -XM_PI, -XM_PI};
XMGLOBALCONST XMVECTORF32 g_XMHalfPi            = {XM_PIDIV2, XM_PIDIV2, XM_PIDIV2, XM_PIDIV2};
XMGLOBALCONST XMVECTORF32 g_XMPi                = {XM_PI, XM_PI, XM_PI, XM_PI};
XMGLOBALCONST XMVECTORF32 g_XMReciprocalPi      = {XM_1DIVPI, XM_1DIVPI, XM_1DIVPI, XM_1DIVPI};
XMGLOBALCONST XMVECTORF32 g_XMTwoPi             = {XM_2PI, XM_2PI, XM_2PI, XM_2PI};
XMGLOBALCONST XMVECTORF32 g_XMReciprocalTwoPi   = {XM_1DIV2PI, XM_1DIV2PI, XM_1DIV2PI, XM_1DIV2PI};
XMGLOBALCONST XMVECTORF32 g_XMEpsilon           = {1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f, 1.192092896e-7f};
XMGLOBALCONST XMVECTORI32 g_XMInfinity          = {0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000};
XMGLOBALCONST XMVECTORI32 g_XMQNaN              = {0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000};
XMGLOBALCONST XMVECTORI32 g_XMQNaNTest          = {0x007FFFFF, 0x007FFFFF, 0x007FFFFF, 0x007FFFFF};
XMGLOBALCONST XMVECTORI32 g_XMAbsMask           = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
XMGLOBALCONST XMVECTORI32 g_XMFltMin            = {0x00800000, 0x00800000, 0x00800000, 0x00800000};
XMGLOBALCONST XMVECTORI32 g_XMFltMax            = {0x7F7FFFFF, 0x7F7FFFFF, 0x7F7FFFFF, 0x7F7FFFFF};
XMGLOBALCONST XMVECTORI32 g_XMNegOneMask		= {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
XMGLOBALCONST XMVECTORI32 g_XMMaskA8R8G8B8      = {0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000};
XMGLOBALCONST XMVECTORI32 g_XMFlipA8R8G8B8      = {0x00000000, 0x00000000, 0x00000000, 0x80000000};
XMGLOBALCONST XMVECTORF32 g_XMFixAA8R8G8B8      = {0.0f,0.0f,0.0f,(float)(0x80000000U)};
XMGLOBALCONST XMVECTORF32 g_XMNormalizeA8R8G8B8 = {1.0f/(255.0f*(float)(0x10000)),1.0f/(255.0f*(float)(0x100)),1.0f/255.0f,1.0f/(255.0f*(float)(0x1000000))};
XMGLOBALCONST XMVECTORI32 g_XMMaskA2B10G10R10   = {0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000};
XMGLOBALCONST XMVECTORI32 g_XMFlipA2B10G10R10   = {0x00000200, 0x00080000, 0x20000000, 0x80000000};
XMGLOBALCONST XMVECTORF32 g_XMFixAA2B10G10R10   = {-512.0f,-512.0f*(float)(0x400),-512.0f*(float)(0x100000),(float)(0x80000000U)};
XMGLOBALCONST XMVECTORF32 g_XMNormalizeA2B10G10R10 = {1.0f/511.0f,1.0f/(511.0f*(float)(0x400)),1.0f/(511.0f*(float)(0x100000)),1.0f/(3.0f*(float)(0x40000000))};
XMGLOBALCONST XMVECTORI32 g_XMMaskX16Y16        = {0x0000FFFF, 0xFFFF0000, 0x00000000, 0x00000000};
XMGLOBALCONST XMVECTORI32 g_XMFlipX16Y16        = {0x00008000, 0x00000000, 0x00000000, 0x00000000};
XMGLOBALCONST XMVECTORF32 g_XMFixX16Y16         = {-32768.0f,0.0f,0.0f,0.0f};
XMGLOBALCONST XMVECTORF32 g_XMNormalizeX16Y16   = {1.0f/32767.0f,1.0f/(32767.0f*65536.0f),0.0f,0.0f};
XMGLOBALCONST XMVECTORI32 g_XMMaskX16Y16Z16W16  = {0x0000FFFF, 0x0000FFFF, 0xFFFF0000, 0xFFFF0000};
XMGLOBALCONST XMVECTORI32 g_XMFlipX16Y16Z16W16  = {0x00008000, 0x00008000, 0x00000000, 0x00000000};
XMGLOBALCONST XMVECTORF32 g_XMFixX16Y16Z16W16   = {-32768.0f,-32768.0f,0.0f,0.0f};
XMGLOBALCONST XMVECTORF32 g_XMNormalizeX16Y16Z16W16 = {1.0f/32767.0f,1.0f/32767.0f,1.0f/(32767.0f*65536.0f),1.0f/(32767.0f*65536.0f)};
XMGLOBALCONST XMVECTORF32 g_XMNoFraction        = {8388608.0f,8388608.0f,8388608.0f,8388608.0f};
XMGLOBALCONST XMVECTORI32 g_XMMaskByte          = {0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF};
XMGLOBALCONST XMVECTORF32 g_XMNegateX           = {-1.0f, 1.0f, 1.0f, 1.0f};
XMGLOBALCONST XMVECTORF32 g_XMNegateY           = { 1.0f,-1.0f, 1.0f, 1.0f};
XMGLOBALCONST XMVECTORF32 g_XMNegateZ           = { 1.0f, 1.0f,-1.0f, 1.0f};
XMGLOBALCONST XMVECTORF32 g_XMNegateW           = { 1.0f, 1.0f, 1.0f,-1.0f};
XMGLOBALCONST XMVECTORI32 g_XMSelect0101        = {XM_SELECT_0, XM_SELECT_1, XM_SELECT_0, XM_SELECT_1};
XMGLOBALCONST XMVECTORI32 g_XMSelect1010        = {XM_SELECT_1, XM_SELECT_0, XM_SELECT_1, XM_SELECT_0};
XMGLOBALCONST XMVECTORI32 g_XMOneHalfMinusEpsilon = { 0x3EFFFFFD, 0x3EFFFFFD, 0x3EFFFFFD, 0x3EFFFFFD};
XMGLOBALCONST XMVECTORI32 g_XMSelect1000        = {XM_SELECT_1, XM_SELECT_0, XM_SELECT_0, XM_SELECT_0};
XMGLOBALCONST XMVECTORI32 g_XMSelect1100        = {XM_SELECT_1, XM_SELECT_1, XM_SELECT_0, XM_SELECT_0};
XMGLOBALCONST XMVECTORI32 g_XMSelect1110        = {XM_SELECT_1, XM_SELECT_1, XM_SELECT_1, XM_SELECT_0};
XMGLOBALCONST XMVECTORI32 g_XMSwizzleXYXY       = {XM_PERMUTE_0X, XM_PERMUTE_0Y, XM_PERMUTE_0X, XM_PERMUTE_0Y};
XMGLOBALCONST XMVECTORI32 g_XMSwizzleXYZX       = {XM_PERMUTE_0X, XM_PERMUTE_0Y, XM_PERMUTE_0Z, XM_PERMUTE_0X};
XMGLOBALCONST XMVECTORI32 g_XMSwizzleYXZW       = {XM_PERMUTE_0Y, XM_PERMUTE_0X, XM_PERMUTE_0Z, XM_PERMUTE_0W};
XMGLOBALCONST XMVECTORI32 g_XMSwizzleYZXW       = {XM_PERMUTE_0Y, XM_PERMUTE_0Z, XM_PERMUTE_0X, XM_PERMUTE_0W};
XMGLOBALCONST XMVECTORI32 g_XMSwizzleZXYW       = {XM_PERMUTE_0Z, XM_PERMUTE_0X, XM_PERMUTE_0Y, XM_PERMUTE_0W};
XMGLOBALCONST XMVECTORI32 g_XMPermute0X0Y1X1Y   = {XM_PERMUTE_0X, XM_PERMUTE_0Y, XM_PERMUTE_1X, XM_PERMUTE_1Y};
XMGLOBALCONST XMVECTORI32 g_XMPermute0Z0W1Z1W   = {XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_1W};
XMGLOBALCONST XMVECTORF32 g_XMFixupY16          = {1.0f,1.0f/65536.0f,0.0f,0.0f};
XMGLOBALCONST XMVECTORF32 g_XMFixupY16W16       = {1.0f,1.0f,1.0f/65536.0f,1.0f/65536.0f};
XMGLOBALCONST XMVECTORI32 g_XMFlipY             = {0,0x80000000,0,0};
XMGLOBALCONST XMVECTORI32 g_XMFlipZ             = {0,0,0x80000000,0};
XMGLOBALCONST XMVECTORI32 g_XMFlipW             = {0,0,0,0x80000000};
XMGLOBALCONST XMVECTORI32 g_XMFlipYZ            = {0,0x80000000,0x80000000,0};
XMGLOBALCONST XMVECTORI32 g_XMFlipZW            = {0,0,0x80000000,0x80000000};
XMGLOBALCONST XMVECTORI32 g_XMFlipYW            = {0,0x80000000,0,0x80000000};
XMGLOBALCONST XMVECTORI32 g_XMMaskHenD3         = {0x7FF,0x7ff<<11,0x3FF<<22,0};
XMGLOBALCONST XMVECTORI32 g_XMMaskDHen3         = {0x3FF,0x7ff<<10,0x7FF<<21,0};
XMGLOBALCONST XMVECTORF32 g_XMAddUHenD3         = {0,0,32768.0f*65536.0f,0};
XMGLOBALCONST XMVECTORF32 g_XMAddHenD3          = {-1024.0f,-1024.0f*2048.0f,0,0};
XMGLOBALCONST XMVECTORF32 g_XMAddDHen3          = {-512.0f,-1024.0f*1024.0f,0,0};
XMGLOBALCONST XMVECTORF32 g_XMMulHenD3          = {1.0f,1.0f/2048.0f,1.0f/(2048.0f*2048.0f),0};
XMGLOBALCONST XMVECTORF32 g_XMMulDHen3          = {1.0f,1.0f/1024.0f,1.0f/(1024.0f*2048.0f),0};
XMGLOBALCONST XMVECTORI32 g_XMXorHenD3          = {0x400,0x400<<11,0,0};
XMGLOBALCONST XMVECTORI32 g_XMXorDHen3          = {0x200,0x400<<10,0,0};
XMGLOBALCONST XMVECTORI32 g_XMMaskIco4          = {0xFFFFF,0xFFFFF000,0xFFFFF,0xF0000000};
XMGLOBALCONST XMVECTORI32 g_XMXorXIco4          = {0x80000,0,0x80000,0x80000000};
XMGLOBALCONST XMVECTORI32 g_XMXorIco4           = {0x80000,0,0x80000,0};
XMGLOBALCONST XMVECTORF32 g_XMAddXIco4          = {-8.0f*65536.0f,0,-8.0f*65536.0f,32768.0f*65536.0f};
XMGLOBALCONST XMVECTORF32 g_XMAddUIco4          = {0,32768.0f*65536.0f,0,32768.0f*65536.0f};
XMGLOBALCONST XMVECTORF32 g_XMAddIco4           = {-8.0f*65536.0f,0,-8.0f*65536.0f,0};
XMGLOBALCONST XMVECTORF32 g_XMMulIco4           = {1.0f,1.0f/4096.0f,1.0f,1.0f/(4096.0f*65536.0f)};
XMGLOBALCONST XMVECTORI32 g_XMMaskDec4          = {0x3FF,0x3FF<<10,0x3FF<<20,0x3<<30};
XMGLOBALCONST XMVECTORI32 g_XMXorDec4           = {0x200,0x200<<10,0x200<<20,0};
XMGLOBALCONST XMVECTORF32 g_XMAddUDec4          = {0,0,0,32768.0f*65536.0f};
XMGLOBALCONST XMVECTORF32 g_XMAddDec4           = {-512.0f,-512.0f*1024.0f,-512.0f*1024.0f*1024.0f,0};
XMGLOBALCONST XMVECTORF32 g_XMMulDec4           = {1.0f,1.0f/1024.0f,1.0f/(1024.0f*1024.0f),1.0f/(1024.0f*1024.0f*1024.0f)};
XMGLOBALCONST XMVECTORI32 g_XMMaskByte4         = {0xFF,0xFF00,0xFF0000,0xFF000000};
XMGLOBALCONST XMVECTORI32 g_XMXorByte4          = {0x80,0x8000,0x800000,0x00000000};
XMGLOBALCONST XMVECTORF32 g_XMAddByte4          = {-128.0f,-128.0f*256.0f,-128.0f*65536.0f,0};
XMGLOBALCONST XMVECTORF32 g_XMFixUnsigned       = {32768.0f*65536.0f,32768.0f*65536.0f,32768.0f*65536.0f,32768.0f*65536.0f};
XMGLOBALCONST XMVECTORF32 g_XMMaxInt            = {65536.0f*32768.0f-128.0f,65536.0f*32768.0f-128.0f,65536.0f*32768.0f-128.0f,65536.0f*32768.0f-128.0f};
XMGLOBALCONST XMVECTORF32 g_XMMaxUInt           = {65536.0f*65536.0f-256.0f,65536.0f*65536.0f-256.0f,65536.0f*65536.0f-256.0f,65536.0f*65536.0f-256.0f};
XMGLOBALCONST XMVECTORF32 g_XMUnsignedFix       = {32768.0f*65536.0f,32768.0f*65536.0f,32768.0f*65536.0f,32768.0f*65536.0f};

#define XMFINLINE __forceinline
#define XMINLINE __inline

#define XMASSERT(A) 

#define _MM_SHUFFLE(fp3,fp2,fp1,fp0) (((fp3) << 6) | ((fp2) << 4) | \
                                     ((fp1) << 2) | ((fp0)))



XMFINLINE XMVECTOR XMVectorReplicate( FLOAT Value )
{
	return _mm_set_ps1( Value );
}


XMFINLINE XMVECTOR XMVectorSet( FLOAT x,  FLOAT y,  FLOAT z,  FLOAT w )
{
	return _mm_set_ps( w, z, y, x );
}


XMFINLINE XMVECTOR XMVector4Dot( FXMVECTOR V1,  FXMVECTOR V2 )
{
	XMVECTOR vTemp2 = V2;
	XMVECTOR vTemp = _mm_mul_ps(V1,vTemp2);
	vTemp2 = _mm_shuffle_ps(vTemp2,vTemp,_MM_SHUFFLE(1,0,0,0)); // Copy X to the Z position and Y to the W position
	vTemp2 = _mm_add_ps(vTemp2,vTemp);          // Add Z = X+Z; W = Y+W;
	vTemp = _mm_shuffle_ps(vTemp,vTemp2,_MM_SHUFFLE(0,3,0,0));  // Copy W to the Z position
	vTemp = _mm_add_ps(vTemp,vTemp2);           // Add Z and W together
	return _mm_shuffle_ps(vTemp,vTemp,_MM_SHUFFLE(2,2,2,2));    // Splat Z and return
}

XMFINLINE XMVECTOR XMVectorZero()
{
	return _mm_setzero_ps();
}

XMFINLINE XMVECTOR XMVectorLess( FXMVECTOR V1,  FXMVECTOR V2 )
{
	return _mm_cmplt_ps( V1, V2 );
}


XMFINLINE XMVECTOR XMQuaternionDot( FXMVECTOR Q1, FXMVECTOR Q2)
{
	return XMVector4Dot(Q1, Q2);
}

XMFINLINE XMVECTOR XMVectorSelect( FXMVECTOR V1,  FXMVECTOR V2,  FXMVECTOR Control )
{
	XMVECTOR vTemp1 = _mm_andnot_ps(Control,V1);
	XMVECTOR vTemp2 = _mm_and_ps(V2,Control);
	return _mm_or_ps(vTemp1,vTemp2);
}

XMFINLINE XMVECTOR XMVectorIsInfinite( FXMVECTOR V )
{
	// Mask off the sign bit
	__m128 vTemp = _mm_and_ps(V,g_XMAbsMask);
	// Compare to infinity
	vTemp = _mm_cmpeq_ps(vTemp,g_XMInfinity);
	// If any are infinity, the signs are true.
	return vTemp;
}

XMFINLINE XMVECTOR XMVectorEqualInt( FXMVECTOR V1,  FXMVECTOR V2 )
{
	__m128i V = _mm_cmpeq_epi32( reinterpret_cast<const __m128i *>(&V1)[0],reinterpret_cast<const __m128i *>(&V2)[0] );
	return reinterpret_cast<__m128 *>(&V)[0];
}

XMFINLINE XMVECTOR XMVectorAbs( FXMVECTOR V )
{
	XMVECTOR vResult = _mm_setzero_ps();
	vResult = _mm_sub_ps(vResult,V);
	vResult = _mm_max_ps(vResult,V);
	return vResult;
}

XMFINLINE XMVECTOR XMVectorReciprocal( FXMVECTOR V )
{
	return _mm_div_ps(g_XMOne,V);
}


XMINLINE XMVECTOR XMVectorATan( FXMVECTOR V )
{
	static CONST XMVECTORF32 ATanConstants0 = {-1.3688768894e+1f, -2.0505855195e+1f, -8.4946240351f, -8.3758299368e-1f};
	static CONST XMVECTORF32 ATanConstants1 = {4.1066306682e+1f, 8.6157349597e+1f, 5.9578436142e+1f, 1.5024001160e+1f};
	static CONST XMVECTORF32 ATanConstants2 = {1.732050808f, 7.320508076e-1f, 2.679491924e-1f, 0.000244140625f}; // <sqrt(3), sqrt(3) - 1, 2 - sqrt(3), Epsilon>
	static CONST XMVECTORF32 ATanConstants3 = {XM_PIDIV2, XM_PI / 3.0f, XM_PI / 6.0f, 8.507059173e+37f}; // <Pi / 2, Pi / 3, Pi / 6, MaxV>

	XMVECTOR VF = XMVectorAbs(V);
	XMVECTOR F_GT_One = _mm_cmpgt_ps(VF,g_XMOne);
	XMVECTOR ReciprocalF = XMVectorReciprocal(VF);
	VF = XMVectorSelect(VF, ReciprocalF, F_GT_One);
	XMVECTOR Zero = XMVectorZero();
	XMVECTOR HalfPi = _mm_load_ps1(&ATanConstants3.f[0]);
	XMVECTOR Angle1 = XMVectorSelect(Zero, HalfPi, F_GT_One);
	// Pi/3
	XMVECTOR vConstants = _mm_load_ps1(&ATanConstants3.f[1]);
	// Pi/6
	XMVECTOR Angle2 = _mm_load_ps1(&ATanConstants3.f[2]);
	Angle2 = XMVectorSelect(Angle2, vConstants, F_GT_One);

	// 1-sqrt(3)
	XMVECTOR FA = _mm_load_ps1(&ATanConstants2.f[1]);
	FA = _mm_mul_ps(FA,VF);
	FA = _mm_add_ps(FA,VF);
	FA = _mm_add_ps(FA,g_XMNegativeOne);
	// sqrt(3)
	vConstants = _mm_load_ps1(&ATanConstants2.f[0]);
	vConstants = _mm_add_ps(vConstants,VF);
	FA = _mm_div_ps(FA,vConstants);

	// 2-sqrt(3)
	vConstants = _mm_load_ps1(&ATanConstants2.f[2]);
	// >2-sqrt(3)?
	vConstants = _mm_cmpgt_ps(VF,vConstants);
	VF = XMVectorSelect(VF, FA, vConstants);
	Angle1 = XMVectorSelect(Angle1, Angle2, vConstants);

	XMVECTOR AbsF = XMVectorAbs(VF);

	XMVECTOR G = _mm_mul_ps(VF,VF);
	XMVECTOR D = _mm_load_ps1(&ATanConstants1.f[3]);
	D = _mm_add_ps(D,G);
	D = _mm_mul_ps(D,G);
	vConstants = _mm_load_ps1(&ATanConstants1.f[2]);
	D = _mm_add_ps(D,vConstants);
	D = _mm_mul_ps(D,G);
	vConstants = _mm_load_ps1(&ATanConstants1.f[1]);
	D = _mm_add_ps(D,vConstants);
	D = _mm_mul_ps(D,G);
	vConstants = _mm_load_ps1(&ATanConstants1.f[0]);
	D = _mm_add_ps(D,vConstants);

	XMVECTOR N = _mm_load_ps1(&ATanConstants0.f[3]);
	N = _mm_mul_ps(N,G);
	vConstants = _mm_load_ps1(&ATanConstants0.f[2]);
	N = _mm_add_ps(N,vConstants);
	N = _mm_mul_ps(N,G);
	vConstants = _mm_load_ps1(&ATanConstants0.f[1]);
	N = _mm_add_ps(N,vConstants);
	N = _mm_mul_ps(N,G);
	vConstants = _mm_load_ps1(&ATanConstants0.f[0]);
	N = _mm_add_ps(N,vConstants);
	N = _mm_mul_ps(N,G);
	XMVECTOR Result = _mm_div_ps(N,D);

	Result = _mm_mul_ps(Result,VF);
	Result = _mm_add_ps(Result,VF);
	// Epsilon
	vConstants = _mm_load_ps1(&ATanConstants2.f[3]);
	vConstants = _mm_cmpge_ps(vConstants,AbsF);
	Result = XMVectorSelect(Result,VF,vConstants);

	XMVECTOR NegativeResult = _mm_mul_ps(Result,g_XMNegativeOne);
	Result = XMVectorSelect(Result,NegativeResult,F_GT_One);
	Result = _mm_add_ps(Result,Angle1);

	Zero = _mm_cmpge_ps(Zero,V);
	NegativeResult = _mm_mul_ps(Result,g_XMNegativeOne);
	Result = XMVectorSelect(Result,NegativeResult,Zero);

	XMVECTOR MaxV = _mm_load_ps1(&ATanConstants3.f[3]);
	XMVECTOR MinV = _mm_mul_ps(MaxV,g_XMNegativeOne);
	// Negate HalfPi
	HalfPi = _mm_mul_ps(HalfPi,g_XMNegativeOne);
	MaxV = _mm_cmple_ps(MaxV,V);
	MinV = _mm_cmpge_ps(MinV,V);
	Result = XMVectorSelect(Result,g_XMHalfPi,MaxV);
	// HalfPi = -HalfPi
	Result = XMVectorSelect(Result,HalfPi,MinV);
	return Result;
}



XMINLINE XMVECTOR XMVectorATan2( FXMVECTOR Y,  FXMVECTOR X )
{
	static CONST XMVECTORF32 ATan2Constants = {XM_PI, XM_PIDIV2, XM_PIDIV4, XM_PI * 3.0f / 4.0f};

	// Mask if Y>0 && Y!=INF
	XMVECTOR YEqualsInfinity = XMVectorIsInfinite(Y);
	// Get the sign of (Y&0x80000000)
	XMVECTOR YSign = _mm_and_ps(Y, g_XMNegativeZero);
	// Get the sign bits of X
	XMVECTOR XIsPositive = _mm_and_ps(X,g_XMNegativeZero);
	// Change them to masks
	XIsPositive = XMVectorEqualInt(XIsPositive,g_XMZero);
	// Get Pi
	XMVECTOR Pi = _mm_load_ps1(&ATan2Constants.f[0]);
	// Copy the sign of Y
	Pi = _mm_or_ps(Pi,YSign);
	XMVECTOR R1 = XMVectorSelect(Pi,YSign,XIsPositive);
	// Mask for X==0
	XMVECTOR vConstants = _mm_cmpeq_ps(X,g_XMZero);
	// Get Pi/2 with with sign of Y
	XMVECTOR PiOverTwo = _mm_load_ps1(&ATan2Constants.f[1]);
	PiOverTwo = _mm_or_ps(PiOverTwo,YSign);
	XMVECTOR R2 = XMVectorSelect(g_XMNegOneMask,PiOverTwo,vConstants);
	// Mask for Y==0
	vConstants = _mm_cmpeq_ps(Y,g_XMZero);
	R2 = XMVectorSelect(R2,R1,vConstants);
	// Get Pi/4 with sign of Y
	XMVECTOR PiOverFour = _mm_load_ps1(&ATan2Constants.f[2]);
	PiOverFour = _mm_or_ps(PiOverFour,YSign);
	// Get (Pi*3)/4 with sign of Y
	XMVECTOR ThreePiOverFour = _mm_load_ps1(&ATan2Constants.f[3]);
	ThreePiOverFour = _mm_or_ps(ThreePiOverFour,YSign);
	vConstants = XMVectorSelect(ThreePiOverFour, PiOverFour, XIsPositive);
	XMVECTOR XEqualsInfinity = XMVectorIsInfinite(X);
	vConstants = XMVectorSelect(PiOverTwo,vConstants,XEqualsInfinity);

	XMVECTOR vResult = XMVectorSelect(R2,vConstants,YEqualsInfinity);
	vConstants = XMVectorSelect(R1,vResult,YEqualsInfinity);
	// At this point, any entry that's zero will get the result
	// from XMVectorATan(), otherwise, return the failsafe value
	vResult = XMVectorSelect(vResult,vConstants,XEqualsInfinity);
	// Any entries not 0xFFFFFFFF, are considered precalculated
	XMVECTOR ATanResultValid = XMVectorEqualInt(vResult,g_XMNegOneMask);
	// Let's do the ATan2 function
	vConstants = _mm_div_ps(Y,X);
	vConstants = XMVectorATan(vConstants);
	// Discard entries that have been declared void

	XMVECTOR R3 = XMVectorSelect( Pi, g_XMZero, XIsPositive );
	vConstants = _mm_add_ps( vConstants, R3 );

	vResult = XMVectorSelect(vResult,vConstants,ATanResultValid);
	return vResult;
}

XMFINLINE XMVECTOR XMVectorRound( FXMVECTOR V )
{
	// To handle NAN, INF and numbers greater than 8388608, use masking
	// Get the abs value
	__m128i vTest = _mm_and_si128(reinterpret_cast<const __m128i *>(&V)[0],g_XMAbsMask);
	// Test for greater than 8388608 (All floats with NO fractionals, NAN and INF
	vTest = _mm_cmplt_epi32(vTest,g_XMNoFraction);
	// Convert to int and back to float for rounding
	__m128i vInt = _mm_cvtps_epi32(V);
	// Convert back to floats
	XMVECTOR vResult = _mm_cvtepi32_ps(vInt);
	// All numbers less than 8388608 will use the round to int
	vResult = _mm_and_ps(vResult,reinterpret_cast<const XMVECTOR *>(&vTest)[0]);
	// All others, use the ORIGINAL value
	vTest = _mm_andnot_si128(vTest,reinterpret_cast<const __m128i *>(&V)[0]);
	vResult = _mm_or_ps(vResult,reinterpret_cast<const XMVECTOR *>(&vTest)[0]);
	return vResult;
}


XMFINLINE XMVECTOR XMVectorModAngles( FXMVECTOR Angles )
{
	// Modulo the range of the given angles such that -XM_PI <= Angles < XM_PI
	XMVECTOR vResult = _mm_mul_ps(Angles,g_XMReciprocalTwoPi);
	// Use the inline function due to complexity for rounding
	vResult = XMVectorRound(vResult);
	vResult = _mm_mul_ps(vResult,g_XMTwoPi);
	vResult = _mm_sub_ps(Angles,vResult);
	return vResult;
}


XMINLINE XMVECTOR XMVectorSin( FXMVECTOR V )
{
	// Force the value within the bounds of pi
	XMVECTOR vResult = XMVectorModAngles(V);
	// Each on is V to the "num" power
	// V2 = V1^2
	XMVECTOR V2  = _mm_mul_ps(vResult,vResult);
	// V1^3
	XMVECTOR vPower = _mm_mul_ps(vResult,V2);    
	XMVECTOR vConstants = _mm_load_ps1(&g_XMSinCoefficients0.f[1]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^5
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients0.f[2]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^7
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients0.f[3]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^9
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients1.f[0]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^11
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients1.f[1]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^13
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients1.f[2]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^15
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients1.f[3]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^17
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients2.f[0]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^19
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients2.f[1]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^21
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients2.f[2]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);

	// V^23
	vPower = _mm_mul_ps(vPower,V2);
	vConstants = _mm_load_ps1(&g_XMSinCoefficients2.f[3]);
	vConstants = _mm_mul_ps(vConstants,vPower);
	vResult = _mm_add_ps(vResult,vConstants);
	return vResult;
}


//------------------------------------------------------------------------------
// Replicate the y component of the vector
XMFINLINE XMVECTOR XMVectorSplatY( FXMVECTOR V )
{
	return _mm_shuffle_ps( V, V, _MM_SHUFFLE(1, 1, 1, 1) );
}

XMFINLINE XMVECTOR XMVectorSplatX( FXMVECTOR V )
{
	return _mm_shuffle_ps( V, V, _MM_SHUFFLE(0, 0, 0, 0) );
}



//------------------------------------------------------------------------------

XMFINLINE XMVECTOR XMQuaternionSlerpV( FXMVECTOR Q0, FXMVECTOR Q1, FXMVECTOR T )
{
	// Result = Q0 * sin((1.0 - t) * Omega) / sin(Omega) + Q1 * sin(t * Omega) / sin(Omega)
	XMVECTOR Omega;
	XMVECTOR CosOmega;
	XMVECTOR SinOmega;
	XMVECTOR V01;
	XMVECTOR S0;
	XMVECTOR S1;
	XMVECTOR Sign;
	XMVECTOR Control;
	XMVECTOR Result;
	XMVECTOR Zero;
	static const XMVECTORF32 OneMinusEpsilon = {1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f};
	static const XMVECTORI32 SignMask2 = {0x80000000,0x00000000,0x00000000,0x00000000};
	static const XMVECTORI32 MaskXY = {0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000};

	XMASSERT((XMVectorGetY(T) == XMVectorGetX(T)) && (XMVectorGetZ(T) == XMVectorGetX(T)) && (XMVectorGetW(T) == XMVectorGetX(T)));

	CosOmega = XMQuaternionDot(Q0, Q1);

	Zero = XMVectorZero();
	Control = XMVectorLess(CosOmega, Zero);
	Sign = XMVectorSelect(g_XMOne, g_XMNegativeOne, Control);

	CosOmega = _mm_mul_ps(CosOmega, Sign);

	Control = XMVectorLess(CosOmega, OneMinusEpsilon);

	SinOmega = _mm_mul_ps(CosOmega,CosOmega);
	SinOmega = _mm_sub_ps(g_XMOne,SinOmega);
	SinOmega = _mm_sqrt_ps(SinOmega);

	Omega = XMVectorATan2(SinOmega, CosOmega);

	V01 = _mm_shuffle_ps(T,T,_MM_SHUFFLE(2,3,0,1));
	V01 = _mm_and_ps(V01,MaskXY);
	V01 = _mm_xor_ps(V01,SignMask2);
	V01 = _mm_add_ps(g_XMIdentityR0, V01);

	S0 = _mm_mul_ps(V01, Omega);
	S0 = XMVectorSin(S0);
	S0 = _mm_div_ps(S0, SinOmega);

	S0 = XMVectorSelect(V01, S0, Control);

	S1 = XMVectorSplatY(S0);
	S0 = XMVectorSplatX(S0);

	S1 = _mm_mul_ps(S1, Sign);
	Result = _mm_mul_ps(Q0, S0);
	S1 = _mm_mul_ps(S1, Q1);
	Result = _mm_add_ps(Result,S1);
	return Result;
}

XMFINLINE XMVECTOR XMQuaternionSlerp( FXMVECTOR Q0, FXMVECTOR Q1, FLOAT t)
{
	XMVECTOR T = XMVectorReplicate(t);
	return XMQuaternionSlerpV(Q0, Q1, T);
}
