#pragma once

#include "remove_duplicates.h"
//#include "search_server.h"

SearchServer CreateSearchServObject();

void RemoveDocumentExecutTest();

void ProcessQueriesTest();
void ProcessQueriesSpeedTest();

void ProcessQueriesJoinedTest();
void ProcessQueriesJoinedSpeedTest();

void BeginEndTest();
void GetWordFreqTest();
void RemoveDocTest();
void RemoveDuplicatTest();
void PaginatorTest();
