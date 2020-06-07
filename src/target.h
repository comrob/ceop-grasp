/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#ifndef __TARGET_H__
#define __TARGET_H__

#include "coords.h"

#include <fstream>

namespace grasp {

    struct STarget {
        const int label;
        const Coords centre;
        const double reward;
        const double radius;

        STarget(const int id, const Coords &pt, double rw, double r) : 
            label(id), centre(pt), reward(rw), radius(r) {}
    };

    typedef std::vector<STarget *> STargetVector;

    struct SDataset {
        STargetVector targets;
        double Tmax;
        unsigned int startID;
        unsigned int endID;
    };

    SDataset loadDataset(std::string &filename, double &budget);

}

std::istream &operator>>(std::istream &is, grasp::STargetVector &tv);

#endif //__TARGET_H__
