/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#include "coords.h"

Coords operator-(Coords a, Coords b) {
	return Coords(a.x - b.x, a.y - b.y);

}

Coords operator+(Coords a, Coords b) {
	return Coords(a.x + b.x, a.y + b.y);
}

Coords operator*(Coords a, double b) {
	return Coords(a.x * b, a.y * b);
}

std::istream &operator>>(std::istream &is, Coords &pt) {
    return is >> pt.x >> pt.y;
}

std::istream& operator>>(std::istream &is, CoordsVector &pts) {
    Coords c;
    while (is >> c) {
        pts.push_back(c);
    }
    return is;
}
