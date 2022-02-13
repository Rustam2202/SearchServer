#include <algorithm>
#include <cstdlib>
#include <map>
#include <mutex>
#include <set>
#include <vector>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
public:
	static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

	struct Access {
		Access(std::map<Key, Value>& ref, const Key& key, std::mutex& guard_mutex) : guard_(guard_mutex), ref_to_value(ref[key]) {}
		std::lock_guard<std::mutex> guard_;
		Value& ref_to_value;
	};

	explicit ConcurrentMap(size_t bucket_count) :sub_maps_(bucket_count), value_mutex_(bucket_count), bucket_count_(bucket_count) {}

	Access operator[](const Key& key) {
		size_t index = static_cast<uint64_t>(key) % bucket_count_;
		return Access{ sub_maps_[index], key, value_mutex_[index] };
	}

	std::map<Key, Value> BuildOrdinaryMap() {
		std::map<Key, Value> result;

		for (size_t i = 0; i < bucket_count_; ++i) {
			std::lock_guard guard(value_mutex_[i]);
			result.merge(sub_maps_[i]);
		}
		return result;
	}

	void erase(const Key& key) {
		size_t index = static_cast<uint64_t>(key) % bucket_count_;
		std::lock_guard guard(value_mutex_[index]);
		sub_maps_[index].erase(key);
	}

private:
	std::vector<std::map<Key, Value>> sub_maps_;
	std::vector<std::mutex> value_mutex_;
	size_t bucket_count_;
};

template <typename Value>
class ConcurrentSet {
public:
	struct Access {
		Access(std::set<Value>& ref, const Value& val, std::mutex& guard_mutex) : guard_(guard_mutex), ref_to_value(val) {}
		std::lock_guard<std::mutex> guard_;
		const Value& ref_to_value;
	};

	explicit ConcurrentSet(size_t bucket_count) :sub_sets_(bucket_count), mutexes_(bucket_count), bucket_count_(bucket_count) {}

	Access operator[](const Value& value) {
		size_t index = static_cast<uint64_t>(value) % bucket_count_;
		return Access{ sub_sets_[index], value, mutexes_[index] };
	}

	std::set<Value> BuildOrdinarySet() {
		std::set<Value> result;

		for (size_t i = 0; i < bucket_count_; ++i) {
			std::lock_guard guard(mutexes_[i]);
			result.merge(sub_sets_[i]);
		}
		return result;
	}

	void insert(Value value) {
		size_t index = static_cast<uint64_t>(value) % bucket_count_;
		std::lock_guard guard(mutexes_[index]);
		sub_sets_[index].insert(value);
	}

private:
	std::vector<std::set<Value>> sub_sets_;
	std::vector<std::mutex> mutexes_;
	size_t bucket_count_;
};