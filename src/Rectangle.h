#pragma once
#include "Point.h"
#include <iostream>

using namespace std;

class Rectangle {
public:
    float x, y, w, h;
    float left, right, top, bottom;

    explicit Rectangle(float x_cord, float y_cord, float width, float height);

    bool contains(const Point& point) const;
    bool intersects(const Rectangle& rect) const;
    float area() const;

    Rectangle intersection(const Rectangle& other) const;
    Rectangle union_with(const Rectangle& other) const;
    float intersection_area(const Rectangle& other) const;
};

ostream& operator<<(ostream& os, const Rectangle& rect);