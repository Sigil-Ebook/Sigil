/************************************************************************
**
**  Copyright (C) 2015 Kevin B. Hendricks, John Schember
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
#ifndef EMBEDDEDPYTHON_H
#define EMBEDDEDPYTHON_H

#include <Python.h>
#include <QCoreApplication>
#include <QString>
#include <QVariant>
#include <QMutex>
#include "Misc/PyObjectPtr.h"

/**
 * Singleton.
 */

class EmbeddedPython
{
    Q_DECLARE_TR_FUNCTIONS(EmbeddedPython)


public:
    static EmbeddedPython* instance();
    ~EmbeddedPython();

    QString embeddedRoot();

    bool addToPythonSysPath(const QString& modulepath);

    QVariant runInPython(const QString &module_name,
                         const QString &function_name,
                         const QVariantList &args,
                         int *pRV,
                         QString &error_traceback,
                         bool ret_python_object = false);

    QVariant callPyObjMethod(PyObjectPtr &pyobj, 
                             const QString &methname, 
                             const QVariantList &args, 
                             int *rv, 
                             QString &tb,
                             bool ret_python_object = false);

private:

    EmbeddedPython();

    QVariant PyObjectToQVariant(PyObject *po, bool ret_python_object = false);

    PyObject *QVariantToPyObject(const QVariant &v);

    QString getPythonErrorTraceback(bool useMsgBox = true);

    static QMutex m_mutex;
    static EmbeddedPython *m_instance;
    static int m_pyobjmetaid;
    static PyThreadState *m_threadstate;
};
#endif // EMBEDDEDPYTHON_H
