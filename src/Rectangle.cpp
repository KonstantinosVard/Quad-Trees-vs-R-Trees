#include "Rectangle.h"
#include <iostream>
#include <algorithm>

using namespace std;

Rectangle::Rectangle(float x_cord, float y_cord, float width, float height)
    : x(x_cord), y(y_cord), w(width), h(height),
      left(x_cord - width / 2),
      right(x_cord + width / 2),
      top(y_cord + height / 2),
      bottom(y_cord - height / 2) {}

ostream& operator<<(ostream& os, const Rectangle& rect) {
    os << "Rectangle(x=" << rect.x << ", y=" << rect.y
       << ", w=" << rect.w << ", h=" << rect.h << ")";
    return os;
}

bool Rectangle::contains(const Point& point) const {
    return left <= point.x &&
           point.x <= right &&
           bottom <= point.y &&
           point.y <= top;
}

bool Rectangle::intersects(const Rectangle& rect) const {
    return !(right < rect.left || rect.right < left ||
             top < rect.bottom || rect.top < bottom);
}

Rectangle Rectangle::union_with(const Rectangle& other) const {
    float new_left = min(left, other.left);
    float new_right = max(right, other.right);
    float new_bottom = min(bottom, other.bottom);
    float new_top = max(top, other.top);
    float new_x = (new_left + new_right) / 2;
    float new_y = (new_bottom + new_top) / 2;
    float new_w = new_right - new_left;
    float new_h = new_top - new_bottom;
    return Rectangle(new_x, new_y, new_w, new_h);
}

float Rectangle::area() const {
    return w * h;
}

Rectangle Rectangle::intersection(const Rectangle& other) const {
    if (!intersects(other)) {
        return Rectangle(0, 0, 0, 0); // empty intersection
    }

    float inter_left = max(left, other.left);
    float inter_right = min(right, other.right);
    float inter_bottom = max(bottom, other.bottom);
    float inter_top = min(top, other.top);

    float inter_w = inter_right - inter_left;
    float inter_h = inter_top - inter_bottom;
    float inter_x = (inter_left + inter_right) / 2;
    float inter_y = (inter_bottom + inter_top) / 2;

    return Rectangle(inter_x, inter_y, inter_w, inter_h);
}

// Assuming Rectangle is defined as in your implementation
float Rectangle::intersection_area(const Rectangle& other) const {
    // Compute rectangle bounds
    float x1_min = x - w / 2.0f;
    float x1_max = x + w / 2.0f;
    float y1_min = y - h / 2.0f;
    float y1_max = y + h / 2.0f;

    float x2_min = other.x - other.w / 2.0f;
    float x2_max = other.x + other.w / 2.0f;
    float y2_min = other.y - other.h / 2.0f;
    float y2_max = other.y + other.h / 2.0f;

    // Find intersection bounds
    float x_inter_min = max(x1_min, x2_min);
    float x_inter_max = min(x1_max, x2_max);
    float y_inter_min = max(y1_min, y2_min);
    float y_inter_max = min(y1_max, y2_max);

    // Check if there is an intersection
    if (x_inter_max <= x_inter_min || y_inter_max <= y_inter_min) {
        return 0.0f;
    }

    // Compute intersection area
    float inter_width = x_inter_max - x_inter_min;
    float inter_height = y_inter_max - y_inter_min;
    return inter_width * inter_height;
}