#include "log_duration.h"
#include "paginator.h"
#include "process_queries.h"
#include "search_server.h"
#include "test_example_functions.h"

#include <iostream>
#include <execution>
#include <random>
#include <string>
#include <vector>

using namespace std;

//void StringViewTest() {
//	string_view stop_words = { "and with" };
//
//	SearchServer search_server(stop_words);
//	int id = 0;
//	for (
//		const string_view text : {
//		"funny pet and nasty rat"s,
//			"funny pet with curly hair"s,
//			"funny pet and not very nasty rat"s,
//			"pet with rat and rat and rat"s,
//			"nasty rat with curly hair"s,
//	}
//	) {
//		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
//	}
//	const char char_text[] = { "curly rat" };
//	search_server.AddDocument(6, char_text, DocumentStatus::ACTUAL, { 1, 2 });
//	const std::string string_text{ "pet hair" };
//	search_server.AddDocument(7, string_text, DocumentStatus::ACTUAL, { 1, 2 });
//
//	const string_view query = "curly and funny -not";
//
//	{
//		const auto [words, status] = search_server.MatchDocument(query, 1);
//		cout << words.size() << " words for document 1"s << endl;
//		// 1 words for document 1
//	}
//
//	{
//		const auto [words, status] = search_server.MatchDocument(execution::seq, query, 2);
//		cout << words.size() << " words for document 2"s << endl;
//		// 2 words for document 2
//	}
//
//	{
//		const auto [words, status] = search_server.MatchDocument(execution::par, query, 3);
//		cout << words.size() << " words for document 3"s << endl;
//		// 0 words for document 3
//	}
//}
//
//string GenerateWord(mt19937& generator, int max_length) {
//	using namespace std;
//
//	const int length = uniform_int_distribution(1, max_length)(generator);
//	string word;
//	word.reserve(length);
//	for (int i = 0; i < length; ++i) {
//		word.push_back(uniform_int_distribution((int)'a', (int)'z')(generator));
//	}
//	return word;
//}
//
//vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
//	vector<string> words;
//	words.reserve(word_count);
//	for (int i = 0; i < word_count; ++i) {
//		words.push_back(GenerateWord(generator, max_length));
//	}
//	sort(words.begin(), words.end());
//	words.erase(unique(words.begin(), words.end()), words.end());
//	return words;
//}
//
//string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int max_word_count) {
//	const int word_count = uniform_int_distribution(1, max_word_count)(generator);
//	string query;
//	for (int i = 0; i < word_count; ++i) {
//		if (!query.empty()) {
//			query.push_back(' ');
//		}
//		query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
//	}
//	return query;
//}
//
//string GenerateQueryDouble(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
//	string query;
//	for (int i = 0; i < word_count; ++i) {
//		if (!query.empty()) {
//			query.push_back(' ');
//		}
//		if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
//			query.push_back('-');
//		}
//		query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
//	}
//	return query;
//}
//
//vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
//	vector<string> queries;
//	queries.reserve(query_count);
//	for (int i = 0; i < query_count; ++i) {
//		queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
//	}
//	return queries;
//}
//
//template <typename QueriesProcessor>
//void Test1(string_view mark, QueriesProcessor processor, const SearchServer& search_server, const vector<string>& queries) {
//	LOG_DURATION("mark");
//	const auto documents_lists = processor(search_server, queries);
//}
//
//void ProcessQueriesTest() {
//	using namespace std;
//
//	SearchServer search_server("and with"s);
//
//	int id = 0;
//	for (const string& text : {
//			"funny pet and nasty rat"s,
//			"funny pet with curly hair"s,
//			"funny pet and not very nasty rat"s,
//			"pet with rat and rat and rat"s,
//			"nasty rat with curly hair"s,
//		}
//		) {
//		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
//	}
//
//	const vector<string> queries = {
//		"nasty rat -not"s,
//		"not very funny nasty pet"s,
//		"curly hair"s
//	};
//
//	id = 0;
//	for (const auto& documents : ProcessQueries(search_server, queries)) {
//		cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << endl;
//	}
//}
//
//void ProcessQueriesSpeedTest() {
//
//#define TEST1(processor) Test1(#processor, processor, search_server, queries)
//
//	//int main() 
//	{
//		mt19937 generator;
//		const auto dictionary = GenerateDictionary(generator, 10000, 25);
//		const auto documents = GenerateQueries(generator, dictionary, 100'000, 10);
//
//		SearchServer search_server(dictionary[0]);
//		for (size_t i = 0; i < documents.size(); ++i) {
//			search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
//		}
//
//		const auto queries = GenerateQueries(generator, dictionary, 10'000, 7);
//		TEST1(ProcessQueries);
//	}
//}
//
//void ProcessQueriesJoinedTest() {
//	SearchServer search_server("and with"s);
//
//	int id = 0;
//	for (
//		const string& text : {
//			"funny pet and nasty rat"s,
//			"funny pet with curly hair"s,
//			"funny pet and not very nasty rat"s,
//			"pet with rat and rat and rat"s,
//			"nasty rat with curly hair"s,
//		}
//		) {
//		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
//	}
//
//	const vector<string> queries = {
//		"nasty rat -not"s,
//		"not very funny nasty pet"s,
//		"curly hair"s
//	};
//
//	//auto a= search_server.FindTopDocuments(queries[0]);
//
//	for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
//		cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
//	}
//
//	/*
//		Document 1 matched with relevance 0.183492
//		Document 5 matched with relevance 0.183492
//		Document 4 matched with relevance 0.167358
//		Document 3 matched with relevance 0.743945
//		Document 1 matched with relevance 0.311199
//		Document 2 matched with relevance 0.183492
//		Document 5 matched with relevance 0.127706
//		Document 4 matched with relevance 0.0557859
//		Document 2 matched with relevance 0.458145
//		Document 5 matched with relevance 0.458145
//	*/
//}
//
//void ProcessQueriesJoinedSpeedTest() {
//	mt19937 generator;
//	const auto dictionary = GenerateDictionary(generator, 10000, 25);
//	const auto documents = GenerateQueries(generator, dictionary, 100'000, 10);
//
//	SearchServer search_server(dictionary[0]);
//	for (size_t i = 0; i < documents.size(); ++i) {
//		search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
//	}
//
//	const auto queries = GenerateQueries(generator, dictionary, 10'000, 7);
//	TEST1(ProcessQueriesJoined);
//}
//
//template <typename ExecutionPolicy>
//void Test3(string_view mark, SearchServer search_server, ExecutionPolicy&& policy) {
//	LOG_DURATION("mark");
//	const int document_count = search_server.GetDocumentCount();
//	for (int id = 0; id < document_count; ++id) {
//		search_server.RemoveDocument(policy, id);
//	}
//	//search_server.RemoveDocument(policy, 453);
//
//	cout << search_server.GetDocumentCount() << endl;
//}
//
//void RemoveDocumentExecutTest() {
//	using namespace std;
//
//	SearchServer search_server("and with"s);
//
//	int id = 0;
//	for (
//		const string& text : {
//			"funny pet and nasty rat"s,
//			"funny pet with curly hair"s,
//			"funny pet and not very nasty rat"s,
//			"pet with rat and rat and rat"s,
//			"nasty rat with curly hair"s,
//		}
//		) {
//		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
//	}
//
//	const string query = "curly and funny"s;
//
//	auto report = [&search_server, &query] {
//		cout << search_server.GetDocumentCount() << " documents total, "s
//			<< search_server.FindTopDocuments(query).size() << " documents for query ["s << query << "]"s << endl;
//	};
//
//	report();
//	// ???????????? ??????
//	search_server.RemoveDocument(5);
//	report();
//	// ???????????? ??????
//	search_server.RemoveDocument(execution::seq, 1);
//	report();
//	// ????????????? ??????
//	search_server.RemoveDocument(execution::par, 2);
//	report();
//}
//
//void RemoveDocumentExecutSpeedTest() {
//#define TEST3(mode) Test3(#mode, search_server, execution::mode)
//
//	mt19937 generator;
//
//	const auto dictionary = GenerateDictionary(generator, 3000, 25); // 10k
//	const auto documents = GenerateQueries(generator, dictionary, 3000, 100); //10k
//
//	SearchServer search_server(dictionary[0]);
//	for (size_t i = 0; i < documents.size(); ++i) {
//		search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
//	}
//
//	TEST3(seq);
//	TEST3(par);
//}
//
//void MatchDocumentExecutTest() {
//	using namespace std;
//	SearchServer search_server("and with"s);
//
//	int id = 0;
//	for (
//		const string& text : {
//			"funny pet and nasty rat"s,
//			"funny pet with curly hair"s,
//			"funny pet and not very nasty rat"s,
//			"pet with rat and rat and rat"s,
//			"nasty rat with curly hair"s,
//		}
//		) {
//		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
//	}
//
//	const string query = "curly and funny -not"s;
//
//	{
//		const auto [words, status] = search_server.MatchDocument(query, 1);
//		cout << words.size() << " words for document 1"s << endl;
//		// 1 words for document 1
//	}
//
//	{
//		const auto [words, status] = search_server.MatchDocument(execution::seq, query, 2);
//		cout << words.size() << " words for document 2"s << endl;
//		// 2 words for document 2
//	}
//
//	{
//		const auto [words, status] = search_server.MatchDocument(execution::par, query, 3);
//		cout << words.size() << " words for document 3"s << endl;
//		// 0 words for document 3
//	}
//}
//
//template <typename ExecutionPolicy>
//void Test4(string_view mark, SearchServer search_server, const string& query, ExecutionPolicy&& policy) {
//	LOG_DURATION("mark");
//	const int document_count = search_server.GetDocumentCount();
//	int word_count = 0;
//	for (int id = 0; id < document_count; ++id) {
//		const auto [words, status] = search_server.MatchDocument(policy, query, id);
//		word_count += words.size();
//	}
//	cout << word_count << endl;
//}
//
//void MatchDocumentExecutSpeedTest() {
//#define TEST4(policy) Test4(#policy, search_server, query, execution::policy)
//
//	mt19937 generator;
//
//	const auto dictionary = GenerateDictionary(generator, 1000, 10);
//	const auto documents = GenerateQueries(generator, dictionary, 10000, 70);
//
//	const string query = GenerateQueryDouble(generator, dictionary, 500, 0.1);
//
//	SearchServer search_server(dictionary[0]);
//	for (size_t i = 0; i < documents.size(); ++i) {
//		search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
//	}
//
//	TEST4(seq);
//	TEST4(par);
//}

string GenerateWord(mt19937& generator, int max_length) {
	const int length = uniform_int_distribution(1, max_length)(generator);
	string word;
	word.reserve(length);
	for (int i = 0; i < length; ++i) {
		word.push_back(uniform_int_distribution((int)'a',(int) 'z')(generator));
	}
	return word;
}

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
	vector<string> words;
	words.reserve(word_count);
	for (int i = 0; i < word_count; ++i) {
		words.push_back(GenerateWord(generator, max_length));
	}
	words.erase(unique(words.begin(), words.end()), words.end());
	return words;
}

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
	string query;
	for (int i = 0; i < word_count; ++i) {
		if (!query.empty()) {
			query.push_back(' ');
		}
		if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
			query.push_back('-');
		}
		query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
	}
	return query;
}

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
	vector<string> queries;
	queries.reserve(query_count);
	for (int i = 0; i < query_count; ++i) {
		queries.push_back(GenerateQuery(generator, dictionary, max_word_count,0.5));
	}
	return queries;
}

void PrintDocument(const Document& document) {
	cout << "{ "s
		<< "document_id = "s << document.id << ", "s
		<< "relevance = "s << document.relevance << ", "s
		<< "rating = "s << document.rating << " }"s << endl;
}

void FindTopDocumentsTest() {
	using namespace std;

	SearchServer search_server("and with"s);

	int id = 0;
	for (
		const string& text : {
			"white cat and yellow hat"s,
			"curly cat curly tail"s,
			"nasty dog with big eyes"s,
			"nasty pigeon john"s,
		}
		) {
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
	}


	cout << "ACTUAL by default:"s << endl;
	// ???????????????? ??????
	for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
		PrintDocument(document);
	}
	cout << "BANNED:"s << endl;
	// ???????????????? ??????
	for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
		PrintDocument(document);
	}

	cout << "Even ids:"s << endl;
	// ???????????? ??????
	for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
		PrintDocument(document);
	}
}

template <typename ExecutionPolicy>
void Test5(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy) {
	LOG_DURATION("mark");
	double total_relevance = 0;
	for (const string_view query : queries) {
		//search_server.FindTopDocuments(policy, query);//
		for (const auto& document : search_server.FindTopDocuments(policy, query)) {
			total_relevance += document.relevance;
		}
	}
	cout << total_relevance << endl;

	/*{
ACTUAL by default:
	{ document_id = 2, relevance = 0.866434, rating = 1 }
	{ document_id = 4, relevance = 0.231049, rating = 1 }
	{ document_id = 1, relevance = 0.173287, rating = 1 }
	{ document_id = 3, relevance = 0.173287, rating = 1 }
BANNED:
	Even ids :
	{ document_id = 2, relevance = 0.866434, rating = 1 }
	{ document_id = 4, relevance = 0.231049, rating = 1 } 
	}*/
}

void FindTopDocumentsSpeedTest() {

#define TEST5(policy) Test5(#policy, search_server, queries, execution::policy)

	mt19937 generator;

	const auto dictionary = GenerateDictionary(generator, 1000, 10);
	const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);

	SearchServer search_server(dictionary[0]);
	for (size_t i = 0; i < documents.size(); ++i) {
		search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
	}

	const auto queries = GenerateQueries(generator, dictionary, 100, 70);

	TEST5(seq);
	TEST5(par);
}


//SearchServer CreateSearchServObject() {
//	using namespace std;
//
//	SearchServer search_server("and with"s);
//	search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
//	search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
//	search_server.AddDocument(3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
//	search_server.AddDocument(4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
//	search_server.AddDocument(5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
//	search_server.AddDocument(6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
//	search_server.AddDocument(7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });
//	search_server.AddDocument(8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });
//	search_server.AddDocument(9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
//
//	return search_server;
//}


//void BeginEndTest() {
//	using namespace std;
//	LOG_DURATION_STREAM("Test Begin and End"s, cout);
//	SearchServer search_server = CreateSearchServObject();
//
//	for (const auto document_id : search_server) {
//		cout << "Documents id: "s << document_id << endl;
//	}
//}
//
//void GetWordFreqTest() {
//	LOG_DURATION_STREAM("Test GetWordFrequencies method", std::cout);
//	SearchServer search_server = CreateSearchServObject();
//
//	search_server.GetWordFrequencies(2);
//	search_server.GetWordFrequencies(999);
//}
//
//void RemoveDocTest() {
//	LOG_DURATION_STREAM("Test RemoveDocument method", std::cerr);
//	SearchServer search_server = CreateSearchServObject();
//
//	search_server.RemoveDocument(3);
//	search_server.RemoveDocument(3);
//}
//
//void RemoveDuplicatTest() {
//	using namespace std;
//	SearchServer search_server = CreateSearchServObject();
//
//	cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << endl;
//	{
//		LOG_DURATION_STREAM("Test RemoveDuplicates function", cerr);
//		RemoveDuplicates(search_server);
//	}
//	cout << "After duplicates removed: "s << search_server.GetDocumentCount() << endl;
//}
//
//void PaginatorTest() {
//	using namespace std;
//	SearchServer search_server = CreateSearchServObject();
//
//	const auto search_results = search_server.FindTopDocuments("funny"s);
//	int page_size = 2;
//	const auto pages = Paginate(search_results, page_size);
//	for (auto page = pages.begin(); page != pages.end(); ++page) {
//		cout << *page << endl;
//		cout << "Page break"s << endl;
//	}
//}
