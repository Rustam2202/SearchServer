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

	void AddDocument(int document_id, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

	template <typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(std::string_view query, DocumentPredicate document_predicate) const;

	std::vector<Document> FindTopDocuments(std::string_view query, DocumentStatus status_query) const;

	std::vector<Document> FindTopDocuments(std::string_view query) const;

	template <typename ExecutionPolicy, typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, std::string_view query, DocumentPredicate document_predicate) const;

	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, std::string_view query, DocumentStatus status) const;

	template <typename ExecutionPolicy>
	std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, std::string_view query) const;

	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;

	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&, const std::string_view& raw_query, int document_id) const;

	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&, const std::string_view& raw_query, int document_id) const;

	int GetDocumentCount() const {
		return documents_.size();
	}

	auto begin() {
		return document_ids_.begin();
	}

	auto end() {
		return document_ids_.end();
	}

	const std::map<std::string, double>& GetWordFrequencies(int document_id) const {
		if (id_to_word_freqs_.count(document_id) == 0) {
			static std::map<std::string, double> empty_map;
			return empty_map;
		}
		return id_to_word_freqs_.at(document_id);
		//return id_to_word_freqs_[document_id];
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

	double ComputeWordInverseDocumentFreq(std::string_view word) const {
		return log(GetDocumentCount() * 1.0 / (*word_to_document_freqs_.find(word)).second.size());
	}

	bool IsStopWord(std::string_view word) const {
		return stop_words_.count(word) > 0;
	}

	static bool IsValidWord(std::string_view word) {
		return std::none_of(word.begin(), word.end(), [](char c) {
			return c >= '\0' && c < ' ';
			});
	}

	std::vector<std::string> SplitIntoWordsNoStop(std::string_view text) const;

	struct Query {
		std::set<std::string_view> plus_words;
		std::set<std::string_view> minus_words;
	};

	struct QueryWord {
		std::string_view data;
		bool is_minus;
		bool is_stop;
	};

	QueryWord ParseQueryWord(std::string_view text) const;

	Query ParseQuery(std::string_view text) const;

	template <typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const std::execution::sequenced_policy&, const Query& query, DocumentPredicate document_predicate) const;
	
	template <typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const std::execution::parallel_policy&, const Query& query, DocumentPredicate document_predicate) const;
};

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const std::execution::sequenced_policy&, const Query& query, DocumentPredicate document_predicate) const {
	std::map<int, double> document_to_relevance;
	for (const std::string_view& word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
		for (const auto& [document_id, term_freq] : (*word_to_document_freqs_.find(word)).second) {
			const auto& document_data = documents_.at(document_id);
			if (document_predicate(document_id, document_data.status, document_data.rating)) {
				document_to_relevance[document_id] += term_freq * inverse_document_freq;
			}
		}
	}

	for (const std::string_view& word : query.minus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		for (const auto& [document_id, _] : (*word_to_document_freqs_.find(word)).second) {
			document_to_relevance.erase(document_id);
		}
	}

	std::vector<Document> matched_documents;
	for (const auto& [document_id, relevance] : document_to_relevance) {
		matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
	}
	return matched_documents;
}

// Такая сложная реализация из-за тренажера, который только в таком виде принял. 
//А так еще было много вариантов, в т.ч. даже более быстрых и без ConcurrentSet (про него в подсказе задания указывалось).
template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const std::execution::parallel_policy&, const Query& query, DocumentPredicate document_predicate) const {
	ConcurrentMap<int, double> document_to_relevance(100);
	ConcurrentSet<int> minus_words(100);

	// minus-words
	{
		static constexpr int PART_COUNT = 8;
		const auto part_length = query.minus_words.size() / PART_COUNT;
		auto part_begin = query.minus_words.begin();
		auto part_end = std::next(part_begin, part_length);

		auto function = [&](std::string_view word) {
			if (word_to_document_freqs_.count(word) > 0) {
				for (const std::pair<const int, double> doc : (*word_to_document_freqs_.find(word)).second) {
					minus_words.insert(doc.first);
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
	}
	std::set<int> minWords = minus_words.BuildOrdinarySet();


	//plus-words
	{
		static constexpr int PART_COUNT = 8;
		const auto part_length = query.plus_words.size() / PART_COUNT;
		auto part_begin = query.plus_words.begin();
		auto part_end = std::next(part_begin, part_length);

		auto function = [&](const std::string_view& word) {
			const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
			for (const auto& [document_id, term_freq] : (*word_to_document_freqs_.find(word)).second) {
				if (minWords.count(document_id) > 0) {
					continue;
				}
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

	std::vector<Document> matched_documents;
	for (const auto& [document_id, relevance] : document_to_relevance.BuildOrdinaryMap()) {
		matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
	}

	return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view query, DocumentPredicate document_predicate) const {
	return FindTopDocuments(std::execution::seq, query, document_predicate);
}

template<typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view query) const {
	return FindTopDocuments(policy, query, DocumentStatus::ACTUAL);
}

template<typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view query, DocumentStatus status_query) const {
	return FindTopDocuments(policy, query, [status_query](int document_id, DocumentStatus status, int rating) { return status == status_query; });
}

template<typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view query, DocumentPredicate document_predicate) const {
	const auto parsed_query = ParseQuery(query);
	auto matched_documents = FindAllDocuments(policy, parsed_query, document_predicate);
	const double EPSILON = 1e-6;

	std::sort(policy, matched_documents.begin(), matched_documents.end(), [&EPSILON](const Document& lhs, const Document& rhs) {
		return  (std::abs(lhs.relevance - rhs.relevance) < EPSILON && lhs.rating > rhs.rating) || lhs.relevance > rhs.relevance;
		});
	if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
	{
		matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
	}
	return matched_documents;
}

template <typename ExecutionPolicy>
void  SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id) {

	if (!document_ids_.count(document_id)) {
		return;
	}

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