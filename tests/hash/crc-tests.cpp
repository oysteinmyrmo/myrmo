#include <myrmo/test/assert.h>
#include <myrmo/hash/crc.h>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(test_data);

static std::string get_file(const std::string &name)
{
	auto fs = cmrc::test_data::get_filesystem();
	auto file = fs.open(name);
	return std::string(file.begin(), file.size());
}

void test_short_strings()
{
	uint32_t hash;

	hash = myrmo::hash::crc32("");
	MYRMO_ASSERT(hash == 0);

	hash = myrmo::hash::crc32("CRYPTO");
	MYRMO_ASSERT(hash == 0x98D0EF03);

	hash = myrmo::hash::crc32("The quick brown fox jumps over the lazy dog");
	MYRMO_ASSERT(hash == 0x414FA339);

	hash = myrmo::hash::crc32("The quick brown fox jumps over the lazy cog");
	MYRMO_ASSERT(hash == 0x4400B5BC);

	hash = myrmo::hash::crc32("crc32");
	MYRMO_ASSERT(hash == 0xAFABD35E);

	hash = myrmo::hash::crc32("SHA-1");
	MYRMO_ASSERT(hash == 0xE79D8FF6);

	hash = myrmo::hash::crc32("md5");
	MYRMO_ASSERT(hash == 0xE86CEBE1);
}

void test_medium_strings()
{
	uint32_t hash;

	// 52 characters = 416 bits
	hash = myrmo::hash::crc32("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
	MYRMO_ASSERT(hash == 0xEA9A1F0C);

	// 55 characters = 440 bits
	hash = myrmo::hash::crc32("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABC");
	MYRMO_ASSERT(hash == 0x77198E02);

	// 56 characters = 448 bits
	hash = myrmo::hash::crc32("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD");
	MYRMO_ASSERT(hash == 0x4DCA12A6);

	// 60 characters = 480 bits
	hash = myrmo::hash::crc32("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGH");
	MYRMO_ASSERT(hash == 0x707CA52F);

	// 64 characters = 512 bits
	hash = myrmo::hash::crc32("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJK");
	MYRMO_ASSERT(hash == 0xD24024DC);
}

void test_long_strings()
{
	uint32_t hash;

	hash = myrmo::hash::crc32(get_file("test_data/lorem_ipsum_10_paragraphs.txt"));
	MYRMO_ASSERT(hash == 0x3BB65504);

	hash = myrmo::hash::crc32(get_file("test_data/lorem_ipsum_10_paragraphs_raw.txt"));
	MYRMO_ASSERT(hash == 0x4CE2EC5F);

	hash = myrmo::hash::crc32(get_file("test_data/random_org_1000_20.txt"));
	MYRMO_ASSERT(hash == 0x61A9967A);
}

void test_urls()
{
	uint32_t hash;

	hash = myrmo::hash::crc32("https://xkcd.com/1354/");
	MYRMO_ASSERT(hash == 0x1FA58FCE);

	hash = myrmo::hash::crc32("https://xkcd.com/936/");
	MYRMO_ASSERT(hash == 0x149A621A);

	hash = myrmo::hash::crc32("https://www.miasmat.no/drommekylling/");
	MYRMO_ASSERT(hash == 0x933111F8);

	hash = myrmo::hash::crc32("https://www.miasmat.no/wp-content/uploads/app/w1280/135.JPG");
	MYRMO_ASSERT(hash == 0xD4F6E4BD);

	hash = myrmo::hash::crc32("https://www.miasmat.no/wp-content/uploads/app/w1080/78.JPG");
	MYRMO_ASSERT(hash == 0x1CFAE14D);

	hash = myrmo::hash::crc32("https://www.miasmat.no/wp-content/uploads/app/w480/130.JPG");
	MYRMO_ASSERT(hash == 0xFC396220);

	hash = myrmo::hash::crc32("https://www.miasmat.no/wp-content/uploads/app/w800/126.JPG");
	MYRMO_ASSERT(hash == 0xA1E89616);

	hash = myrmo::hash::crc32("https://www.miasmat.no/wp-content/uploads/app/w800/99.JPG");
	MYRMO_ASSERT(hash == 0x4461C82C);

	hash = myrmo::hash::crc32("https://www.miasmat.no/wp-content/uploads/app/w800/133.JPG");
	MYRMO_ASSERT(hash == 0xA254CAC3);
}

int main()
{
	// Note: Results are validated at https://crccalc.com/, http://www.zorc.breitbandkatze.de/crc.html and crc32 on the command line.
	test_short_strings();
	test_medium_strings();
	test_long_strings();
	test_urls();
	return 0;
}
