#include "Point.h"
#include "Rectangle.h"
#include <vector>

using namespace std;

struct QuadTreeStats {
    int max_depth;
    int min_depth;
    float avg_points_per_leaf;
    float stddev_points_per_leaf;
    int total_leaves;
    int internal_nodes;
    int total_points;
    float q1_points_per_leaf;
    float q3_points_per_leaf;
};

class QuadTree{
public:
    Rectangle boundary;
    int capacity;
    vector<Point> points;
    bool divided;
    QuadTree* northwest;
    QuadTree* northeast;
    QuadTree* southwest;
    QuadTree* southeast;

    QuadTree(Rectangle boundary, int capacity);
    ~QuadTree();

    bool insert(const Point& point);
    void subdivide(); 
    void print_tree(int depth = 0, const std::string& quadrant = "ROOT") const;
    void save_structure(std::ofstream& out) const;
    QuadTreeStats collect_stats() const;
    vector<Point> range_query(const Rectangle& range_rect) const;
    vector<pair<Point, float>> knn_query(const Point& query, int k) const;
};

