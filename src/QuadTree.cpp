#include "QuadTree.h"
#include "HeapEntry.h"
#include <queue>
#include <fstream>
#include <numeric>
#include <cmath>   
#include <functional>
#include <algorithm>


using namespace std;


QuadTree::QuadTree(Rectangle boundary, int capacity)
    : boundary(boundary), capacity(capacity), divided(false),
      northwest(nullptr),  northeast(nullptr), southwest(nullptr), southeast(nullptr) {}


QuadTree::~QuadTree() {
    delete northwest;
    delete northeast;
    delete southwest;
    delete southeast;
}


void QuadTree::subdivide() {
    float x = boundary.x;
    float y = boundary.y;
    float w = boundary.w/2;
    float h = boundary.h/2;

    Rectangle nw(x - w / 2, y + h / 2, w, h);
    Rectangle ne(x + w / 2, y + h / 2, w, h);
    Rectangle sw(x - w / 2, y - h / 2, w, h);
    Rectangle se(x + w / 2, y - h / 2, w, h);

    northwest = new QuadTree(nw, capacity);
    northeast = new QuadTree(ne, capacity);
    southwest = new QuadTree(sw, capacity);
    southeast = new QuadTree(se, capacity);

    divided = true;
}


bool QuadTree::insert(const Point& point) {
    if (!boundary.contains(point))
        return false;

    if (!divided && points.size() < capacity) {
        for (const Point& p : points) {
            if (p.id == point.id) { 
                return false;
            }
        }
        points.push_back(point);
        return true;
    }

    if (!divided) {
        subdivide();

        for (const Point& p : points) {
            for (QuadTree* quadrant : {northeast, northwest, southeast, southwest}) {
                if (quadrant->insert(p)) {
                    break;
                }
            }
        }
        points.clear();
    }

    for (QuadTree* quadrant : {northeast, northwest, southeast, southwest}) {
        if (quadrant->insert(point)) {
            return true;
        }
    }

    return false;
}

vector<Point> QuadTree::range_query(const Rectangle& range_rect) const {
    vector<Point> found;

    if (!boundary.intersects(range_rect))
        return found;

    if (divided) {
        for (QuadTree* quadrant : {northwest, northeast, southwest, southeast}) {
            vector<Point> subfound = quadrant->range_query(range_rect);
            found.insert(found.end(), subfound.begin(), subfound.end());
        }
    }

    else {
        for (const Point& p: points){
            if (range_rect.contains(p)) {
                found.push_back(p);
            }
        }
    }
    return found;
}


vector<pair<Point, float>> QuadTree::knn_query(const Point& query, int k) const {
    priority_queue<HeapEntry<QuadTree>> heap;
    vector<pair<Point, float>> results;
    int counter = 0;

    heap.emplace(query.distance_to_rectangle(boundary), counter++, this);

    while (!heap.empty() && results.size() < k) {
        HeapEntry<QuadTree> entry = heap.top();
        heap.pop();

        if (holds_alternative<Point>(entry.data)) {
            Point p = get<Point>(entry.data);
            results.emplace_back(p, entry.dist);
        } 
        else {
            const QuadTree* node = get<const QuadTree*>(entry.data);
            
            if (!node->divided) {
                for (const Point& p : node->points) {
                    float dist = query.distance_to_point(p);
                    heap.emplace(dist, counter++, p);
                }
            } 
            else {
                for (QuadTree* child : {node->northwest, node->northeast, node->southwest, node->southeast}) {
                    if (child) {
                        float dist = query.distance_to_rectangle(child->boundary);
                        heap.emplace(dist, counter++, child);
                    }
                }
            }
        }
    }

    return results;
}


void QuadTree::print_tree(int depth, const std::string& quadrant) const {
    string indent(depth * 2, ' '); // 2 spaces per level

    cout << indent << "Node [" << quadrant << "] (divided: " << divided
              << ", boundary: [" << boundary.x << ", " << boundary.y
              << ", " << boundary.w << ", " << boundary.h << "])\n";

    if (!divided) {
        for (const Point& p : points) {
            cout << indent << "  Point: (" << p.x << ", " << p.y << ")\n";
        }
    } else {
        if (northwest) northwest->print_tree(depth + 1, "NW");
        if (northeast) northeast->print_tree(depth + 1, "NE");
        if (southwest) southwest->print_tree(depth + 1, "SW");
        if (southeast) southeast->print_tree(depth + 1, "SE");
    }
}

void QuadTree::save_structure(std::ofstream& out) const {
    // Output this node's boundary rectangle
    out << boundary.x << "," << boundary.y << "," << boundary.w << "," << boundary.h << "\n";

    if (divided) {
        if (northwest) northwest->save_structure(out);
        if (northeast) northeast->save_structure(out);
        if (southwest) southwest->save_structure(out);
        if (southeast) southeast->save_structure(out);
    }
}


QuadTreeStats QuadTree::collect_stats() const {
    int min_depth = INT_MAX;
    int max_depth = 0;
    vector<int> leaf_sizes;
    int internal_nodes = 0;

    function<void(const QuadTree*, int)> dfs = [&](const QuadTree* node, int depth) {
        if (!node->divided) {
            min_depth = min(min_depth, depth);
            max_depth = max(max_depth, depth);
            leaf_sizes.push_back(node->points.size());
        } else {
            internal_nodes++;
            for (QuadTree* child : {node->northwest, node->northeast, node->southwest, node->southeast}) {
                if (child) dfs(child, depth + 1);
            }
        }
    };

    dfs(this, 0);

    int total_leaves = leaf_sizes.size();
    int total_points = accumulate(leaf_sizes.begin(), leaf_sizes.end(), 0);
    float avg_points = total_leaves > 0 ? static_cast<float>(total_points) / total_leaves : 0.0f;

    float sum_sq_diff = 0.0f;
    for (int n : leaf_sizes) {
        float diff = n - avg_points;
        sum_sq_diff += diff * diff;
    }
    float stddev = total_leaves > 1 ? sqrt(sum_sq_diff / (total_leaves - 1)) : 0.0f;

    sort(leaf_sizes.begin(), leaf_sizes.end());
    float q1 = 0.0f, q3 = 0.0f;
    if (!leaf_sizes.empty()) {
        int mid = total_leaves / 2;
        q1 = leaf_sizes[mid / 2];
        q3 = leaf_sizes[(mid + total_leaves) / 2];
    }

    return QuadTreeStats{max_depth, min_depth, avg_points, stddev,
                         total_leaves, internal_nodes, total_points, q1, q3};
}