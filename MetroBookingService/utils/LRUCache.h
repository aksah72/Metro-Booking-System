#pragma once
#include <unordered_map>
#include <list>
using namespace std;

template<typename Key, typename Value>
class LRUCache {
private:
    int capacity;

    list<pair<Key, Value>> cacheList;

    unordered_map<
        Key,
        typename list<pair<Key, Value>>::iterator
    > cacheMap;

public:
    LRUCache(int cap) {
        capacity = cap;
    }

    bool exists(Key key) {
        return cacheMap.find(key) != cacheMap.end();
    }

    Value get(Key key) {
        auto it = cacheMap[key];

        cacheList.splice(
            cacheList.begin(),
            cacheList,
            it
        );

        return it->second;
    }

    void put(Key key, Value value) {

        if (exists(key)) {
            cacheList.erase(cacheMap[key]);
        }
        else if (cacheList.size() == capacity) {

            auto last = cacheList.back();
            cacheMap.erase(last.first);
            cacheList.pop_back();
        }

        cacheList.push_front({key, value});
        cacheMap[key] = cacheList.begin();
    }
};