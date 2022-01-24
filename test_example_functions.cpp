#include "log_duration.h"
#include "paginator.h"
#include "test_example_functions.h"

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
