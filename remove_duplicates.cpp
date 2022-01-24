#include <algorithm>

#include "remove_duplicates.h"

// O(W N log N), W = максимальное количество слов в документе, log N = GetWordFrequencies(), N = for (const int document_id : search_server)
void RemoveDuplicates(SearchServer& search_server) {
	std::set<int> id_to_remove;
	std::map<std::set<std::string>, int> sets_of_words;

	for (const int document_id : search_server) {

		std::set<std::string> words_to_set;
		for (auto word_and_freq : search_server.GetWordFrequencies(document_id)) {
			words_to_set.insert(word_and_freq.first);
		}

		if (!sets_of_words.count(words_to_set)) {
			sets_of_words.insert({ words_to_set, document_id });
		}
		else {
			id_to_remove.insert(document_id);
		}
	}

	for (const int id : id_to_remove) {
		std::cout << "Found duplicate document id " << id << std::endl;
		search_server.RemoveDocument(id);
	}
}