#pragma once

#include <algorithm>
#include <cmath>
#include <execution>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "document.h"
#include "string_processing.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
	template <typename StringContainer>
	explicit SearchServer(const StringContainer& stop_words)
		/*: stop_words_(MakeUniqueNonEmptyStrings(stop_words))*/ {
		/*if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
			throw std::invalid_argument("Some of stop words are invalid");
		}*/
	}

	explicit SearchServer(const std::string& stop_words_text)
		: SearchServer(SplitIntoWords(stop_words_text)) {}

	explicit SearchServer(const std::string_view& stop_words_text)
		: SearchServer(SplitIntoWords(stop_words_text)) {}

	void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

	void AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings);

	template <typename DocumentPredicate>
	std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const;
	std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
		return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
			return document_status == status;
			});
	}
	std::vector<Document> FindTopDocuments(const std::string& raw_query) const {
		return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
	}

	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

	std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;

	template <typename ExecutionPolicy>
	std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const std::string& raw_query, int document_id) const;

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
	std::map<std::string, std::map<int, double>> word_to_document_freqs_;
	std::map<int, std::map<std::string, double>> id_to_word_freqs_;
	std::map<int, DocumentData> documents_;
	std::set<int> document_ids_;

	static int ComputeAverageRating(const std::vector<int>& ratings);

	double ComputeWordInverseDocumentFreq(const std::string& word) const {
		return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
	}

	bool IsStopWord(const std::string& word) const {
		return stop_words_.count(word) > 0;
	}

	bool IsStopWord(const std::string_view& word) const {
		return stop_words_.count(word) > 0;
	}

	static bool IsValidWord(const std::string& word) {
		return none_of(word.begin(), word.end(), [](char c) {
			return c >= '\0' && c < ' ';
			});
	}

	static bool IsValidWord(const std::string_view& word) {
		return none_of(word.begin(), word.end(), [](char c) {
			return c >= '\0' && c < ' ';
			});
	}

	std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

	std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view& text) const;

	struct Query {
		std::set<std::string> plus_words;
		std::set<std::string> minus_words;
	};

	struct QueryWord {
		std::string data;
		bool is_minus;
		bool is_stop;
	};

	QueryWord ParseQueryWord(const std::string& text) const;

	Query ParseQuery(const std::string& text) const;

	template <typename DocumentPredicate>
	std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
};

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
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

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
	std::map<int, double> document_to_relevance;
	for (const std::string& word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
		for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
			const auto& document_data = documents_.at(document_id);
			if (document_predicate(document_id, document_data.status, document_data.rating)) {
				document_to_relevance[document_id] += term_freq * inverse_document_freq;
			}
		}
	}

	for (const std::string& word : query.minus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
			document_to_relevance.erase(document_id);
		}
	}

	std::vector<Document> matched_documents;
	for (const auto [document_id, relevance] : document_to_relevance) {
		matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
	}
	return matched_documents;
}

template <typename ExecutionPolicy>
std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy, const std::string& raw_query, int document_id) const {
	if (document_id < 0 || documents_.count(document_id) == 0) {
		throw std::out_of_range("Invalid document_id");
	}
	if (!IsValidWord(raw_query)) {
		throw std::invalid_argument("Invalid query");
	}
	
	const auto query = ParseQuery(raw_query);
	
	std::vector<std::string> matched_words(query.plus_words.size());

	if (std::any_of(policy, query.minus_words.begin(), query.minus_words.end(), [&](auto& word) {
		return word_to_document_freqs_.at(word).count(document_id);
		}
	)
		) {
		return { {}, documents_.at(document_id).status };
	}

	std::copy_if(policy, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(),
		[&](std::string word) {
			return word_to_document_freqs_.at(word).count(document_id);
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

	// Людмила-1
	/*auto it_doc = id_to_word_freqs_.find(document_id);
	std::vector<map<std::string, std::map<int, double>>::iterator> its_word_to_doc_freq(it_doc->second.size());
	std::transform(std::execution::par, it_doc->second.begin(), it_doc->second.end(), its_word_to_doc_freq.begin(),
		[&](auto& word_freqs) {
			return word_to_document_freqs_.find(word_freqs.first);
		});

	std::for_each(std::execution::par, its_word_to_doc_freq.begin(), its_word_to_doc_freq.end(),
		[&](auto& word_docs_freq) {
			word_docs_freq->second.erase(document_id);
		});*/

		// Людмила-2
		/*auto it_doc = id_to_word_freqs_.find(document_id);
		using namespace std;
		vector<map<string, map<int, double>>::iterator> its_word_to_doc_freq;
		its_word_to_doc_freq.reserve(it_doc->second.size());

		for_each(execution::par, it_doc->second.begin(), it_doc->second.end(),
			[&](auto& word_freqs) {
				its_word_to_doc_freq.push_back(word_to_document_freqs_.find(word_freqs.first));
			});

		for_each(execution::par, its_word_to_doc_freq.begin(), its_word_to_doc_freq.end(),
			[&](auto& word_docs_freq) {
				word_docs_freq->second.erase(document_id);
			});*/

			// transorm-for_each
			//std::vector<std::map<std::string, double>*> keys(id_to_word_freqs_.size());
			//std::transform(std::execution::par, id_to_word_freqs_.begin(), id_to_word_freqs_.end(), keys.begin(),
			//	[](auto& doc) {
			//		return &doc.second;
			//	}
			//);
			//std::for_each(policy, keys.begin(), keys.end(),	[&](auto& key) {
			//		std::for_each(policy, (*key).begin(), (*key).end(), [&](auto& word) {
			//			//word_to_document_freqs_.at(word).erase(document_id);
			//			}
			//		);
			//		
			//		for (auto [word, _] : *key) {
			//			word_to_document_freqs_.at(word).erase(document_id);
			//		}
			//	}
			//);

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