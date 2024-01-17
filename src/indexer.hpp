#pragma once
#include "prelude.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <unordered_map>

template <typename Key, typename Value>
using map = std::unordered_map<Key, Value>;

struct Document {
	map<String, U64> freqs;
	String filename;

	Document(std::string_view name, map<String, U64>&& mp);

	Document(std::string_view name);

	void show_freqs() const noexcept;
};

fn filter_divs(std::string_view html) noexcept -> String;

fn to_words(std::string_view lines) noexcept -> Vector<String>;

fn index_directory(std::filesystem::path dir) noexcept -> Vector<Document>;

void cache_index(Ref<const Vector<Document>> documents) noexcept;

fn load_index() noexcept -> Vector<Document>;

fn get_cache_dir() noexcept -> std::filesystem::path;

fn count_query(Ref<const Vector<Document>> documents,
	       Ref<const String> query) noexcept
    -> Vector<std::pair<String, F64>>;

fn count_tfidf(Ref<const Vector<Document>> documents,
	       Ref<const String> query) noexcept
    -> Vector<std::pair<String, F64>>;

fn top_n_matches(Ref<const Vector<Document>> documents, Ref<const String> query, U64 n) noexcept
    -> Vector<std::pair<String, F64>>;
