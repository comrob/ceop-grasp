/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#ifndef __PATH_H__
#define __PATH_H__

#include "coords.h"
#include "target.h"

#include <vector>
#include <math.h>
#include <functional>

namespace grasp {

    struct SPath {
        STargetVector tv;
        CoordsVector cv;

        SPath& operator=(const SPath &p);

        int size();

        void push_back(STarget *t, Coords &c);

        double length();

        double reward();

        SPath remove(int idx);

        void clear();

        bool contains(STarget const *t);

        Coords getBestWaypoint(int idx1, int idx2, STarget* w);

        void optimize();
    };

}

#endif //__PATH_H__