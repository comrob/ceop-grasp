/*
 * Date:      2020/01/31 10:33
 * Author:    Petra Stefanikova, Petr Vana, Jan Faigl
 */

#ifndef __CANVASVIEW_GRASP_H__
#define __CANVASVIEW_GRASP_H__

#include <vector>

#include <boost/foreach.hpp>

#include <crl/gui/gui.h>
#include <crl/gui/colors.h>
#include <crl/gui/renderer.h>
#include <crl/gui/canvas.h>

#include "target.h"

namespace grasp {
    typedef std::vector<grasp::STarget *> TargetPtrVector;
} // end namespace grasp

/// ----------------------------------------------------------------------------
inline crl::gui::CCanvasBase& operator<<(crl::gui::CCanvasBase &canvas, const grasp::STarget &target)
{
    canvas << target.coords.x << target.coords.y;
    return canvas;
}

/// ----------------------------------------------------------------------------
inline crl::gui::CCanvasBase& operator<<(crl::gui::CCanvasBase &canvas, const grasp::TargetPtrVector &targets)
{
    BOOST_FOREACH(const grasp::STarget *target, targets) {
                    canvas << target->coords.x << target->coords.y;
                }
    return canvas;
}

#endif //__CANVASVIEW_GRASP_H__
