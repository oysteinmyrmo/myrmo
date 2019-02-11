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
#include <string>
#include <cstdint>
#include <cstdio>
#include <fstream>

namespace myrmo { namespace cache
{
	// TODO: Support streaming of large files, both readn and write.
	class DiskCache
	{
	public:
		typedef std::string (*hashFunction)(const std::string& uri);

		DiskCache() = delete;

		DiskCache(const std::string& cacheDir, hashFunction func)
			: mCacheDir(cacheDir)
			, mHashFunction(func)
		{}

		// TODO: Take in std::vector instead!
		bool read(const std::string& uri, char** data, size_t* size) const
		{
			bool couldRead = false;
			std::ifstream f(filename(uri), std::ios::binary);

			if (f.is_open())
			{
				f.seekg(0, std::ios::end);
				size_t fSize = f.tellg();
				f.seekg(0, std::ios::beg);

				*data = new char[fSize]; // TODO: Fill std::vector!
				*size = fSize;
				f.read(*data, fSize);
				f.close();
				couldRead = true;
			}

			return couldRead;
		}

		bool write(const std::string uri, const std::string& data) const
		{
			return write(uri, data.c_str(), data.size());
		}

		bool write(const std::string uri, const char* data, size_t size) const
		{
			bool couldWrite = false;
			std::ofstream f(filename(uri), std::ios::binary);

			if (f.is_open())
			{
				f.write(data, size);
				f.close();
				couldWrite = true;
			}

			return couldWrite;
		}

		int remove(const std::string& uri) const
		{
			int result = 0;
			std::string fileName = filename(uri.c_str());
			std::ifstream f(fileName);
			if (f.good())
				result = std::remove(filename(uri).c_str());
			return result;
		}

	private:
		std::string filename(const std::string& uri) const
		{
			return mCacheDir + "/" + mHashFunction(uri);
		}

	private:
		std::string  mCacheDir;
		hashFunction mHashFunction;
	};
	
}} // End namespace myrmo::cache
