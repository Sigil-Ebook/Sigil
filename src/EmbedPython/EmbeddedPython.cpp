/************************************************************************
**
**  Copyright (C) 2015-2025  Kevin Hendricks
**  Copyright (C) 2015-2025  Doug Massay
**  Copyright (C) 2015       John Schember <john@nachtimwald.com>
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

#include "EmbedPython/EmbeddedPython.h"
#include <QString>
#include <QByteArray>
#include <QList>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QMetaType>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include "Misc/Utility.h"
#include "sigil_constants.h"

#define DBG if(1)

// IMPORTANT NOTE:  This interface does NOT support passing/converting the type bool
// All bool values should be converted to integeres with values of 0 for false and 1 for true

/**
 * Possibly Useful QMetaTypes::Type types
 *
 * QMetaType::Bool             1    bool
 * QMetaType::Int              2    int
 * QMetaType::UInt             3    unsigned int
 * QMetaType::Double           6    double
 * QMetaType::QChar            7    QChar
 * QMetaType::QString         10    QString
 * QMetaType::QByteArray      12    QByteArray
 * QMetaType::Long            32    long
 * QMetaType::LongLong         4    LongLong
 * QMetaType::Short           33    short
 * QMetaType::Char            34    char
 * QMetaType::ULong           35    unsigned long
 * QMetaType::ULongLong        5    ULongLong
 * QMetaType::UShort          36    unsigned short
 * QMetaType::SChar           40    signed char
 * QMetaType::UChar           37    unsigned char
 * QMetaType::Float           38    float
 * QMetaType::QVariant        41    QVariant
 * QMetaType::QVariantPair    58    QVariantPair (std:pair<>)
 * QMetaType::QVariantList     9    QVariantList
 * QMetaType::QStringList     11    QStringList
 * QMetaType::QVariantMap      8    QVariantMap
 * QMetaType::QVariantHash    28    QVariantHash
 * QMetaType::User          1024    Base value for User registered Type
 * QMetaType::UnknownType      0    This is an invalid type id. It is returned from QMetaType for types that are not registered
 */


/**
 *  // example of how to run a python function inside a specific module
 *
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
 *  // Where multiply.py is:
 *
 *  #!/usr/bin/env python3
 *                                                                                                           
 *  def multiply(a,b):
 *      print("Will compute", a, "times", b)
 *      c  = a * b
 *      return c
 */

/**
 * // example of how to use the callPyObjMethod
 *
 *  // First invoke module function to get the Python object
 *
 *  PyObjectPtr MyClass::get_object()
 *  {
 *      int rv = 0;
 *      QString traceback;
 *      QString v1 = QString("Hello");
 *      QList<QVariant> args;
 *      args.append(QVariant(v1));
 *  
 *      QVariant res = m_epp->runInPython( QString("multiply"),
 *                                         QString("get_object"),
 *                                         args,
 *                                         &rv,
 *                                         traceback,
 *                                         true);
 *      if (rv == 0) {
 *          return PyObjectPtr(res);
 *      } else {
 *          return PyObjectPtr();
 *      }
 *  }
 *
 * // Now invoke its "get_len" method
 *
 * QString MyClass:use_object(PyObjectPtr v)
 * {
 *     int rv = 0;
 *     QString traceback;
 *     QList<QVariant> args;
 *     QVariant res = m_epp->callPyObjMethod(v,
 *                                           QString("get_len"),
 *                                           args,
 *                                           &rv,
 *                                           traceback);
 *     if (rv == 0) {
 *         return res.toString();
 *     } else {
 *         return QString("Error: ") + QString::number(rv);
 *     }
 * }
 *
 *
 * # With the following python code inside of multiply.py
 *
 * class TestMe:
 *     def __init__(self, storeme):
 *         self.storeme = storeme
 *         self.mylen = len(self.storeme)
 * 
 *     def get_me(self):
 *         return self.storeme
 *
 *     def get_len(self):
 *         return self.mylen
 * 
 * def get_object(v1):
 *     tme = TestMe(v1)
 *     return tme
 */


QMutex EmbeddedPython::m_mutex;

EmbeddedPython* EmbeddedPython::m_instance = 0;
int EmbeddedPython::m_pyobjmetaid = 0;
int EmbeddedPython::m_listintmetaid = 0;
int EmbeddedPython::m_stdpairintintmetaid = 0;

PyThreadState * EmbeddedPython::m_threadstate = NULL;

EmbeddedPython* EmbeddedPython::instance()
{
    if (m_instance == 0) {
        m_instance = new EmbeddedPython();
    }
    return m_instance;
}

EmbeddedPython::EmbeddedPython()
{

    // Use new Python PyConfig and init routines
    PyStatus status;
    PyConfig config;

#if defined(BUNDLING_PYTHON)
    // initialize to be embedded (isolated)
    PyConfig_InitIsolatedConfig(&config);
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    if (APPIMAGE_BUILD) {
        QString interppath = QCoreApplication::applicationDirPath() + "/python3";
        qDebug() << "Embedded interpreter path: " << interppath;
        PyConfig_SetString(&config, &config.executable, interppath.toStdWString().c_str());
    }
#endif // !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
#else
    // Linux, NetBSD, and everyone else (no Bundling)
#if defined(LINUX_VIRT_PY)
    // virt_python_bin is an extern const QString from constants.cpp
    if (!virt_python_bin.isEmpty()) {
        PyConfig_InitIsolatedConfig(&config);
        PyConfig_SetString(&config, &config.executable, virt_python_bin.toStdWString().c_str());
    } else {
        PyConfig_InitPythonConfig(&config);
    }
#else
    PyConfig_InitPythonConfig(&config);
#endif // defined(LINUX_VIRT_PY)
#endif // defined(BUNDLING_PYTHON)

    //From https://github.com/python/cpython/blob/main/Python/initconfig.c
    // Isolated Config is equivalent to
    //    config->_config_init = (int)_PyConfig_INIT_ISOLATED;
    //    config->isolated = 1;
    //    config->use_environment = 0;
    //    config->user_site_directory = 0;
    //    config->dev_mode = 0;
    //    config->install_signal_handlers = 0;
    //    config->use_hash_seed = 0;
    //    config->faulthandler = 0;
    //    config->tracemalloc = 0;
    //    config->perf_profiling = 0;
    //    config->int_max_str_digits = _PY_LONG_DEFAULT_MAX_STR_DIGITS;
    //    config->safe_path = 1;
    //    config->pathconfig_warnings = 0;
    // #ifdef MS_WINDOWS
    //    config->legacy_windows_stdio = 0;
    // #endif

    status = PyConfig_Read(&config);
    if (PyStatus_Exception(status)) {
        qDebug() << "EmbeddedPython constructor error: could not read the config";
        qDebug() << QString(status.err_msg);
        PyConfig_Clear(&config);
        return;
    }

#if defined(BUNDLING_PYTHON)
    config.write_bytecode = 0;
    config.optimization_level = 2;
    config.module_search_paths_set = 1;

#if defined(__APPLE__)
    QDir exedir(QCoreApplication::applicationDirPath());
    exedir.cdUp();
    QString pyhomepath = exedir.absolutePath() + PYTHON_MAIN_PREFIX;
    foreach (const QString &src_path, PYTHON_SYS_PATHS) {
        QString pysyspath = pyhomepath + PYTHON_LIB_PATH + src_path;
        status = PyWideStringList_Append(&config.module_search_paths, pysyspath.toStdWString().c_str());
        if (PyStatus_Exception(status)) {
            qDebug() << "EmbeddedPython constructor error: Could not set sys.path";
            qDebug() << QString(status.err_msg);
        }
    }
#elif defined(Q_OS_WIN32) // Windows
    QString pyhomepath = QCoreApplication::applicationDirPath();
    foreach (const QString &src_path, PYTHON_SYS_PATHS) {
        QString pysyspath = pyhomepath + PYTHON_MAIN_PATH + src_path;
        status = PyWideStringList_Append(&config.module_search_paths, pysyspath.toStdWString().c_str());
        if (PyStatus_Exception(status)) {
            qDebug() << "EmbeddedPython constructor error: Could not set sys.path";
            qDebug() << QString(status.err_msg);
        }
    }
#else // *nix
    QDir exedir(QCoreApplication::applicationDirPath());
    exedir.cdUp();
    QString pyhomepath = exedir.absolutePath() + PYTHON_MAIN_PREFIX;
    foreach (const QString &src_path, PYTHON_SYS_PATHS) {
        QString pysyspath = pyhomepath + PYTHON_LIB_PATH + src_path;
        qDebug() << "sys.path = " << pysyspath;
        status = PyWideStringList_Append(&config.module_search_paths, pysyspath.toStdWString().c_str());
        if (PyStatus_Exception(status)) {
            qDebug() << "EmbeddedPython constructor error: Could not set sys.path";
            qDebug() << QString(status.err_msg);
        }
    }
#endif
    
#endif  // BUNDLING PYTHON
    
    // Use new Python PyConfig and init routines
    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        qDebug() << "EmbeddedPython constructor error: Could not initialize from config";
        qDebug() << QString(status.err_msg);
        PyConfig_Clear(&config);
        return;
    }
    PyConfig_Clear(&config);
    // Debug output for python paths
    DBG PyRun_SimpleString("import sys; print(f'{sys.executable=}\\n{sys.prefix=}\\n{sys.exec_prefix=}\\n{sys.base_prefix=}\\n{sys.base_exec_prefix=}\\n{sys.path=}')");

    m_threadstate = PyEval_SaveThread();
    m_pyobjmetaid = qMetaTypeId<PyObjectPtr>();
    m_listintmetaid = qMetaTypeId<QList<int> >();
    m_stdpairintintmetaid = qMetaTypeId<std::pair<int, int> >();
}


EmbeddedPython::~EmbeddedPython()
{
    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
    m_pyobjmetaid = 0;
    m_listintmetaid = 0;
    m_stdpairintintmetaid = 0;
    PyEval_RestoreThread(m_threadstate);
    Py_Finalize();
}

QString EmbeddedPython::embeddedRoot()
{
    QString     embedded_root;
    QStringList embedded_roots;
    QDir        d;

#ifdef Q_OS_MAC
    embedded_roots.append(QCoreApplication::applicationDirPath() + "/../python3lib/");
#elif defined(Q_OS_WIN32)
    embedded_roots.append(QCoreApplication::applicationDirPath() + "/python3lib/");
#elif !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // all flavours of linux / unix
    // user supplied environment variable to 'share/sigil' directory will overrides everything
    if (!sigil_extra_root.isEmpty()) {
        embedded_roots.append(sigil_extra_root + "/python3lib/");
    } else {
        embedded_roots.append(sigil_share_root + "/python3lib/");
    }
#endif
    
    Q_FOREACH (QString s, embedded_roots) {
        if (d.exists(s)) {
            embedded_root = s;
            break;
        }
    }

    QDir base(embedded_root);
    return base.absolutePath();
}

bool EmbeddedPython::addToPythonSysPath(const QString &mpath)
{
    EmbeddedPython::m_mutex.lock();
    PyGILState_STATE gstate = PyGILState_Ensure();
        
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
    PyGILState_Release(gstate);
    EmbeddedPython::m_mutex.unlock();
    return success;
}

// run interpreter without initiating/locking/unlocking GIL 
// in a single thread at a time
QVariant EmbeddedPython::runInPython(const QString &mname, 
                                     const QString &fname, 
                                     const QVariantList &args, 
                                     int *rv, 
                                     QString &tb,
                                     bool ret_python_object)
{
    EmbeddedPython::m_mutex.lock();
    PyGILState_STATE gstate = PyGILState_Ensure();
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

    res = PyObjectToQVariant(pyres, ret_python_object);

cleanup:
    if (PyErr_Occurred() != NULL) {
        QString default_error = "Module Error: " + mname + " " + fname;
        tb = getPythonErrorTraceback(default_error);
    }
    Py_XDECREF(pyres);
    Py_XDECREF(pyargs);
    Py_XDECREF(func);
    Py_XDECREF(module);
    Py_XDECREF(moduleName);

    PyGILState_Release(gstate);
    EmbeddedPython::m_mutex.unlock();
    return res;
}


// given an existing python object instance, invoke one of its methods 
// grabs mutex to prevent need for Python GIL
QVariant EmbeddedPython::callPyObjMethod(PyObjectPtr &pyobj, 
                                         const QString &methname, 
                                         const QVariantList &args, 
                                         int *rv, 
                                         QString &tb,
                                         bool ret_python_object)
{
    EmbeddedPython::m_mutex.lock();
    PyGILState_STATE gstate = PyGILState_Ensure();

    QVariant  res        = QVariant(QString());
    PyObject* obj        = pyobj.object();
    PyObject* func       = NULL;
    PyObject* pyargs     = NULL;
    PyObject* pyres      = NULL;
    int       idx        = 0;
     
    func = PyObject_GetAttrString(obj,methname.toUtf8().constData());
    if (func == NULL) {
         *rv = -1;
         goto cleanup;
    }

    if (!PyCallable_Check(func)) {
        *rv = -2;
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
        *rv = -3;
        goto cleanup;
    }

    *rv = 0;

    res = PyObjectToQVariant(pyres, ret_python_object);

    cleanup:
    if (PyErr_Occurred() != NULL) {
        QString default_error = "Python Object Method Invocation Error: " + methname;
        tb = getPythonErrorTraceback(default_error);
     }
    Py_XDECREF(pyres);
    Py_XDECREF(pyargs);
    Py_XDECREF(func);

    PyGILState_Release(gstate);
    EmbeddedPython::m_mutex.unlock();
    return res;
}


// *** below here all routines are private and only invoked 
// *** from runInPython and callPyObjMethod with lock held


// Convert PyObject types to their QVariant equivalents 
// call recursively to allow populating QVariant lists and lists of lists
QVariant EmbeddedPython::PyObjectToQVariant(PyObject *po, bool ret_python_object)
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
            res = QVariant(QString::fromLatin1(reinterpret_cast<const char *>(PyUnicode_1BYTE_DATA(po)), -1));

        } else if (kind == PyUnicode_2BYTE_KIND) {
            res = QVariant(QString::fromUtf16(reinterpret_cast<char16_t*>(PyUnicode_2BYTE_DATA(po)), -1));

        } else if (kind == PyUnicode_4BYTE_KIND) {
            // PyUnicode_4BYTE_KIND
            res = QVariant(QString::fromUcs4(reinterpret_cast<char32_t*>(PyUnicode_4BYTE_DATA(po)), -1));
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

    } else if (ret_python_object) {
        QVariant var;
        var.setValue(PyObjectPtr(po));
        res = var;
    } else { 
       // do nothing here to return null value
    }
    return res;
}

// Convert QVariant to a Python Equivalent Type
// call recursively to allow populating tuples/lists and lists of lists
PyObject* EmbeddedPython::QVariantToPyObject(const QVariant &v)
{
    PyObject* value = NULL;
    bool      ok;
    switch (v.typeId()) {
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
            // since QString's utf-16 may contain surrogates or may only be pure ascii we have no easy
            // to know the proper string storage type to use internal to python (latin1, ucs2, ucs4)
            // so punt and create utf-8 and let python handle the conversion internally via its c-api
            value = Py_BuildValue("s", v.toString().toUtf8().constData());
            break;
        case QMetaType::QByteArray:
            value = Py_BuildValue("y", v.toByteArray().constData());
            break;
        case QMetaType::QStringList:
            {
              QStringList vlist = v.toStringList();
              value = PyList_New(vlist.size());
              int pos = 0;
              foreach(QString av, vlist) {
                  // since QString's utf-16 may contain surrogates or may only be pure ascii we have no easy
                  // to know the proper string storage type to use internal to python (latin1, ucs2, ucs4)
                  // so punt and create utf-8 and let python handle the conversion internally via its c-api
                  PyObject* strval = Py_BuildValue("s", av.toUtf8().constData());
                  PyList_SetItem(value, pos, strval);
                  pos++;
               }
            }
            break;
#if 0
         // this does not seem to be well documented or implemented by Qt
         case QMetaType::QVariantPair:
            {
              QVariantPair pi = v.value<QVariantPair>();
              value = PyTuple_New(2);
              PyTuple_SetItem(value, 0, QVariantToPyObject(pi.first));
              PyTuple_SetItem(value, 1, QVariantToPyObject(pi.second));
            }
            break;
#endif
         case QMetaType::QVariantList:
            {
              QVariantList vlist = v.toList();
              value = PyList_New(vlist.size());
              int pos = 0;
              foreach(QVariant av, vlist) {
                  PyList_SetItem(value, pos, QVariantToPyObject(av));
                  pos++;
              }
            }
            break;
        default:
          {
            if (v.typeId() >= QMetaType::User && (v.userType() ==  m_pyobjmetaid)) {
                PyObjectPtr op = v.value<PyObjectPtr>();
                value = op.object();
                // Need to increment object count otherwise will go away when Py_XDECREF used on pyargs
                Py_XINCREF(value);

            } else if (v.typeId() >= QMetaType::User && (v.userType() ==  m_listintmetaid)) {
                QList<int> alist = v.value<QList<int> >();
                value = PyList_New(alist.size());
                int pos = 0;
                foreach(int i, alist) {
                    PyList_SetItem(value, pos, Py_BuildValue("i", i));
                    pos++;
                }
            } else if (v.typeId() >= QMetaType::User && (v.userType() ==  m_stdpairintintmetaid)) {
                std::pair<int,int> pi = v.value<std::pair<int,int> >();
                value = PyTuple_New(2);
                PyTuple_SetItem(value, 0, Py_BuildValue("i", pi.first));
                PyTuple_SetItem(value, 1, Py_BuildValue("i", pi.second));
            } else {
              // Ensure we don't have any holes.
              value = Py_BuildValue("u", "");
            }
          }
          break;
    }
    return value;
}


// get traceback from inside interpreter upon error
QString EmbeddedPython::getPythonErrorTraceback(const QString& default_message, bool useMsgBox)
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
            tblist.append(default_message);
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
