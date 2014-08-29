#include <Qt>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QMessageBox>
#include "MainUI/MainWindow.h"
#include "MainUI/BookBrowser.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/TempFolder.h"
#include "Tabs/TabManager.h"
#include "BookManipulation/XhtmlDoc.h"
#include "PluginRunner.h"

const QString ADOBE_FONT_ALGO_ID         = "http://ns.adobe.com/pdf/enc#RC";
const QString IDPF_FONT_ALGO_ID          = "http://www.idpf.org/2008/embedding";

const QString PluginRunner::SEP = QString(QChar(31));
const QString PluginRunner::OPFFILEINFO = "OEBPS/content.opf" + SEP + SEP + "application/oebps-package+xml";
const QString PluginRunner::NCXFILEINFO = "OEBPS/toc.ncx" + SEP + SEP + "application/x-dtbncx+xml";
const QStringList PluginRunner::CHANGESTAGS = QStringList() << "deleted" << "added" << "modified";


PluginRunner::PluginRunner(TabManager* tabMgr, QWidget * parent)
    : QDialog(parent),
    m_pluginName(""), 
    m_outputDir(m_folder.GetPath()),
    m_pluginOutput(""),
    m_algorithm(""),
    m_result(""),
    m_xhtml_net_change(0),
    m_mainWindow(qobject_cast<MainWindow *>(parent)),
    m_tabManager(tabMgr),
    m_ready(false)

{
    // get book manipulation objects
    m_book = m_mainWindow->GetCurrentBook();
    m_bookBrowser = m_mainWindow->GetBookBrowser();
    m_bookRoot = m_book->GetFolderKeeper().GetFullPathToMainFolder();

    // set default font obfuscation algorithm to use
    // ADOBE_FONT_ALGO_ID or IDPF_FONT_ALGO_ID ??
    QList< Resource * > fonts = m_book->GetFolderKeeper().GetResourceListByType(Resource::FontResourceType);
    foreach (Resource * resource, fonts) {
        FontResource * font_resource = qobject_cast< FontResource * > (resource);
        QString algorithm = font_resource->GetObfuscationAlgorithm();
        if (!algorithm.isEmpty()) {
            m_algorithm = algorithm;
            break;
        }
    }

    // build hashes of href (book root relative path) to resources
    QList< Resource * > resources = m_book->GetFolderKeeper().GetResourceList();
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

int PluginRunner::exec(const QString &name)
{
    QHash <QString, QStringList> plugininfo;
    QHash <QString, QString> enginepath;
    QStringList fields;
    SettingsStore settings;
    QString launcher_root;

    m_ready = false;
    m_pluginName = name;

    // get plugin settings from SettingsStore
    enginepath = settings.pluginEnginePaths();
    plugininfo = settings.pluginInfo();
    if (! plugininfo.keys().contains(m_pluginName)) {
        Utility::DisplayStdErrorDialog("Error: A plugin by that name does not exist");
        reject();
        return QDialog::Rejected;
    }

    // set up paths and things for the plugin and interpreter
    m_pluginsFolder = pluginsPath();
    fields = plugininfo[m_pluginName];
    m_engine = fields.at(EngineField);
    m_pluginType = fields.at(TypeField);
    m_enginePath = enginepath[m_engine];
    if (m_enginePath.isEmpty()) {
        Utility::DisplayStdErrorDialog("Error: Interpreter " + m_engine + " has no path set");
        reject();
        return QDialog::Rejected;
    }

    // The launcher and plugin path are both platform specific and engine/interpreter specific 
    launcher_root = QCoreApplication::applicationDirPath(); 

#ifdef Q_OS_MAC
    launcher_root += "/../plugin_launchers/";
#elif defined(Q_OS_WIN32)
    launcher_root += "/plugin_launchers/";
#elif !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // all flavours of linux / unix
    launcher_root += "/../share/" + QCoreApplication::applicationName().toLower() + "/plugin_launchers/";
    // user supplied environment variable to plugin launcher directory will overrides everything
    const QString env_launcher_location = QString(getenv("SIGIL_PLUGIN_LAUNCHERS"));
    if (!env_launcher_location.isEmpty()) {
        launcher_root = env_launcher_location + "/";
    }
#endif

    if (m_engine == "python2.7") {
        m_launcherPath = launcher_root + "/python2_7/launcher.py";
        m_pluginPath = m_pluginsFolder + "/" + m_pluginName + "/" + m_pluginName + ".py";
        if (!QFileInfo(m_launcherPath).exists()) {
            Utility::DisplayStdErrorDialog("Installation Error: plugin launcher " + m_launcherPath + " does not exist");
            reject();
            return QDialog::Rejected;
        }
    } else if (m_engine == "lua5.2") {
        m_launcherPath = launcher_root + "/lua5_2/launcher.lua";
        m_pluginPath = m_pluginsFolder + "/" + m_pluginName + "/" + m_pluginName + ".lua";
        if (!QFileInfo(m_launcherPath).exists()) {
            Utility::DisplayStdErrorDialog("Installation Error: plugin launcher " + m_launcherPath + " does not exist");
            reject();
            return QDialog::Rejected;
        }
    } else {
        Utility::DisplayStdErrorDialog("Error: plugin engine " + m_engine + " is not supported (yet!)");
        reject();
        return QDialog::Rejected;
    }

    ui.nameLbl->setText(m_pluginName);
    ui.statusLbl->setText("Status: ready");
    ui.progressBar->setRange(0,100);
    ui.progressBar->reset();
    ui.cancelButton->setEnabled(true);
    m_ready = true;

    return QDialog::exec();
}

QString PluginRunner::pluginsPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/plugins";
}

void PluginRunner::startPlugin() 
{
    QStringList args;
    if (!m_ready) {
        Utility::DisplayStdErrorDialog("Error: plugin can not start");
        return;
    }
    ui.textEdit->clear();
    ui.textEdit->setOverwriteMode(true);
    ui.textEdit->setPlainText("");

    // prepare for the plugin by flushing all current book changes to disk
    m_mainWindow->SaveTabData();
    m_book->GetFolderKeeper().SuspendWatchingResources();
    m_book->SaveAllResourcesToDisk();
    m_book->GetFolderKeeper().ResumeWatchingResources();
    ui.startButton->setEnabled(false);
    ui.okButton->setEnabled(false);
    ui.cancelButton->setEnabled(true);


    args.append(QDir::toNativeSeparators(m_launcherPath));
    args.append(QDir::toNativeSeparators(m_bookRoot));
    args.append(QDir::toNativeSeparators(m_outputDir));
    args.append(m_pluginType);
    args.append(QDir::toNativeSeparators(m_pluginPath));
    QString executable = QDir::toNativeSeparators(m_enginePath);

    m_process.start(executable, args);
    ui.statusLbl->setText("Status: running");

    // this starts the infinite progress bar
    ui.progressBar->setRange(0,0);
}


void PluginRunner::processOutput() 
{
    QByteArray newbytedata = m_process.readAllStandardOutput();
    m_pluginOutput = m_pluginOutput + newbytedata ;
}


void PluginRunner::pluginFinished(int exitcode, QProcess::ExitStatus exitstatus) 
{
    if (exitstatus == QProcess::CrashExit) {
        ui.textEdit->append("Launcher process crashed");
    } 
    // launcher exiting properly does not mean target plugin succeeded or failed
    // we need to parse the response xml to find the true result of target plugin
    ui.okButton->setEnabled(true);
    ui.cancelButton->setEnabled(false);

    // this stops the progress bar at full
    ui.progressBar->setRange(0,100);
    ui.progressBar->setValue(100);

    ui.statusLbl->setText("Status: finished");

    if (!processResultXML()) {
        return;
    }
    if (m_result != "success") {
        return;
    }

    // before modifying xhtmnl files make sure they are well formed
    if (! checkIsWellFormed() ) {
        ui.statusLbl->setText("Status: No Changes Made");
        return;
    }

    // don't allow changes to proceed if they will remove the very last xhtml/html file
    if (m_xhtml_net_change < 0) {
        QList< Resource * > htmlresources = m_book->GetFolderKeeper().GetResourceListByType(Resource::HTMLResourceType);
        if (htmlresources.count() + m_xhtml_net_change < 0) {
            Utility::DisplayStdErrorDialog("Error: Plugin Tried to Remove the Last XHTML file .. aborting changes");
            ui.statusLbl->setText("Status: No Changes Made");
            return;
        }
    }

    // everthing looks good so now make any necessary changes
    bool book_modified = false;

    m_book->GetFolderKeeper().SuspendWatchingResources();

    if (!m_filesToDelete.isEmpty()) {

        // before deleting make sure a tab of at least one of the remaining html files will be open
        // to prevent deleting the last tab when deleting resources
        QList < Resource * > remainingResources = m_xhtmlFiles.values();
        QList <Resource *> tabResources = m_tabManager->GetTabResources();
        bool tabs_will_remain = false;
        foreach (Resource * tab_resource, tabResources) {
            if (remainingResources.contains(tab_resource)) {
                tabs_will_remain = true;
                break;
            }
        }
        if (! tabs_will_remain) {
            Resource * xhtmlresource = remainingResources.at(0);
            m_mainWindow->OpenResource(*xhtmlresource);
        }
        if (deleteFiles(m_filesToDelete)) {
            book_modified = true;
        }
    }
    if (!m_filesToAdd.isEmpty()) {
        if (addFiles(m_filesToAdd)) {
            book_modified = true;
        }
    }
    if (!m_filesToModify.isEmpty()) {
        if (modifyFiles(m_filesToModify)) {
            book_modified = true;
        }
    }

    // now make these changes known to Sigil
    m_book->GetFolderKeeper().ResumeWatchingResources();
    if (book_modified) {
        m_bookBrowser->BookContentModified();
        m_bookBrowser->Refresh();
        m_book->SetModified();
        // QWebSettings::clearMemoryCaches() and updates current tab
        m_mainWindow->ResourcesAddedOrDeleted();
    }
    ui.statusLbl->setText("Status: " + m_result);
}


void PluginRunner::processError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        ui.textEdit->append("Plugin failed to start");
    }
    ui.okButton->setEnabled(true);
    ui.cancelButton->setEnabled(false);

    ui.progressBar->setRange(0,100);
    ui.progressBar->reset();

    ui.statusLbl->setText("Status: error");
}


void PluginRunner::cancelPlugin()
{
    if (m_process.state() == QProcess::Running) {
        m_process.kill();
    }
    ui.okButton->setEnabled(true);

    ui.progressBar->setRange(0,100);
    ui.progressBar->reset();

    ui.textEdit->append("Plugin cancelled");
    ui.statusLbl->setText("Status: cancelled");
    ui.cancelButton->setEnabled(false);
}

bool PluginRunner::processResultXML()
{
    QXmlStreamReader reader(m_pluginOutput);
    reader.setNamespaceProcessing(false);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            QString name = reader.name().toString(); 
            if (name == "result") {
                QString result = reader.readElementText();
                m_result = result;
                ui.textEdit->setPlainText("Status: " + result);
            } else if (name == "msg") {
                QString msg = reader.readElementText();
                ui.textEdit->append(msg);
            } else if (CHANGESTAGS.contains(name)) {
                QStringList info;
                QString href;
                QString id;
                QString mime;
                QXmlStreamAttributes attr = reader.attributes();
                href = attr.value("href").toString();
                id = attr.value("id").toString();
                mime =  attr.value("media-type").toString();
                info << href << id << mime;
                QString fileinfo = info.join(SEP);
                if (reader.name() == "deleted") {
                    m_filesToDelete.append(fileinfo);
                    if (mime == "application/xhtml+xml") {
                        m_xhtml_net_change--;
                        if (m_xhtmlFiles.contains(href)) {
                            m_xhtmlFiles.remove(href);
                        }
                    }
                } else if (reader.name() == "added") {
                    m_filesToAdd.append(fileinfo);
                    if (mime == "application/xhtml+xml") m_xhtml_net_change++;
                } else {
                    m_filesToModify.append(fileinfo);
                }
            }
        }
    }
    if (reader.hasError()) {
        Utility::DisplayStdErrorDialog("Error Parsing Result XML:  " + reader.errorString());
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
    QStringList filesToCheck;
    if (!m_filesToAdd.isEmpty()) {
        foreach (QString fileinfo, m_filesToAdd) {
            QStringList fdata = fileinfo.split(SEP);
            QString href = fdata[ hrefField ];
            QString id   = fdata[ idField   ];
            QString mime = fdata[ mimeField ];
            if (mime == "application/oebps-package+xml")      filesToCheck.append(href);
            else if (mime == "application/x-dtbncx+xml")      filesToCheck.append(href);
            else if (mime == "application/oebs-page-map+xml") filesToCheck.append(href);
            else if (mime == "application/xhtml+xml")         filesToCheck.append(href);
        }
    }
    if (!m_filesToModify.isEmpty()) {
        foreach (QString fileinfo, m_filesToModify) {
            QStringList fdata = fileinfo.split(SEP);
            QString href = fdata[ hrefField ];
            QString id   = fdata[ idField   ];
            QString mime = fdata[ mimeField ];
            if (mime == "application/oebps-package+xml")     filesToCheck.append(href);
            else if (mime == "application/x-dtbncx+xml")      filesToCheck.append(href);
            else if (mime == "application/oebs-page-map+xml") filesToCheck.append(href);
            else if (mime == "application/xhtml+xml")         filesToCheck.append(href);
        }
    }
    if (!filesToCheck.isEmpty()) {
        foreach (QString href, filesToCheck) {
            QString filePath = m_outputDir + "/" + href;
            ui.statusLbl->setText("Status: checking " + href);
            QString data = Utility::ReadUnicodeTextFile(filePath);
            XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(data);
            if (error.line != -1) {
                errors.append("Incorrect XHTML/XML: " + href + " Line/Col " + QString::number(error.line) + 
                        "," + QString::number(error.column) + " " + error.message);
                well_formed = false;
            }
        }
    }
    if ((!well_formed) && (!errors.isEmpty())) {
        // Throw Up a Dialog to See if they want to proceed
        proceed = false;
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        msgBox.setWindowTitle("Check Report");
        msgBox.setText("Incorrect XHTML/XML Detected\nAre you Sure You Want to Continue?");
        msgBox.setDetailedText(errors.join("\n"));
        QPushButton * yesButton = msgBox.addButton(QMessageBox::Yes);
        QPushButton * noButton =  msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(noButton);
        msgBox.exec();
        if (msgBox.clickedButton() == yesButton) {
            proceed = true;
        }
    }
    return proceed;
}


void PluginRunner::ensureTabsWillRemain()
{
    // before we can delete file make sure a tab of at least one 
    // of the remaining html files will be open to prevent deleting 
    // the last tab when deleting resources                                                             
    QList < Resource * > remainingResources = m_xhtmlFiles.values();
    QList < Resource * > tabResources = m_tabManager->GetTabResources();
    bool tabs_will_remain = false;
    foreach (Resource * tab_resource, tabResources) {
        if (remainingResources.contains(tab_resource)) {
            tabs_will_remain = true;
            break;
        }
    }
    if (! tabs_will_remain) {
        Resource * xhtmlresource = remainingResources.at(0);
        m_mainWindow->OpenResource(*xhtmlresource);
    }
}


bool PluginRunner::deleteFiles(const QStringList & files)
{
    QList <Resource *> tabResources=m_tabManager->GetTabResources();
    bool changes_made = false;
    ui.statusLbl->setText("Status: cleaning up - deleting files");
    foreach (QString fileinfo, files) {
        QStringList fdata = fileinfo.split(SEP);
        QString href = fdata[ hrefField ];
        QString id   = fdata[ idField   ];
        QString mime = fdata[ mimeField ];
        // content.opf and toc.ncx can not be added or deleted
        if (mime == "application/oebps-package+xml") continue;
        if (mime == "application/x-dtbncx+xml") continue;
        Resource * resource = m_hrefToRes.value(href, NULL);
        if (resource) {
            ui.statusLbl->setText("Status: deleting " + resource->Filename());

            if(tabResources.contains(resource)) {
                m_tabManager->CloseTabForResource(*resource);
            }
            m_book->GetFolderKeeper().RemoveResource(*resource);
            changes_made = true;
        }
    }
    if (changes_made) {
        m_bookBrowser->ResourcesDeleted();
    }
    return changes_made;
}


bool PluginRunner::addFiles(const QStringList & files)
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
            ui.statusLbl->setText("Status: Loading " + fi.fileName());
            // FIXME:  We should copy this epub someplace easy to access by user first
#ifdef Q_OS_MAC
            MainWindow *new_window = new MainWindow(epubPath);
            new_window->show();
#else
            m_mainWindow->LoadFile(epubPath);
#endif
            return true;
        }

        // content.opf and toc.ncx can not be added or deleted
        if (mime == "application/oebps-package+xml") continue;
        if (mime == "application/x-dtbncx+xml") continue;

        // No need to copy to ebook root as AddContentToFolder does that for us
        QString inpath = m_outputDir + "/" + href;
        QFileInfo fi(inpath);
        ui.statusLbl->setText("Status: adding " + fi.fileName());

        Resource & resource = m_book->GetFolderKeeper().AddContentFileToFolder(inpath,false);

        // AudioResource, VideoResource, FontResource, ImageResource do not appear to be cached

        // For new Editable Resources must do the equivalent of the InitialLoad
        // Order is important as some resource types inherit from other resource types

        // NOTE! AddContentFileToFolder returns a resource reference and not a pointer

        if (resource.Type() == Resource::FontResourceType && !m_algorithm.isEmpty()) {

            FontResource * font_resource = qobject_cast< FontResource * > (&resource);
            font_resource->SetObfuscationAlgorithm(m_algorithm);

        } else  if (resource.Type() == Resource::HTMLResourceType) {

            HTMLResource * html_resource = qobject_cast< HTMLResource * > (&resource);
            html_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

        } else if (resource.Type() == Resource::CSSResourceType) {

            CSSResource * css_resource = qobject_cast< CSSResource * > (&resource);
            css_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

        } else if (resource.Type() == Resource::SVGResourceType) {

            SVGResource * svg_resource = qobject_cast< SVGResource * > (&resource);
            svg_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

        } else if (resource.Type() == Resource::MiscTextResourceType) {

            MiscTextResource * misctext_resource = qobject_cast< MiscTextResource * > (&resource);
            misctext_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

        } else if (resource.Type() == Resource::XMLResourceType) {

            XMLResource * xml_resource = qobject_cast< XMLResource * > (&resource);
            xml_resource->SetText(Utility::ReadUnicodeTextFile(inpath));
        }
    }
    return true;
}


bool PluginRunner::modifyFiles(const QStringList & files)
{
    ui.statusLbl->setText("Status: cleaning up - modifying files");
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
    if  (!modifyopf.isEmpty()) newfiles.append(modifyopf);
    if  (!modifyncx.isEmpty()) newfiles.append(modifyncx);

    foreach (QString fileinfo, newfiles) {
        QStringList fdata = fileinfo.split(SEP);
        QString href = fdata[ hrefField ];
        QString id   = fdata[ idField   ];
        QString mime = fdata[ mimeField ];
        QString inpath = m_outputDir + "/" + href;
        QString outpath = m_bookRoot + "/" + href;
        QFileInfo fi(outpath);
        ui.statusLbl->setText("Status: modifying " + fi.fileName());
        Utility::ForceCopyFile(inpath, outpath);
        Resource * resource = m_hrefToRes.value(href);
        if (resource) {

            // AudioResource, VideoResource, FontResource, ImageResource do not appear to be editable

            // For Editable Resources must relaod them from modified file
            // Order below is important as some resouirce types inherit from other resource types

            if (resource->Type() == Resource::HTMLResourceType) {

                HTMLResource * html_resource = qobject_cast< HTMLResource * > (resource);
                html_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

            } else if (resource->Type() == Resource::CSSResourceType) {

                CSSResource * css_resource = qobject_cast< CSSResource * > (resource);
                css_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

            } else if (resource->Type() == Resource::SVGResourceType) {

                SVGResource * svg_resource = qobject_cast< SVGResource * > (resource);
                svg_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

            } else if (resource->Type() == Resource::MiscTextResourceType) {

                MiscTextResource * misctext_resource = qobject_cast< MiscTextResource * > (resource);
                misctext_resource->SetText(Utility::ReadUnicodeTextFile(inpath));

            } else if (resource->Type() == Resource::OPFResourceType) { 

                OPFResource * opf_resource = qobject_cast< OPFResource * > (resource);
                opf_resource->SetText(Utility::ReadUnicodeTextFile(outpath));

            } else if (resource->Type() == Resource::NCXResourceType) { 

                NCXResource * ncx_resource = qobject_cast< NCXResource * > (resource);
                ncx_resource->SetText(Utility::ReadUnicodeTextFile(outpath));

            } else if (resource->Type() == Resource::XMLResourceType) {

                XMLResource * xml_resource = qobject_cast< XMLResource * > (resource);
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
    connect(&m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
    connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(pluginFinished(int, QProcess::ExitStatus)));
    connect(&m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
    connect(ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
}
