/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#include <iostream>
#ifndef __COORDS_H__
#define __COORDS_H__

#include <vector>
#include <math.h>

/// ----------------------------------------------------------------------------
/// @brief Coords
/// ----------------------------------------------------------------------------
struct Coords {
    double x;
    double y;

    Coords(Coords  &c) : x(c.x), y(c.y) {}
    Coords(const Coords  &c) : x(c.x), y(c.y) {}
    Coords() {}
    Coords(double x, double y) : x(x), y(y) {}
    Coords& operator=(const Coords &c) {
        if (this != &c) {
            x = c.x;
            y = c.y;
        }
        return *this;
    }

    double length() {
        return sqrt(1.0 * x * x + 1.0 * y * y);
    }

    inline double squared_distance(const Coords  &c) const {
        return squared_distance(*this, c);
    }

    inline static double squared_distance(const Coords  &c1, const Coords  &c2) {
        double dx = c1.x - c2.x;
        double dy = c1.y - c2.y;
        return dx*dx + dy*dy;
    }
};

Coords operator-(Coords a, Coords b);

Coords operator+(Coords a, Coords b);

Coords operator*(Coords a, double b);

typedef std::vector<Coords> CoordsVector;

std::istream& operator>>(std::istream &is, Coords &pt);

std::istream& operator>>(std::istream &is, CoordsVector &pts);

#endif

/* end of coords.h */
