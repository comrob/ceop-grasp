/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <crl/logging.h>
#include <crl/perf_timer.h>

#include <crl/gui/guifactory.h>
#include <crl/gui/win_adjust_size.h>

#include <crl/boost_args_config.h>
#include <crl/config.h>

#include "grasp.h"

using crl::logger;
using namespace grasp;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

typedef crl::gui::CCanvasBase Canvas;

/// ----------------------------------------------------------------------------
/// Program options variables
/// ----------------------------------------------------------------------------
std::string guiType = "none";

crl::CConfig guiConfig;
crl::CConfig graspConfig;
std::string canvasOutput = "";

/// ----------------------------------------------------------------------------
/// Global variable
/// ----------------------------------------------------------------------------
crl::gui::CGui *g = 0;
#define GUI(x)  if(gui) { x;}

/// ----------------------------------------------------------------------------
bool parseArgs(int argc, char *argv[]) {
    bool ret = true;
    std::string configFile;
    std::string guiConfigFile;
    std::string loggerCfg = "";

    po::options_description desc("General options");
    desc.add_options()
            ("help,h", "produce help message")
            ("config,c", po::value<std::string>(&configFile)->default_value(std::string(argv[0]) + ".cfg"),
             "configuration file")
            ("logger-config,l", po::value<std::string>(&loggerCfg)->default_value(loggerCfg),
             "logger configuration file");
    try {
        po::options_description guiOptions("Gui options");
        crl::gui::CGuiFactory::getConfig(guiConfig);
        crl::gui::CWinAdjustSize::getConfig(guiConfig);
        guiConfig.add<double>("gui-add-x",
                              "add the given value to the loaded goals x coord to determine the canvas size and transformation",
                              0);
        guiConfig.add<double>("gui-add-y",
                              "add the given value to the loaded goals y coord to determine the canvas size and transformation",
                              0);
        boost_args_add_options(guiConfig, "", guiOptions);
        guiOptions.add_options()
                ("canvas-output", po::value<std::string>(&canvasOutput), "result canvas outputfile");

        po::options_description graspOptions("GRASP options");
        boost_args_add_options(CGRASP::getConfig(graspConfig), "", graspOptions);

        po::options_description cmdline_options;
        cmdline_options.add(desc).add(guiOptions).add(graspOptions);

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, cmdline_options), vm);
        po::notify(vm);

        std::ifstream ifs(configFile.c_str());
        store(parse_config_file(ifs, cmdline_options), vm);
        po::notify(vm);
        ifs.close();
        ifs.open(guiConfigFile.c_str());
        store(parse_config_file(ifs, cmdline_options), vm);
        po::notify(vm);
        ifs.close();

        if (vm.count("help")) {
            std::cerr << std::endl;
            std::cerr << cmdline_options << std::endl;
            ret = false;
        }
        if (
                ret &&
                loggerCfg != "" &&
                fs::exists(fs::path(loggerCfg))
                ) {
            crl::initLogger("grasp", loggerCfg.c_str());
        } else {
            crl::initLogger("grasp");
        }
        const std::string problemFile = graspConfig.get<std::string>("problem");
        if (!fs::exists(fs::path(problemFile))) {
            ERROR("Problem file '" + problemFile + "' does not exists");
            ret = false;
        }
    } catch (std::exception &e) {
        std::cerr << std::endl;
        std::cerr << "Error in parsing arguments: " << e.what() << std::endl;
        ret = false;
    }
    return ret;
}

/// ----------------------------------------------------------------------------
CoordsVector getCoordsFromDataset(SDataset &ds) {
    CoordsVector cv;
    for (int i = 0; i < ds.targets.size(); ++i) {
       cv.push_back(ds.targets[i]->centre);
    }
    return cv;
}

/// - main ---------------------------------------------------------------------
int main(int argc, char *argv[]) {
    Canvas *canvas = 0;
    int ret = -1;
    if (parseArgs(argc, argv)) {
        INFO("Start Logging");
        try {
            CoordsVector pts;
            SDataset ds;
            // loading of the problem to dataset of targets 'ds' and coordinates of the dataset targets 'pts'
            {
                crl::CPerfTimer t("Load problem time real:");
                ds = loadDataset(graspConfig.get<std::string>("problem"), graspConfig.get<double>("budget"));
                pts = getCoordsFromDataset(ds);
            }
            crl::gui::CWinAdjustSize::adjust(pts, guiConfig);
            if ((g = crl::gui::CGuiFactory::createGui(guiConfig)) != 0) {
                INFO("Start gui " + guiConfig.get<std::string>("gui"));
                canvas = new Canvas(*g);
            }
            CGRASP grasp(graspConfig, ds);
            grasp.setCanvas(canvas);
            {
                crl::CPerfTimer t("Total solve time: ");
                grasp.solve();
            }
            INFO("End Logging");
            if (canvas) {
                if (canvasOutput.size()) {
                    canvas->save(canvasOutput);
                }
                if (!guiConfig.get<bool>("nowait")) {
                    INFO("click to exit");
                    canvas->click();
                }
                delete canvas;
                delete g;
            }
        } catch (crl::exception &e) {
            ERROR("Exception " << e.what() << "!");
        } catch (std::exception &e) {
            ERROR("Runtime error " << e.what() << "!");
        }
        ret = EXIT_SUCCESS;
    }
    crl::shutdownLogger();
    return ret;
}
