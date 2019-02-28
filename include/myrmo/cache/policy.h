#pragma once
#include <string>
#include <vector>
#include <set>
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <functional>

namespace myrmo { namespace cache { namespace policy
{
	enum class Error : unsigned int
	{
		NoError,
		AlreadyExists,
		DoesNotExist,
		DataCorrupted,
		ErroneousHashSize,
		HashDoesNotExist
	};

	struct EvictionPolicy
	{
		virtual ~EvictionPolicy() {};

		virtual Error setHashSize(const size_t hashSize) = 0;
		virtual Error setIndexData(const std::vector<char>& indexData) = 0;
		virtual Error exists(const std::string& hash) = 0;
		virtual Error add(const std::string& hash) = 0;
		virtual Error remove(const std::string& hash) = 0;
		virtual std::string getIndexData() const = 0;
		virtual std::string back() const = 0;
		virtual std::string front() const = 0;
		virtual void forEach(std::function<void(const std::string&hash)> callback) = 0;
		virtual void clear() = 0;
		virtual size_t count() const = 0;
	};

	class LRU : public EvictionPolicy
	{
	public:
		LRU(){}
		~LRU() override {}
		LRU(const LRU&) = delete;
		LRU(LRU&&) = delete;

		Error setHashSize(const size_t hashSize) override
		{
			mHashSize = hashSize;
			return Error::NoError;
		}

		Error setIndexData(const std::vector<char>& indexData) override
		{
			Error error = Error::NoError;
			const size_t reserve = std::max(mHashSize * 100, indexData.size() / mHashSize + 10 * mHashSize);
			mData.clear();
			mData.reserve(reserve);
			assert((indexData.size() % mHashSize) == 0);
			mData.insert(mData.end(), indexData.begin(), indexData.end());
			return error;
		}

		Error exists(const std::string& hash) override
		{
			Error error = Error::NoError;

			if (hash.size() != mHashSize)
			{
				error = Error::ErroneousHashSize;
				assert(false);
			}
			else if ((mData.size() % mHashSize) != 0)
			{
				error = Error::DataCorrupted;
				assert(false);
			}

			if (error == Error::NoError)
			{
				error = Error::DoesNotExist;
				for (size_t i = 0; i < mData.size(); i += mHashSize)
				{
					if (memcmp(&mData[i], &hash[0], mHashSize) == 0)
					{
						// Move hash to front.
						mData.erase(mData.begin() + i, mData.begin() + i + mHashSize);
						mData.insert(mData.begin(), hash.begin(), hash.end());
						error = Error::NoError;
						break;
					}
				}
			}

			return error;
		}

		Error add(const std::string& hash) override
		{
			Error error = Error::NoError;
			assert(exists(hash) == Error::DoesNotExist);
			assert(hash.size() == mHashSize);
			mData.insert(mData.begin(), hash.begin(), hash.end());
			return error;
		}

		Error remove(const std::string& hash) override
		{
			Error error = Error::HashDoesNotExist;

			for (int i = 0; i < mData.size(); i += mHashSize)
			{
				if (memcmp(&mData[i], hash.data(), mHashSize) == 0)
				{
					std::vector<char> updated;
					updated.reserve(mData.capacity());
					updated.insert(updated.end(), mData.begin(), mData.begin() + i);
					updated.insert(updated.end(), mData.begin() + i + mHashSize, mData.end());
					mData.swap(updated);
					error = Error::NoError;
					break;
				}
			}

			return error;
		}

		std::string getIndexData() const override
		{
			std::string indexData;
			indexData.reserve(mData.size());
			indexData.insert(indexData.begin(), mData.begin(), mData.end());
			return indexData;
		}

		std::string back() const override
		{
			std::string hash;
			if (count())
			{
				hash.reserve(mHashSize);
				hash.insert(hash.begin(), mData.end() - mHashSize, mData.end());
			}
			return hash;
		}

		std::string front() const override
		{
			std::string hash;
			if (count())
			{
				hash.reserve(mHashSize);
				hash.insert(hash.begin(), mData.begin(), mData.begin() + mHashSize);
			}
			return hash;
		}

		void forEach(std::function<void(const std::string&hash)> callback) override
		{
			for (size_t i = 0; i < mData.size(); i += mHashSize)
			{
				std::string hash;
				hash.reserve(mHashSize);
				hash.insert(hash.begin(), mData.begin() + i, mData.begin() + i + mHashSize);
				callback(hash);
			}
		}

		void clear() override
		{
			mData.clear();
			mHashSize = 0;
		}

		size_t count() const override
		{
			return mData.size() / mHashSize; // Disregarding index file
		}

	private:
		std::vector<char> mData;
		size_t mHashSize;
	};

}}} // End namespace myrmo::cache::policy
