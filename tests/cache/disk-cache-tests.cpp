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

myrmo::cache::LRUDiskCache::Error imageExists(myrmo::cache::LRUDiskCache& cache, size_t pos, std::vector<char>* data)
{
	std::string file(images[pos].name);
	return cache.read(images[pos].name, data);
}

myrmo::cache::LRUDiskCache::Error insertImage(myrmo::cache::LRUDiskCache& cache, size_t pos)
{
	std::string file(images[pos].name);
	std::string data(get_file(file));
	return cache.write(file, data);
}

myrmo::cache::LRUDiskCache::Error deleteImage(myrmo::cache::LRUDiskCache& cache, size_t pos)
{
	return cache.remove(images[pos].name);
}

void test_insert_read_delete_all_images()
{
	using namespace myrmo::cache;

	{
		LRUDiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1);
		std::vector<char> data;

		// Assert no images exist.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(imageExists(cache, i, &data) == LRUDiskCache::Error::FileDoesNotExist);

		// Assert insertion of all images ok.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(insertImage(cache, i) == LRUDiskCache::Error::NoError);

		// Assert all images can be fetched.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(imageExists(cache, i, &data) == LRUDiskCache::Error::NoError);

		// Assert cache size/count.
		MYRMO_ASSERT(cache.size() == allImagesSize());
		MYRMO_ASSERT(cache.count() == IMAGE_COUNT);

		// Assert bytes are ok.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
		{
			std::string image(get_file(images[i].name));
			MYRMO_ASSERT(cache.read(images[i].name, &data) == LRUDiskCache::Error::NoError);
			MYRMO_ASSERT(image.size() == data.size());
			MYRMO_ASSERT(memcmp(image.data(), data.data(), data.size()) == 0);
		}
	}

	{
		LRUDiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1);
		std::vector<char> data;

		// Assert all images can be fetched.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
			MYRMO_ASSERT(imageExists(cache, i, &data) == LRUDiskCache::Error::NoError);

		// Assert cache size/count;
		MYRMO_ASSERT(cache.size() == allImagesSize());
		MYRMO_ASSERT(cache.count() == IMAGE_COUNT);

		// Asert bytes are ok.
		for (size_t i = 0; i < IMAGE_COUNT; i++)
		{
			std::string image(get_file(images[i].name));
			MYRMO_ASSERT(cache.read(images[i].name, &data) == LRUDiskCache::Error::NoError);
			MYRMO_ASSERT(image.size() == data.size());
			MYRMO_ASSERT(memcmp(image.data(), data.data(), data.size()) == 0);
		}

		// Remove every second image.
		size_t removedSize = 0;
		for (size_t i = 0; i < IMAGE_COUNT; i += 2)
		{
			MYRMO_ASSERT(deleteImage(cache, i) == LRUDiskCache::Error::NoError);
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
				MYRMO_ASSERT(imageExists(cache, i, &data) == LRUDiskCache::Error::FileDoesNotExist);
			}
			else
			{
				std::string image(get_file(images[i].name));
				MYRMO_ASSERT(cache.read(images[i].name, &data) == LRUDiskCache::Error::NoError);
				MYRMO_ASSERT(image.size() == data.size());
				MYRMO_ASSERT(memcmp(image.data(), data.data(), data.size()) == 0);
			}
		}
	}

	LRUDiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1);
	MYRMO_ASSERT(cache.clear() == LRUDiskCache::Error::NoError);
	MYRMO_ASSERT(cache.size() == 0);
	MYRMO_ASSERT(cache.count() == 0);
}

void test_disk_cache_eviction_policy()
{
	// 1. Create small disk cache. Add files so that the maximum size is exceeded.
	// 2. Check that the correct files and number of files exists.
	// 3. Verify LRU order.
	// 4. Do multiple times: a) get a file and b) check LRU order
}

int main()
{
	{
		using namespace myrmo::cache;
		LRUDiskCache cache(MYRMO_TESTS_CACHE_DIR, myrmo::hash::sha1);
		MYRMO_ASSERT(cache.clear() == LRUDiskCache::Error::NoError);
	}

	test_insert_read_delete_all_images();
	return 0;
}
