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
        SparseSet(): sparsePages() {
            denseArray.reserve(Capacity);
            denseToEntityArray.reserve(Capacity);
        }

        bool contains(size_t id) override {
            if (id >= Capacity)
                return false;
            const auto index = getDenseIndex(id);
            return !denseArray.empty() && index != tombstone;
        }

        bool add(size_t id, const T& value) {
            if (id > Capacity || contains(id))
                return false;
            size_t index = getDenseIndex(id);
            if (index != tombstone) {
                denseArray[index] = value;
                denseToEntityArray[index] = id;
                return true;
            }

            setDenseIndex(id, denseArray.size());
            denseArray.push_back(value);
            denseToEntityArray.push_back(id);
            return true;
        }

        bool remove(size_t id) override {
            if (id > Capacity || !contains(id))
                return false;
            size_t index = getDenseIndex(id);

            setDenseIndex(denseToEntityArray.back(), index);
            setDenseIndex(id, tombstone);

            std::swap(denseArray[index], denseArray.back());
            std::swap(denseToEntityArray[index], denseToEntityArray.back());

            denseArray.pop_back();
            denseToEntityArray.pop_back();
            return true;
        }

        const T* get(size_t id) {
            ECS_ASSERT(contains(id), "Couldn't find id of " << id << " in sparse set of type " << typeid(T).name() << "!");
            size_t index = getDenseIndex(id);
            return denseArray[index];
        }

        const T& get(size_t id) const {
            ECS_ASSERT(contains(id), "Couldn't find id of " << id << " in sparse set of type " << typeid(T).name() << "!");
            size_t index = getDenseIndex(id);
            return denseArray[index];
        }

    private:
        size_t getDenseIndex(size_t id) {
            int page = id / Capacity;
            int index = page % Capacity;
            if (page < sparsePages.size() && index < Capacity)
                return sparsePages[page][index];
            return tombstone;
        }

        void setDenseIndex(size_t id, size_t index) {
            int page = id / Capacity;
            int pageIndex = page % Capacity;

            if (page >= sparsePages.size()) {
                sparsePages.resize(page + 1);
                sparsePages[page].fill(tombstone);
            }

            sparsePages[page][pageIndex] = index;
        }

        static constexpr size_t tombstone = std::numeric_limits<size_t>::max();
        using sparse = std::array<size_t, Capacity>;

        std::vector<sparse> sparsePages;
        std::vector<T> denseArray;
        std::vector<size_t> denseToEntityArray;
    };
}
