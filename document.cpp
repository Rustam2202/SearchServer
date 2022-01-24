#include "document.h"

std::ostream& operator<<(std::ostream& out, const Document& document) {
	using namespace std;
	out << "{ "s
		<< "document_id = "s << document.id << ", "s
		<< "relevance = "s << document.relevance << ", "s
		<< "rating = "s << document.rating << " }"s;
	return out;
}

/*
void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
	try {
		search_server.AddDocument(document_id, document, status, ratings);
	}
	catch (const exception& e) {
		cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << endl;
	}
}


void FindTopDocuments(const SearchServer& search_server, const string& raw_query) {
	cout << "Результаты поиска по запросу: "s << raw_query << endl;
	try {
		for (const Document& document : search_server.FindTopDocuments(raw_query)) {
			PrintDocument(document);
		}
	}
	catch (const exception& e) {
		cout << "Ошибка поиска: "s << e.what() << endl;
	}
}

void FindTopDocuments(const SearchServer& search_server, const string& raw_query, unsigned id) {
	cout << "Результаты поиска по запросу: "s << raw_query << ", id > " << id << endl;
	try {
		for (const Document& document : search_server.FindTopDocuments(raw_query, [id](int document_id, DocumentStatus status, int rating) { return document_id > id; })) {
			PrintDocument(document);
		}
	}
	catch (const exception& e) {
		cout << "Ошибка поиска: "s << e.what() << endl;
	}
}

void FindTopDocuments(const SearchServer& search_server, const string& raw_query, int ratings) {
	cout << "Результаты поиска по запросу: "s << raw_query << ", рейтинг > " << ratings << endl;
	try {
		for (const Document& document : search_server.FindTopDocuments(raw_query, [ratings](int document_id, DocumentStatus status, int rating) { return rating > ratings; })) {
			PrintDocument(document);
		}
	}
	catch (const exception& e) {
		cout << "Ошибка поиска: "s << e.what() << endl;
	}
}

void FindTopDocuments(const SearchServer& search_server, const string& raw_query, DocumentStatus status_q) {
	cout << "Результаты поиска по запросу по статусу: "s << raw_query << endl;
	try {
		for (const Document& document : search_server.FindTopDocuments(raw_query, [status_q](int document_id, DocumentStatus status, int rating) { return status == status_q; })) {
			PrintDocument(document);
		}
	}
	catch (const exception& e) {
		cout << "Ошибка поиска: "s << e.what() << endl;
	}
}

void MatchDocuments(const SearchServer& search_server, const string& query) {
	try {
		cout << "Матчинг документов по запросу: "s << query << endl;
		const int document_count = search_server.GetDocumentCount();
		for (int index = 0; index < document_count; ++index) {
			const int document_id = search_server.GetDocumentId(index);
			const auto [words, status] = search_server.MatchDocument(query, document_id);
			PrintMatchDocumentResult(document_id, words, status);
		}
	}
	catch (const exception& e) {
		cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << endl;
	}
}
*/