#include <myrmo/test/assert.h>
#include <myrmo/cache/disk.h>
#include <myrmo/hash/sha1.h>

#include <string>
#include <cstdio>
#include <array>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(test_data);

static std::string get_file(const std::string &name)
{
	auto fs = cmrc::test_data::get_filesystem();
	auto file = fs.open(name);
	return std::string(file.begin(), file.size());
}

struct ImageTestData
{
	std::string name;
	int size; // in bytes
};

constexpr size_t IMAGE_COUNT = 23;
std::array<ImageTestData, IMAGE_COUNT> images {{
	{ "test_data/122.JPG", 332004 },
	{ "test_data/125.JPG", 323953 },
	{ "test_data/115.JPG", 314677 },
	{ "test_data/114.JPG", 306891 },
	{ "test_data/120.JPG", 267281 },
	{ "test_data/121.JPG", 249116 },
	{ "test_data/124.JPG", 243953 },
	{ "test_data/128.JPG", 240013 },
	{ "test_data/111.JPG", 222219 },
	{ "test_data/103.JPG", 214214 },
	{ "test_data/123.JPG", 206680 },
	{ "test_data/107.JPG", 205015 },
	{ "test_data/108.JPG", 201858 },
	{ "test_data/112.JPG", 198872 },
	{ "test_data/118.JPG", 196191 },
	{ "test_data/106.JPG", 185726 },
	{ "test_data/117.JPG", 179626 },
	{ "test_data/110.JPG", 174580 },
	{ "test_data/109.JPG", 170556 },
	{ "test_data/126.JPG", 154536 },
	{ "test_data/116.JPG", 146006 },
	{ "test_data/119.JPG", 138155 },
	{ "test_data/113.JPG", 136281 }
}};

size_t allImagesSize()
{
	size_t size = 0;
	for (size_t i = 0; i < IMAGE_COUNT; i++)
		size += images[i].size;
	return size;
}

myrmo::cache::DiskCache::Error imageExists(myrmo::cache::DiskCache& cache, size_t pos, std::vector<char>* data)
{
	std::string file(images[pos].name);
	return cache.read(images[pos].name, data);
}

myrmo::cache::DiskCache::Error insertImage(myrmo::cache::DiskCache& cache, size_t pos)
{
	std::string file(images[pos].name);
	std::string data(get_file(file));
	return cache.write(file, data);
}

myrmo::cache::DiskCache::Error deleteImage(myrmo::cache::DiskCache& cache, size_t pos)
{
	return cache.remove(images[pos].name);
}

void test_insert_read_delete_all_images()
{
	using namespace myrmo::cache;

	{
		DiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1, new policy::LRU());
		std::vector<char> data;

		// Assert no images exist.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(imageExists(cache, i, &data) == DiskCache::Error::FileDoesNotExist);

		// Assert insertion of all images ok.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(insertImage(cache, i) == DiskCache::Error::NoError);

		// Assert all images can be fetched.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(imageExists(cache, i, &data) == DiskCache::Error::NoError);

		// Assert cache size/count.
		MYRMO_ASSERT(cache.size() == allImagesSize());
		MYRMO_ASSERT(cache.count() == IMAGE_COUNT);

		// Assert bytes are ok.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
		{
			std::string image(get_file(images[i].name));
			MYRMO_ASSERT(cache.read(images[i].name, &data) == DiskCache::Error::NoError);
			MYRMO_ASSERT(image.size() == data.size());
			MYRMO_ASSERT(memcmp(image.data(), data.data(), data.size()) == 0);
		}
	}

	{
		DiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1, new policy::LRU());
		std::vector<char> data;

		// Assert all images can be fetched.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(imageExists(cache, i, &data) == DiskCache::Error::NoError);

		// Assert cache size/count;
		MYRMO_ASSERT(cache.size() == allImagesSize());
		MYRMO_ASSERT(cache.count() == IMAGE_COUNT);

		// Asert bytes are ok.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
		{
			std::string image(get_file(images[i].name));
			MYRMO_ASSERT(cache.read(images[i].name, &data) == DiskCache::Error::NoError);
			MYRMO_ASSERT(image.size() == data.size());
			MYRMO_ASSERT(memcmp(image.data(), data.data(), data.size()) == 0);
		}

		// Remove every second image.
		size_t removedSize = 0;
		for (size_t i = 0; i < IMAGE_COUNT; i += 2)
		{
			MYRMO_ASSERT(deleteImage(cache, i) == DiskCache::Error::NoError);
			removedSize += images[i].size;
		}

		// Assert cache size/count;
		MYRMO_ASSERT(cache.count() == (IMAGE_COUNT / 2));
		MYRMO_ASSERT(cache.size() == (allImagesSize() - removedSize));

		// Check existance / verify data.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
		{
			if ((i % 2) == 0)
			{
				MYRMO_ASSERT(imageExists(cache, i, &data) == DiskCache::Error::FileDoesNotExist);
			}
			else
			{
				std::string image(get_file(images[i].name));
				MYRMO_ASSERT(cache.read(images[i].name, &data) == DiskCache::Error::NoError);
				MYRMO_ASSERT(image.size() == data.size());
				MYRMO_ASSERT(memcmp(image.data(), data.data(), data.size()) == 0);
			}
		}
	}

	DiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1, new policy::LRU());
	MYRMO_ASSERT(cache.clear() == DiskCache::Error::NoError);
	MYRMO_ASSERT(cache.size() == 0);
	MYRMO_ASSERT(cache.count() == 0);
}

void test_disk_cache_eviction_policy()
{
	using namespace myrmo::cache;
	const size_t cacheSizeInMiB = 1;
	const size_t cacheSizeInBytes = cacheSizeInMiB * 1048576;

	{
		DiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1, new policy::LRU(), cacheSizeInMiB); // 1 MiB disk cache
		std::vector<char> data;

		// Assert no images exist.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(imageExists(cache, i, &data) == DiskCache::Error::FileDoesNotExist);

		// Assert insertion of all images ok.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(insertImage(cache, i) == DiskCache::Error::NoError);

		// Assert only the last MiB og image data is in the cache.
		size_t size = 0;
		int pos = IMAGE_COUNT - 1;
		while ((pos >= 0) && (size < (cacheSizeInBytes - images[pos].size)))
		{
			size += images[pos].size;
			--pos;
		}

		MYRMO_ASSERT(cache.size() == size);
		MYRMO_ASSERT(cache.size() < cacheSizeInBytes);
		MYRMO_ASSERT(cache.count() == 6);

		size_t asserted = 0;
		for (int i = IMAGE_COUNT - 1; i >= 0; i--)
		{
			if (asserted >= cache.count())
			{
				MYRMO_ASSERT(imageExists(cache, i, &data) == myrmo::cache::DiskCache::Error::FileDoesNotExist);
			}
			else
			{
				// Note: LRU order will be reversed because of this.
				MYRMO_ASSERT(imageExists(cache, i, &data) == myrmo::cache::DiskCache::Error::NoError);
				++asserted;
			}
		}

		// Will evict images IMAGE_COUNT - 1 and IMAGE_COUNT - 2.
		MYRMO_ASSERT(insertImage(cache, 0) == DiskCache::Error::NoError);
		MYRMO_ASSERT(cache.size() == 977682);
		MYRMO_ASSERT(cache.count() == 5);
		MYRMO_ASSERT(imageExists(cache, 0, &data) == myrmo::cache::DiskCache::Error::NoError);
		MYRMO_ASSERT(imageExists(cache, IMAGE_COUNT - 1, &data) == myrmo::cache::DiskCache::Error::FileDoesNotExist);
		MYRMO_ASSERT(imageExists(cache, IMAGE_COUNT - 2, &data) == myrmo::cache::DiskCache::Error::FileDoesNotExist);
		MYRMO_ASSERT(imageExists(cache, IMAGE_COUNT - 3, &data) == myrmo::cache::DiskCache::Error::NoError);

		// Will evict IMAGE_COUNT - 4 and IMAGE_COUNT - 5
		MYRMO_ASSERT(insertImage(cache, 7) == DiskCache::Error::NoError);
		MYRMO_ASSERT(cache.size() == 892603);
		MYRMO_ASSERT(cache.count() == 4);
		MYRMO_ASSERT(imageExists(cache, 7, &data) == myrmo::cache::DiskCache::Error::NoError);
		MYRMO_ASSERT(imageExists(cache, IMAGE_COUNT - 4, &data) == myrmo::cache::DiskCache::Error::FileDoesNotExist);
		MYRMO_ASSERT(imageExists(cache, IMAGE_COUNT - 5, &data) == myrmo::cache::DiskCache::Error::FileDoesNotExist);
		MYRMO_ASSERT(imageExists(cache, IMAGE_COUNT - 6, &data) == myrmo::cache::DiskCache::Error::NoError);
	}

	DiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1, new policy::LRU(), cacheSizeInMiB);
	std::vector<char> data;

	MYRMO_ASSERT(cache.size() == 892603);
	MYRMO_ASSERT(cache.count() == 4);
	MYRMO_ASSERT(imageExists(cache, 7, &data) == myrmo::cache::DiskCache::Error::NoError);
	MYRMO_ASSERT(imageExists(cache, IMAGE_COUNT - 4, &data) == myrmo::cache::DiskCache::Error::FileDoesNotExist);
	MYRMO_ASSERT(imageExists(cache, IMAGE_COUNT - 5, &data) == myrmo::cache::DiskCache::Error::FileDoesNotExist);
	MYRMO_ASSERT(imageExists(cache, IMAGE_COUNT - 6, &data) == myrmo::cache::DiskCache::Error::NoError);

	MYRMO_ASSERT(cache.clear() == DiskCache::Error::NoError);
	MYRMO_ASSERT(cache.size() == 0);
	MYRMO_ASSERT(cache.count() == 0);
}

int main()
{
	{
		using namespace myrmo::cache;
		DiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1, new policy::LRU());
		MYRMO_ASSERT(cache.clear() == DiskCache::Error::NoError);
	}

	test_insert_read_delete_all_images();
	test_disk_cache_eviction_policy();

	return 0;
}
