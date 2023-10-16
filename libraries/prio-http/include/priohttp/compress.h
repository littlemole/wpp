#ifndef INCLUDE_PROMISE_HTTP_COMPRESS_H_
#define INCLUDE_PROMISE_HTTP_COMPRESS_H_

#include <string>
#include <memory>

namespace prio  {


class CompressorImpl;
class DecompressorImpl;

class Compressor
{
public:

	typedef std::shared_ptr<Compressor> Ptr;

	Compressor();
	~Compressor();

	static Ptr create();

	std::string compress(const std::string& s);
	std::string flush();

private:
	std::unique_ptr<CompressorImpl> pimpl_;
};

class Decompressor
{
public:

	typedef std::shared_ptr<Decompressor> Ptr;

	Decompressor();
	~Decompressor();

	static Ptr create();

	void decompress(const std::string& s);
	std::string flush();

private:
	std::unique_ptr<DecompressorImpl> pimpl_;
};

}

#endif /* INCLUDE_PROMISE_HTTP_COMPRESS_H_ */
