#include "indexer.hpp"
#include "prelude.hpp"
#include <cctype>
#include <cstdlib>

fn filter_divs(std::string_view html) noexcept -> String
{
	var buffer = String();
	while (not html.empty()) {
		if (html.front() == '<') [[unlikely]] {
			while (html.front() != '>')
				html.remove_prefix(1);
			html.remove_prefix(1);
		} else {
			buffer += html.front();
			html.remove_prefix(1);
		}
	}

	return buffer;
}

fn to_words(std::string_view lines) noexcept -> Vector<String>
{
	var buffer = Vector<String>();
	let valid_char = [](Char ch) {
		return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
	};
	while (not lines.empty()) {
		if (not valid_char(lines.front())) [[unlikely]] {
			while (not lines.empty() &&
			       not valid_char(lines.front())) {
				lines.remove_prefix(1);
			}
		} else {
			var str = String();
			while (valid_char(lines.front())) {
				str += std::toupper(lines.front());
				lines.remove_prefix(1);
			}
			buffer.push_back(str);
		}
	}

	return buffer;
}

fn index_directory(std::filesystem::path dir) noexcept -> Vector<Document>
{
	std::cout << "creating index of: " << dir << std::endl;
	var documents = Vector<Document>();
	for (let& dir_entry :
	     std::filesystem::recursive_directory_iterator(dir)) {
		let path = dir_entry.path();
		if (path.extension() != ".html")
			continue;
		let document = Document(path.native());
		documents.push_back(document);
	}

	return documents;
}

fn count_query(Ref<const Vector<Document>> documents,
	       Ref<const String> query) noexcept
    -> Vector<std::pair<String, F64>>
{
	var matches = Vector<std::pair<String, F64>>();
	for (let& document : documents) {
		if (document.freqs.contains(query)) {
			matches.push_back(
			    {document.filename, F64(document.freqs.at(query))});
		}
	}
	return matches;
}

fn count_tfidf(Ref<const Vector<Document>> documents,
	       Ref<const String> query) noexcept
    -> Vector<std::pair<String, F64>>
{
	var matches = Vector<std::pair<String, F64>>();
	var tf = F64();
	var df = U64();
	for (let& document : documents) {
		if (document.freqs.contains(query)) {
			tf = F64(document.freqs.at(query)) /
			     F64(document.freqs.size());
			++df;
			matches.push_back({document.filename, tf});
		}
	}

	for (var & [ _, count ] : matches) {
		count /= df;
	}
	return matches;
}

void cache_index(Ref<const Vector<Document>> documents) noexcept
{
	std::cout << "caching index in: ";
	using namespace std::filesystem;
	/* check if cache dir exists and has an index */
	let cache_dir = get_cache_dir();
	create_directories(cache_dir);
	let cache = cache_dir / "cache.txt"s;

	std::cout << cache << std::endl;

	/* index into cache dir */
	var f = std::ofstream();
	if (not exists(cache)) {
		f.open(cache.c_str());
		for (let& document : documents) {
			f << document.filename << ' ' << document.freqs.size() << '\n';
			for (let& [word, count] : document.freqs) {
				f << word << ' ' << count << '\n';
			}
		}
		f.close();
	}
}

fn get_cache_dir() noexcept -> std::filesystem::path
{
	using namespace std::filesystem;
	var cache_dir = String();
	if (std::getenv("XDG_CACHE_HOME")) {
		cache_dir = path(std::getenv("XDG_CACHE_HOME")) / "stl-index";
	} else if (std::getenv("HOME")) {
		cache_dir = path(std::getenv("HOME")) / ".cache" / "stl-index";
	} else {
		cache_dir = path(".");
		std::cerr << "Cache directory not found at `$XDG_CACHE_HOME` or `$HOME/.cache`." << std::endl;
		std::cerr << "Cache will be dumped in the current directory. Do with it what you will." << std::endl;
	}

	return cache_dir;
}

fn load_index() noexcept -> Vector<Document>
{
	std::cout << "loading index from: ";
	var documents = Vector<Document>();
	let cache_dir = get_cache_dir();
	var cache = std::ifstream();
	var path = cache_dir / "cache.txt";
	std::cout << path << std::endl;
	cache.open(path);
	if (cache.is_open()) {
		var n = 0ULL;
		var count = 0ULL;
		while (not cache.eof()) {
			var freqs = Vector<std::pair<String, U64>>();
			var filename = String();
			var word = String();
			cache >> filename;
			cache >> n;
			while (n-- && not cache.eof()) {
				cache >> word;
				cache >> count;
				freqs.push_back({ word, count });
			}
			documents.emplace_back(filename, std::unordered_map(freqs.cbegin(), freqs.cend()));
		}
	}

	return documents;
}

Document::Document(std::string_view name, map<String, U64>&& mp)
    : freqs(mp), filename(name)
{
}

Document::Document(std::string_view name) : filename(name)
{
	var f = std::ifstream(filename);

	if (not f.is_open()) {
		std::cout << "Failed to open: " << name << std::endl;
	} else {
		/* weird notation for converting a file to string */
		var buffer = std::stringstream();
		buffer << f.rdbuf();
		let html = buffer.str();

		let words = to_words(filter_divs(html));
		for (let& str : words)
			++freqs[str];
	}
}

void Document::show_freqs() const noexcept
{
	for (let & [ term, freq ] : freqs)
		std::cout << term << ": " << freq << std::endl;
}

fn top_n_matches(Ref<const Vector<Document>> documents, Ref<const String> query, U64 n) noexcept
    -> Vector<std::pair<String, F64>>
{
	var matches = count_tfidf(documents, query);
	let greater = [](let& x, let& y) { return x.second > y.second; };
	if (matches.size() < n)
		ra::sort(matches, greater);
	else
		ra::partial_sort(matches, matches.begin() + n, greater);

	matches.resize(n);

	return matches;
}
