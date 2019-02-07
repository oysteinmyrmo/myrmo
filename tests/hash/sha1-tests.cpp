#include <myrmo/test/assert.h>
#include <myrmo/hash/sha1.h>

#include "long_strings.h"

void test_short_strings()
{
	std::string hash;

	hash = myrmo::hash::sha1("");
	MYRMO_ASSERT(hash == "da39a3ee5e6b4b0d3255bfef95601890afd80709");

	hash = myrmo::hash::sha1("CRYPTO");
	MYRMO_ASSERT(hash == "b024e87163fd7adccbcb144afe318916ca200f94");

	hash = myrmo::hash::sha1("The quick brown fox jumps over the lazy dog");
	MYRMO_ASSERT(hash == "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");

	hash = myrmo::hash::sha1("The quick brown fox jumps over the lazy cog");
	MYRMO_ASSERT(hash == "de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3");

	hash = myrmo::hash::sha1("SHA1");
	MYRMO_ASSERT(hash == "e1744a525099d9a53c0460ef9cb7ab0e4c4fc939");

	hash = myrmo::hash::sha1("SHA-1");
	MYRMO_ASSERT(hash == "c571b86549e49bf223cf648388c46288c2241b5a");

	hash = myrmo::hash::sha1("md5");
	MYRMO_ASSERT(hash == "c1ea94f7e524679d0cf34ab7b0b28abe41ba732b");
}

void test_medium_strings()
{
	std::string hash;

	// 52 characters = 416 bits
	hash = myrmo::hash::sha1("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
	MYRMO_ASSERT(hash == "0d88334c8103bdb5a8e7e93a81878a519a95c760");

	// 55 characters = 440 bits
	hash = myrmo::hash::sha1("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABC");
	MYRMO_ASSERT(hash == "ebd854f0c7c9f58a1d5af5dee2b4c039902f945b");

	// 56 characters = 448 bits
	hash = myrmo::hash::sha1("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCD");
	MYRMO_ASSERT(hash == "45fe53c317500145812034ecf0061f12af48d782");

	// 60 characters = 480 bits
	hash = myrmo::hash::sha1("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGH");
	MYRMO_ASSERT(hash == "b9688882acb99cb938efa062388c10c8edd6dad8");

	// 64 characters = 512 bits
	hash = myrmo::hash::sha1("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJK");
	MYRMO_ASSERT(hash == "e35dbc73b66138acbbbf224729361960c2507b5b");
}

void test_long_strings()
{
	std::string hash;

	hash = myrmo::hash::sha1(long_strings::lorem_ipsum_10_paragraphs);
	MYRMO_ASSERT(hash == "56bcb1962985e684147828b8a489e929ec7ea879");

	hash = myrmo::hash::sha1(long_strings::lorem_ipsum_10_paragraphs_raw);
	MYRMO_ASSERT(hash == "8312fa7072dbd91d4c48c3346f7c248ae94ac84b");

	hash = myrmo::hash::sha1(long_strings::random_org_1000_20);
	MYRMO_ASSERT(hash == "84799a65a1543749b579dc7156743da0b573b926");
}

void test_urls()
{
	std::string hash;

	hash = myrmo::hash::sha1("https://xkcd.com/1354/");
	MYRMO_ASSERT(hash == "7f0803edde8cbd039aa28ba000e0f45372d394b4");

	hash = myrmo::hash::sha1("https://xkcd.com/936/");
	MYRMO_ASSERT(hash == "01458cc42655df971eb9b6e92f65830c7dc2ca87");

	hash = myrmo::hash::sha1("https://www.miasmat.no/drommekylling/");
	MYRMO_ASSERT(hash == "6374eb5438fb39691839280b1e04e0413edf2558");

	hash = myrmo::hash::sha1("https://www.miasmat.no/wp-content/uploads/app/w1280/135.JPG");
	MYRMO_ASSERT(hash == "a5d796236c10cf461d55ab59c9afc0f21f5f06bb");

	hash = myrmo::hash::sha1("https://www.miasmat.no/wp-content/uploads/app/w1080/78.JPG");
	MYRMO_ASSERT(hash == "340fd1006f7ba3b63d8b5259e1de40939db7325c");

	hash = myrmo::hash::sha1("https://www.miasmat.no/wp-content/uploads/app/w480/130.JPG");
	MYRMO_ASSERT(hash == "147c021d114687d14983bfb52b831450eca930b2");

	hash = myrmo::hash::sha1("https://www.miasmat.no/wp-content/uploads/app/w800/126.JPG");
	MYRMO_ASSERT(hash == "6d64c67658e27b6c266fec3fdda9a84f5a74c8a4");

	hash = myrmo::hash::sha1("https://www.miasmat.no/wp-content/uploads/app/w800/99.JPG");
	MYRMO_ASSERT(hash == "dc4d98ff26117d1cf5669cd6254d166f1334580a");

	hash = myrmo::hash::sha1("https://www.miasmat.no/wp-content/uploads/app/w800/133.JPG");
	MYRMO_ASSERT(hash == "2935675a409f251c6a48a96fa4eb198b789a8f0f");
}

int main()
{
	test_short_strings();
	test_medium_strings();
	test_long_strings();
	test_urls();
	return 0;
}
