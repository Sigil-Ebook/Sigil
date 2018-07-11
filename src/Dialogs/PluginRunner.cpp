#include <Qt>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProcessEnvironment>

#include "MainUI/MainWindow.h"
#include "MainUI/BookBrowser.h"
#include "Misc/Plugin.h"
#include "Misc/PluginDB.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/TempFolder.h"
#include "Tabs/TabManager.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Dialogs/PluginRunner.h"
#include "sigil_constants.h"

// These are defined in Importers/ImportEPUB.cpp and can be made available by including sigil_constants.h
//const QString ADOBE_FONT_ALGO_ID         = "http://ns.adobe.com/pdf/enc#RC";
//const QString IDPF_FONT_ALGO_ID          = "http://www.idpf.org/2008/embedding";

const QString PluginRunner::SEP = QString(QChar(31));
const QString PluginRunner::OPFFILEINFO = "OEBPS/content.opf" + SEP + SEP + "application/oebps-package+xml";
const QString PluginRunner::NCXFILEINFO = "OEBPS/toc.ncx" + SEP + SEP + "application/x-dtbncx+xml";
const QStringList PluginRunner::CHANGESTAGS = QStringList() << "deleted" << "added" << "modified";


PluginRunner::PluginRunner(TabManager *tabMgr, QWidget *parent)
    : QDialog(parent),
      m_mainWindow(qobject_cast<MainWindow *>(parent)),
      m_tabManager(tabMgr),
      m_outputDir(m_folder.GetPath()),
      m_pluginName(""),
      m_pluginOutput(""),
      m_algorithm(""),
      m_result(""),
      m_xhtml_net_change(0),
      m_ready(false)

{
    // get book manipulation objects
    m_book = m_mainWindow->GetCurrentBook();
    m_bookBrowser = m_mainWindow->GetBookBrowser();
    m_bookRoot = m_book->GetFolderKeeper()->GetFullPathToMainFolder();

    // set default font obfuscation algorithm to use
    // ADOBE_FONT_ALGO_ID or IDPF_FONT_ALGO_ID ??
    QList<Resource *> fonts = m_book->GetFolderKeeper()->GetResourceListByType(Resource::FontResourceType);
    foreach (Resource * resource, fonts) {
        FontResource *font_resource = qobject_cast<FontResource *> (resource);
        QString algorithm = font_resource->GetObfuscationAlgorithm();
        if (!algorithm.isEmpty()) {
            m_algorithm = algorithm;
            break;
        }
    }

    // build hashes of href (book root relative path) to resources
    QList<Resource *> resources = m_book->GetFolderKeeper()->GetResourceList();
    foreach (Resource * resource, resources) {
        QString href = resource->GetRelativePathToRoot();
        if (resource->Type() == Resource::HTMLResourceType) {
            m_xhtmlFiles[href] = resource;
        }
        m_hrefToRes[href] = resource;
    }

    ui.setupUi(this);
    connectSignalsToSlots();
}


PluginRunner::~PluginRunner()
{
}

QStringList PluginRunner::SupportedEngines()
{
    QStringList engines;
    engines << "python3.4" << "python2.7,python3.4" << "python3.4,python2.7";
    return engines;
}

int PluginRunner::exec(const QString &name)
{
    QHash <QString, QStringList> plugininfo;
    PluginDB *pdb = PluginDB::instance();
    Plugin *plugin;
    SettingsStore settings;
    QString launcher_root;

    m_ready = false;

    plugin = pdb->get_plugin(name);
    if (plugin == NULL) {
        Utility::DisplayStdErrorDialog(tr("Error: A plugin by that name does not exist"));
        reject();
        return QDialog::Rejected;
    }

    m_pluginName = name;

    // set up paths and things for the plugin and interpreter
    m_pluginsFolder = PluginDB::pluginsPath();
    m_pluginType = plugin->get_type();
    m_pluginAutoClose = plugin->get_autoclose();

    m_engine = plugin->get_engine();
    // Use the bundled interpreter if user requested it (and plugin supports it)
    QString bundled_interp_path = PluginDB::buildBundledInterpPath();
    if (m_engine.contains("python3.4") && settings.useBundledInterp() && !bundled_interp_path.isEmpty()) {
        m_enginePath = bundled_interp_path;
    }
    else { // Otherwise, parse to find correct external interpreter path
        // handle case of multiple engines
        QStringList engineList;
        if (m_engine.contains(",")) {
            engineList = m_engine.split(",");
        } else {
            engineList.append(m_engine);
        }
        foreach(QString engine, engineList) {
            m_enginePath = pdb->get_engine_path(engine);
            if (!m_enginePath.isEmpty()) break;
        } 
        if (m_enginePath.isEmpty()) {
            Utility::DisplayStdErrorDialog(tr("Error: Interpreter") + " " + m_engine + " " + tr("has no path set"));
            reject();
            return QDialog::Rejected;
        }
    }
    // The launcher and plugin path are both platform specific and engine/interpreter specific
    launcher_root = PluginDB::launcherRoot();

    // Note: Keep SupportedEngines() in sync with the engine calling code here.
    if ( m_engine.contains("python3.4") ) {
        m_launcherPath = launcher_root + "/python/launcher.py";
        m_pluginPath = m_pluginsFolder + "/" + m_pluginName + "/" + "plugin.py";
        if (!QFileInfo(m_launcherPath).exists()) {
            Utility::DisplayStdErrorDialog(tr("Installation Error: plugin launcher") +
                                           " " + m_launcherPath + " " + tr("does not exist"));
            reject();
            return QDialog::Rejected;
        }
    } else {
        Utility::DisplayStdErrorDialog(tr("Error: plugin engine") +
                                       " " + m_engine + " " + tr("is not supported (yet!)"));
        reject();
        return QDialog::Rejected;
    }

    ui.nameLbl->setText(m_pluginName);
    ui.statusLbl->setText(tr("Status: ready"));
    ui.progressBar->setRange(0,100);
    ui.progressBar->reset();
    ui.cancelButton->setEnabled(true);
    ui.showButton->setVisible(false);
    ui.showButton->setEnabled(false);
    m_ready = true;


    // autostart
    if (plugin->get_autostart() == "true") {
        ui.startButton->setVisible(false);
        if (m_pluginAutoClose == "true") {
            ui.showButton->setEnabled(true);
            ui.showButton->setVisible(true);
            ui.textEdit->setVisible(false);
            resize(500, 100);
        }
        QTimer::singleShot(300, ui.startButton, SLOT(click()));
    }
    return QDialog::exec();
}


void PluginRunner::showConsole()
{
    ui.textEdit->setVisible(true);
    ui.showButton->setEnabled(false);
    ui.showButton->setVisible(false);
    resize(789, 550);
}


void PluginRunner::writeSigilCFG()
{
    QStringList cfg = QStringList() << QCoreApplication::applicationDirPath();
    SettingsStore settings;
    cfg << Utility::DefinePrefsDir();
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    cfg << Utility::LinuxHunspellDictionaryDirs().join(":");
#endif
    cfg << settings.uiLanguage();
    cfg << settings.dictionary();
    if (m_mainWindow->isWindowModified()) {
        cfg << QString("True");
    } else {
        cfg << QString("False");
    }
    cfg << m_mainWindow->GetCurrentFilePath();
    QList <Resource *> selected_resources = m_bookBrowser->AllSelectedResources();
    foreach(Resource * resource, selected_resources) {
        cfg << resource->GetRelativePathToRoot();
    }
    Utility::WriteUnicodeTextFile(cfg.join("\n"), m_outputDir + "/sigil.cfg");
}


void PluginRunner::startPlugin()
{
    QStringList args;
    SettingsStore settings;
    if (!m_ready) {
        Utility::DisplayStdErrorDialog(tr("Error: plugin can not start"));
        return;
    }
    ui.textEdit->clear();
    ui.textEdit->setOverwriteMode(true);
    ui.textEdit->setPlainText("");

    // create the sigil cfg file in the output directory
    writeSigilCFG();

    // prepare for the plugin by flushing all current book changes to disk
    m_mainWindow->SaveTabData();
    m_book->GetFolderKeeper()->SuspendWatchingResources();
    m_book->SaveAllResourcesToDisk();
    m_book->GetFolderKeeper()->ResumeWatchingResources();
    ui.startButton->setEnabled(false);
    ui.okButton->setEnabled(false);
    ui.cancelButton->setEnabled(true);

    if (settings.useBundledInterp()) {
        // -E Ignore python ENV vars
        // -O Basic Optimization (also changes the bytecode file extension from .pyc to .pyo)
        // -B Don't write bytecode
        // -u sets python for unbuffered io
#ifdef Q_OS_MAC
        args.append(QString("-EBu"));
#elif defined(Q_OS_WIN32)
        args.append(QString("-Ou"));
#elif !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
        args.append(QString("-Ou"));
#endif
    }
    else {
        args.append(QString("-u"));
    }
    args.append(QDir::toNativeSeparators(m_launcherPath));
    args.append(QDir::toNativeSeparators(m_bookRoot));
    args.append(QDir::toNativeSeparators(m_outputDir));
    args.append(m_pluginType);
    args.append(QDir::toNativeSeparators(m_pluginPath));
    QString executable = QDir::toNativeSeparators(m_enginePath);

#ifdef Q_OS_MAC
    // On Mac OS X, it appears that QProcess does not inherit the callers process environment at all
    // which directly contradicts the Qt documentation.
    // So we simply read the system environment and set it for QProcess manually
    // so that python getpreferredencoding() and stdout/stderr/stdin encodings to get properly set
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (settings.useBundledInterp()) {
        env.insert("QT_PLUGIN_PATH", QDir(QCoreApplication::applicationDirPath() + "/../PlugIns").absolutePath());
        env.insert("QT_QPA_PLATFORM_PLUGIN_PATH", QDir(QCoreApplication::applicationDirPath() + "/../PlugIns/platforms").absolutePath());
    }
    m_process.setProcessEnvironment(env);
#elif defined(Q_OS_WIN32)
    if (settings.useBundledInterp()) {
        // Start with system environment.
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        // Set Python env variables to control how the bundled interpreter finds it's various pieces
        // (and to isolate the bundled interpreter from any system Python).
        // Relative to the interpreter binary to make it easier to relocate the bundled Python.
        env.insert("PYTHONHOME", QDir::toNativeSeparators(QFileInfo(m_enginePath).absolutePath()));
        env.insert("PYTHONIOENCODING", "UTF-8");
        // Remove all other Python environment variables to avoid potential system Python interference.
        // (Windows relevant Python env vars from v3.4 thru v3.6 (no debug build vars))
        QStringList vars_to_unset;
        vars_to_unset << "PYTHONPATH" << "PYTHONOPTIMIZE" << "PYTHONDEBUG" << "PYTHONSTARTUP"
                      << "PYTHONINSPECT" << "PYTHONUNBUFFERED" << "PYTHONVERBOSE" << "PYTHONCASEOK"
                      << "PYTHONDONTWRITEBYTECODE" << "PYTHONHASHSEED" << "PYTHONNOUSERSITE" << "PYTHONUSERBASE"
                      << "PYTHONWARNINGS" << "PYTHONFAULTHANDLER" << "PYTHONTRACEMALLOC" << "PYTHONASYNCIODEBUG"
                      << "PYTHONMALLOC" << "PYTHONMALLOCSTATS" << "PYTHONLEGACYWINDOWSFSENCODING" 
                      << "PYTHONLEGACYWINDOWSIOENCODING";
        foreach(QString envvar, vars_to_unset) {
            env.remove(envvar);
        }
        // Qt5.7+ variable that may interfere in the future.
        env.remove("QT_QPA_PLATFORMTHEME");
        // Replace Qt environment variable with our own (for bundled PyQt5)
        env.insert("QT_QPA_PLATFORM_PLUGIN_PATH", QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/platforms"));
        env.insert("QT_PLUGIN_PATH", QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));
        // Prepend Sigil program directory to PATH so the bundled interpreter
        // can find the correct Qt libs (for PyQt5) and the Python dll.
        env.insert("PATH", QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + PATH_LIST_DELIM + env.value("PATH")));
        // Set bundled Python environment.
        m_process.setProcessEnvironment(env);
        // If launched by another program, the new working directory could mess with how the
        // bundled interpreter finds/loads PyQt5. So set it manually to the bundled interpreter's directory.
        m_process.setWorkingDirectory(QDir::toNativeSeparators(QFileInfo(m_enginePath).absolutePath()));
    }
#elif !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    QString appdir = QCoreApplication::applicationDirPath();
    if (settings.useBundledInterp()) {  // Linux bundled Python settings
        // Start with system environment.
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        // Set Python env variables to control how the bundled interpreter finds it's various pieces
        // (and to isolate the bundled interpreter from any system Python).
        // Relative to the interpreter binary to make it easier to relocate the bundled Python.
        QString pythonhome = QDir::toNativeSeparators(appdir + "/python3");
        env.insert("PYTHONHOME", pythonhome);
        env.insert("PYTHONIOENCODING", "UTF-8");
        // Remove all other Python environment variables to avoid potential system Python interference.
        // (Linux relevant Python env vars from v3.4 thru v3.6 (no debug build vars))
        QStringList vars_to_unset;
        vars_to_unset << "PYTHONPATH" << "PYTHONOPTIMIZE" << "PYTHONDEBUG" << "PYTHONSTARTUP"
                      << "PYTHONINSPECT" << "PYTHONUNBUFFERED" << "PYTHONVERBOSE" << "PYTHONCASEOK"
                      << "PYTHONDONTWRITEBYTECODE" << "PYTHONHASHSEED" << "PYTHONNOUSERSITE" << "PYTHONUSERBASE"
                      << "PYTHONWARNINGS" << "PYTHONFAULTHANDLER" << "PYTHONTRACEMALLOC" << "PYTHONASYNCIODEBUG"
                      << "PYTHONMALLOC" << "PYTHONMALLOCSTATS";
        foreach(QString envvar, vars_to_unset) {
            env.remove(envvar);
        }
        // Qt5.7+ variable that may interfere in the future.
        env.remove("QT_QPA_PLATFORMTHEME");
        // Replace Qt environment variables with our own (for bundled PyQt5)
        env.insert("QT_QPA_PLATFORM_PLUGIN_PATH", QDir::toNativeSeparators(appdir + "/platforms"));
        env.insert("QT_PLUGIN_PATH", appdir);
        // Prepend Sigil program directory to LD_LIBRARY_PATH so the bundled interpreter
        // can find the included Qt libs (for PyQt5) and the Python dll.
        QStringList ld = env.value("LD_LIBRARY_PATH", "").split(PATH_LIST_DELIM);
        // Make sure Sigil's appdir appears only once ... and first.
        ld.removeAll(appdir);
        ld.prepend(appdir);
        // Reset modified LD_LIBRARY_PATH
        env.insert("LD_LIBRARY_PATH", ld.join(PATH_LIST_DELIM));
        // Set bundled Linux Python interpreter environment.
        m_process.setProcessEnvironment(env);
        // If launched by another program (calibre), the new working directory could mess with how the
        // bundled interpreter finds/loads PyQt5. So set it manually to the bundled interpreter's directory.
        m_process.setWorkingDirectory(QDir::toNativeSeparators(QFileInfo(m_enginePath).absolutePath()));
    }
    else {  // Linux native Python settings
        // Start with system environment.
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        // Remove Sigil's appdir from LD_LIBRARY_PATH if it exists in environment so system Python can run unimpeded.
        QStringList ld = env.value("LD_LIBRARY_PATH", "").split(PATH_LIST_DELIM);
        ld.removeAll(appdir);
        // Reset modified LD_LIBRARY_PATH or unset if empty
        if (!ld.count()==0) {
            env.insert("LD_LIBRARY_PATH", ld.join(PATH_LIST_DELIM));
        }
        else {
            env.remove("LD_LIBRARY_PATH");
        }
        // Set native Linux Python interpreter environment.
        m_process.setProcessEnvironment(env);
    }
#endif

    m_process.start(executable, args);
    ui.statusLbl->setText(tr("Status: running"));

    // this starts the infinite progress bar
    ui.progressBar->setRange(0,0);
}


void PluginRunner::processOutput()
{
    QByteArray newbytedata = m_process.readAllStandardOutput();
    ui.textEdit->insertPlainText(newbytedata);
    m_pluginOutput = m_pluginOutput + newbytedata;
}

void PluginRunner::pluginFinished(int exitcode, QProcess::ExitStatus exitstatus)
{
    if (exitstatus == QProcess::CrashExit) {
        ui.textEdit->append(tr("Launcher process crashed"));
    }
    // launcher exiting properly does not mean target plugin succeeded or failed
    // we need to parse the response xml to find the true result of target plugin
    ui.okButton->setEnabled(true);
    ui.cancelButton->setEnabled(false);

    // this stops the progress bar at full
    ui.progressBar->setRange(0,100);
    ui.progressBar->setValue(100);

    ui.statusLbl->setText(tr("Status: finished"));

    if (!processResultXML()) {
        ui.textEdit->append(m_pluginOutput);
        return;
    }
    if (m_result != "success") {
        ui.statusLbl->setText(tr("Status: failed"));
        return;
    }
    else {
        ui.statusLbl->setText(tr("Status: success"));
        return;
    }	    

    // before modifying xhtml files make sure they are well formed
    if (!checkIsWellFormed()) {
        ui.statusLbl->setText(tr("Status: No Changes Made"));
        return;
    }

    // don't allow changes to proceed if they will remove the very last xhtml/html file
    if (m_xhtml_net_change < 0) {
        QList<Resource *> htmlresources = m_book->GetFolderKeeper()->GetResourceListByType(Resource::HTMLResourceType);
        if (htmlresources.count() + m_xhtml_net_change < 0) {
            Utility::DisplayStdErrorDialog(tr("Error: Plugin Tried to Remove the Last XHTML file .. aborting changes"));
            ui.statusLbl->setText(tr("Status: No Changes Made"));
            return;
        }
    }

    // everthing looks good so now make any necessary changes
    bool book_modified = false;

    m_book->GetFolderKeeper()->SuspendWatchingResources();

    if (!m_filesToAdd.isEmpty()) {
        if (addFiles(m_filesToAdd)) {
            book_modified = true;
        }
    }
    if (!m_filesToDelete.isEmpty()) {
        // before deleting make sure a tab of at least one of the remaining html files will be open
        // to prevent deleting the last tab when deleting resources
        QList <Resource *> remainingResources = m_xhtmlFiles.values();
        QList <Resource *> tabResources = m_tabManager->GetTabResources();
        bool tabs_will_remain = false;
        foreach (Resource * tab_resource, tabResources) {
            if (remainingResources.contains(tab_resource)) {
                tabs_will_remain = true;
                break;
            }
        }
        if (!tabs_will_remain) {
            Resource *xhtmlresource = remainingResources.at(0);
            m_mainWindow->OpenResource(xhtmlresource);
        }

        if (deleteFiles(m_filesToDelete)) {
            book_modified = true;
        }
    }
    if (!m_filesToModify.isEmpty()) {
        if (modifyFiles(m_filesToModify)) {
            book_modified = true;
        }
    }
    if (m_pluginType == "validation") {
        m_mainWindow->SetValidationResults(m_validationResults);
    }

    // now make these changes known to Sigil
    m_book->GetFolderKeeper()->ResumeWatchingResources();

#ifdef Q_OS_MAC
    // On OS X a new window with the book is opened. The current one's content is not
    // replaced so we don't want to set it as modified if it's an input plugin.
    if (m_pluginType != "input") {
#endif
        if (book_modified) {
            m_bookBrowser->BookContentModified();
            m_bookBrowser->Refresh();
            m_book->SetModified();
            // clearMemoryCaches() and updates current tab
            m_mainWindow->ResourcesAddedOrDeleted();
        }
#ifdef Q_OS_MAC
    }
#endif
    ui.statusLbl->setText(tr("Status:") + " " + m_result);

    // Validation plugins we auto close the plugin runner dialog
    // since they'll see the results in the results panel.
    //
    // XXX: Technically we're only checking if validation results
    // were checked. A plugin could do other things and set validation
    // results too. We really should check that everything else a
    // plugin can set is really empty before calling accept because
    // it could have actual info the user needs to see in the dialog.
    if ((m_pluginType == "validation") || (m_pluginAutoClose == "true")) {
        accept();
        return;
    }
}


void PluginRunner::processError()
{
    QByteArray newbytedata = m_process.readAllStandardError();
    ui.textEdit->append(newbytedata);
}


void PluginRunner::processError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        ui.textEdit->append(tr("Plugin failed to start"));
    }
    ui.okButton->setEnabled(true);
    ui.cancelButton->setEnabled(false);

    ui.progressBar->setRange(0,100);
    ui.progressBar->reset();

    ui.statusLbl->setText(tr("Status: error"));
}


void PluginRunner::cancelPlugin()
{
    if (m_process.state() == QProcess::Running) {
        m_process.kill();
    }
    ui.okButton->setEnabled(true);

    ui.progressBar->setRange(0,100);
    ui.progressBar->reset();

    ui.textEdit->append(tr("Plugin cancelled"));
    ui.statusLbl->setText(tr("Status: cancelled"));
    ui.cancelButton->setEnabled(false);
}

bool PluginRunner::processResultXML()
{
    // ignore any extraneous information before wrapper xml at the end
    int start_pos = m_pluginOutput.indexOf("<?xml ");
    while (start_pos != -1) {
        m_pluginOutput =  m_pluginOutput.mid(start_pos, -1);
        start_pos = m_pluginOutput.indexOf("<?xml ", 1);
    }
    QXmlStreamReader reader(m_pluginOutput);
    reader.setNamespaceProcessing(false);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            if (name == "result") {
                QString result = reader.readElementText();
                m_result = result;
                ui.textEdit->setPlainText(tr("Status:") + " " + result);
            } else if (name == "msg") {
                QString msg = reader.readElementText();
                ui.textEdit->append(msg);
            } else if (CHANGESTAGS.contains(name)) {
                QStringList info;
                QString href;
                QString id;
                QString mime;
                QXmlStreamAttributes attr = reader.attributes();
                href = Utility::URLDecodePath(attr.value("href").toString());
                id = attr.value("id").toString();
                mime =  attr.value("media-type").toString();
                info << href << id << mime;
                QString fileinfo = info.join(SEP);
                if (reader.name() == "deleted") {
                    m_filesToDelete.append(fileinfo);
                    if (mime == "application/xhtml+xml") {
		        // only count deleting xhtml files that are 
		        // currently resources (skip unmanifested files)
                        if (m_xhtmlFiles.contains(href)) {
                            m_xhtml_net_change--;
                            m_xhtmlFiles.remove(href);
                        }
                    }
                } else if (reader.name() == "added") {
                    m_filesToAdd.append(fileinfo);
                    if (mime == "application/xhtml+xml") {
                        m_xhtml_net_change++;
                    }
                } else {
                    m_filesToModify.append(fileinfo);
                }
            } else if (name == "validationresult") {
                QXmlStreamAttributes attr = reader.attributes();

                QString type;
                ValidationResult::ResType vtype;
                type = attr.value("type").toString();
                if (type == "info") {
                    vtype = ValidationResult::ResType_Info;
                } else if (type == "warning") {
                    vtype = ValidationResult::ResType_Warn;
                } else if (type == "error") {
                    vtype = ValidationResult::ResType_Error;
                } else {
                    continue;
                }

                QString linenumber;
                bool   line_ok;
                int vlinenumber = -1;
                linenumber = attr.value("linenumber").toString();
                vlinenumber = linenumber.toInt(&line_ok);
                if (!line_ok) {
                    vlinenumber = -1;
                }

                QString charoffset;
                bool   coff_ok;
                int vcharoffset = -1;
                charoffset = attr.value("charoffset").toString();
                vcharoffset = charoffset.toInt(&coff_ok);
                if (!coff_ok) {
                    vcharoffset = -1;
                }

                m_validationResults.append(ValidationResult(vtype, attr.value("filename").toString(), vlinenumber, vcharoffset, attr.value("message").toString()));
            }
        }
    }
    if (reader.hasError()) {
        Utility::DisplayStdErrorDialog(tr("Error Parsing Result XML:  ") + reader.errorString());
        return false;
    }
    return true;
}



bool PluginRunner::checkIsWellFormed()
{
    bool well_formed = true;
    bool proceed = true;
    QStringList errors;
    // Build of list of xhtml, html, and xml files that were modifed or added
    QStringList xhtmlFilesToCheck;
    QStringList xmlFilesToCheck;
    if (!m_filesToAdd.isEmpty()) {
        foreach (QString fileinfo, m_filesToAdd) {
            QStringList fdata = fileinfo.split(SEP);
            QString href = fdata[ hrefField ];
            QString id   = fdata[ idField   ];
            QString mime = fdata[ mimeField ];
            if (mime == "application/oebps-package+xml") {
                xmlFilesToCheck.append(href);
            } else if (mime == "application/x-dtbncx+xml") {
                xmlFilesToCheck.append(href);
            } else if (mime == "application/oebs-page-map+xml") {
                xmlFilesToCheck.append(href);
            } else if (mime == "application/xhtml+xml") {
                xhtmlFilesToCheck.append(href);
            } else if (mime == "application/smil+xml") {
                xmlFilesToCheck.append(href);
            }
        }
    }
    if (!m_filesToModify.isEmpty()) {
        foreach (QString fileinfo, m_filesToModify) {
            QStringList fdata = fileinfo.split(SEP);
            QString href = fdata[ hrefField ];
            QString id   = fdata[ idField   ];
            QString mime = fdata[ mimeField ];
            if (mime == "application/oebps-package+xml") {
                xmlFilesToCheck.append(href);
            } else if (mime == "application/x-dtbncx+xml") {
                xmlFilesToCheck.append(href);
            } else if (mime == "application/oebs-page-map+xml") {
                xmlFilesToCheck.append(href);
            } else if (mime == "application/xhtml+xml") {
                xhtmlFilesToCheck.append(href);
            } else if (mime == "application/smil+xml") {
                xmlFilesToCheck.append(href);
            }
        }
    }
    if (!xhtmlFilesToCheck.isEmpty()) {
        foreach (QString href, xhtmlFilesToCheck) {
            QString filePath = m_outputDir + "/" + href;
            ui.statusLbl->setText(tr("Status: checking") + " " + href);
            QString data = Utility::ReadUnicodeTextFile(filePath);
            XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(data);
            if (error.line != -1) {
                errors.append(tr("Incorrect XHTML:") + " " + href + " " + tr("Line/Col") + " " + QString::number(error.line) +
                              "," + QString::number(error.column) + " " + error.message);
                well_formed = false;
            }
        }
    }
    if (!xmlFilesToCheck.isEmpty()) {
        foreach (QString href, xmlFilesToCheck) {
            // can't really validate without a full dtd so
            // auto repair any xml file changes to be safe
            QString filePath = m_outputDir + "/" + href;
            ui.statusLbl->setText(tr("Status: checking") + " " + href);
            QString mtype = "application/oebs-page-map+xml";
            if (href.endsWith(".opf")) mtype = "application/oebps-package+xml";
            if (href.endsWith(".ncx")) mtype = "application/x-dtbncx+xml";
            if (href.endsWith(".smil")) mtype = "application/oebps-package+xml";
            QString data = Utility::ReadUnicodeTextFile(filePath);
            QString newdata = CleanSource::ProcessXML(data, mtype);
            Utility::WriteUnicodeTextFile(newdata, filePath);
        }
    }
    if ((!well_formed) && (!errors.isEmpty())) {
        // Throw Up a Dialog to See if they want to proceed
        proceed = false;
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        msgBox.setWindowTitle(tr("Check Report"));
        msgBox.setText(tr("Incorrect XHTML/XML Detected\nAre you Sure You Want to Continue?"));
        msgBox.setDetailedText(errors.join("\n"));
        QPushButton *yesButton = msgBox.addButton(QMessageBox::Yes);
        QPushButton *noButton =  msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(noButton);
        msgBox.exec();
        if (msgBox.clickedButton() == yesButton) {
            proceed = true;
        }
    }
    return proceed;
}


bool PluginRunner::deleteFiles(const QStringList &files)
{
    QList <Resource *> tabResources=m_tabManager->GetTabResources();
    bool changes_made = false;
    ui.statusLbl->setText(tr("Status: cleaning up - deleting files"));
    foreach (QString fileinfo, files) {
        QStringList fdata = fileinfo.split(SEP);
        QString href = fdata[ hrefField ];
        QString id   = fdata[ idField   ];
        QString mime = fdata[ mimeField ];
        // opf and ncx files can not be added or deleted
	// if they are current resources
        if (mime == "application/oebps-package+xml") {
	    if (m_hrefToRes.contains(href)) {
                continue;
	    }
        }
        if (mime == "application/x-dtbncx+xml") {
	    if (m_hrefToRes.contains(href)) {
                continue;
	    }
        }
        // under epub3 the nav can not be deleted either
        Resource * nav_resource = m_book->GetConstOPF()->GetNavResource();
        Resource *resource = m_hrefToRes.value(href, NULL);
        if (nav_resource && (nav_resource == resource)) {
            continue;
        }
        if (resource) {
            ui.statusLbl->setText(tr("Status: deleting") + " " + resource->Filename());

            if (tabResources.contains(resource)) {
                m_tabManager->CloseTabForResource(resource);
            }
            m_book->GetFolderKeeper()->RemoveResource(resource);
            resource->Delete();
            changes_made = true;
        } else {
           // try to remove non-manifested, non-resource files inside book folder
           // force it to be inside book root for safety
           QString fullpath = "/" + href;
           fullpath.replace("/../","/");
           fullpath = m_bookRoot + fullpath;
           if (Utility::SDeleteFile(fullpath)) {
               changes_made = true;
           }
        }
    }
    if (changes_made) {
        m_bookBrowser->ResourcesDeleted();
    }
    return changes_made;
}


bool PluginRunner::addFiles(const QStringList &files)
{
    ui.statusLbl->setText("Status: adding files");
    foreach (QString fileinfo, files) {
        QStringList fdata = fileinfo.split(SEP);
        QString href = fdata[ hrefField ];
        QString id   = fdata[ idField   ];
        QString mime = fdata[ mimeField ];

        // handle input plugin
        if (m_pluginType == "input" && mime == "application/epub+zip") {
            QString epubPath = m_outputDir + "/" + href;
            QFileInfo fi(epubPath);
            ui.statusLbl->setText(tr("Status: Loading") + " " + fi.fileName());
#ifdef Q_OS_MAC
            MainWindow *new_window = new MainWindow(epubPath, true);
            new_window->show();
#else
            // For Linux and Windows will replace current book
            // So Throw Up a Dialog to See if they want to proceed
            bool proceed = false;
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
            msgBox.setWindowTitle(tr("Input Plugin"));
            msgBox.setText(tr("Your current book will be completely replaced losing any unsaved changes ...  Are you sure you want to proceed"));
            QPushButton *yesButton = msgBox.addButton(QMessageBox::Yes);
            QPushButton *noButton =  msgBox.addButton(QMessageBox::No);
            msgBox.setDefaultButton(noButton);
            msgBox.exec();
            if (msgBox.clickedButton() == yesButton) {
                proceed = true;
            }
            if (proceed) {
                m_mainWindow->LoadFile(epubPath, true);
            }
#endif
            return true;
        }

        // content.opf and toc.ncx can not be added or deleted
        if (mime == "application/oebps-package+xml") {
            continue;
        }
        if (mime == "application/x-dtbncx+xml") {
            continue;
        }

        // No need to copy to ebook root as AddContentToFolder does that for us
        QString inpath = m_outputDir + "/" + href;
        QFileInfo fi(inpath);
        ui.statusLbl->setText(tr("Status: adding") + " " + fi.fileName());

        Resource *resource = m_book->GetFolderKeeper()->AddContentFileToFolder(inpath,false);

        // AudioResource, VideoResource, FontResource, ImageResource do not appear to be cached

        // For new Editable Resources must do the equivalent of the InitialLoad
        // Order is important as some resource types inherit from other resource types

        // NOTE! AddContentFileToFolder returns a resource reference and not a pointer

        if (resource->Type() == Resource::FontResourceType && !m_algorithm.isEmpty()) {
            FontResource *font_resource = qobject_cast<FontResource *>(resource);
            font_resource->SetObfuscationAlgorithm(m_algorithm);
        } else  if (resource->Type() == Resource::HTMLResourceType) {
            HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
            html_resource->SetText(Utility::ReadUnicodeTextFile(inpath));
            // remember to add this new file to the list of remaining xhtml resources
            QString href = resource->GetRelativePathToRoot();
            m_xhtmlFiles[href] = resource;
            m_hrefToRes[href] = resource;
        } else if (resource->Type() == Resource::CSSResourceType) {
            CSSResource *css_resource = qobject_cast<CSSResource *>(resource);
            css_resource->SetText(Utility::ReadUnicodeTextFile(inpath));
        } else if (resource->Type() == Resource::SVGResourceType) {
            SVGResource *svg_resource = qobject_cast<SVGResource *>(resource);
            svg_resource->SetText(Utility::ReadUnicodeTextFile(inpath));
        } else if (resource->Type() == Resource::MiscTextResourceType) {
            MiscTextResource *misctext_resource = qobject_cast<MiscTextResource *>(resource);
            misctext_resource->SetText(Utility::ReadUnicodeTextFile(inpath));
        } else if (resource->Type() == Resource::XMLResourceType) {
            XMLResource *xml_resource = qobject_cast<XMLResource *>(resource);
            xml_resource->SetText(Utility::ReadUnicodeTextFile(inpath));
        }
    }
    return true;
}


bool PluginRunner::modifyFiles(const QStringList &files)
{
    ui.statusLbl->setText(tr("Status: cleaning up - modifying files"));
    // rearrange list to force content.opf and toc.ncx modifications to be done last
    QStringList newfiles;
    QString modifyopf;
    QString modifyncx;
    foreach (QString fileinfo, files) {
        if (fileinfo == OPFFILEINFO) {
            modifyopf = fileinfo;
        } else if (fileinfo == NCXFILEINFO) {
            modifyncx = fileinfo;
        } else {
            newfiles.append(fileinfo);
        }
    }
    if  (!modifyopf.isEmpty()) {
        newfiles.append(modifyopf);
    }
    if  (!modifyncx.isEmpty()) {
        newfiles.append(modifyncx);
    }

    foreach (QString fileinfo, newfiles) {
        QStringList fdata = fileinfo.split(SEP);
        QString href = fdata[ hrefField ];
        QString id   = fdata[ idField   ];
        QString mime = fdata[ mimeField ];
        QString inpath = m_outputDir + "/" + href;
        QString outpath = m_bookRoot + "/" + href;
        QFileInfo fi(outpath);
        ui.statusLbl->setText(tr("Status: modifying") + " " + fi.fileName());
        Utility::ForceCopyFile(inpath, outpath);
        Resource *resource = m_hrefToRes.value(href);
        if (resource) {

            // AudioResource, VideoResource, FontResource, ImageResource do not appear to be editable

            // For Editable Resources must relaod them from modified file
            // Order below is important as some resouirce types inherit from other resource types

            if (resource->Type() == Resource::HTMLResourceType) {

                HTMLResource *html_resource = qobject_cast<HTMLResource *> (resource);
                html_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

            } else if (resource->Type() == Resource::CSSResourceType) {

                CSSResource *css_resource = qobject_cast<CSSResource *> (resource);
                css_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

            } else if (resource->Type() == Resource::SVGResourceType) {

                SVGResource *svg_resource = qobject_cast<SVGResource *> (resource);
                svg_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

            } else if (resource->Type() == Resource::MiscTextResourceType) {

                MiscTextResource *misctext_resource = qobject_cast<MiscTextResource *> (resource);
                misctext_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

            } else if (resource->Type() == Resource::OPFResourceType) {

                OPFResource *opf_resource = qobject_cast<OPFResource *> (resource);
                opf_resource->SetText(Utility::ReadUnicodeTextFile(outpath));

            } else if (resource->Type() == Resource::NCXResourceType) {

                NCXResource *ncx_resource = qobject_cast<NCXResource *> (resource);
                ncx_resource->SetText(Utility::ReadUnicodeTextFile(outpath));

            } else if (resource->Type() == Resource::XMLResourceType) {

                XMLResource *xml_resource = qobject_cast<XMLResource *> (resource);
                xml_resource->SetText(Utility::ReadUnicodeTextFile(inpath));
            }
        }
    }
    return true;
}

void PluginRunner::connectSignalsToSlots()
{
    connect(ui.startButton, SIGNAL(clicked()), this, SLOT(startPlugin()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelPlugin()));
    connect(ui.showButton, SIGNAL(clicked()), this, SLOT(showConsole()));
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(pluginFinished(int, QProcess::ExitStatus)));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
    connect(&m_process, SIGNAL(readyReadStandardError()), this, SLOT(processError()));
    connect(&m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
}
