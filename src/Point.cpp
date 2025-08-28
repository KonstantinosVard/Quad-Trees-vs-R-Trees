#include "Point.h"
#include "Rectangle.h"
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

Point::Point(int id, float x_cord, float y_cord) : id(id), x(x_cord), y(y_cord) {}

ostream& operator<<(ostream& os, const Point& point) {
    os << "Point(" << point.x << ", " << point.y << ")";
    return os;
}


float Point::distance_to_point(const Point& point) const {
    float dx = x - point.x;
    float dy = y - point.y;
    return sqrt(dx*dx + dy*dy);
}

float Point::distance_to_rectangle(const Rectangle& rect) const {
    float dx = max({rect.left - x,  0.0f , x - rect.right});
    float dy = max({rect.bottom - y, 0.0f, y - rect.top});
    return sqrt(dx * dx + dy * dy);
}