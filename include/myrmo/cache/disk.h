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
#include <myrmo/cache/policy.h>

#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <list>
#include <algorithm>
#include <memory>

namespace myrmo { namespace cache
{
	// TODO: Support streaming of large files, both read and write.
	class DiskCache
	{
	public:
		enum class Error : unsigned int
		{
			NoError,
			FileDoesNotExist,
			FileSizeGreaterThanMaxCacheSize,
			FileExists,
			CouldNotDeleteFile,
			CouldNotClearSpaceForFile,
			CouldNotWriteFile,
			CouldNotWriteIndexFile
		};

		typedef std::string (*hashFunction)(const std::string& uri);

		DiskCache() = delete;
		DiskCache(const DiskCache& cache) = delete;

		DiskCache(const std::string& cacheDir, hashFunction func, policy::EvictionPolicy* policy, size_t cacheSizeInMegaBytes = 50)
			: mCacheDir(cacheDir)
			, mHashFunction(func)
			, mPolicy(policy)
			, mMaxCacheSize(cacheSizeInMegaBytes * 1048576)
			, mCacheSize(0)
		{
			const std::string hash(mHashFunction("myrmo_disk_cache_index"));
			const std::string fName = file_path(hash);
			mPolicy->setHashSize(hash.size());

			std::vector<char> data;
			Error error = read("myrmo_disk_cache_index", &data, true);
			if (error != Error::NoError)
				data.clear();
			mPolicy->setIndexData(data);

			// Calculate initial disk cache size.
			mPolicy->forEach([&](const std::string& hash)
			{
				std::ifstream f(file_path(hash), std::ifstream::ate | std::ifstream::binary);
				if (f.is_open())
					mCacheSize += f.tellg();
				f.close();
			});
		}

		~DiskCache()
		{
			Error error = writeIndexFile();
			assert(error == Error::NoError);
		}

		Error read(const std::string& uri, std::vector<char>* data, bool isIndexFile = false)
		{
			Error error = Error::FileDoesNotExist;
			const std::string hash(mHashFunction(uri));

			if (isIndexFile || (mPolicy->exists(hash) == policy::Error::NoError))
			{
				std::ifstream f(file_path(hash), std::ios::binary);

				if (f.is_open())
				{
					f.seekg(0, std::ios::end);
					const size_t fSize = f.tellg();
					f.seekg(0, std::ios::beg);

					data->resize(fSize, '\0');
					f.read(&(*data)[0], fSize);
					f.close();

					error = Error::NoError;
				}
			}

			return error;
		}

		Error write(const std::string& uri, const char* data, size_t size)
		{
			Error error = Error::NoError;
			const std::string hash(mHashFunction(uri));
			const std::string fName(file_path(hash));
			assert(mPolicy->exists(hash) == policy::Error::DoesNotExist);

			std::ifstream i(fName, std::ios::binary);
			if (i.good())
			{
				error = Error::FileExists;
				i.close();
			}
			else
			{
				std::ofstream f(file_path(hash), std::ios::binary);
				if (f.is_open())
				{
					error = evictUntilEnoughSpace(size);
					if (error == Error::NoError)
					{
						f.write(data, size);
						mCacheSize += size;
						mPolicy->add(hash);
						error = writeIndexFile();
					}
					f.close();
				}
				else
				{
					error = Error::CouldNotWriteFile;
				}
			}

			return error;
		}

		Error write(const std::string& uri, const std::string& data)
		{
			return write(uri, data.c_str(), data.size());
		}

		Error clear()
		{
			Error error = Error::NoError;

			std::string hash = mPolicy->back();
			while (hash.size())
			{
				error = removeFile(hash);
				if (error != Error::NoError)
					break;

				mPolicy->remove(hash);
				hash = mPolicy->back();
			}

			if (error == Error::NoError)
				assert(mCacheSize == 0);

			return error;
		}

		inline Error remove(const std::string& uri)
		{
			Error error = removeFile(mHashFunction(uri));
			if (error == Error::NoError)
				writeIndexFile();
			return error;
		}

		size_t size() const
		{
			return mCacheSize; // Disregarding index file
		}

		size_t count() const
		{
			return mPolicy->count(); // Disregarding index file
		}

	private:
		inline Error removeFile(const std::string& hash, bool isIndexFile = false)
		{
			Error error = Error::NoError;

			const std::string fileName = file_path(hash);
			std::ifstream f(fileName, std::ios::binary);
			if (f.good())
			{
				f.seekg(0, std::ios::end);
				const size_t fSize = f.tellg();
				f.seekg(0, std::ios::beg);

				bool removed = std::remove(fileName.c_str()) == 0;
				if (removed)
				{
					mCacheSize -= fSize;
					if (!isIndexFile)
						mPolicy->remove(hash);
				}
				else
				{
					error = Error::CouldNotDeleteFile;
				}
			}
			else
			{
				error = Error::FileDoesNotExist;
			}

			return error;
		}

		inline std::string file_path(const std::string& hash) const
		{
			return mCacheDir + "/" + hash;
		}

		inline Error writeIndexFile() const
		{
			Error error = Error::CouldNotWriteIndexFile;

			const std::string hash(mHashFunction("myrmo_disk_cache_index"));
			const std::string fName = mCacheDir + "/" + hash;
			std::ofstream f(fName, std::ios::binary);

			if (f.is_open())
			{
				std::string indexData = mPolicy->getIndexData();
				f.write(indexData.c_str(), indexData.size());
				f.close();
				error = Error::NoError;
			}

			return error;
		}

		inline Error evictUntilEnoughSpace(const size_t size)
		{
			Error error = Error::NoError;

			size_t errorCount = 0;
			while ((mCacheSize + size) > mMaxCacheSize)
			{
				if (mPolicy->count() == 0)
				{
					error = Error::FileSizeGreaterThanMaxCacheSize;
					assert(mCacheSize == 0); // We should not end up here unless mCacheSize is 0.
					break;
				}
				else
				{
					const std::string hash = mPolicy->back();
					error = removeFile(hash);
					assert(error == Error::NoError); // The cache is corrupt if we end up removing files that does not exist.
					if (error == Error::NoError)
					{
						errorCount = 0;
					}
					else
					{
						if (++errorCount > 5)
						{
							assert(false); // Tried to remove on file 5 times and failed. Cannot tell why.
							error = Error::CouldNotClearSpaceForFile;
							break;
						}
					}
				}
			}

			return error;
		}

	private:
		std::string  mCacheDir;
		hashFunction mHashFunction;
		std::unique_ptr<policy::EvictionPolicy> mPolicy;

		const size_t mMaxCacheSize;
		size_t mCacheSize; // In megabytes
	};

}} // End namespace myrmo::cache
