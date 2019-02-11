#include <myrmo/test/assert.h>
#include <myrmo/cache/disk.h>
#include <myrmo/hash/sha1.h>

#include <string>
#include <cstdio>
#include <array>

// Simulating some file data.
struct FileData
{
	std::string url;
	std::string data;
};

std::array<FileData, 5> fileData {{
	{ "https://www.miasmat.no/wp-content/uploads/app/w240/135.JPG",  "135_w240"  },
	{ "https://www.miasmat.no/wp-content/uploads/app/w480/135.JPG",  "135_w480"  },
	{ "https://www.miasmat.no/wp-content/uploads/app/w800/135.JPG",  "135_w800"  },
	{ "https://www.miasmat.no/wp-content/uploads/app/w1080/135.JPG", "135_w1080" },
	{ "https://www.miasmat.no/wp-content/uploads/app/w1280/135.JPG", "135_w1280" }
}};

void clearCache(myrmo::cache::DiskCache& diskCache)
{
	for (const auto& f : fileData)
	{
		MYRMO_ASSERT(diskCache.remove(f.url) == 0);
	}
}

int main()
{
	std::string cacheDir(MYRMO_TESTS_CACHE_DIR);
	myrmo::cache::DiskCache diskCache(cacheDir, myrmo::hash::sha1);

	for (const auto& f : fileData)
	{
		diskCache.write(f.url, f.data.c_str(), f.data.size());
	}

	for (const auto& f : fileData)
	{
		char* data;
		size_t size;
		diskCache.read(f.url, &data, &size);
		MYRMO_ASSERT(size == f.data.size())
		MYRMO_ASSERT(memcmp(data, f.data.data(), size) == 0);
	}
	
	return 0;
}
