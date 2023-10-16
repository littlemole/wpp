#include <stdio.h>
#include <zlib.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include "priohttp/compress.h"
#include <reprocpp/ex.h>

/* CHUNK is the size of the memory chunk used by the zlib routines. */

#define CHUNK 0x4000

/* The following macro calls a zlib routine and checks the return
   value. If the return value ("status") is not OK, it prints an error
   message. Zlib's error statuses are all less
   than zero. */

#define CALL_ZLIB(x) {                                                  \
        int status;                                                     \
        status = x;                                                     \
        if (status < 0) {                                               \
            fprintf (stdout,                                            \
                     "%s:%d: %s returned a bad status of %d.\n",        \
                     __FILE__, __LINE__, #x, status);                   \
            throw repro::Ex("gzip error");                              \
        }                                                               \
    }

/* These are parameters to deflateInit2. See
   http://zlib.net/manual.html for the exact meanings. */

#define windowBits 15
#define GZIP_ENCODING 16
#define ENABLE_ZLIB_GZIP 32

static void strm_init (z_stream * strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree  = Z_NULL;
    strm->opaque = Z_NULL;
    CALL_ZLIB (deflateInit2 (strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                             windowBits | GZIP_ENCODING, 8,
                             Z_DEFAULT_STRATEGY));
}

namespace prio  {


class CompressorImpl
{
public:

	CompressorImpl();
	~CompressorImpl();

	std::string compress(const std::string& s);
	std::string flush();

private:
    unsigned char buf_[CHUNK];
    z_stream strm_;
};

CompressorImpl::CompressorImpl()
{
	 strm_init (& strm_);
}

CompressorImpl::~CompressorImpl()
{
    deflateEnd (& strm_);
}

std::string CompressorImpl::compress(const std::string& s)
{
    std::ostringstream oss;

    strm_.next_in = (unsigned char *) s.c_str();
    strm_.avail_in = s.size();;

    do
    {
      int have;
      strm_.avail_out = CHUNK;
      strm_.next_out = buf_;

      CALL_ZLIB (deflate (& strm_,0));

      have = CHUNK - strm_.avail_out;
      oss.write( (const char*)buf_,have);
    }
    while (strm_.avail_out == 0);

    return oss.str();
}

std::string CompressorImpl::flush()
{
	std::ostringstream oss;

    do
    {
		int have;
		strm_.avail_in = 0;
		strm_.avail_out = CHUNK;
		strm_.next_out = buf_;

		CALL_ZLIB (deflate (& strm_, Z_FINISH));

		have = CHUNK - strm_.avail_out;
	    oss.write( (const char*)buf_,have);
    }
    while (strm_.avail_out == 0);

    return oss.str();
}






Compressor::Compressor()
{
	pimpl_.reset(new CompressorImpl);
}

Compressor::~Compressor()
{
}

Compressor::Ptr Compressor::create()
{
	return std::make_shared<Compressor>();
}


std::string Compressor::compress(const std::string& s)
{
	return pimpl_->compress(s);
}

std::string Compressor::flush()
{
	return pimpl_->flush();
}


/////////////////////////////////////////


class DecompressorImpl
{
public:

	DecompressorImpl();
	~DecompressorImpl();

	void decompress(const std::string& s);
	std::string flush();

private:
    unsigned char buf_[CHUNK];
    z_stream strm_;

	std::ostringstream oss_;
};

DecompressorImpl::DecompressorImpl()
{
    strm_.zalloc = Z_NULL;
    strm_.zfree = Z_NULL;
    strm_.opaque = Z_NULL;
    strm_.avail_in = 0;
    strm_.next_in = 0;
    CALL_ZLIB (inflateInit2 (& strm_, windowBits | ENABLE_ZLIB_GZIP));
}

DecompressorImpl::~DecompressorImpl()
{
    inflateEnd (& strm_);
}

void DecompressorImpl::decompress(const std::string& s)
{
    strm_.next_in = (unsigned char*) s.c_str();
	strm_.avail_in = s.size();
	do
	{
		unsigned have;
		strm_.avail_out = CHUNK;
		strm_.next_out = buf_;
		CALL_ZLIB (inflate (& strm_, Z_NO_FLUSH));

		have = CHUNK - strm_.avail_out;
		oss_.write((const char*)buf_,have);
	}
	while (strm_.avail_out == 0);
}

std::string DecompressorImpl::flush()
{
	return oss_.str();
}






Decompressor::Decompressor()
{
	pimpl_.reset(new DecompressorImpl);
}

Decompressor::~Decompressor()
{
}

Decompressor::Ptr Decompressor::create()
{
	return std::make_shared<Decompressor>();
}


void Decompressor::decompress(const std::string& s)
{
	pimpl_->decompress(s);
}

std::string Decompressor::flush()
{
	return pimpl_->flush();
}


}



