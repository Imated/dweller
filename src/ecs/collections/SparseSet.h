#pragma once
#include <array>
#include <assert.h>
#include <iostream>
#include <vector>
#include <ostream>
#include <limits>
#include <typeinfo>

namespace ecs::collections {
#define ECS_ASSERT(condition, msg) \
    if (!condition) { \
        std::cerr << "[ECS ERROR]: " << msg << std::endl; \
        assert(false); \
    }

    class ISpareSet {
        public:
        virtual ~ISpareSet() = default;
        virtual bool contains(size_t id) = 0;
        virtual bool remove(size_t id) = 0;
    };

    template<typename T, size_t Capacity = 1024>
    class SparseSet : ISpareSet {
    public:
        SparseSet(): sparseArray() {
            sparseArray.fill(tombstone);
            denseArray.reserve(Capacity);
            denseToEntityArray.reserve(Capacity);
        }

        bool contains(size_t id) override {
            if (id >= Capacity)
                return false;
            auto index = sparseArray[id];
            return index != tombstone && index < denseArray.size() && denseToEntityArray[index] == id;
        }

        bool add(size_t id, const T& value) {
            if (id > Capacity || contains(id))
                return false;
            sparseArray[id] = denseArray.size();
            denseArray.push_back(value);
            denseToEntityArray.push_back(id);
            return true;
        }

        bool remove(size_t id) override {
            if (id > Capacity || !contains(id))
                return false;
            size_t index = sparseArray[id];
            size_t lastIndex = denseArray.size() - 1;
            std::swap(denseArray[index], denseArray[lastIndex]);
            std::swap(denseToEntityArray[index], denseToEntityArray[lastIndex]);
            sparseArray[denseToEntityArray[index]] = index;
            denseArray.pop_back();
            denseToEntityArray.pop_back();
            sparseArray[id] = tombstone;
            return true;
        }

        const T* get(size_t id) {
            ECS_ASSERT(contains(id), "Couldn't find id of " << id << " in sparse set of type " << typeid(T).name() << "!");
            return denseArray[sparseArray[id]];
        }

        const T& get(size_t id) const {
            ECS_ASSERT(contains(id), "Couldn't find id of " << id << " in sparse set of type " << typeid(T).name() << "!");
            return denseArray[sparseArray[id]];
        }

    private:
        static constexpr size_t tombstone = std::numeric_limits<size_t>::max();;
        using sparse = std::array<size_t, Capacity>;

        sparse sparseArray;
        std::vector<T> denseArray;
        std::vector<size_t> denseToEntityArray;
    };
}
