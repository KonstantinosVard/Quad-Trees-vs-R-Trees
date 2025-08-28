#pragma once

#include <iostream>
using namespace std;

class Rectangle;

class Point{
public:
    int id;
    float x, y;

    explicit Point(int id, float x_cord, float y_cord);

    float distance_to_point(const Point& point) const;
    float distance_to_rectangle(const Rectangle& rect) const;
};

ostream& operator<<(ostream& os, const Point& point);