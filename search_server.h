#pragma once

#include "concurrent_map.h"
#include "document.h"
#include "log_duration.h"
#include "string_processing.h"

#include <algorithm>
#include <cmath>
#include <execution>
#include <future>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
	template <typename StringContainer>
	explicit SearchServer(const StringContainer& stop_words)
		: stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
		if (!std::all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
			throw std::invalid_argument("Some of stop words are invalid");
		}
	}

	explicit SearchServer(const std::string& stop_words_text)
		: SearchServer(SplitIntoWords(stop_words_text)) {}

	explicit SearchServer(const std::string_view& stop_words_text)
		: SearchServer(SplitIntoWords(stop_words_text)) {}

	//	void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

	void AddDocument(int document_id, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

	template <typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const;
	std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentStatus status) const {
		return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
			return document_status == status;
			});
	}
	std::vector<Document> FindTopDocuments(const std::string_view& raw_query) const {
		return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
	}

	template <typename ExecutionPolicy, typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view& raw_query, DocumentPredicate document_predicate) const;

	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view& raw_query, DocumentStatus status) const;

	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view& raw_query) const;

	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;

	template <typename ExecutionPolicy>
	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const std::string_view& raw_query, int document_id) const;

	int GetDocumentCount() const {
		return documents_.size();
	}

	auto begin() {
		return document_ids_.begin();
	}
	auto end() {
		return document_ids_.end();
	}

	// O(log N)
	const std::map<std::string, double>& GetWordFrequencies(int document_id) const {
		if (id_to_word_freqs_.count(document_id) == 0) {
			static std::map<std::string, double> empty_map;
			return empty_map;
		}
		return id_to_word_freqs_.at(document_id);
	}

	void RemoveDocument(int document_id);

	template <typename ExecutionPolicy>
	void RemoveDocument(ExecutionPolicy&& policy, int document_id);

private:
	struct DocumentData {
		int rating;
		DocumentStatus status;
	};
	const std::set<std::string, std::less<>> stop_words_;
	std::map<std::string, std::map<int, double>, std::less<>> word_to_document_freqs_;
	std::map<int, std::map<std::string, double>> id_to_word_freqs_;
	std::map<int, DocumentData> documents_;
	std::set<int> document_ids_;

	static int ComputeAverageRating(const std::vector<int>& ratings);

	double ComputeWordInverseDocumentFreq(const std::string& word) const {
		return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
	}

	/*double ComputeWordInverseDocumentFreq(const std::string_view& word) const {
		return log(GetDocumentCount() * 1.0 / word_to_document_freqs_[word].size());
	}*/

	bool IsStopWord(const std::string& word) const {
		return stop_words_.count(word) > 0;
	}

	bool IsStopWord(const std::string_view& word) const {
		return stop_words_.count(word) > 0;
	}

	/*static bool IsValidWord(const std::string& word) {
		return std::none_of(word.begin(), word.end(), [](char c) {
			return c >= '\0' && c < ' ';
			});
	}*/

	static bool IsValidWord(const std::string_view& word) {
		return std::none_of(word.begin(), word.end(), [](char c) {
			return c >= '\0' && c < ' ';
			});
	}

	std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

	std::vector<std::string> SplitIntoWordsNoStop(const std::string_view& text) const;

	struct Query {
		/*std::set<std::string, std::less<>> plus_words;
		std::set<std::string, std::less<>> minus_words;*/
		std::set<std::string_view> plus_words;
		std::set<std::string_view> minus_words;
	};

	struct QueryWord {
		//std::string data;
		std::string_view data;
		bool is_minus;
		bool is_stop;
	};

	//	QueryWord ParseQueryWord(const std::string& text) const;

	//	Query ParseQuery(const std::string& text) const;

	QueryWord ParseQueryWord(const std::string_view& text) const;

	Query ParseQuery(const std::string_view& text) const;

	template <typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;

	template <typename ExecutionPolicy, typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(ExecutionPolicy&& policy, const Query& query, DocumentPredicate document_predicate) const;
};

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const {
	const auto query = ParseQuery(raw_query);

	auto matched_documents = FindAllDocuments(query, document_predicate);

	sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
		if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
			return lhs.rating > rhs.rating;
		}
		else {
			return lhs.relevance > rhs.relevance;
		}
		});
	if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
		matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
	}

	return matched_documents;
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view& raw_query, DocumentPredicate document_predicate) const {
	const auto query = ParseQuery(raw_query);

	auto matched_documents = SearchServer::FindAllDocuments(policy, query, document_predicate);

	std::sort(policy, matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
		if (std::abs(lhs.relevance - rhs.relevance) < 1e-6) {
			return lhs.rating > rhs.rating;
		}
		else {
			return lhs.relevance > rhs.relevance;
		}
		});
	if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
		matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
	}

	return matched_documents;
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view& raw_query, DocumentStatus status) const {
	return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
		return document_status == status;
		});
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view& raw_query) const {
	return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
	std::map<int, double> document_to_relevance;
	for (const std::string_view& word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		const double inverse_document_freq = ComputeWordInverseDocumentFreq((std::string)word);
		for (const auto [document_id, term_freq] : word_to_document_freqs_.at((std::string)word)) {
			const auto& document_data = documents_.at(document_id);
			if (document_predicate(document_id, document_data.status, document_data.rating)) {
				document_to_relevance[document_id] += term_freq * inverse_document_freq;
			}
		}
	}

	for (const std::string_view& word : query.minus_words) {
		if (word_to_document_freqs_.count((std::string)word) == 0) {
			continue;
		}
		for (const auto [document_id, _] : word_to_document_freqs_.at((std::string)word)) {
			document_to_relevance.erase(document_id);
		}
	}

	std::vector<Document> matched_documents;
	for (const auto [document_id, relevance] : document_to_relevance) {
		matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
	}
	return matched_documents;
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy&& policy, const Query& query, DocumentPredicate document_predicate) const {
	//std::map<int, double> document_to_relevance;
	ConcurrentMap<int, double> document_to_relevance(100);

	// plus-words
	{
		static constexpr int PART_COUNT = 8;
		const auto part_length = query.plus_words.size() / PART_COUNT;
		auto part_begin = query.plus_words.begin();
		auto part_end = std::next(part_begin, part_length);

		auto function = [&](const std::string_view& word) {
			const double inverse_document_freq = ComputeWordInverseDocumentFreq((std::string)word);

			/*std::for_each(word_to_document_freqs_.at((std::string)word).begin(), word_to_document_freqs_.at((std::string)word).end(),
				[&](std::pair<const int, const double> doc) {
					const auto& document_data = documents_.at(doc.first);
					if (document_predicate(doc.first, document_data.status, document_data.rating)) {
						document_to_relevance[doc.first].ref_to_value += doc.second * inverse_document_freq;
					}
				}
			);*/

			for (const auto& [document_id, term_freq] : word_to_document_freqs_.at((std::string)word)) {
				const auto& document_data = documents_.at(document_id);
				if (document_predicate(document_id, document_data.status, document_data.rating)) {
					document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
				}
			}
		};

		std::vector<std::future<void>> futures;
		for (int i = 0;	i < PART_COUNT;	++i,
			part_begin = part_end, part_end = (i == PART_COUNT - 1 ? query.plus_words.end() : next(part_begin, part_length))) {
			futures.push_back(std::async([function, part_begin, part_end] {
				std::for_each(part_begin, part_end, function);
				}));
		}
		for (int i = 0; i < futures.size(); ++i) {
			futures[i].get();
		}
	}


	// minus-words
	/*ConcurrentSet<std::string> minus_words(100);
	{
		static constexpr int PART_COUNT = 8;
		const auto part_length = query.minus_words.size() / PART_COUNT;
		auto part_begin = query.minus_words.begin();
		auto part_end = std::next(part_begin, part_length);

		auto function = [&]( std::string_view word) {
			if (word_to_document_freqs_.count((std::string)word) > 0) {
				minus_words.insert((std::string)word);
			}
		};

		std::vector<std::future<void>> futures;
		for (int i = 0;	i < PART_COUNT;	++i,
			part_begin = part_end, part_end = (i == PART_COUNT - 1 ? query.minus_words.end() : next(part_begin, part_length))) {
			futures.push_back(std::async([function, part_begin, part_end] {
				std::for_each(part_begin, part_end, function);
				}));
		}
		for (int i = 0; i < futures.size(); ++i) {
			futures[i].get();
		}
	}*/


	/*{
		static constexpr int PART_COUNT = 8;
		const auto part_length = query.minus_words.size() / PART_COUNT;
		auto part_begin = query.minus_words.begin();
		auto part_end = std::next(part_begin, part_length);
		auto function = [&](const std::string_view& word) {
			if (word_to_document_freqs_.count((std::string)word) > 0) {
				for (const auto& [document_id, _] : word_to_document_freqs_.at((std::string)word)) {
					document_to_relevance.erase(document_id);
				}
			}
		};
		std::vector<std::future<void>> futures;
		for (int i = 0;	i < PART_COUNT;	++i,
			part_begin = part_end, part_end = (i == PART_COUNT - 1 ? query.minus_words.end() : next(part_begin, part_length))) {
			futures.push_back(std::async([function, part_begin, part_end] {
				std::for_each(part_begin, part_end, function);
				}));
		}
		for (int i = 0; i < futures.size(); ++i) {
			futures[i].get();
		}
	}*/

	std::for_each(policy, query.minus_words.begin(), query.minus_words.end(),
		[&](const std::string_view& word) {
			if (word_to_document_freqs_.count((std::string)word) > 0) {
				for (const auto& [document_id, _] : word_to_document_freqs_.at((std::string)word)) {
					document_to_relevance.erase(document_id);
				}
			}
		}
	);


	// match-docs-assembly
	/*std::vector<Document> matched_documents;
	matched_documents.reserve(100000);
	auto temp = document_to_relevance.BuildOrdinaryMap();
	std::for_each(policy, temp.begin(), temp.end(),
		[&](const std::pair<const int,double>& doc) {
			matched_documents.push_back({ doc.first,doc.second, documents_.at(doc.first).rating });
		}
	);*/

	std::vector<Document> matched_documents;
	for (const auto& [document_id, relevance] : document_to_relevance.BuildOrdinaryMap()) {
		matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
	}

	return matched_documents;
}

template <typename ExecutionPolicy>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy, const std::string_view& raw_query, int document_id) const {
	if (document_id < 0 || documents_.count(document_id) == 0) {
		throw std::out_of_range("Invalid document_id");
	}
	if (!IsValidWord(raw_query)) {
		throw std::invalid_argument("Invalid query");
	}

	const auto query = ParseQuery(raw_query);

	std::vector<std::string_view> matched_words(query.plus_words.size() + query.minus_words.size());

	if (std::any_of(policy, query.minus_words.begin(), query.minus_words.end(), [&](auto& word) {
		return word_to_document_freqs_.at((std::string)word).count(document_id);
		}
	)
		) {
		return { {}, documents_.at(document_id).status };
	}

	std::copy_if(policy, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
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

template <typename ExecutionPolicy>
void  SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id) {

	if (!document_ids_.count(document_id)) {
		return;
	}

	// "авторское" решение
	std::vector<std::map<int, double>*> keys(word_to_document_freqs_.size());
	std::transform(std::execution::par, word_to_document_freqs_.begin(), word_to_document_freqs_.end(), keys.begin(),
		[](auto& doc) {
			return &doc.second;
		}
	);

	std::for_each(policy, keys.begin(), keys.end(),
		[document_id](auto& key) {
			(*key).erase(document_id);
		}
	);

	id_to_word_freqs_.erase(document_id);
	documents_.erase(document_id);
	document_ids_.erase(document_id);
}