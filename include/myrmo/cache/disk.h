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
#include <vector>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <list>
#include <algorithm>

namespace myrmo { namespace cache
{
	// TODO: Support streaming of large files, both read and write.
	class LRUDiskCache
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

		LRUDiskCache() = delete;
		LRUDiskCache(const LRUDiskCache& cache) = delete;

		LRUDiskCache(const std::string& cacheDir, hashFunction func, size_t cacheSizeInMegaBytes = 50)
			: mCacheDir(cacheDir)
			, mHashFunction(func)
			, mMaxCacheSize(cacheSizeInMegaBytes * 1048576)
			, mCacheSize(0)
		{
			Error error = readIndexFile();
			assert(error == Error::NoError);
		}

		~LRUDiskCache()
		{
			Error error = writeIndexFile();
			assert(error == Error::NoError);
		}

		Error read(const std::string& uri, std::vector<char>* data)
		{
			Error error = Error::FileDoesNotExist;
			const std::string hash(mHashFunction(uri));
			std::ifstream f(file_path(hash), std::ios::binary);

			if (f.is_open())
			{
				f.seekg(0, std::ios::end);
				const size_t fSize = f.tellg();
				f.seekg(0, std::ios::beg);

				data->resize(fSize, '\0');
				f.read(&(*data)[0], fSize);
				f.close();

				auto it = std::find(mLRUData.begin(), mLRUData.end(), hash);
				assert(it != mLRUData.end()); // The item must exist in the LRU structure when we could read it from disk.
				if (it != mLRUData.end())
					mLRUData.splice(mLRUData.begin(), mLRUData, it); // Move item to front

				error = Error::NoError;
			}

			return error;
		}

		Error write(const std::string& uri, const char* data, size_t size)
		{
			Error error = Error::NoError;
			const std::string hash(mHashFunction(uri));
			const std::string fName(file_path(hash));

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
						mLRUData.push_front(hash);
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

			// Copy the list because removeFile alters mLRUData.
			std::vector<std::string> copy;
			copy.reserve(mLRUData.size());
			for (const auto& it : mLRUData)
				copy.emplace_back(it);

			for (const auto& item : copy)
			{
				error = removeFile(item);
				if (error != Error::NoError)
					break;
			}

			if (error == Error::NoError)
			{
				assert(mLRUData.size() == 0);
				error = removeIndexFile();
				mCacheSize = 0;
			}

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
			return mLRUData.size(); // Disregarding index file
		}

	private:
		inline Error removeFile(const std::string& hash)
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
					auto it = std::find(mLRUData.begin(), mLRUData.end(), hash);
					if (it != mLRUData.end()) // The index file is not in the mLRUData, so we must allow it != mLRUData.end()
						mLRUData.erase(it);
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

			const std::string hash(mHashFunction("myrmo_lru_cache_data"));
			const std::string fName = mCacheDir + "/" + hash;
			std::ofstream f(fName, std::ios::binary);

			if (f.is_open())
			{
				const size_t hashSize = hash.size();
				for (auto it = mLRUData.begin(); it != mLRUData.end(); it++)
				{
					assert(it->size() == hashSize); // If not the cache would be corrupted.
					f.write(it->c_str(), hashSize);
				}

				f.close();
				error = Error::NoError;
			}

			return error;
		}

		inline Error readIndexFile()
		{
			Error error = Error::NoError;

			const std::string hash(mHashFunction("myrmo_lru_cache_data"));
			const std::string fName = file_path(hash);
			std::ifstream f(fName, std::ios::binary);

			if (f.is_open())
			{
				const size_t hashSize = hash.size();

				f.seekg(0, std::ios::end);
				const size_t fSize = f.tellg();
				assert((fSize % hashSize) == 0); // Otherwise the data is corrupted
				f.seekg(0, std::ios::beg);

				size_t pos = 0;
				while (pos < fSize)
				{
					std::string element;
					element.resize(hashSize, '\0');
					f.read(&element[0], hashSize);

					std::ifstream currentFile(file_path(element), std::ifstream::ate | std::ifstream::binary);
					if (currentFile.is_open())
					{
						mCacheSize += currentFile.tellg();
						mLRUData.emplace_back(element);
					}

					currentFile.close();
					pos += hashSize;
				}

				f.close();
			}

			return error;
		}

		inline Error removeIndexFile()
		{
			const std::string hash(mHashFunction("myrmo_lru_cache_data"));
			removeFile(hash);
			return Error::NoError;
		}

		inline Error evictUntilEnoughSpace(size_t size)
		{
			Error error = Error::NoError;

			size_t errorCount = 0;
			while ((mCacheSize + size) > mMaxCacheSize)
			{
				if (mLRUData.empty())
				{
					error = Error::FileSizeGreaterThanMaxCacheSize;
					assert(mCacheSize == 0); // We should not end up here unless mCacheSize is 0.
					break;
				}
				else
				{
					const std::string& hash = mLRUData.back();
					error = removeFile(hash);
					assert(error == Error::NoError); // The cache is corrupt if we end up removing files that does not exist.
					if (error == Error::NoError)
					{
						errorCount = 0;
						mLRUData.pop_back();
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

		std::list<std::string> mLRUData; // Least Recenty Used is at the end.
		const size_t mMaxCacheSize;
		size_t mCacheSize; // In megabytes
	};

}} // End namespace myrmo::cache
