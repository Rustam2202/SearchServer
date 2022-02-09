#include <algorithm>
#include <cstdlib>
//#include <future>
#include <map>
#include <mutex>
//#include <numeric>
//#include <string>
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
		size_t index = key % bucket_count_;
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

private:
	std::vector<std::map<Key, Value>> sub_maps_;
	std::vector<std::mutex> value_mutex_;
	size_t bucket_count_;
};

// Авторское решение
/*
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
*/