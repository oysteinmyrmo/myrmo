/* Copyright © 2019 Øystein Myrmo (oystein.myrmo@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#include <myrmo/util/bits.h>

#include <string>
#include <cstdint>
#include <cassert>
#include <cstdio>

namespace myrmo { namespace hash
{
	// Notes:
	// - Nice website for testing/debugging SHA1: https://cse.unl.edu/~ssamal/crypto/genhash.php
	// TODO:
	// - Support little endian?
	// - Support streaming?
	inline std::string sha1(const std::string& message)
	{
		std::string result;
		result.resize(40);

		size_t bitLength = message.size() * 8;
		if ((bitLength % 512) >= 448)
			bitLength += 1024 - (bitLength % 512); // Not enough space for both the 1 and the 64 byte message length. Pad through next block.
		else
			bitLength += 512 - (bitLength % 512);  // Pad to end of current block.
		assert((bitLength % 512) == 0);

		size_t byteLength = bitLength / 8;
		unsigned char* data = new unsigned char[byteLength];
		memset(data, 0, byteLength);
		memcpy(data, message.c_str(), message.size());
		data[message.size()] = 0x80;

		const uint64_t message_size_bits = message.size() * 8;
		for (int i = 0; i < 8; i++)
			data[byteLength - i - 1] = (unsigned char)(message_size_bits >> (i * 8));

		unsigned int h0 = 0x67452301;
		unsigned int h1 = 0xEFCDAB89;
		unsigned int h2 = 0x98BADCFE;
		unsigned int h3 = 0x10325476;
		unsigned int h4 = 0xC3D2E1F0;

		const size_t nBlocks = bitLength / 512;
		size_t block = 0;
		while (block < nBlocks)
		{
			unsigned int words[80];
			for (int i = 0; i < 16; i++)
				words[i] =	(((unsigned int)data[block*64 + i*4 + 0]) << 24) +
							(((unsigned int)data[block*64 + i*4 + 1]) << 16) +
							(((unsigned int)data[block*64 + i*4 + 2]) << 8)  +
							(((unsigned int)data[block*64 + i*4 + 3]) << 0);

			for (int i = 16; i < 80; i++)
				words[i] = util::bits::left_rotate<1>(words[i-3] ^ words[i-8] ^ words[i-14] ^ words[i-16]);

			unsigned int a = h0;
			unsigned int b = h1;
			unsigned int c = h2;
			unsigned int d = h3;
			unsigned int e = h4;
			unsigned int f;
			unsigned int k;

			// TODO: Optimize loop
			for (int i = 0; i < 80; i++)
			{
				if (i < 20)
				{
					f = (b & c) | ((~b) & d);
					k = 0x5A827999;
				}
				else if (i < 40)
				{
					f = b ^ c ^ d;
					k = 0x6ED9EBA1;
				}
				else if (i < 60)
				{
					f = (b & c) | (b & d) | (c & d);
					k = 0x8F1BBCDC;
				}
				else // i < 80
				{
					f = b ^ c ^ d;
					k = 0xCA62C1D6;
				}

				const unsigned int temp = util::bits::left_rotate<5>(a) + f + e + k + words[i];
				e = d;
				d = c;
				c = util::bits::left_rotate<30>(b);
				b = a;
				a = temp;
			}

			h0 = h0 + a;
			h1 = h1 + b;
			h2 = h2 + c;
			h3 = h3 + d;
			h4 = h4 + e;

			block++;
		}

		snprintf(&result[0],  32, "%08x", h0);
		snprintf(&result[8],  32, "%08x", h1);
		snprintf(&result[16], 32, "%08x", h2);
		snprintf(&result[24], 32, "%08x", h3);
		snprintf(&result[32], 32, "%08x", h4);

		delete[] data;
		return result;
	}

}} // End namespace myrmo::hash
