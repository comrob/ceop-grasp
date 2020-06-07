/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#include "target.h"
#include <math.h>

using namespace grasp;

namespace grasp {

    SDataset loadDataset(std::string &filename, double &budget) {
        SDataset loadedDataset;
        loadedDataset.startID = 0;
        loadedDataset.endID = 1;
        std::ifstream in(filename.c_str(), std::ifstream::in);

        if (!in) {
            std::cerr << "Cannot open " << filename << std::endl;
        } else {
            loadedDataset.Tmax = budget;
            in >> loadedDataset.targets;
        }

        return loadedDataset;
    }
}

std::istream &operator>>(std::istream &is, STargetVector &tv) {
    int label;
    Coords centre;
    double reward;
    double radius;
    while (is >> label >> centre >> reward >> radius) {
        tv.push_back(new STarget(label, centre, reward, radius));
    }
    return is;
}
