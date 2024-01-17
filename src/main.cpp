/* ========================================================================
 *
 *     Filename:  main.cpp
 *  Description:  HTML Parser and Browser for cppreference
 *       Author:  Diego A. Estrada Rivera
 *      Version:  0.0.1
 *
 * ======================================================================== */
#include "indexer.hpp"
#include "prelude.hpp"
#include <cctype>

namespace filesystem = std::filesystem;

proc Main([[maybe_unused]] std::span<const std::string_view> args) noexcept
    -> I32
{
	let cppref =
	    filesystem::path{"/home/Michorron/Git/cppreference-doc/"
			     "reference/en.cppreference.com"};

	let cache = get_cache_dir() / "cache.txt";
	var cache_created = exists(cache);
	let documents = cache_created ? load_index() : index_directory(cppref) ;
	if (not cache_created) cache_index(documents);

	std::cout << "Succesfully indexed " << cppref.filename() << "!"
		  << std::endl;

	var query = String();
	std::cout << "What is your query?\nType here: ";
	std::getline(std::cin, query);

	cache_index(documents);

	let greater = [](let& x, let& y) { return x.second > y.second; };

	for (var& c : query) c = std::toupper(c);
	var matches = count_tfidf(documents, query);
	if (matches.size() < 5)
		ra::sort(matches, greater);
	else
		ra::partial_sort(matches, matches.begin() + 5, greater);

	for (var idx = 0LLU; let & [ name, score ] : matches) {
		if (idx > 5)
			break;
		std::cout << name << " had " << score << " score.\n";
		++idx;
	}
	return 0;
}
