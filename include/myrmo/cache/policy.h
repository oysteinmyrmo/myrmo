#pragma once
#include <string>
#include <vector>
#include <list>
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
		ErroneousHashSize
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
		virtual const std::string& back() const = 0;
		virtual const std::string& front() const = 0;
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
			mData.clear();
			assert((indexData.size() % mHashSize) == 0);
			for (size_t i = 0; i < indexData.size(); i += mHashSize)
				mData.insert(mData.end(), std::string(&indexData[i], mHashSize));
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

			if (error == Error::NoError)
			{
				error = Error::DoesNotExist;
				for (auto it = mData.begin(); it != mData.end(); it++)
				{
					if (*it == hash)
					{
						// Move hash to front.
						mData.splice(mData.begin(), mData, it);
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
			mData.insert(mData.begin(), hash);
			return error;
		}

		Error remove(const std::string& hash) override
		{
			mData.remove(hash);
			return Error::NoError;
		}

		std::string getIndexData() const override
		{
			std::string indexData;
			indexData.reserve(mData.size() * mHashSize);
			for (auto it = mData.begin(); it != mData.end(); it++)
				indexData.insert(indexData.size(), *it);
			return indexData;
		}

		const std::string& back() const override
		{
			return mData.back();
		}

		const std::string& front() const override
		{
			return mData.front();
		}

		void forEach(std::function<void(const std::string&hash)> callback) override
		{
			for (const auto& hash : mData)
				callback(hash);
		}

		void clear() override
		{
			mData.clear();
		}

		size_t count() const override
		{
			return mData.size();
		}

	private:
		std::list<std::string> mData;
		size_t mHashSize;
	};

}}} // End namespace myrmo::cache::policy
