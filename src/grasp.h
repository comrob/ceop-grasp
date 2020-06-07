/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#ifndef __GRASP_H__
#define __GRASP_H__

#include <crl/config.h>
#include <crl/alg/algorithm.h>

#include <crl/logging.h>

#include <crl/gui/shape.h>

#include "target.h"
#include "path.h"

namespace grasp {

    class CGRASP : public crl::CAlgorithm {
        typedef crl::CAlgorithm Base;

    public:
        static crl::CConfig &getConfig(crl::CConfig &config);

        CGRASP(crl::CConfig &config, SDataset &ds);

        ~CGRASP();

        std::string getVersion(void);

        std::string getRevision(void);

        void solve(void);

    protected:
        void load(void);
        void initialize(void);
        void after_init(void);
        void iterate(int iter);
        void save(void);
        void release(void);

        void defineResultLog(void);
        void fillResultRecord(int trial);

    private:
        void drawPath(void); // the best path
        void drawPath(SPath &path);

        const bool SAVE_RESULTS;
        const bool SAVE_SETTINGS;
        const bool SAVE_INFO;
        const bool SAVE_PIC;
        
        const bool WAYPOINT_OPT;
        static constexpr float CL_RESTRICT = 0.2;
        static constexpr bool CHECK_FEASIBILITY = false;

        crl::gui::CShape shapeTargets;
        crl::gui::CShape shapePath;
        crl::gui::CShape shapePathNodes;
        crl::gui::CShape shapeRadius;

        SDataset dataset;
        SPath curPath;
        SPath bestPath;

        std::string method;

        bool addLocation(int blocked = -1);
        void localSearch();

        void restrict(double best);

        bool twoOpt(SPath &curPath);
        SPath twoOptSwap(SPath &tour, const int &i, const int &j);
    };
}


#endif //__GRASP_H__
