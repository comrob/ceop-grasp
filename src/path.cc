/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#include "path.h"
#include "grasp.h"

#include <crl/logging.h>

using namespace crl;

namespace grasp {

    SPath& SPath::operator=(const SPath &p) {
        if (this != &p) {
            tv = p.tv;
            cv = p.cv;
        }
        return *this;
    }

    int SPath::size() {
        return tv.size();
    }

    void SPath::push_back(STarget *t, Coords &c) {
        tv.push_back(t);
        cv.push_back(c);
    }

    double SPath::length() {
        double len = 0;
        for (int i = 0; i < size() - 1; ++i) {
            len += sqrt(cv[i].squared_distance(cv[i + 1]));
        }
        return len;
    }

    double SPath::reward() {
        double rew = 0;
        for (auto target : tv) {
            rew += target->reward;
        }
        return rew;
    }

    SPath SPath::remove(int idx) {
        SPath new_path;
        for (int i = 0; i < tv.size(); ++i) {
            if (i < idx || idx < i) {
                new_path.push_back(tv[i], cv[i]);
            }
        }

        return new_path;
    }

    void SPath::clear() {
        tv.clear();
        cv.clear();
    }

    bool SPath::contains(STarget const *t) {
        if (tv.empty()) return false;

        bool ret = false;
        for (STarget *target : tv) {
            if (target->label == t->label) {
                ret = true;
                break;
            }
        }

        return ret;
    }

    Coords findWaypointLocation(Coords& p1, Coords& p2, STarget* w) {
        Coords x;
        double dist = (p1-p2).length();
        if (dist < 1e-10) {
            x = p1;
        } else {
            Coords dir = p2 - p1;
            dir = dir * (1 / dir.length());
            Coords dir2 = w->centre - p1;
            double dist_norm = dir.x * dir2.x + dir.y * dir2.y;
            dist_norm = fmin(dist, fmax(0.0, dist_norm));
            x = p1 + dir * dist_norm;
        }

        Coords diff = x - w->centre;
        double len = diff.length();
        if (len < w->radius){
            return x;
        } else  {
            double alpha = atan2(diff.y, diff.x);
            Coords ret(cos(alpha), sin(alpha));
            ret = ret *  w->radius +  w->centre;
            return ret;
        }
    }

    Coords SPath::getBestWaypoint(int idx1, int idx2, STarget* w) {
        return findWaypointLocation(cv[idx1], cv[idx2], w);
    }

    void SPath::optimize() {
        for (int n = 0; n < 3; ++n)  {
            for (int i = 1; i < size() - 1; ++i) {
                Coords c = findWaypointLocation(cv[i - 1], cv[i + 1], tv[i]);

                Coords &prev = cv[i-1];
                Coords &cur = cv[i];
                Coords &after = cv[i+1];
                double l1 = (prev - cur).length() + (after - cur).length();
                double l2 = (prev - c).length() + (after - c).length();
                if (l2 < l1) {
                    cv[i] = c;
                }
            }
        }
    }

}