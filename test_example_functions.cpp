#include "log_duration.h"
#include "paginator.h"
#include "process_queries.h"
#include "test_example_functions.h"

//#include <iostream>
//#include <string>
//#include <vector>

SearchServer CreateSearchServObject() {
	using namespace std;

	SearchServer search_server("and with"s);
	search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

	return search_server;
}

void ProcessQueriesTest() {
	using namespace std;

	SearchServer search_server("and with"s);

	int id = 0;
	for (const string& text : {
			"funny pet and nasty rat"s,
			"funny pet with curly hair"s,
			"funny pet and not very nasty rat"s,
			"pet with rat and rat and rat"s,
			"nasty rat with curly hair"s,
		}
		) {
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
	}

	const vector<string> queries = {
		"nasty rat -not"s,
		"not very funny nasty pet"s,
		"curly hair"s
	};

	id = 0;
	for (const auto& documents : ProcessQueries(search_server, queries)) {
		cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << endl;
	}
}

#include "search_server.h"

#include "log_duration.h"

#include <iostream>
#include <random>
#include <string>
#include <vector>

void ProcessQueriesSpeedTest() {
	using namespace std;

	string GenerateWord(mt19937 & generator, int max_length) {
		const int length = uniform_int_distribution(1, max_length)(generator);
		string word;
		word.reserve(length);
		for (int i = 0; i < length; ++i) {
			word.push_back(uniform_int_distribution('a', 'z')(generator));
		}
		return word;
	}

	vector<string> GenerateDictionary(mt19937 & generator, int word_count, int max_length) {
		vector<string> words;
		words.reserve(word_count);
		for (int i = 0; i < word_count; ++i) {
			words.push_back(GenerateWord(generator, max_length));
		}
		sort(words.begin(), words.end());
		words.erase(unique(words.begin(), words.end()), words.end());
		return words;
	}

	string GenerateQuery(mt19937 & generator, const vector<string>&dictionary, int max_word_count) {
		const int word_count = uniform_int_distribution(1, max_word_count)(generator);
		string query;
		for (int i = 0; i < word_count; ++i) {
			if (!query.empty()) {
				query.push_back(' ');
			}
			query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
		}
		return query;
	}

	vector<string> GenerateQueries(mt19937 & generator, const vector<string>&dictionary, int query_count, int max_word_count) {
		vector<string> queries;
		queries.reserve(query_count);
		for (int i = 0; i < query_count; ++i) {
			queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
		}
		return queries;
	}

	template <typename QueriesProcessor>
	void Test(string_view mark, QueriesProcessor processor, const SearchServer & search_server, const vector<string>&queries) {
		LOG_DURATION(mark);
		const auto documents_lists = processor(search_server, queries);
	}

#define TEST(processor) Test(#processor, processor, search_server, queries)

	//int main() 
	{
		mt19937 generator;
		const auto dictionary = GenerateDictionary(generator, 10000, 25);
		const auto documents = GenerateQueries(generator, dictionary, 100'000, 10);

		SearchServer search_server(dictionary[0]);
		for (size_t i = 0; i < documents.size(); ++i) {
			search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
		}

		const auto queries = GenerateQueries(generator, dictionary, 10'000, 7);
		TEST(ProcessQueries);
	}
}

void BeginEndTest() {
	using namespace std;
	LOG_DURATION_STREAM("Test Begin and End"s, cout);
	SearchServer search_server = CreateSearchServObject();

	for (const auto document_id : search_server) {
		cout << "Documents id: "s << document_id << endl;
	}
}

void GetWordFreqTest() {
	LOG_DURATION_STREAM("Test GetWordFrequencies method", std::cout);
	SearchServer search_server = CreateSearchServObject();

	search_server.GetWordFrequencies(2);
	search_server.GetWordFrequencies(999);
}

void RemoveDocTest() {
	LOG_DURATION_STREAM("Test RemoveDocument method", std::cerr);
	SearchServer search_server = CreateSearchServObject();

	search_server.RemoveDocument(3);
	search_server.RemoveDocument(3);
}

void RemoveDuplicatTest() {
	using namespace std;
	SearchServer search_server = CreateSearchServObject();

	cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << endl;
	{
		LOG_DURATION_STREAM("Test RemoveDuplicates function", cerr);
		RemoveDuplicates(search_server);
	}
	cout << "After duplicates removed: "s << search_server.GetDocumentCount() << endl;
}

void PaginatorTest() {
	using namespace std;
	SearchServer search_server = CreateSearchServObject();

	const auto search_results = search_server.FindTopDocuments("funny"s);
	int page_size = 2;
	const auto pages = Paginate(search_results, page_size);
	for (auto page = pages.begin(); page != pages.end(); ++page) {
		cout << *page << endl;
		cout << "Page break"s << endl;
	}
}
