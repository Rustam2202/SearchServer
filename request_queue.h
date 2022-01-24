#pragma once

#include <algorithm>
#include <deque>

#include "search_server.h"

class RequestQueue {
public:
	explicit RequestQueue(const SearchServer& search_server)
		: search_server_(search_server) {}

	template <typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
	std::vector<Document> AddFindRequest(const std::string& raw_query);

	int GetNoResultRequests() const;

private:
	const static int sec_in_day_ = 1440;
	int seconds_current = 0;
	struct QueryResult {
		std::vector<Document>find_docs;
	};
	std::deque<QueryResult> requests_;
	const SearchServer& search_server_;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
	std::vector<Document> finded = search_server_.FindTopDocuments(raw_query, document_predicate);

	QueryResult finded_in_struct;
	finded_in_struct.find_docs = finded;
	requests_.push_back(finded_in_struct);

	if (requests_.size() > sec_in_day_) {
		requests_.pop_front();
	}
	return finded;
}