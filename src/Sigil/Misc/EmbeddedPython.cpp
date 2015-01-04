/************************************************************************
**
**  Copyright (C) 2014  John Schember <john@nachtimwald.com>
**  Copyright (C) 2014  Kevin Hendricks
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

#include "Misc/EmbeddedPython.h"

#include <QString>
#include <QByteArray>
#include <QList>
#include <QStringList>
#include <QVariant>
#include <QMetaType>
#include <QStandardPaths>
#include <QDir>
#include "Misc/Utility.h"

/**
 * Possibly Useful QMetaTypes::Type types
 *
 * QMetaType::Bool             1	bool
 * QMetaType::Int              2	int
 * QMetaType::UInt             3	unsigned int
 * QMetaType::Double           6	double
 * QMetaType::QChar            7	QChar
 * QMetaType::QString         10	QString
 * QMetaType::QByteArray      12	QByteArray
 * QMetaType::Long            32	long
 * QMetaType::LongLong         4	LongLong
 * QMetaType::Short           33	short
 * QMetaType::Char            34	char
 * QMetaType::ULong           35	unsigned long
 * QMetaType::ULongLong        5	ULongLong
 * QMetaType::UShort          36	unsigned short
 * QMetaType::SChar           40	signed char
 * QMetaType::UChar           37	unsigned char
 * QMetaType::Float           38	float
 * QMetaType::QVariant        41	QVariant
 * QMetaType::QVariantList     9	QVariantList
 * QMetaType::QStringList     11	QStringList
 * QMetaType::QVariantMap      8	QVariantMap
 * QMetaType::QVariantHash    28	QVariantHash
 * QMetaType::UnknownType      0	This is an invalid type id. It is returned from QMetaType for types that are not registered
*/

/**
 * a simple example of how to use code:
 *  void EmbeddedPython::multiply_pushed(int val1, int val2)
 *  {
 *      int rv   = 0;
 *      QString error_traceback;
 *      QList<QVariant> args;
 *      args.append(QVariant(val1));
 *      args.append(QVariant(val2));
 *      QVariant res = runInPython(QString("multiply"),
 *                                 QString("multiply"),
 *                                 args,
 *                                 &rv,
 *                                 error_traceback);
 *      if (rv == 0) {
 *          // no errors
 *      } else {
 *         // error occured 
 *      }
 *  }
 *
 *  Where multiply.py is:
 *
 *  #!/usr/bin/env python3
 *                                                                                                           
 *  def multiply(a,b):
 *      print("Will compute", a, "times", b)
 *      c  = a * b
 *      return c
 */

QMutex EmbeddedPython::m_mutex;
EmbeddedPython* EmbeddedPython::m_instance = 0;

EmbeddedPython* EmbeddedPython::instance()
{
    if (m_instance == 0) {
        m_instance = new EmbeddedPython();
    }
    return m_instance;
}

EmbeddedPython::EmbeddedPython()
{
    Py_Initialize();
}

EmbeddedPython::~EmbeddedPython()
{
    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
    Py_Finalize();
}

QString EmbeddedPython::embeddedRoot()
{
    QString embedded_root = QCoreApplication::applicationDirPath();

#ifdef Q_OS_MAC
    embedded_root += "/../python3lib/";
#elif defined(Q_OS_WIN32)
    embedded_root += "/python3lib/";
#elif !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // all flavours of linux / unix
    embedded_root += "/../share/" + QCoreApplication::applicationName().toLower() + "/python3lib/";
    // user supplied environment variable to embedded python code directory will overrides everything
    const QString env_embedded_location = QString(getenv("SIGIL_EMBEDDED_PYTHON_CODE"));
    if (!env_embedded_location.isEmpty()) {
        embedded_root = env_embedded_location + "/";
    }
#endif
    QDir base(embedded_root);
    return base.absolutePath();
}

bool EmbeddedPython::addToPythonSysPath(const QString& mpath)
{
    EmbeddedPython::m_mutex.lock();
    PyObject* sysPath    = NULL;
    PyObject* aPath = NULL;
    bool success = false;

    // PySys_GetObject borrows a reference */
    sysPath = PySys_GetObject((char*)"path");
    if (sysPath != NULL) {
        aPath = PyUnicode_FromString(mpath.toUtf8().constData());
        if (aPath != NULL) {
            PyList_Append(sysPath, aPath);
            success = true;
        }
    }
    Py_XDECREF(aPath);
    EmbeddedPython::m_mutex.unlock();
    return success;
}

// run interpreter without initiating/locking/unlocking GIL 
// in a single thread at a time
QVariant EmbeddedPython::runInPython(const QString& mname, 
                                     const QString& fname, 
                                     const QVariantList& args, 
                                     int *rv, 
                                     QString & tb)
{
    EmbeddedPython::m_mutex.lock();
    QVariant  res        = QVariant(QString());
    PyObject *moduleName = NULL;
    PyObject *module     = NULL;
    PyObject *func       = NULL;
    PyObject *pyargs     = NULL;
    PyObject *pyres      = NULL;
    int       idx        = 0;
        
    moduleName = PyUnicode_FromString(mname.toUtf8().constData());
    if (moduleName == NULL) {
        *rv = -1;
        goto cleanup;
    }

    module = PyImport_Import(moduleName);
    if (module == NULL) {
        *rv = -2;
        goto cleanup;
    }

    func = PyObject_GetAttrString(module,fname.toUtf8().constData());
    if (func == NULL) {
        *rv = -3;
        goto cleanup;
    }

    if (!PyCallable_Check(func)) {
        *rv = -4;
        goto cleanup;
    }

    // Build up Python argument List from args
    pyargs = PyTuple_New(args.size());
    idx = 0;
    foreach(QVariant arg, args) {
        PyTuple_SetItem(pyargs, idx, QVariantToPyObject(arg));
        idx++;
    }

    pyres = PyObject_CallObject(func, pyargs);
    if (pyres == NULL) {
        *rv = -5;
        goto cleanup;
    }

    *rv = 0;

    res = PyObjectToQVariant(pyres);

cleanup:
    if (PyErr_Occurred() != NULL) {
        tb = getPythonErrorTraceback();
    }
    Py_XDECREF(pyres);
    Py_XDECREF(pyargs);
    Py_XDECREF(func);
    Py_XDECREF(module);
    Py_XDECREF(moduleName);

    EmbeddedPython::m_mutex.unlock();
    return res;
}


// *** below here all routines are private and only invoked 
// *** from runInPython with lock held


// Convert PyObject types to their QVariant equivalents 
// call recursively to allow populating QVariant lists and lists of lists
QVariant EmbeddedPython::PyObjectToQVariant(PyObject* po)
{
    QVariant res = QVariant(QString());

    if ((po) == NULL)
        return res;

    if (PyLong_Check(po)) {
        res = QVariant(PyLong_AsLongLong(po));

    } else if (PyFloat_Check(po)) {
        res = QVariant(PyFloat_AsDouble(po));

    } else if (PyBytes_Check(po)) {
        res = QVariant(QByteArray(PyBytes_AsString(po)));

    } else if (PyUnicode_Check(po)) {

        int kind = PyUnicode_KIND(po);

        if (PyUnicode_READY(po) != 0)
            return res;

        if (kind == PyUnicode_1BYTE_KIND) {
            // latin 1 according to PEP 393
            res = QVariant(QString::fromLatin1(reinterpret_cast<const char *>PyUnicode_1BYTE_DATA(po), -1));

        } else if (kind == PyUnicode_2BYTE_KIND) {
            res = QVariant(QString::fromUtf16(PyUnicode_2BYTE_DATA(po), -1));

        } else if (kind == PyUnicode_4BYTE_KIND) {
            // PyUnicode_4BYTE_KIND
            res = QVariant(QString::fromUcs4(PyUnicode_4BYTE_DATA(po), -1));

        } else {
            // convert to utf8 since not a known
            res = QVariant(QString::fromUtf8(PyUnicode_AsUTF8(po),-1));
        }

    } else if (PyTuple_Check(po)) {
        QVariantList vlist;
        int n = PyTuple_Size(po);
        for (int i=0; i< n; i++) {
            vlist.append(PyObjectToQVariant(PyTuple_GetItem(po,i)));
        }
        res = QVariant(vlist);

    } else if (PyList_Check(po)) {
        QVariantList vlist;
        int n = PyList_Size(po);
        for (int i=0; i< n; i++) {
            vlist.append(PyObjectToQVariant(PyList_GetItem(po,i)));
        }
        res = QVariant(vlist);

    } else {
       // do nothing here to return null value
    }
    return res;
}

// Convert QVariant to a Python Equivalent Type
// call recursively to allow populating tuples/lists and lists of lists
PyObject* EmbeddedPython::QVariantToPyObject(QVariant & v)
{
    PyObject* value = NULL;
    bool      ok;
    switch ((QMetaType::Type)v.type()) {
        case QMetaType::Double:
            value = Py_BuildValue("d", v.toDouble(&ok));
            break;
        case QMetaType::Float:
            value = Py_BuildValue("f", v.toFloat(&ok));
            break;
        case QMetaType::Int:
            value = Py_BuildValue("i", v.toInt(&ok));
            break;
        case QMetaType::UInt:
            value = Py_BuildValue("I", v.toUInt(&ok));
            break;
        case QMetaType::LongLong:
            value = Py_BuildValue("L", v.toLongLong(&ok));
            break;
        case QMetaType::ULongLong:
            value = Py_BuildValue("K", v.toULongLong(&ok));
            break;
        case QMetaType::QString:
            value = PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, v.toString().utf16(), v.toString().size());
            // value = Py_BuildValue("s", v.toString().toUtf8().constData());
            break;
        case QMetaType::QByteArray:
            value = Py_BuildValue("y", v.toByteArray().constData());
            break;
        case QMetaType::QStringList:
            // fall through
        case QMetaType::QVariantList: {
                QVariantList vlist = v.toList();
                value = PyList_New(vlist.size());
                int idx=0;
                foreach(QVariant av, vlist) {
                    PyList_SetItem(value, idx, QVariantToPyObject(av));
                    idx++;
                }
            } break;
        default:
            // Ensure we don't have any holes.
            value = Py_BuildValue("u", "");
            break;
        }
    return value;
}


// get traceback from inside interpreter upon error
QString EmbeddedPython::getPythonErrorTraceback(bool useMsgBox)
{
    PyObject     *etype      = NULL;
    PyObject     *evalue     = NULL;
    PyObject     *etraceback = NULL;
    PyObject     *mod        = NULL;
    PyObject     *elist      = NULL;
    QStringList  tblist;
    
    PyErr_Fetch(&etype, &evalue, &etraceback);
    PyErr_NormalizeException(&etype, &evalue, &etraceback);

    mod = PyImport_ImportModule("traceback");

    if (mod) {
        elist   = PyObject_CallMethod(mod, "format_exception", "OOO", etype, evalue, etraceback);
        if (elist != NULL) {
            tblist = PyObjectToQVariant(elist).toStringList();
        } else {
            tblist.append(QString("Error: traceback report is missing"));
        }
    } else {
        tblist.append(QString("Error: traceback module failed to load"));
    }

    Py_XDECREF(elist);
    Py_XDECREF(mod);
    Py_XDECREF(etraceback);
    Py_XDECREF(evalue);
    Py_XDECREF(etype);

    PyErr_Clear();

    QString tb = tblist.join(QString("\n"));
    if (useMsgBox) {
        QString message = QString(tr("Embedded Python Error"));
        Utility::DisplayStdErrorDialog(message, tb);
    }
    return tb;
}
