/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#include <boost/foreach.hpp>

#include <crl/random.h>
#include <crl/logging.h>
#include <crl/assert.h>
#include <crl/file_utils.h>

#include <crl/gui/shape.h>
#include <crl/gui/shapes.h>
#include <crl/gui/colormap.h>
#include "canvasview_coords.h"

#include <math.h>
#include <cstdio>
#include <limits>

#include "grasp.h"


using namespace grasp;

using namespace crl;
using namespace crl::gui;


bool isFeasible(SPath& p, STargetVector & targets, double Tmax) {
    if (p.length() > Tmax) return false;

    bool used[targets.size()];
    for(auto&a:used) a = false;
    for (int i = 0; i < p.size(); ++i) {
        if (used[p.tv[i]->label]) {
            DEBUG("USED TWICE " << p.tv[i]->label);
            return false;
        } else if ((p.tv[i]->centre - p.cv[i]).length() > p.tv[i]->radius + 1e-10) {
            DEBUG("t:" + crl::string_format<int>(p.tv[i]->label) + " " + crl::string_format<int>(i) 
                + " diff:" + crl::string_format<double>((p.tv[i]->centre - p.cv[i]).length()));
            return false;
        } else {
            used[p.tv[i]->label] = true;
        }
    }
    return true;
}

/// - protected method ---------------------------------------------------------
void CGRASP::iterate(int iter) {
    curPath.clear();
    bestPath.clear();
    // initialization of the solution vector
    Coords initLocation = dataset.targets[dataset.startID]->centre;
    Coords finalLocation = dataset.targets[dataset.endID]->centre;
    
    if (dataset.Tmax < 
        (initLocation - finalLocation).length()) {
            drawPath();
            fillResultRecord(iter);
            return;
    }

    curPath.push_back(dataset.targets[dataset.startID], initLocation);
    curPath.push_back(dataset.targets[dataset.endID], finalLocation);

    bestPath = curPath;
    drawPath();

    // construction phase
    while (addLocation()) {
        drawPath(curPath);
    }

    bestPath = curPath;

    // local search phase
    localSearch();

    drawPath();
    
    // feasibility check
    if (CHECK_FEASIBILITY && !isFeasible(bestPath, dataset.targets, dataset.Tmax)){
        ERROR("IS NOT FEASIBLE!");
        exit(1);
    }

    fillResultRecord(iter);
}

/// - help structure of difference to currPath
struct SDiffPath {
    SPath* path;
    STarget* target;
    Coords coords;
    int i;
    // Removed part [from, to]
    int from = -1;
    int to = -1;
    
    double len, rew;

    SDiffPath(SPath &curPath, double length, double reward){
        path = &curPath;
        this->len = length;
        this->rew = reward;
    }

    void getLenRewAfterAdding() {
        len -= (path->cv[i-1] - path->cv[i]).length();
        len += (path->cv[i-1] - coords).length();
        len += (coords - path->cv[i]).length();
        rew +=  target->reward;
    }

    void removeFirst(int a) {
        from = to = a;
        if (from == i - 1) { // Removed part starts just before inserted point (i)
            len -= (path->cv[from-1] - path->cv[from]).length();
            len -= (path->cv[from] - coords).length();
            len += (path->cv[from-1] - coords).length();
        }else if (to == i) { // Removed part starts just after inserted point (i)
            len -= (coords - path->cv[from]).length();
            len -= (path->cv[from] - path->cv[from+1]).length();
            len += (coords - path->cv[to+1]).length();
        } else { // General case
            len -= (path->cv[from-1] - path->cv[from]).length();
            len -= (path->cv[from] - path->cv[from+1]).length();
            len += (path->cv[from-1] - path->cv[from+1]).length();
        }
        rew -= path->tv[from]->reward;
    }

    void removeNext() {
        to++;
        if (to == i - 1) { // Removed part ends just before inserted point (i)
            len -= (path->cv[from-1] - path->cv[to]).length();
            len -= (path->cv[to] - coords).length();
            len += (path->cv[from-1] - coords).length();
        } else if (to == i) { // Removed part ends just after the inserted point (i)
            len -= (coords - path->cv[to]).length();
            len -= (path->cv[to] - path->cv[to+1]).length();
            len += (coords - path->cv[to+1]).length();
        } else if (to > i && i > from ) { // Interted point (i) is in the removed part
            len -= (coords - path->cv[to]).length();
            len -= (path->cv[to] - path->cv[to+1]).length();
            len += (coords - path->cv[to+1]).length();
        } else if (from == i) { // Removed part starts just after inserted point (i)
            len -= (coords - path->cv[to]).length();
            len -= (path->cv[to] - path->cv[to+1]).length();
            len += (coords - path->cv[to+1]).length();
        } else { // General case
            len -= (path->cv[from-1] - path->cv[to]).length();
            len -= (path->cv[to] - path->cv[to+1]).length();
            len += (path->cv[from-1] - path->cv[to+1]).length();
        }
        rew -= path->tv[to]->reward;
    }

    void addAtBegining() {
        //INFO("ADD " << from << " " << to)
        if (from == i - 1) { // Removed part starts just before inserted point (i)
            len -= (path->cv[from-1] - coords).length();
            len += (path->cv[from-1] - path->cv[from]).length();
            len += (path->cv[from] - coords).length();
        } else if (to == i - 1) { // Removed part ends just before inserted point (i)
            len -= (path->cv[from-1] - coords).length();
            len += (path->cv[from-1] - path->cv[from]).length();
            len += (path->cv[from] - coords).length();
        } else if (from == i) { // Removed part starts just after inserted point (i)
            len -= (coords - path->cv[to+1]).length();
            len += (coords - path->cv[from]).length();
            len += (path->cv[from] - path->cv[to+1]).length();
        } else if (from < i && i <= to) { // Interted point (i) is in the removed part
            len -= (path->cv[from-1] - coords).length();
            len += (path->cv[from-1] - path->cv[from]).length();
            len += (path->cv[from] - coords).length();
        } else { // General case
            len -= (path->cv[from-1] - path->cv[to+1]).length();
            len += (path->cv[from-1] - path->cv[from]).length();
            len += (path->cv[from] - path->cv[to+1]).length();
        }
        rew += path->tv[from]->reward;
        from++;
    }
};

/// from the help structure SDiffPath create SPath
SPath createPath(SDiffPath &dp, SPath& curPath) {
    SPath new_path;
    int addedIdx = -1;
    int idx = -1;
    for (int i = 0; i < curPath.size(); i++) {
        if (i == dp.i) {
            idx++;
            addedIdx = idx;
            new_path.push_back(dp.target, dp.coords);
        }
        if (dp.from == -1 || i < dp.from || i > dp.to) {
            idx++;
            new_path.push_back(curPath.tv[i], curPath.cv[i]);
        }
    }

    return new_path;
}

/// - updating SDiffPath 'stour' by adding target 'w'
void find_best_position(SPath &p, STarget* w, SDiffPath &stour) {
    int label = -1;
    double minLen = std::numeric_limits<double>::max();

    double length = 0;
    Coords coords;
    
    for (int i = 1; i < p.size(); ++i) {
        double org_len = (p.cv[i-1] - p.cv[i]).length();
        // lower bounds
        double lb_prolong = - org_len
                            + (p.cv[i-1] - w->centre).length() 
                            + (w->centre - p.cv[i]).length() 
                            - 2 * w->radius;
        if (lb_prolong > minLen) continue;

        coords = p.getBestWaypoint(i-1, i, w);

        length = - org_len
                    + sqrt(p.cv[i-1].squared_distance(coords))
                    + sqrt(p.cv[i].squared_distance(coords));

        if (length < minLen - 1e-10) {
            label = i;
            minLen = length;
            stour.coords = coords;
        }
    }
    
    stour.target = w;
    stour.i = label;
    stour.getLenRewAfterAdding();
}

std::vector<SDiffPath> CL;

/// - private method -----------------------------------------------------------
bool CGRASP::addLocation(int blocked) {
    double length0 = curPath.length();
    double reward0 = curPath.reward();
    double best_rew = reward0;

    int l = curPath.size(); // number of targets in a path
    CL.clear();

    // choose unadded vertex from path
    for (int w = 2; w < dataset.targets.size(); ++w) {
        STarget* wTarget = dataset.targets[w];
        if (!curPath.contains(wTarget) && w != blocked) {

            // insertion to the curent path
            SDiffPath stour1(curPath, length0, reward0);
            find_best_position(curPath, wTarget, stour1);

            if (stour1.len <= dataset.Tmax && stour1.rew > reward0 + 1e-10) {
                CL.push_back(stour1);
                if (stour1.rew > best_rew){
                    best_rew = stour1.rew;
                }
            } else { // if it's not feasible, try to remove segment
                SDiffPath stour2 = stour1;
                int a = 1;
                int b = 1;
                stour2.removeFirst(a);

                // try to remove interval/segment to make the path feasible
                for (; a < l-1 ; ++a) {
                    if (a > stour2.to){
                        stour2.removeNext(); 
                        b++;
                    }

                    if(a > 1){
                        stour2.addAtBegining();  
                    }                                         

                    while (b != (l-2) && stour2.len > dataset.Tmax) {
                        b++;
                        stour2.removeNext();                                           
                    }

                    if (stour2.len <= dataset.Tmax && stour2.rew >= CL_RESTRICT * best_rew) {
                        if (stour2.rew > reward0 + 1e-10){
                            CL.push_back(stour2);

                            if (stour2.rew > best_rew) best_rew = stour2.rew;

                        } else if ((fabs(stour2.rew - reward0) < 1e-10 && 
                                    stour2.len < length0 - 1e-10)) {
                            CL.push_back(stour2);

                            if (stour2.rew > best_rew) best_rew = stour2.rew;
                        }
                    }
                }
            }
        }
    }

    // randomly choose from the candidate list CL
    if (!CL.empty()) {
        restrict(best_rew);
        SDiffPath dp = CL[0];
        if (CL.size() > 1) {
            dp = CL[crl::CRandom::next(CL.size() - 1)];
        }

        curPath = createPath(dp, curPath);

        return true;
    }

    return false;
}

/// - private method -----------------------------------------------------------
void CGRASP::localSearch() {
    bool improved = true;

    while (improved) {
        SPath r = bestPath;
        int l = bestPath.size();
        improved = false;

        // try to replace target with some others 
        for (int i = 1; i < l - 1; ++i) {
            curPath = r.remove(i);
            twoOpt(curPath);
            if(WAYPOINT_OPT) curPath.optimize();

            while (addLocation(r.tv[i]->label)) {}

            double rewardCur = curPath.reward();
            double rewardBest = bestPath.reward();
            if (rewardCur > rewardBest + 1e-10 || 
                (fabs(rewardCur - rewardBest) < 1e-10 && 
                curPath.length() < bestPath.length() - 1e-10)) {
                bestPath = curPath;
                improved = true;
                drawPath();
            }
        }
    }
}

/// - private method -----------------------------------------------------------
void CGRASP::restrict(double best) {
    std::vector<SDiffPath> RCL;
    for (int i = 0; i < CL.size(); ++i) {
        if (CL[i].rew >= CL_RESTRICT * best) {
            RCL.push_back(CL[i]);
        }
    }
    CL = RCL;
}

/// - private method -----------------------------------------------------------
bool CGRASP::twoOpt(SPath &path) {
    bool changePerformed = true;
    int numTwoOpts = 0;
    while (changePerformed) {
        changePerformed = false;
        int numTargets = path.size();
        double actualLength = path.length();
        double dist[numTargets][numTargets];
        for (int i = 0; i < numTargets - 1; i++) {
            for (int j = i + 1; j < numTargets; j++) {
                dist[i][j] = (path.cv[i] - path.cv[j]).length();
            }
        }

        for (int i = 1; i < numTargets - 2; i++) {
            for (int j = i + 1; j < numTargets - 1; j++) {
                double optLength = actualLength
                                    - dist[i-1][i] - dist[j][j+1]
                                    + dist[i-1][j] + dist[i][j+1];
                if (optLength < actualLength - 1e-10) {
                    path = twoOptSwap(path, i, j);
                    changePerformed = true;
                    numTwoOpts++;
                    break;
                }
            }
            if (changePerformed) break;
        }
    }
    return (numTwoOpts > 0);
}

/// - private method -----------------------------------------------------------
SPath CGRASP::twoOptSwap(SPath &tour, const int &i, const int &j) {
    int size = tour.size();

    SPath newTour;

    // 1. take route[0] to route[i-1] and add them in order to new_route
    for (int c = 0; c <= i - 1; ++c) {
        newTour.push_back(tour.tv[c], tour.cv[c]);
    }

    // 2. take route[i] to route[j] and add them in reverse order to new_route
    int dec = 0;
    for (int c = j; c >= i; --c) {
        newTour.push_back(tour.tv[c], tour.cv[c]);
    }

    // 3. take route[j+1] to end and add them in order to new_route
    for (int c = j + 1; c < size; ++c) {
        newTour.push_back(tour.tv[c], tour.cv[c]);
    }

    return newTour;
}

/*
 *
 * FUNCTIONS OF VENDORS LIBRARY
 *
 */

/// - static method ------------------------------------------------------------
crl::CConfig &CGRASP::getConfig(crl::CConfig &config) {
    // basic properties not included in the crl_algorithm
    Base::getConfig(config);
    config.add<bool>("save-info", "disable/enable save info", true);
    config.add<bool>("save-settings", "disable/enable save settings", true);

    config.add<std::string>("result-path", "file name for the final found path (ring) as sequence of points",
                            "path");
    config.add<std::string>("pic-dir", "relative directory in result directory to store pictures from each iteration");
    config.add<std::string>("pic-ext",
                            "extension of pic, eps, png, pdf, svg (supported by particular gui renderer rendered",
                            "png");
    config.add<bool>("save-pic", "enable/disable saving pictures (after each refine)");
    config.add<bool>("wait-to-click", "enable/disable waiting for click after redrawing new best path", false);
    //
    // Problem specification
    config.add<std::string>("problem", "problem file");
    config.add<double>("budget", "budget for OP", 40);
    config.add<std::string>("method", "specify method name in the result log", "grasp-ceop");
    //
    // GRASP specitication
    config.add<bool>("waypoint-optimization", "enable/disable waypoint optimization in local search phase", false);
    //
    // Gui properties
    config.add<std::string>("draw-shape-targets", "shape of the target", Shape::CITY());
    config.add<std::string>("draw-shape-path", "shape of the path", Shape::RED_LINE());
    config.add<std::string>("draw-shape-path-nodes", "shape of the path nodes", Shape::MAP_VERTEX());
    config.add<std::string>("draw-shape-radius", "shape of the neighborhood radius", Shape::POLYGON_FILL());
    config.add<bool>("draw-targets-reward", "draw colored regions depending on rewards", false);
    config.add<bool>("draw-target-location-reward", "draw colored target locations (or centers for regions) depending on rewards", false);
    config.add<std::string>("draw-targets-reward-palette", "file of rewards palette", "./etc/jet.txt");

    config.add<bool>("draw-path", "enable/Disable drawing ring in the final shoot", true);
    config.add<bool>("draw-path-nodes", "enable/disable drawing path vertices(nodes)", true);
    return config;
}

/// - constructor --------------------------------------------------------------
CGRASP::CGRASP(crl::CConfig &config, SDataset &ds) : Base(config, "TRIAL"),
                                                       SAVE_RESULTS(config.get<bool>("save-results")),
                                                       SAVE_SETTINGS(config.get<bool>("save-settings")),
                                                       SAVE_INFO(config.get<bool>("save-info")),
                                                       SAVE_PIC(config.get<bool>("save-pic")),
                                                       WAYPOINT_OPT(config.get<bool>("waypoint-optimization")) {
    shapeTargets.setShape(config.get<std::string>("draw-shape-targets"));
    shapePath.setShape(config.get<std::string>("draw-shape-path"));
    shapePathNodes.setShape(config.get<std::string>("draw-shape-path-nodes"));
    shapeRadius.setShape(config.get<std::string>("draw-shape-radius"));

    method = config.get<std::string>("method");

    CGRASP::dataset = ds;

    const std::string fname = config.get<std::string>("problem");
    if (name.size() == 0) {
        std::string n = getBasename(fname);
        size_t i = n.rfind(".txt");
        if (i != std::string::npos) {
            name = n.erase(i, 4);
        }
    }
}

/// - destructor ---------------------------------------------------------------
CGRASP::~CGRASP() {
}

/// - public method ------------------------------------------------------------
std::string CGRASP::getVersion(void) {
    return "GRASP-CEOP";
}

/// - public method ------------------------------------------------------------
std::string CGRASP::getRevision(void) {
    return "IRMAS'20";
}

/// - public method ------------------------------------------------------------
void CGRASP::solve(void) {
    crl::CRandom::randomize();
    Base::solve();
}

/// - protected method ---------------------------------------------------------
void CGRASP::load(void) {
    if (canvas) {
        // resize area
        *canvas << canvas::AREA;
        for (STarget * target : dataset.targets) {
	   const double x = target->centre.x;
	   const double y = target->centre.y;
	   const double r = target->radius;
           *canvas << x << y << (x+r) << (y+r) << (x-r) << (y-r);
        }
        *canvas << canvas::END;
        
        // draw regions
	crl::gui::CColorMap map;
	std::string pallete = config.get<std::string>("draw-targets-reward-palette");
	const bool rt = config.get<bool>("draw-targets-reward");
	const bool rtl = config.get<bool>("draw-target-location-reward");
	if (crl::isFile(pallete) and (rt or rtl)) { // load pallete
	   map.load(pallete);
	   double minReward = std::numeric_limits<double>::max();
	   double maxReward = std::numeric_limits<double>::min();
	   for (STarget * t : dataset.targets) {
	      if (t->reward < minReward) {
		 minReward = t->reward;
	      }
	      if (t->reward > maxReward) {
		 maxReward = t->reward;
	      }
	   }
	   if (minReward == maxReward) {
	      minReward = 0.99 * maxReward;
	   }
	   map.setRange(minReward, maxReward);
	}
	if (rt) {
            *canvas << "regions" << shapeRadius << canvas::ARC;
            for (STarget * target : dataset.targets) {
                if (target->label == dataset.startID || target->label == dataset.endID) continue;
                SColor color = map.getColor(target->reward);
                color.alpha = 0.3;
                *canvas << canvas::FILL_COLOR << color;
                *canvas << target->centre.x << target->centre.y << target->radius << 0.0 << 2 * M_PI;
            }
        } else {
            *canvas << "regions" << shapeRadius << canvas::ARC;
            for (STarget * target : dataset.targets) {
                if (target->label == dataset.startID || target->label == dataset.endID) continue;
                *canvas << target->centre.x << target->centre.y << target->radius << 0.0 << 2 * M_PI;
            }
        }

        // draw points
        *canvas << "targets" << shapeTargets << canvas::POINT;
        for (STarget * target : dataset.targets) {
	   if (rtl) {
	      SColor color = map.getColor(target->reward);
	      *canvas << canvas::FILL_COLOR << color;
	   }
            *canvas << target->centre.x << target->centre.y;
        }
        *canvas << canvas::REDRAW;

        
   }
    DEBUG("GRASP::load -- done");
}

/// - protected method ---------------------------------------------------------
void CGRASP::initialize(void) {
}

/// - protected method ---------------------------------------------------------
void CGRASP::after_init(void) {
}

/// - protected method ---------------------------------------------------------
void CGRASP::save(void) {
    std::string dir;
    updateResultRecordTimes(); //update timers as load and initilization is outside class
    if (SAVE_SETTINGS) {
        saveSettings(getOutputIterPath(config.get<std::string>("settings"), dir));
    }
    if (SAVE_INFO) {
        saveInfo(getOutputIterPath(config.get<std::string>("info"), dir));
    }
    if (SAVE_RESULTS) {
        std::string file = getOutputIterPath(config.get<std::string>("result-path"), dir);
        assert_io(createDirectory(dir), "Can not create file in the path'" + file + "'");

        const int i = 0;
        std::stringstream ss;
        ss << file << "-" << std::setw(2) << std::setfill('0') << i << ".txt";
        std::ofstream ofs(ss.str().c_str());
        assert_io(not ofs.fail(), "Cannot create path '" + ss.str() + "'");
        ofs << std::setprecision(14);
        for (int i = 0; i < bestPath.size(); ++i) {
            ofs << bestPath.cv[i].x << " " << bestPath.cv[i].y << " " << bestPath.tv[i]->reward << std::endl;
        }
        assert_io(not ofs.fail(), "Error occur during path saving");
        ofs.close();
    }
    if (canvas) { // map must be set
        if (config.get<bool>("draw-path")) {
            drawPath();
        }
        saveCanvas();
    }
}

/// - protected method ---------------------------------------------------------
void CGRASP::release(void) {
}

/// - protected method ---------------------------------------------------------
void CGRASP::defineResultLog(void) {
    static bool resultLogInitialized = false;
    if (!resultLogInitialized) {
        resultLog << result::newcol << "NAME";
        resultLog << result::newcol << "METHOD";
        resultLog << result::newcol << "TRIAL";
        resultLog << result::newcol << "RTIME";
        resultLog << result::newcol << "CTIME";
        resultLog << result::newcol << "UTIME";
        resultLog << result::newcol << "BUDGET";
        resultLog << result::newcol << "RADIUS";
        resultLog << result::newcol << "LENGTH";
        resultLog << result::newcol << "REWARDS";
        resultLogInitialized = true;
    }
}

/// - protected method ---------------------------------------------------------
void CGRASP::fillResultRecord(int trial) {
    resultLog << result::newrec << name << method << trial;
    long t[3] = {0l, 0l, 0l};
    tLoad.addTime(t);
    tInit.addTime(t);
    tSolve.addTime(t);
    tSave.addTime(t);
    resultLog << t[0] << t[1] << t[2];
    DEBUG("time");
    double solutionLength = bestPath.length();
    DEBUG("len");
    double solutionReward = bestPath.reward();
    DEBUG("rew");
    double radius = dataset.targets[2]->radius;
    resultLog
            << dataset.Tmax // budget
            << radius
            << solutionLength
            << solutionReward
            << crl::result::endrec;
    INFO("Trial " << iter << " best solution reward: " << solutionReward << " length: " << solutionLength);
}

/// - private method -----------------------------------------------------------
void CGRASP::drawPath(void) {
    drawPath(bestPath);
}

/// - private method -----------------------------------------------------------
void CGRASP::drawPath(SPath &path) {
    if (canvas) {
        *canvas
               << canvas::CLEAR << "path" << "path"
               << CShape(config.get<std::string>("draw-shape-path"))
               << canvas::LINESTRING
               << path.cv
               << canvas::END;

        if (config.get<bool>("draw-path-nodes")) {
            *canvas
                  << canvas::CLEAR << "nodes" << "nodes"
                  << canvas::POINT << shapePathNodes << path.cv;
        }
        if (config.get<bool>("wait-to-click")) {
            canvas->click();
        }
        canvas->redraw();

    }
}

void printPath(SPath &p) {
    for (int i = 0; i < p.size(); i++) {
        DEBUG(i << " label:" << p.tv[i]->label << " (" << p.cv[i].x << "," << p.cv[i].y << ")");
    }
}
