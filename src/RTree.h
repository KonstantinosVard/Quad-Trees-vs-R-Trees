#pragma once
#include "Point.h"
#include "Rectangle.h"
#include <vector>
#include <variant>

using namespace std;

enum class SortMethod {Z_ORDER, STR};

class RTree {
public:
    Rectangle boundary;
    int min_entries;   
    int max_entries;
    vector<Point> points;
    bool is_leaf;
    vector<RTree*> children;

    RTree(Rectangle boundary, int min_entries, int max_entries);
    ~RTree();

    // void insert(const vector<Point>& points);
    void insert(const vector<Point>& points, SortMethod method);
    void print_tree(int depth = 0) const;
    int get_depth() const;
    float get_avg_occupancy() const;
    vector<Point> range_query(const Rectangle& range_rect) const;
    vector<pair<Point, float>> knn_query(const Point& query, int k) const;
    vector<float> get_avg_overlap_per_level() const;
    variant<vector<Point>, vector<vector<Point>>> sort_points(const vector<Point>& points, float min_x, float max_x, float min_y, float max_y, SortMethod method) const;
        void insert_sorted(const variant<vector<Point>, vector<vector<Point>>>& sorted_data);


};