#include <algorithm>
#include <cstdlib>
#include <map>
#include <mutex>
#include <set>
#include <vector>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
private:
	struct Bucket {
		std::mutex mutex;
		std::map<Key, Value> map;
	};

public:
	static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

	struct Access {
		std::lock_guard<std::mutex> guard;
		Value& ref_to_value;

		Access(const Key& key, Bucket& bucket)
			: guard(bucket.mutex)
			, ref_to_value(bucket.map[key]) {
		}
	};

	explicit ConcurrentMap(size_t bucket_count)
		: buckets_(bucket_count) {
	}

	Access operator[](const Key& key) {
		auto& bucket = buckets_[static_cast<uint64_t>(key) % buckets_.size()];
		return { key, bucket };
	}

	std::map<Key, Value> BuildOrdinaryMap() {
		std::map<Key, Value> result;
		for (auto& [mutex, map] : buckets_) {
			std::lock_guard g(mutex);
			result.insert(map.begin(), map.end());
		}
		return result;
	}

private:
	std::vector<Bucket> buckets_;
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