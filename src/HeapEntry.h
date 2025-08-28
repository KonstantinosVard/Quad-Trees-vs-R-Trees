#pragma once
#include <variant>
#include <tuple>
#include "Point.h"

using namespace std;

template <typename NodeType>
struct HeapEntry {
    float dist;
    int counter;
    variant<Point, const NodeType*> data;

    HeapEntry(float d, int c, const NodeType* n) : dist(d), counter(c), data(n) {}
    HeapEntry(float d, int c, const Point& p) : dist(d), counter(c), data(p) {}

    bool operator<(const HeapEntry& other) const {
        return tie(dist, counter) > tie(other.dist, other.counter);
    }
};