#pragma once

#include "remove_duplicates.h"
#include "search_server.h"

SearchServer CreateSearchServObject();

void BeginEndTest();
void GetWordFreqTest();
void RemoveDocTest();
void RemoveDuplicatTest();
void PaginatorTest();

// Test old request
/* {
	for (int i = 0; i < 1439; ++i) {
		request_queue.AddFindRequest("empty request"s);
	}
	request_queue.AddFindRequest("curly dog"s);
	request_queue.AddFindRequest("big collar"s);
	request_queue.AddFindRequest("sparrow"s);
	std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
}
*/