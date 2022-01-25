#include "process_queries.h"

#include <algorithm>
#include <iterator>
#include <execution>
#include <functional>
#include <utility>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries) {

	std::vector<std::vector<Document>> result(queries.size());
	std::transform(std::execution::par, std::make_move_iterator(queries.begin()), std::make_move_iterator(queries.end()), std::make_move_iterator(result.begin()),
		[&search_server](const std::string& query) {return std::move(search_server.FindTopDocuments(query)); });

	return result;
}