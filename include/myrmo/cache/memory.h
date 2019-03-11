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
#include <unordered_map>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <memory>

namespace myrmo { namespace cache
{
	class MemoryCache
	{
	public:
		enum class Error : unsigned int
		{
			NoError,
			ItemDoesNotExist,
			CouldNotRemoveItem,
			SizeExceedsCacheSize
		};

		struct DataRef
		{
			size_t position;
			size_t size;
			size_t end() const { return position + size; }
		};

		typedef std::string (*hashFunction)(const std::string& uri);

		MemoryCache() = delete;
		MemoryCache(const MemoryCache& cache) = delete;
		MemoryCache(MemoryCache&& cache) = delete;
		~MemoryCache() {}

		MemoryCache(hashFunction func, policy::EvictionPolicy* policy, size_t cacheSizeInMegaBytes = 10)
			: mHashFunction(func)
			, mPolicy(policy)
			, mMaxCacheSize(cacheSizeInMegaBytes * 1048576)
		{
			const std::string hash(mHashFunction("myrmo_memory_cache"));
			mPolicy->setHashSize(hash.size());
			mData.reserve(mMaxCacheSize);
		}

		Error read(const std::string& uri, std::vector<char>* data)
		{
			Error error = Error::ItemDoesNotExist;
			const std::string hash(mHashFunction(uri));

			if (mPolicy->exists(hash) == policy::Error::NoError)
			{
				const auto& it = mDataRefs.find(hash);
				assert(it != mDataRefs.end());
				if (it != mDataRefs.end())
				{
					size_t start = it->second.position;
					size_t end = it->second.end();
					assert(end >= start);
					assert((start < mData.size()) && (end <= mData.size()));

					std::vector<char> out;
					out.reserve(it->second.size);
					out.insert(out.begin(), mData.begin() + start, mData.begin() + end);
					data->swap(out);
					error = Error::NoError;
				}
			}

			return error;
		}

		Error write(const std::string& uri, const char* data, size_t size)
		{
			Error error = Error::NoError;
			const std::string hash(mHashFunction(uri));
			assert(mPolicy->exists(hash) == policy::Error::DoesNotExist);

			error = evictUntilEnoughSpace(size);
			if (error == Error::NoError)
			{
				assert((mData.size() + size) <= mMaxCacheSize);
				size_t position = mData.size();
				mData.insert(mData.end(), data, data + size);
				mDataRefs.insert({hash, { position, size }});
				mPolicy->add(hash);
			}

			return error;
		}

		Error write(const std::string& uri, const std::string& data)
		{
			return write(uri, data.c_str(), data.size());
		}

		Error clear()
		{
			mData.clear();
			mPolicy->clear();
			assert(mPolicy->count() == 0);
			assert(size() == 0);
			return Error::NoError;
		}

		inline Error remove(const std::string& uri)
		{
			Error error = Error::NoError;
			const std::string hash(mHashFunction(uri));
			assert(mPolicy->exists(hash) == policy::Error::NoError);
			policy::Error pError = mPolicy->remove(hash);

			if (pError == policy::Error::NoError)
				error = removeItem(hash);
			else
				error = Error::ItemDoesNotExist;

			return error;
		}

		size_t size() const
		{
			return mData.size();
		}

		size_t count() const
		{
			return mPolicy->count();
		}

	private:
		inline Error removeItem(const std::string& hash)
		{
			Error error = Error::ItemDoesNotExist;
			const auto it = mDataRefs.find(hash);
			assert(it != mDataRefs.end());
			if (it != mDataRefs.end())
			{
				DataRef removed = it->second;
				mData.erase(mData.begin() + removed.position, mData.begin() + removed.end());
				mDataRefs.erase(it);
				for (auto& it : mDataRefs) // TODO: Consider changing data structure(s) because of this iteration.
				{
					if (it.second.position > removed.position)
						it.second.position -= removed.size;
				}
				error = Error::NoError;
			}
			return error;
		}

		inline Error evictUntilEnoughSpace(const size_t size)
		{
			Error error = Error::NoError;

			if (size > mMaxCacheSize)
			{
				error = Error::SizeExceedsCacheSize;
			}
			else
			{
				while ((error == Error::NoError) && (mData.size() + size) > mMaxCacheSize)
				{
					const std::string hash = mPolicy->back();
					error = removeItem(hash);
					assert(error == Error::NoError);
					if (error == Error::NoError)
					{
						policy::Error pError = mPolicy->remove(hash);
						assert(pError == policy::Error::NoError);
						if (pError != policy::Error::NoError)
						{
							error = Error::CouldNotRemoveItem;
						}
					}
				}
			}

			return error;
		}

	private:
		hashFunction mHashFunction;
		std::unique_ptr<policy::EvictionPolicy> mPolicy;

		const size_t mMaxCacheSize;
		std::vector<char> mData;
		std::unordered_map<std::string, DataRef> mDataRefs;
	};

}} // End namespace myrmo::cache
