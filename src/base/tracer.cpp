/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	  $Id$
 *
 *****************************************************************************/

#include "tracer.h"
#include "logger.h"

#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <stdio.h>

// measures the perf traces.
#include <OgreTimer.h>

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace Opde {

template <> Tracer *Singleton<Tracer>::ms_Singleton = 0;

Tracer::Tracer(Ogre::Timer *timer)
    : mTimer(timer), mTraceFrameNum(0),
      mFrameStartTime(mTimer->getMicroseconds()) {}

Tracer::~Tracer() {}

Tracer &Tracer::getSingleton(void) {
    assert(ms_Singleton);
    return (*ms_Singleton);
}

Tracer *Tracer::getSingletonPtr(void) { return ms_Singleton; }

void Tracer::traceStartFrame() {
    // spit out the traces?
    unsigned long now = mTimer->getMicroseconds();
    unsigned long spentTime = now - mFrameStartTime;

    // For now, I'm spitting this into normal log. Later I'll
    // write it into a special file.
    LOG_DEBUG("PERF_FRAME f=%zu t=%zu spent=%zu", mTraceFrameNum, now, spentTime);

    for (TraceLog::const_iterator ci = mTraces.begin(), iend = mTraces.end();
         ci != iend; ++ci) {
        if (ci->function) {
            if (ci->entry) {
                LOG_DEBUG("PERF_TRACE_SCOPE %zu ENTER '%s' '%p'",
                          ci->time - mFrameStartTime, ci->text, ci->data);
            } else {
                LOG_DEBUG("PERF_TRACE_SCOPE %zu EXIT(%zu) '%s' '%p'",
                          ci->time - mFrameStartTime, ci->spent, ci->text,
                          ci->data);
            }
        } else {
            LOG_DEBUG("PERF_TRACE_POINT %zu '%s' '%p'", ci->time - mFrameStartTime,
                      ci->text, ci->data);
        }
    }

    ++mTraceFrameNum;
    mTraces.clear();
    mTraces.reserve(256);
    mFrameStartTime = mTimer->getMicroseconds();
}

/** logs a tracer record used for performance tracing */
unsigned long Tracer::trace(bool start, const char *func, const void *data) {
    TraceRecord trace;
    trace.time = mTimer->getMicroseconds();
    trace.entry = start;
    trace.text = func;
    trace.function = true;
    trace.data = data;
    mTraces.push_back(trace);
    return trace.time;
}

/** logs a tracer record used for performance tracing */
void Tracer::trace_endpoint(const char *func,
                            const void *data, unsigned long start)
{
    TraceRecord trace;
    trace.time = mTimer->getMicroseconds();
    trace.entry = false;
    trace.text = func;
    trace.function = true;
    trace.data = data;
    trace.spent = trace.time - start;
    mTraces.push_back(trace);
}

/** logs a tracer record used for performance tracing */
void Tracer::tracePoint(const char *text) {
    TraceRecord trace;
    trace.time = mTimer->getMicroseconds();
    trace.entry = true;
    trace.text = text;
    trace.function = false;
    mTraces.push_back(trace);
}


} // namespace Opde
