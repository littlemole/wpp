#ifndef _WIN32
#include <unistd.h>
#include <crypt.h>
#else
#define _CRT_RAND_S  
#include <stdlib.h>
#include <stdio.h>  
#include <limits.h>  
#endif

#include <sstream>
#include <fstream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>

#include "priocpp/common.h"

namespace prio {

	size_t unix_timestamp()
	{
		return std::time(0);
	}


	std::string trim(const std::string& input)
	{
		size_t start = input.find_first_not_of(" \t\r\n");
		if (start == std::string::npos) return "";

		size_t end = input.find_last_not_of(" \t\r\n");
		if (end == std::string::npos) end = input.size() - 1;

		return input.substr(start, end - start + 1);
	}

#ifndef _WIN32
	std::string nonce(unsigned int n)
	{
		int randomData = open("/dev/urandom", O_RDONLY);
		char myRandomData[256];
		size_t randomDataLen = 0;
		while (randomDataLen < n)
		{
			ssize_t result = read(randomData, myRandomData + randomDataLen, n - randomDataLen);
			if (result < 0)
			{
				// error, unable to read /dev/random
			}
			randomDataLen += result;
		}
		close(randomData);
		return std::string(myRandomData, n);
	}
#else
	std::string nonce(unsigned int n)
	{
		char myRandomData[256];
		size_t randomDataLen = 0;
		while (randomDataLen < n)
		{
			unsigned int v;
			rand_s(&v);
			myRandomData[randomDataLen] = v;
			randomDataLen++;
		}

		return std::string(myRandomData, n);
	}
#endif


	static const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";


	static inline bool is_base64(unsigned char c)
	{
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

	std::string base64_encode(const std::string& bytes_to_encode)
	{
		return base64_encode((unsigned char const*)bytes_to_encode.c_str(), bytes_to_encode.size());
	}

	std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len)
	{
		std::string ret;
		int i = 0;
		int j = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		while (in_len--) {
			char_array_3[i++] = *(bytes_to_encode++);
			if (i == 3) {
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (i = 0; (i < 4); i++)
					ret += base64_chars[char_array_4[i]];
				i = 0;
			}
		}

		if (i)
		{
			for (j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (j = 0; (j < i + 1); j++)
				ret += base64_chars[char_array_4[j]];

			while ((i++ < 3))
				ret += '=';

		}

		return ret;

	}

	std::string base64_decode(const std::string& encoded_string)
	{
		size_t in_len = encoded_string.size();
		size_t i = 0;
		size_t j = 0;
		size_t in_ = 0;

		unsigned char char_array_4[4], char_array_3[3];
		std::string ret;

		while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
			char_array_4[i++] = encoded_string[in_]; in_++;
			if (i == 4) {
				for (i = 0; i < 4; i++)
					char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
					ret += char_array_3[i];
				i = 0;
			}
		}

		if (i) {
			for (j = i; j < 4; j++)
				char_array_4[j] = 0;

			for (j = 0; j < 4; j++)
				char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
		}

		return ret;
	}





}


