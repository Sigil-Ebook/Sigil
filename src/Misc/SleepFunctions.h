/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#pragma once
#ifndef SLEEPFUNCTIONS_H
#define SLEEPFUNCTIONS_H

#include <QtCore/QThread>

// For unexplained reasons, the very nice cross-platform
// sleep functions present in QThread are protected.
// This class is a helper to make them public.
class SleepFunctions : public QThread
{

public:

    // Sleep for num "seconds"
    static void sleep(unsigned long seconds) {
        QThread::sleep(seconds);
    }

    // Sleep for num "milliseconds"
    static void msleep(unsigned long milliseconds) {
        QThread::msleep(milliseconds);
    }

    // Sleep for num "microseconds"
    static void usleep(unsigned long microseconds) {
        QThread::usleep(microseconds);
    }
};

#endif // SLEEPFUNCTIONS_H
