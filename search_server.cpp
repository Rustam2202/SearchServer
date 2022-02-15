#include "search_server.h"

#include <algorithm>
#include <iterator>
#include <execution>
#include <stdexcept>

void SearchServer::AddDocument(int document_id, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings) {
	if ((document_id < 0) || (documents_.count(document_id) > 0)) {
		throw std::invalid_argument("Invalid document_id");
	}
	const std::vector<std::string> words = SplitIntoWordsNoStop(document);

	const double inv_word_count = 1.0 / words.size();
	for (const std::string& word : words) {
		word_to_document_freqs_[word][document_id] += inv_word_count;
		id_to_word_freqs_[document_id][word] += inv_word_count;
	}
	documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
	document_ids_.insert(document_id);
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
	if (ratings.empty()) {
		return 0;
	}
	int rating_sum = 0;
	for (const int rating : ratings) {
		rating_sum += rating;
	}
	return rating_sum / static_cast<int>(ratings.size());
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query, int document_id) const {
	if (document_id < 0 || documents_.count(document_id) == 0) {
		throw std::out_of_range("Invalid document_id");
	}
	if (!IsValidWord(raw_query)) {
		throw std::invalid_argument("Invalid query");
	}
	std::string_view s_raw_query{ raw_query };
	const auto query = ParseQuery(s_raw_query);

	std::vector<std::string_view> matched_words;
	for (const std::string_view& word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if (word_to_document_freqs_.at((std::string)word).count(document_id)) {
			matched_words.push_back(word);
		}
	}
	for (const std::string_view& word : query.minus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if (word_to_document_freqs_.at((std::string)word).count(document_id)) {
			matched_words.clear();
			break;
		}
	}
	return { matched_words, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, const std::string_view& raw_query, int document_id) const {
	if (document_id < 0 || documents_.count(document_id) == 0) {
		throw std::out_of_range("Invalid document_id");
	}
	if (!IsValidWord(raw_query)) {
		throw std::invalid_argument("Invalid query");
	}

	const auto query = ParseQuery(raw_query);

	std::vector<std::string_view> matched_words(query.plus_words.size() + query.minus_words.size());

	if (std::any_of(std::execution::seq, query.minus_words.begin(), query.minus_words.end(), [&](auto& word) {
		return word_to_document_freqs_.at((std::string)word).count(document_id);
		}
	)
		) {
		return { {}, documents_.at(document_id).status };
	}

	std::copy_if(std::execution::seq, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
		[&](std::string_view word) {
			return word_to_document_freqs_.at((std::string)word).count(document_id);
		}
	);

	std::sort(std::execution::par, matched_words.begin(), matched_words.end());
	auto word_end = std::unique(std::execution::par, matched_words.begin(), matched_words.end());
	matched_words.erase(word_end, matched_words.end());
	if (matched_words[0].empty()) {
		matched_words.erase(matched_words.begin());
	}

	return { matched_words, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, const std::string_view& raw_query, int document_id) const {
	if (document_id < 0 || documents_.count(document_id) == 0) {
		throw std::out_of_range("Invalid document_id");
	}
	if (!IsValidWord(raw_query)) {
		throw std::invalid_argument("Invalid query");
	}

	const auto query = ParseQuery(raw_query);

	std::vector<std::string_view> matched_words(query.plus_words.size() + query.minus_words.size());

	if (std::any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(), [&](auto& word) {
		return word_to_document_freqs_.at((std::string)word).count(document_id);
		}
	)
		) {
		return { {}, documents_.at(document_id).status };
	}

	std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
		[&](std::string_view word) {
			return word_to_document_freqs_.at((std::string)word).count(document_id);
		}
	);

	std::sort(std::execution::par, matched_words.begin(), matched_words.end());
	auto word_end = std::unique(std::execution::par, matched_words.begin(), matched_words.end());
	matched_words.erase(word_end, matched_words.end());
	if (matched_words[0].empty()) {
		matched_words.erase(matched_words.begin());
	}

	return { matched_words, documents_.at(document_id).status };
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
	if (text.empty()) {
		throw std::invalid_argument("Query word is empty");
	}
	std::string_view word = text;
	bool is_minus = false;
	if (word[0] == '-') {
		is_minus = true;
		word = word.substr(1);
	}
	if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
		throw std::invalid_argument("Query word is invalid");
	}
	return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(std::string_view text) const {
	Query result;

	for (const std::string_view& word : SplitIntoWords(text)) {
		const auto query_word = ParseQueryWord(word);
		if (!query_word.is_stop) {
			if (query_word.is_minus) {
				result.minus_words.insert(query_word.data);
			}
			else {
				result.plus_words.insert(query_word.data);
			}
		}
	}

	return result;
}

void SearchServer::RemoveDocument(int document_id) {
	if (!std::binary_search(document_ids_.begin(), document_ids_.end(), document_id)) {
		return;
	}

	for (auto& docs_freqs : word_to_document_freqs_) {
		docs_freqs.second.erase(document_id);
	}

	id_to_word_freqs_.erase(document_id);
	documents_.erase(document_id);
	document_ids_.erase(document_id);
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(std::string_view text) const {
	std::vector<std::string> words;
	for (const std::string_view& word : SplitIntoWords(text)) {
		if (!IsValidWord(word)) {
			throw std::invalid_argument("Word is invalid");
		}
		if (!IsStopWord(word)) {
			std::string temp(word.begin(), word.end());
			words.push_back(temp);
		}
	}
	return words;
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view query) const {
	return FindTopDocuments(std::execution::seq, query);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view query, DocumentStatus status_query) const {
	return FindTopDocuments(std::execution::seq, query, status_query);
}
