#include "StdAfx.h"
#include "MDX_Hash.h"
////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
///////////////////////////// DEFINES //////////////////////////////////////////////////
/* ROTATE_LEFT rotates x left n bits.
	15-April-2003 Sony: use MSVC intrinsic to save some cycles
 */
#ifdef _MSC_VER
#pragma intrinsic(_rotl)
#define ROTATE_LEFT(x, n) _rotl((x), (n))
#else
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#endif

// Constants for MD4Transform
#define MD4_S11 3
#define MD4_S12 7
#define MD4_S13 11
#define MD4_S14 19
#define MD4_S21 3
#define MD4_S22 5
#define MD4_S23 9
#define MD4_S24 13
#define MD4_S31 3
#define MD4_S32 9
#define MD4_S33 11
#define MD4_S34 15

// Basic MD4 functions
#define MD4_F(x, y, z) ((((y) ^ (z)) & (x)) ^ (z))
#define MD4_G(x, y, z) (((z) & ((x) ^ (y))) | ((x) & (y)))
#define MD4_H(x, y, z) ((x) ^ (y) ^ (z))

// Partial transformations
#define MD4_FF(a, b, c, d, x, s) \
{ \
	(a) += MD4_F((b), (c), (d)) + (x); \
	(a) = ROTATE_LEFT((a), (s)); \
}

#define MD4_GG(a, b, c, d, x, s) \
{ \
	(a) += MD4_G((b), (c), (d)) + (x) + (ULONG)0x5A827999; \
	(a) = ROTATE_LEFT((a), (s)); \
}

#define MD4_HH(a, b, c, d, x, s) \
{ \
	(a) += MD4_H((b), (c), (d)) + (x) + (ULONG)0x6ED9EBA1; \
	(a) = ROTATE_LEFT((a), (s)); \
}

// defines for MD5
#define MD5_S11 7
#define MD5_S12 12
#define MD5_S13 17
#define MD5_S14 22
#define MD5_S21 5
#define MD5_S22 9
#define MD5_S23 14
#define MD5_S24 20
#define MD5_S31 4
#define MD5_S32 11
#define MD5_S33 16
#define MD5_S34 23
#define MD5_S41 6
#define MD5_S42 10
#define MD5_S43 15
#define MD5_S44 21

/* F, G, H and I are basic MD5 functions. */
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define MD5_FF(a, b, c, d, x, s, ac) { \
	(a) += MD5_F ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
#define MD5_GG(a, b, c, d, x, s, ac) { \
	(a) += MD5_G ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
#define MD5_HH(a, b, c, d, x, s, ac) { \
	(a) += MD5_H ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
#define MD5_II(a, b, c, d, x, s, ac) { \
	(a) += MD5_I ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
///////////////////////////// CONSTANTS ///////////////////////////////////////////////
static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const TCHAR s_acHexDigits[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
////////////////////////////////////////////////////////////////////////////////////////
MD5Sum::MD5Sum()
{
	m_strMD5Hash = "";
}
////////////////////////////////////////////////////////////////////////////////////////
MD5Sum::MD5Sum(CString sSource)
{
	Calculate(sSource);
}
////////////////////////////////////////////////////////////////////////////////////////
CString MD5Sum::Calculate(CString sSource)
{
	MD5_CTX context;
	unsigned char digest[16];

	MD5Init (&context);
	MD5Update (&context, (unsigned char *)sSource.GetBuffer(0), sSource.GetLength());
	MD5Final (digest, &context);

	m_strMD5Hash = "";
	for (int i = 0; i < 16; i++)
	{
		CString sT;
		sT.Format(_T("%02x"), digest[i]);
		m_strMD5Hash += sT;
	}

	return m_strMD5Hash;
}
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// MD5 initialization. Begins an MD5 operation, writing a new context.
void MD5Init (MD5_CTX *context)
{
	context->count[0] = context->count[1] = 0;
	/* Load magic initialization constants.*/
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
}
////////////////////////////////////////////////////////////////////////////////////////
// MD5 block update operation. Continues an MD5 message-digest
// operation, processing another message block, and updating the
// context.
void MD5Update (MD5_CTX *context, unsigned char *input, unsigned int inputLen)
{
	unsigned int i, index, partLen;
	/* Compute number of bytes mod 64 */
	index = (unsigned int)((context->count[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ((context->count[0] += ((UINT4)inputLen << 3))  < ((UINT4)inputLen << 3))
	{
		context->count[1]++;
	}
	context->count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	/* Transform as many times as possible.*/
	if (inputLen >= partLen) 
	{
		MD5_memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
		MD5Transform (context->state, context->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
		{
			MD5Transform (context->state, &input[i]);
		}
		index = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	MD5_memcpy((POINTER)&context->buffer[index], (POINTER)&input[i],inputLen-i);
}
////////////////////////////////////////////////////////////////////////////////////////
// MD5 finalization. Ends an MD5 message-digest operation, writing the
//  the message digest and zeroizing the context.
void MD5Final (unsigned char digest[16], MD5_CTX *context)
{
	unsigned char bits[8];
	unsigned int index, padLen;

	/* Save number of bits */
	Encode (bits, context->count, 8);

	/* Pad out to 56 mod 64.*/
	index = (unsigned int)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	MD5Update (context, PADDING, padLen);

	/* Append length (before padding) */
	MD5Update (context, bits, 8);
	/* Store state in digest */
	Encode (digest, context->state, 16);

	/* Zeroize sensitive information.*/
	MD5_memset ((POINTER)context, 0, sizeof (*context));
}
////////////////////////////////////////////////////////////////////////////////////////
/* MD5 basic transformation. Transforms state based on block. */
static void __fastcall MD5Transform (UINT4 state[4], unsigned char block[64])
{
	UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	Decode (x, block, 64);

	/* Round 1 */
	MD5_FF (a, b, c, d, x[ 0], MD5_S11, 0xd76aa478); /* 1 */
	MD5_FF (d, a, b, c, x[ 1], MD5_S12, 0xe8c7b756); /* 2 */
	MD5_FF (c, d, a, b, x[ 2], MD5_S13, 0x242070db); /* 3 */
	MD5_FF (b, c, d, a, x[ 3], MD5_S14, 0xc1bdceee); /* 4 */
	MD5_FF (a, b, c, d, x[ 4], MD5_S11, 0xf57c0faf); /* 5 */
	MD5_FF (d, a, b, c, x[ 5], MD5_S12, 0x4787c62a); /* 6 */
	MD5_FF (c, d, a, b, x[ 6], MD5_S13, 0xa8304613); /* 7 */
	MD5_FF (b, c, d, a, x[ 7], MD5_S14, 0xfd469501); /* 8 */
	MD5_FF (a, b, c, d, x[ 8], MD5_S11, 0x698098d8); /* 9 */
	MD5_FF (d, a, b, c, x[ 9], MD5_S12, 0x8b44f7af); /* 10 */
	MD5_FF (c, d, a, b, x[10], MD5_S13, 0xffff5bb1); /* 11 */
	MD5_FF (b, c, d, a, x[11], MD5_S14, 0x895cd7be); /* 12 */
	MD5_FF (a, b, c, d, x[12], MD5_S11, 0x6b901122); /* 13 */
	MD5_FF (d, a, b, c, x[13], MD5_S12, 0xfd987193); /* 14 */
	MD5_FF (c, d, a, b, x[14], MD5_S13, 0xa679438e); /* 15 */
	MD5_FF (b, c, d, a, x[15], MD5_S14, 0x49b40821); /* 16 */
	
	 /* Round 2 */
	 MD5_GG (a, b, c, d, x[ 1], MD5_S21, 0xf61e2562); /* 17 */
	 MD5_GG (d, a, b, c, x[ 6], MD5_S22, 0xc040b340); /* 18 */
	 MD5_GG (c, d, a, b, x[11], MD5_S23, 0x265e5a51); /* 19 */
	 MD5_GG (b, c, d, a, x[ 0], MD5_S24, 0xe9b6c7aa); /* 20 */
	 MD5_GG (a, b, c, d, x[ 5], MD5_S21, 0xd62f105d); /* 21 */
	 MD5_GG (d, a, b, c, x[10], MD5_S22,  0x2441453); /* 22 */
	 MD5_GG (c, d, a, b, x[15], MD5_S23, 0xd8a1e681); /* 23 */
	 MD5_GG (b, c, d, a, x[ 4], MD5_S24, 0xe7d3fbc8); /* 24 */
	 MD5_GG (a, b, c, d, x[ 9], MD5_S21, 0x21e1cde6); /* 25 */
	 MD5_GG (d, a, b, c, x[14], MD5_S22, 0xc33707d6); /* 26 */
	 MD5_GG (c, d, a, b, x[ 3], MD5_S23, 0xf4d50d87); /* 27 */
	 MD5_GG (b, c, d, a, x[ 8], MD5_S24, 0x455a14ed); /* 28 */
	 MD5_GG (a, b, c, d, x[13], MD5_S21, 0xa9e3e905); /* 29 */
	 MD5_GG (d, a, b, c, x[ 2], MD5_S22, 0xfcefa3f8); /* 30 */
	 MD5_GG (c, d, a, b, x[ 7], MD5_S23, 0x676f02d9); /* 31 */
	 MD5_GG (b, c, d, a, x[12], MD5_S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	MD5_HH (a, b, c, d, x[ 5], MD5_S31, 0xfffa3942); /* 33 */
	MD5_HH (d, a, b, c, x[ 8], MD5_S32, 0x8771f681); /* 34 */
	MD5_HH (c, d, a, b, x[11], MD5_S33, 0x6d9d6122); /* 35 */
	MD5_HH (b, c, d, a, x[14], MD5_S34, 0xfde5380c); /* 36 */
	MD5_HH (a, b, c, d, x[ 1], MD5_S31, 0xa4beea44); /* 37 */
	MD5_HH (d, a, b, c, x[ 4], MD5_S32, 0x4bdecfa9); /* 38 */
	MD5_HH (c, d, a, b, x[ 7], MD5_S33, 0xf6bb4b60); /* 39 */
	MD5_HH (b, c, d, a, x[10], MD5_S34, 0xbebfbc70); /* 40 */
	MD5_HH (a, b, c, d, x[13], MD5_S31, 0x289b7ec6); /* 41 */
	MD5_HH (d, a, b, c, x[ 0], MD5_S32, 0xeaa127fa); /* 42 */
	MD5_HH (c, d, a, b, x[ 3], MD5_S33, 0xd4ef3085); /* 43 */
	MD5_HH (b, c, d, a, x[ 6], MD5_S34,  0x4881d05); /* 44 */
	MD5_HH (a, b, c, d, x[ 9], MD5_S31, 0xd9d4d039); /* 45 */
	MD5_HH (d, a, b, c, x[12], MD5_S32, 0xe6db99e5); /* 46 */
	MD5_HH (c, d, a, b, x[15], MD5_S33, 0x1fa27cf8); /* 47 */
	MD5_HH (b, c, d, a, x[ 2], MD5_S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	MD5_II (a, b, c, d, x[ 0], MD5_S41, 0xf4292244); /* 49 */
	MD5_II (d, a, b, c, x[ 7], MD5_S42, 0x432aff97); /* 50 */
	MD5_II (c, d, a, b, x[14], MD5_S43, 0xab9423a7); /* 51 */
	MD5_II (b, c, d, a, x[ 5], MD5_S44, 0xfc93a039); /* 52 */
	MD5_II (a, b, c, d, x[12], MD5_S41, 0x655b59c3); /* 53 */
	MD5_II (d, a, b, c, x[ 3], MD5_S42, 0x8f0ccc92); /* 54 */
	MD5_II (c, d, a, b, x[10], MD5_S43, 0xffeff47d); /* 55 */
	MD5_II (b, c, d, a, x[ 1], MD5_S44, 0x85845dd1); /* 56 */
	MD5_II (a, b, c, d, x[ 8], MD5_S41, 0x6fa87e4f); /* 57 */
	MD5_II (d, a, b, c, x[15], MD5_S42, 0xfe2ce6e0); /* 58 */
	MD5_II (c, d, a, b, x[ 6], MD5_S43, 0xa3014314); /* 59 */
	MD5_II (b, c, d, a, x[13], MD5_S44, 0x4e0811a1); /* 60 */
	MD5_II (a, b, c, d, x[ 4], MD5_S41, 0xf7537e82); /* 61 */
	MD5_II (d, a, b, c, x[11], MD5_S42, 0xbd3af235); /* 62 */
	MD5_II (c, d, a, b, x[ 2], MD5_S43, 0x2ad7d2bb); /* 63 */
	MD5_II (b, c, d, a, x[ 9], MD5_S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	/* Zeroize sensitive information.*/
	MD5_memset ((POINTER)x, 0, sizeof (x));
}
////////////////////////////////////////////////////////////////////////////////////////
// Encodes input (UINT4) into output (unsigned char). Assumes len is
//  a multiple of 4.
static void Encode (unsigned char *output, UINT4 *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4) 
	{
		output[j] = (unsigned char)(input[i] & 0xff);
		output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}
////////////////////////////////////////////////////////////////////////////////////////
// Decodes input (unsigned char) into output (UINT4). Assumes len is
// a multiple of 4.
static void Decode (UINT4 *output, unsigned char *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
	{
		output[i] = ((UINT4)input[j]) 
			| (((UINT4)input[j+1]) << 8) 
			|(((UINT4)input[j+2]) << 16) 
			| (((UINT4)input[j+3]) << 24);
	}
}
////////////////////////////////////////////////////////////////////////////////////////
// Note: Replace "for loop" with standard memcpy if possible.
static void MD5_memcpy (POINTER output, POINTER input, unsigned int len)
{
	unsigned int i;
	for (i = 0; i < len; i++)
		output[i] = input[i];
}
////////////////////////////////////////////////////////////////////////////////////////
/* Note: Replace "for loop" with standard memset if possible. */
static void MD5_memset (POINTER output, int value, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
	{
		 ((char *)output)[i] = (char)value;
	}
}
////////////////////////////////////////////////////////////////////////////////////////
static void __fastcall MD4Transform(ULONG Hash[4], ULONG x[16])
{
	ULONG a = Hash[0];
	ULONG b = Hash[1];
	ULONG c = Hash[2];
	ULONG d = Hash[3];

	/* Round 1 */
	MD4_FF(a, b, c, d, x[ 0], MD4_S11); // 01
	MD4_FF(d, a, b, c, x[ 1], MD4_S12); // 02
	MD4_FF(c, d, a, b, x[ 2], MD4_S13); // 03
	MD4_FF(b, c, d, a, x[ 3], MD4_S14); // 04
	MD4_FF(a, b, c, d, x[ 4], MD4_S11); // 05
	MD4_FF(d, a, b, c, x[ 5], MD4_S12); // 06
	MD4_FF(c, d, a, b, x[ 6], MD4_S13); // 07
	MD4_FF(b, c, d, a, x[ 7], MD4_S14); // 08
	MD4_FF(a, b, c, d, x[ 8], MD4_S11); // 09
	MD4_FF(d, a, b, c, x[ 9], MD4_S12); // 10
	MD4_FF(c, d, a, b, x[10], MD4_S13); // 11
	MD4_FF(b, c, d, a, x[11], MD4_S14); // 12
	MD4_FF(a, b, c, d, x[12], MD4_S11); // 13
	MD4_FF(d, a, b, c, x[13], MD4_S12); // 14
	MD4_FF(c, d, a, b, x[14], MD4_S13); // 15
	MD4_FF(b, c, d, a, x[15], MD4_S14); // 16

	/* Round 2 */
	MD4_GG(a, b, c, d, x[ 0], MD4_S21); // 17
	MD4_GG(d, a, b, c, x[ 4], MD4_S22); // 18
	MD4_GG(c, d, a, b, x[ 8], MD4_S23); // 19
	MD4_GG(b, c, d, a, x[12], MD4_S24); // 20
	MD4_GG(a, b, c, d, x[ 1], MD4_S21); // 21
	MD4_GG(d, a, b, c, x[ 5], MD4_S22); // 22
	MD4_GG(c, d, a, b, x[ 9], MD4_S23); // 23
	MD4_GG(b, c, d, a, x[13], MD4_S24); // 24
	MD4_GG(a, b, c, d, x[ 2], MD4_S21); // 25
	MD4_GG(d, a, b, c, x[ 6], MD4_S22); // 26
	MD4_GG(c, d, a, b, x[10], MD4_S23); // 27
	MD4_GG(b, c, d, a, x[14], MD4_S24); // 28
	MD4_GG(a, b, c, d, x[ 3], MD4_S21); // 29
	MD4_GG(d, a, b, c, x[ 7], MD4_S22); // 30
	MD4_GG(c, d, a, b, x[11], MD4_S23); // 31
	MD4_GG(b, c, d, a, x[15], MD4_S24); // 32

	/* Round 3 */
	MD4_HH(a, b, c, d, x[ 0], MD4_S31); // 33
	MD4_HH(d, a, b, c, x[ 8], MD4_S32); // 34
	MD4_HH(c, d, a, b, x[ 4], MD4_S33); // 35
	MD4_HH(b, c, d, a, x[12], MD4_S34); // 36
	MD4_HH(a, b, c, d, x[ 2], MD4_S31); // 37
	MD4_HH(d, a, b, c, x[10], MD4_S32); // 38
	MD4_HH(c, d, a, b, x[ 6], MD4_S33); // 39
	MD4_HH(b, c, d, a, x[14], MD4_S34); // 40
	MD4_HH(a, b, c, d, x[ 1], MD4_S31); // 41
	MD4_HH(d, a, b, c, x[ 9], MD4_S32); // 42
	MD4_HH(c, d, a, b, x[ 5], MD4_S33); // 43
	MD4_HH(b, c, d, a, x[13], MD4_S34); // 44
	MD4_HH(a, b, c, d, x[ 3], MD4_S31); // 45
	MD4_HH(d, a, b, c, x[11], MD4_S32); // 46
	MD4_HH(c, d, a, b, x[ 7], MD4_S33); // 47
	MD4_HH(b, c, d, a, x[15], MD4_S34); // 48

	Hash[0] += a;
	Hash[1] += b;
	Hash[2] += c;
	Hash[3] += d;
}
////////////////////////////////////////////////////////////////////////////////////////
//	CreateHashFromInput() generates a hash from the next 'dwLength' bytes of 'file' or 'pbyteMem'
//		The hash is returned in '*pbyteHash'.
void CreateMD4HashFromInput(CFile* pFile, UCHAR *pbyteMem, ULONG dwLength, UCHAR *pbyteHash)
{
	EMULE_TRY

	ULONG *pdwHash = reinterpret_cast<ULONG*>(pbyteHash);

	pdwHash[0] = 0x67452301;
	pdwHash[1] = 0xEFCDAB89;
	pdwHash[2] = 0x98BADCFE;
	pdwHash[3] = 0x10325476;

	ULONG		dwRequired = dwLength;
	UCHAR		pbyteBuff[1024];	//	should be small enough to eliminate stack size check

	if (pbyteMem != NULL)
	{
	//	Process memory block
		while (dwRequired >= 64)
		{
			MD4Transform(pdwHash, reinterpret_cast<ULONG*>(pbyteMem));
			pbyteMem += 64;
			dwRequired -= 64;
		}
	//	Copy the rest
		if (dwRequired != 0)
			memcpy(pbyteBuff, pbyteMem, dwRequired);
	}
	else
	{
	//	Process a pFile
		ULONG		dwBlockSz = 16 * 4096;
		void		*pAlloc = malloc(dwBlockSz);
		UCHAR		*pbytePtr = reinterpret_cast<UCHAR*>(pAlloc);

		if (pbytePtr == NULL)
		{
		//	Use small local buffer in case of low memory
			pbytePtr = pbyteBuff;
			dwBlockSz = sizeof(pbyteBuff);
		}

		while (dwRequired >= dwBlockSz)
		{
			pFile->Read(pbytePtr, dwBlockSz);
			ULONG i = 0;
			do
			{
				MD4Transform(pdwHash, (ULONG*)(pbytePtr + i));
				i += 64;
			} while(i < dwBlockSz);
			dwRequired -= dwBlockSz;
		}
		if (dwRequired >= 64)
		{
			ULONG dwLen = dwRequired & ~63;

			pFile->Read(pbytePtr, dwLen);
			ULONG i = 0;
			do
			{
				MD4Transform(pdwHash, reinterpret_cast<ULONG*>(pbytePtr + i));
				i += 64;
			} while(i < dwLen);
			dwRequired -= dwLen;
		}
	//	Read the rest
		if (dwRequired != 0)
			pFile->Read(pbyteBuff, dwRequired);
		if (pAlloc != NULL)
			free(pAlloc);
	}
	// in byte scale 512 = 64, 448 = 56
	pbyteBuff[dwRequired++] = 0x80;
	if (dwRequired > 56)
	{
		memset(&pbyteBuff[dwRequired], 0, 64 - dwRequired);
		MD4Transform(pdwHash, reinterpret_cast<ULONG*>(pbyteBuff));
		dwRequired = 0;
	}
	memset(&pbyteBuff[dwRequired], 0, 56 - dwRequired);
// Add size (convert to bits)
	*reinterpret_cast<ULONG*>(&pbyteBuff[56]) = (dwLength << 3);
	*reinterpret_cast<ULONG*>(&pbyteBuff[60]) = (dwLength >> 29);

	MD4Transform(pdwHash, reinterpret_cast<ULONG*>(pbyteBuff));

	EMULE_CATCH
}
////////////////////////////////////////////////////////////////////////////////////////
CString HashToString(int iHashLength, const UCHAR *hash)
{
	CString strHash;
	UINT uiCh;
	int	iCharPos;

	for (int i = 0; i < iHashLength; i++)
	{
		iCharPos = i*2;
		uiCh = static_cast<UINT>(hash[i]);
		strHash.Insert(iCharPos, s_acHexDigits[uiCh >> 4]);
		strHash.Insert((iCharPos+1), s_acHexDigits[uiCh & 0xf]);
	}
	return strHash;
}
