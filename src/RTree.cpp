#include "RTree.h"
#include "HeapEntry.h"
#include <algorithm>
#include <queue>
#include <iostream>
#include <limits>
#include <cstdint>
#include "libmorton/include/libmorton/morton.h"
#include <iomanip>
#include <map>
#include <cmath>
#include <set>
#include <tuple>
#include <vector>
#include <functional>
#include <numeric>

using namespace std;

RTree::RTree(Rectangle boundary, int min_entries, int max_entries)
    : boundary(boundary), min_entries(min_entries), max_entries(max_entries), is_leaf(true) {}

RTree::~RTree() {
    for (RTree* child : children) {
        delete child;
    }
}

Rectangle compute_boundary(const vector<Point>& points) {
    if (points.empty()) return Rectangle(0, 0, 0, 0);
    float min_x = points[0].x, max_x = points[0].x;
    float min_y = points[0].y, max_y = points[0].y;
    for (const Point& p : points) {
        min_x = min(min_x, p.x);
        max_x = max(max_x, p.x);
        min_y = min(min_y, p.y);
        max_y = max(max_y, p.y);
    }
    return Rectangle((min_x + max_x) / 2, (min_y + max_y) / 2, max_x - min_x, max_y - min_y);
}

Rectangle compute_boundary(const vector<RTree*>& nodes) {
    if (nodes.empty()) return Rectangle(0, 0, 0, 0);
    Rectangle rect = nodes[0]->boundary;
    for (size_t i = 1; i < nodes.size(); ++i) {
        rect = rect.union_with(nodes[i]->boundary);
    }
    return rect;
}

uint64_t interleave_bits(uint32_t x, uint32_t y) {
    uint64_t z = 0;
    for (int i = 0; i < 32; ++i) {
        z |= ((uint64_t)(y >> i) & 1) << (2 * i + 1);
        z |= ((uint64_t)(x >> i) & 1) << (2 * i);
    }
    return z;
}

// Sorting function
variant<vector<Point>, vector<vector<Point>>> RTree::sort_points(const vector<Point>& points, float min_x, float max_x, float min_y, float max_y, SortMethod method) const {
    if (points.empty()) {
        return vector<Point>{};
    }

    if (method == SortMethod::Z_ORDER) {
        // Z-order sorting
        auto z_order = [min_x, max_x, min_y, max_y](const Point& p) {
            float x_norm = (p.x - min_x) / (max_x - min_x);
            float y_norm = (p.y - min_y) / (max_y - min_y);
            uint32_t x_int = static_cast<uint32_t>(x_norm * (1ULL << 32));
            uint32_t y_int = static_cast<uint32_t>(y_norm * (1ULL << 32));
            return interleave_bits(x_int, y_int);
        };

        vector<Point> sorted_points = points;
        sort(sorted_points.begin(), sorted_points.end(), [z_order](const Point& a, const Point& b) {
            return z_order(a) < z_order(b);
        });

        return sorted_points;
    } else { // STR
        // Step 1: Sort points by x-coordinate
        vector<Point> sorted_points = points;
        sort(sorted_points.begin(), sorted_points.end(), [](const Point& a, const Point& b) {
            return a.x < b.x;
        });

        // Step 2: Divide into vertical strips
        size_t n = sorted_points.size();
        size_t S = static_cast<size_t>(ceil(sqrt(static_cast<double>(n) / max_entries)));
        if (S == 0) S = 1; // Avoid division by zero
        size_t points_per_strip = (n + S - 1) / S; // Ceiling division

        // Step 3: Create strips and sort by y-coordinate
        vector<vector<Point>> strips;
        for (size_t i = 0; i < n; i += points_per_strip) {
            size_t strip_end = min(i + points_per_strip, n);
            vector<Point> strip(sorted_points.begin() + i, sorted_points.begin() + strip_end);
            sort(strip.begin(), strip.end(), [](const Point& a, const Point& b) {
                return a.y < b.y;
            });
            strips.push_back(strip);
        }

        return strips;
    }
}

// Insertion function for sorted points
void RTree::insert_sorted(const variant<vector<Point>, vector<vector<Point>>>& sorted_data) {
    // Build leaf nodes
    vector<RTree*> leaves;

    if (holds_alternative<vector<Point>>(sorted_data)) {
        // Z-order: treat as one strip
        const auto& sorted_points = get<vector<Point>>(sorted_data);
        size_t n = sorted_points.size();
        for (size_t i = 0; i < n; i += max_entries) {
            size_t leaf_end = min(i + max_entries, n);
            vector<Point> leaf_points(sorted_points.begin() + i, sorted_points.begin() + leaf_end);
            RTree* leaf = new RTree(compute_boundary(leaf_points), min_entries, max_entries);
            leaf->points = leaf_points;
            leaf->is_leaf = true;
            leaves.push_back(leaf);
        }
    } else {
        // STR: process each strip
        const auto& strips = get<vector<vector<Point>>>(sorted_data);
        for (const auto& strip : strips) {
            for (size_t j = 0; j < strip.size(); j += max_entries) {
                size_t leaf_end = min(j + max_entries, strip.size());
                vector<Point> leaf_points(strip.begin() + j, strip.begin() + leaf_end);
                RTree* leaf = new RTree(compute_boundary(leaf_points), min_entries, max_entries);
                leaf->points = leaf_points;
                leaf->is_leaf = true;
                leaves.push_back(leaf);
            }
        }
    }

    // Adjust last leaf if necessary
    if (leaves.size() >= 2) {
        RTree* last_leaf = leaves.back();
        RTree* prev_leaf = leaves[leaves.size() - 2];
        size_t r = last_leaf->points.size();
        if (r > 0 && r < static_cast<size_t>(min_entries) && prev_leaf->points.size() > static_cast<size_t>(min_entries)) {
            size_t needed = min_entries - r;
            size_t to_take = min(needed, prev_leaf->points.size() - min_entries);
            for (size_t j = 0; j < to_take; ++j) {
                last_leaf->points.insert(last_leaf->points.begin(), prev_leaf->points.back());
                prev_leaf->points.pop_back();
            }
            last_leaf->boundary = compute_boundary(last_leaf->points);
            prev_leaf->boundary = compute_boundary(prev_leaf->points);
        }
    }

    // Build upper levels
    vector<RTree*> current_level = leaves;
    while (current_level.size() > 1) {
        vector<RTree*> next_level;
        size_t m = current_level.size();
        size_t j = 0;
        while (j < m) {
            size_t remaining = m - j;
            size_t node_size = min(static_cast<size_t>(max_entries), remaining);
            vector<RTree*> node_children(current_level.begin() + j, current_level.begin() + j + node_size);
            RTree* parent = new RTree(compute_boundary(node_children), min_entries, max_entries);
            parent->children = node_children;
            parent->is_leaf = false;
            next_level.push_back(parent);
            j += node_size;
        }
        // Adjust last node if necessary
        if (next_level.size() >= 2) {
            RTree* last_node = next_level.back();
            RTree* prev_node = next_level[next_level.size() - 2];
            size_t r = last_node->children.size();
            if (r > 0 && r < static_cast<size_t>(min_entries) && prev_node->children.size() > static_cast<size_t>(min_entries)) {
                size_t needed = min_entries - r;
                size_t to_take = min(needed, prev_node->children.size() - min_entries);
                for (size_t k = 0; k < to_take; ++k) {
                    last_node->children.insert(last_node->children.begin(), prev_node->children.back());
                    prev_node->children.pop_back();
                }
                last_node->boundary = compute_boundary(last_node->children);
                prev_node->boundary = compute_boundary(prev_node->children);
            }
        }
        current_level = next_level;
    }

    // Set this node as the root
    if (!current_level.empty()) {
        RTree* root = current_level[0];
        this->boundary = root->boundary;
        this->children = move(root->children);
        this->points = move(root->points);
        this->is_leaf = root->is_leaf;
        delete root;
    }
}

// Modified insert function
void RTree::insert(const vector<Point>& points, SortMethod method) {
    if (points.empty()) return;

    // Compute dataset bounds
    float min_x = points[0].x, max_x = points[0].x;
    float min_y = points[0].y, max_y = points[0].y;
    for (const auto& p : points) {
        min_x = min(min_x, p.x);
        max_x = max(max_x, p.x);
        min_y = min(min_y, p.y);
        max_y = max(max_y, p.y);
    }

    // Clear existing data
    for (RTree* child : children) delete child;
    children.clear();
    this->points.clear();
    is_leaf = true;

    // Sort points
    auto sorted_data = sort_points(points, min_x, max_x, min_y, max_y, method);

    // Insert sorted data
    insert_sorted(sorted_data);
}

vector<Point> RTree::range_query(const Rectangle& range_rect) const {
    vector<Point> found;
    
    if (!boundary.intersects(range_rect))
        return found;

    if (is_leaf) {
        for (const Point& p : points) {
            if (range_rect.contains(p)) {
                found.push_back(p);
            }
        }
    } 
    else {
        for (RTree* child : children) {
            vector<Point> subfound = child->range_query(range_rect);
            found.insert(found.end(), subfound.begin(), subfound.end());
        }
    }
    
    return found;
}

vector<pair<Point, float>> RTree::knn_query(const Point& query, int k) const {
    priority_queue<HeapEntry<RTree>> heap;
    vector<pair<Point, float>> results;
    int counter = 0;

    heap.emplace(query.distance_to_rectangle(boundary), counter++, this);

    while (!heap.empty() && results.size() < k) {
        HeapEntry<RTree> entry = heap.top();
        heap.pop();

        if (holds_alternative<Point>(entry.data)) {
            Point p = get<Point>(entry.data);
            results.emplace_back(p, entry.dist);
        } 
        else {
            const RTree* node = get<const RTree*>(entry.data);

            if (node->is_leaf) {
                for (const Point& p : node->points) {
                    float point_dist = query.distance_to_point(p);
                    heap.emplace(point_dist, counter++, p);
                }
            } 
            else {
                for (RTree* child : node->children) {
                    float child_dist = query.distance_to_rectangle(child->boundary);
                    heap.emplace(child_dist, counter++, child);
                }
            }
        }
    }

    return results;
}

void RTree::print_tree(int depth) const {
    string indent(depth * 2, ' '); // 2 spaces per level
    cout << indent << "Node (is_leaf: " << is_leaf << ", boundary: ["
              << boundary.x << ", " << boundary.y << ", "
              << boundary.w << ", " << boundary.h << "])\n";

    if (is_leaf) {
        for (const Point& p : points) {
            cout << indent << "  Point: (" << p.x << ", " << p.y << ")\n";
        }
    } else {
        int i = 0;
        for (RTree* child : children) {
            cout << indent << "  -> Child " << i++ << ":\n";
            child->print_tree(depth + 1);
        }
    }
}

int RTree::get_depth() const {
    if (is_leaf) return 1;
    return 1 + children[0]->get_depth();
}
float RTree::get_avg_occupancy() const {
    vector<int> occupancies;
    function<void(const RTree*)> collect = [&](const RTree* node) {
        if (node->is_leaf) {
            occupancies.push_back(node->points.size());
        } else {
            occupancies.push_back(node->children.size());
            for (RTree* child : node->children) {
                collect(child);
            }
        }
    };
    collect(this);
    return accumulate(occupancies.begin(), occupancies.end(), 0.0f) / occupancies.size();
}

vector<float> RTree::get_avg_overlap_per_level() const {
    vector<float> overlaps_per_level;
    if (is_leaf && points.empty()) {
        return overlaps_per_level; // Empty tree
    }

    // Use a queue for level-order traversal
    queue<pair<const RTree*, int>> q;
    vector<vector<const RTree*>> nodes_by_level;

    // Start with the root at level 0
    q.emplace(this, 0);
    while (!q.empty()) {
        auto [node, level] = q.front();
        q.pop();

        // Ensure nodes_by_level has enough space
        if (level >= nodes_by_level.size()) {
            nodes_by_level.resize(level + 1);
        }
        nodes_by_level[level].push_back(node);

        // Add children to the queue
        if (!node->is_leaf) {
            for (const RTree* child : node->children) {
                q.emplace(child, level + 1);
            }
        }
    }

    // Compute average overlap for each level
    for (const auto& level_nodes : nodes_by_level) {
        // Group nodes by parent to only compute overlaps between siblings
        map<const RTree*, vector<const RTree*>> parent_to_nodes;
        for (const RTree* node : level_nodes) {
            // Find parent (or null for root)
            const RTree* parent = nullptr;
            for (const auto& level_above : nodes_by_level) {
                for (const RTree* candidate : level_above) {
                    if (!candidate->is_leaf && find(candidate->children.begin(), candidate->children.end(), node) != candidate->children.end()) {
                        parent = candidate;
                        break;
                    }
                }
                if (parent) break;
            }
            parent_to_nodes[parent].push_back(node);
        }

        // Compute total overlap for this level
        float total_overlap = 0.0f;
        size_t node_count = level_nodes.size();
        size_t pair_count = 0;

        for (const auto& [parent, siblings] : parent_to_nodes) {
            // Compute pairwise overlaps for siblings
            for (size_t i = 0; i < siblings.size(); ++i) {
                for (size_t j = i + 1; j < siblings.size(); ++j) {
                    total_overlap += siblings[i]->boundary.intersection_area(siblings[j]->boundary);
                    pair_count++;
                }
            }
        }

        // Compute average overlap per node
        float avg_overlap = (node_count > 0 && pair_count > 0) ? total_overlap / node_count : 0.0f;
        overlaps_per_level.push_back(avg_overlap);
    }

    return overlaps_per_level;
}