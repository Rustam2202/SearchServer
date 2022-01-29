#pragma once

#include <set>
#include <string>
#include <vector>

std::vector<std::string> SplitIntoWords(const std::string& text);

std::vector<std::string_view> SplitIntoWords(const std::string_view& text);

//template <typename StringContainer>
//std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
//	std::set<std::string> non_empty_strings;
//	for (const std::string& str : strings) {
//		if (!str.empty()) {
//			non_empty_strings.insert(str);
//		}
//	}
//	return non_empty_strings;
//}

template <typename StringContainer, typename StringType>
std::set<StringType> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
	std::set<std::string> non_empty_strings;
	for (const std::string& str : strings) {
		if (!str.empty()) {
			non_empty_strings.insert(str);
		}
	}
	return non_empty_strings;
}