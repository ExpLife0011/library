/**
 * Copyright (C) 2008 Meteor Liu
 *
 * This code has been released into the Public Domain.
 * You may do whatever you like with it.
 *
 * @file
 * @author Meteor Liu <meteor1113@gmail.com>
 * @date 2009-01-01
 */


// #include <standard library headers>
#include <fstream>

// #include <other library headers>
#if defined(__GNUC__) || defined(__GNU_LIBRARY__)
#include <byteswap.h>
#include <endian.h>
#endif

// #include "customer headers"
#include "sha1.h"


#pragma warning(push)
#pragma warning(disable: 4996)


/*
  To obtain the highest speed on processors with 32-bit words, this code
  needs to determine the order in which bytes are packed into such words.
  The following block of code is an attempt to capture the most obvious
  ways in which various environemnts specify their endian definitions.
  It may well fail, in which case the definitions will need to be set by
  editing at the points marked **** EDIT HERE IF NECESSARY **** below.
*/
#define SHA_LITTLE_ENDIAN   1234 /* byte 0 is least significant (i386) */
#define SHA_BIG_ENDIAN      4321 /* byte 0 is most significant (mc68k) */

#if !defined(PLATFORM_BYTE_ORDER)
#if defined(LITTLE_ENDIAN) || defined(BIG_ENDIAN)
#  if defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN)
#    if defined(BYTE_ORDER)
#      if   (BYTE_ORDER == LITTLE_ENDIAN)
#        define PLATFORM_BYTE_ORDER SHA_LITTLE_ENDIAN
#      elif (BYTE_ORDER == BIG_ENDIAN)
#        define PLATFORM_BYTE_ORDER SHA_BIG_ENDIAN
#      endif
#    endif
#  elif defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
#    define PLATFORM_BYTE_ORDER SHA_LITTLE_ENDIAN
#  elif !defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN)
#    define PLATFORM_BYTE_ORDER SHA_BIG_ENDIAN
#  endif
#elif defined(_LITTLE_ENDIAN) || defined(_BIG_ENDIAN)
#  if defined(_LITTLE_ENDIAN) && defined(_BIG_ENDIAN)
#    if defined(_BYTE_ORDER)
#      if   (_BYTE_ORDER == _LITTLE_ENDIAN)
#        define PLATFORM_BYTE_ORDER SHA_LITTLE_ENDIAN
#      elif (_BYTE_ORDER == _BIG_ENDIAN)
#        define PLATFORM_BYTE_ORDER SHA_BIG_ENDIAN
#      endif
#    endif
#  elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#    define PLATFORM_BYTE_ORDER SHA_LITTLE_ENDIAN
#  elif !defined(_LITTLE_ENDIAN) && defined(_BIG_ENDIAN)
#    define PLATFORM_BYTE_ORDER SHA_BIG_ENDIAN
#  endif
#elif 0     /* **** EDIT HERE IF NECESSARY **** */
#define PLATFORM_BYTE_ORDER SHA_LITTLE_ENDIAN
#elif 0     /* **** EDIT HERE IF NECESSARY **** */
#define PLATFORM_BYTE_ORDER SHA_BIG_ENDIAN
#elif (('1234' >> 24) == '1')
#  define PLATFORM_BYTE_ORDER SHA_LITTLE_ENDIAN
#elif (('4321' >> 24) == '1')
#  define PLATFORM_BYTE_ORDER SHA_BIG_ENDIAN
#endif
#endif

#if !defined(PLATFORM_BYTE_ORDER)
#  error Please set undetermined byte order (lines 87 or 89 of sha1.c).
#endif

#define rotl32(x,n) (((x) << n) | ((x) >> (32 - n)))

#if (PLATFORM_BYTE_ORDER == SHA_BIG_ENDIAN)
#define swap_b32(x) (x)
#elif defined(bswap_32)
#define swap_b32(x) bswap_32(x)
#else
#define swap_b32(x) ((rotl32((x), 8) & 0x00ff00ff) | (rotl32((x), 24) & 0xff00ff00))
#endif

#define SHA1_MASK   (SHA1_BLOCK_SIZE - 1)

/* reverse byte order in 32-bit words   */

#define ch(x,y,z)       (((x) & (y)) ^ (~(x) & (z)))
#define parity(x,y,z)   ((x) ^ (y) ^ (z))
#define maj(x,y,z)      (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/* A normal version as set out in the FIPS. This version uses   */
/* partial loop unrolling and is optimised for the Pentium 4    */

#define rnd(f,k)                                        \
    t = a; a = rotl32(a,5) + f(b,c,d) + e + k + w[i];   \
    e = d; d = c; c = rotl32(b, 30); b = t

/* SHA1 final padding and digest calculation  */

#if (PLATFORM_BYTE_ORDER == SHA_LITTLE_ENDIAN)
static sha1_32t  mask[4] =
{   0x00000000, 0x000000ff, 0x0000ffff, 0x00ffffff };
static sha1_32t  bits[4] =
{   0x00000080, 0x00008000, 0x00800000, 0x80000000 };
#else
static sha1_32t  mask[4] =
{   0x00000000, 0xff000000, 0xffff0000, 0xffffff00 };
static sha1_32t  bits[4] =
{   0x80000000, 0x00800000, 0x00008000, 0x00000080 };
#endif


namespace encrypt
{


    /**
     * description
     */
    SHA1::SHA1()
    {
        Init();
    }


    /**
     * description
     */
    SHA1::~SHA1()
    {
    }


    void SHA1::Init()
    {
        _ctx.count[0] = _ctx.count[1] = 0;
        _ctx.hash[0] = 0x67452301;
        _ctx.hash[1] = 0xefcdab89;
        _ctx.hash[2] = 0x98badcfe;
        _ctx.hash[3] = 0x10325476;
        _ctx.hash[4] = 0xc3d2e1f0;
    }

    /* SHA1 hash data in an array of bytes into hash buffer and call the        */
    /* hash_compile function as required.                                       */

    void SHA1::Update(const void* in, unsigned int len)
    {
        const unsigned char* data = (const unsigned char*)in;
        sha1_32t pos = (sha1_32t)(_ctx.count[0] & SHA1_MASK),
            space = SHA1_BLOCK_SIZE - pos;
        const unsigned char *sp = data;

        if((_ctx.count[0] += len) < len)
            ++(_ctx.count[1]);

        while(len >= space)     /* tranfer whole blocks while possible  */
        {
            memcpy(((unsigned char*)_ctx.wbuf) + pos, sp, space);
            sp += space; len -= space; space = SHA1_BLOCK_SIZE; pos = 0;
            Compile();
        }

        memcpy(((unsigned char*)_ctx.wbuf) + pos, sp, len);
    }


    void SHA1::Final()
    {
        sha1_32t    i = (sha1_32t)(_ctx.count[0] & SHA1_MASK);

        /* mask out the rest of any partial 32-bit word and then set    */
        /* the next byte to 0x80. On big-endian machines any bytes in   */
        /* the buffer will be at the top end of 32 bit words, on little */
        /* endian machines they will be at the bottom. Hence the AND    */
        /* and OR masks above are reversed for little endian systems    */
        /* Note that we can always add the first padding byte at this   */
        /* because the buffer always contains at least one empty slot   */
        _ctx.wbuf[i >> 2] = (_ctx.wbuf[i >> 2] & mask[i & 3]) | bits[i & 3];

        /* we need 9 or more empty positions, one for the padding byte  */
        /* (above) and eight for the length count.  If there is not     */
        /* enough space pad and empty the buffer                        */
        if(i > SHA1_BLOCK_SIZE - 9)
        {
            if(i < 60) _ctx.wbuf[15] = 0;
            Compile();
            i = 0;
        }
        else    /* compute a word index for the empty buffer positions  */
            i = (i >> 2) + 1;

        while(i < 14) /* and zero pad all but last two positions      */
            _ctx.wbuf[i++] = 0;

        /* assemble the eight byte counter in in big-endian format */
        _ctx.wbuf[14] = swap_b32((_ctx.count[1] << 3) | (_ctx.count[0] >> 29));
        _ctx.wbuf[15] = swap_b32(_ctx.count[0] << 3);

        Compile();

        /* extract the hash value as bytes in case the hash buffer is   */
        /* misaligned for 32-bit words                                  */
        for(i = 0; i < SHA1_DIGEST_SIZE; ++i)
        {
            _digest[i] = (unsigned char)(_ctx.hash[i >> 2] >> 8 * (~i & 3));
        }
    }


    /**
     * Convert unsigned char array to hex string.
     */
    std::string SHA1::BytesToHexString(const unsigned char* input,
                                       unsigned int length)
    {
        std::string str;
        str.reserve(length << 1);
        char b[3];
        for (unsigned int i = 0; i < length; i++)
        {
            sprintf(b, "%02X", input[i]);
            str.append(1, b[0]);
            str.append(1, b[1]);
        }
        return str;
    }


    void SHA1::Compile()
    {
        sha1_32t    w[80], i, a, b, c, d, e, t;

        /* note that words are compiled from the buffer into 32-bit */
        /* words in big-endian order so an order reversal is needed */
        /* here on little endian machines                           */
        for(i = 0; i < SHA1_BLOCK_SIZE / 4; ++i)
            w[i] = swap_b32(_ctx.wbuf[i]);

        for(i = SHA1_BLOCK_SIZE / 4; i < 80; ++i)
            w[i] = rotl32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

        a= _ctx.hash[0];
        b= _ctx.hash[1];
        c= _ctx.hash[2];
        d= _ctx.hash[3];
        e= _ctx.hash[4];

        for(i = 0; i < 20; ++i)
        {
            rnd(ch, 0x5a827999);
        }

        for(i = 20; i < 40; ++i)
        {
            rnd(parity, 0x6ed9eba1);
        }

        for(i = 40; i < 60; ++i)
        {
            rnd(maj, 0x8f1bbcdc);
        }

        for(i = 60; i < 80; ++i)
        {
            rnd(parity, 0xca62c1d6);
        }

        _ctx.hash[0] += a;
        _ctx.hash[1] += b;
        _ctx.hash[2] += c;
        _ctx.hash[3] += d;
        _ctx.hash[4] += e;
    }


    std::string SHA1Data(const void* data, unsigned int length)
    {
        SHA1 obj;
        obj.Update(data, length);
        obj.Final();
        return obj.ToString();
    }


    std::string SHA1String(const std::string& str)
    {
        SHA1 obj;
        obj.Update((const unsigned char*)str.c_str(),
                   static_cast<unsigned int>(str.length()));
        obj.Final();
        return obj.ToString();
    }


    std::string SHA1File(const std::string& file)
    {
        SHA1 obj;

        std::ifstream fs(file.c_str(), std::ios::binary);
        const unsigned int BUFFER_SIZE = 1024;
        char buffer[BUFFER_SIZE];
        while (!fs.eof())
        {
            fs.read(buffer, BUFFER_SIZE);
            std::streamsize length = fs.gcount();
            if (length > 0)
            {
                obj.Update((const unsigned char*)buffer, length);
            }
        }
        fs.close();

        obj.Final();
        return obj.ToString();
    }

}

#pragma warning(pop)