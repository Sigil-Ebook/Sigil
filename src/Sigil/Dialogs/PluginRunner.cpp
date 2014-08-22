#include <Qt>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include "MainUI/MainWindow.h"
#include "MainUI/BookBrowser.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/TempFolder.h"
#include "Tabs/TabManager.h"
#include "PluginRunner.h"

const QString ADOBE_FONT_ALGO_ID         = "http://ns.adobe.com/pdf/enc#RC";
const QString IDPF_FONT_ALGO_ID          = "http://www.idpf.org/2008/embedding";

const QString PluginRunner::SEP = QString(QChar(31));
const QString PluginRunner::OPFFILEINFO = "OEBPS/content.opf" + SEP + SEP + "application/oebps-package+xml";
const QString PluginRunner::NCXFILEINFO = "OEBPS/toc.ncx" + SEP + SEP + "application/x-dtbncx+xml";
const QStringList PluginRunner::CHANGESTAGS = QStringList() << "deleted" << "added" << "modified";

 
PluginRunner::PluginRunner(QString name, QWidget * parent)
  : QDialog(parent),
    m_pluginName(name), 
    m_outputDir(m_folder.GetPath()),
    m_pluginOutput(""),
    m_algorithm(""),
    m_mainWindow(qobject_cast<MainWindow *>(parent)),
    m_ready(false)

{
    QHash < QString, QStringList > plugininfo;
    QHash < QString, QString > enginepath;
    QStringList fields;

    // get book manipulation objects
    m_book = m_mainWindow->GetCurrentBook();
    m_bookBrowser = m_mainWindow->GetBookBrowser();
    m_bookRoot = m_book->GetFolderKeeper().GetFullPathToMainFolder();
    
    // set font obfuscation algorithm to use
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

    // build hash of href relative path from root to resources
    QList< Resource * > resources = m_book->GetFolderKeeper().GetResourceList();
    foreach (Resource * resource, resources) {
        QString href = resource->GetRelativePathToRoot();
        if (href.endsWith("OEBPS/content.opf")) {
            href = "OEBPS/content.opf";
        } else if (href.endsWith("OEBPS/toc.ncx")) {
            href = "OEBPS/toc.ncx";
        }
        m_hrefToRes[href] = resource;
    }

    // get plugin settings from SettingsStore
    SettingsStore settings;
    enginepath = settings.pluginEnginePaths();
    plugininfo = settings.pluginInfo();
    if (! plugininfo.keys().contains(m_pluginName)) {
        Utility::DisplayStdErrorDialog("Error: A plugin by that name does not exist");
        return;
    }

    // set up paths and things for the plugin and interpreter
    m_pluginsFolder = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/plugins";
    fields = plugininfo[m_pluginName];
    m_engine = fields.at(EngineField);
    m_pluginType = fields.at(TypeField);
    m_enginePath = enginepath[m_engine];
    if (m_enginePath.isEmpty()) {
        Utility::DisplayStdErrorDialog("Error: Interpreter " + m_engine + " has no path set");
        return;
    }

    // The launcher and plugin path are both platform specific and engine/interpreter specific 
    QString launcher_root = QCoreApplication::applicationDirPath(); 

#ifdef Q_OS_MAC
    launcher_root = launcher_root + "/..";
#endif
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // all flavours of linux / unix
    launcher_root = launcher_root + "/../share/" + QCoreApplication::applicationName().toLower();
    // user supplied environment variable to plugin launcher directory will overrides everything
    const QString env_launcher_location = QString(getenv("SIGIL_PLUGIN_LAUNCHERS"));
     if (!env_launcher_location.isEmpty()) {
         launcher_root = env_launcher_location;
     }

#endif

    if (m_engine == "python2.7") {
        m_launcherPath = launcher_root + "/python2_7/launcher.py";
        m_pluginPath = m_pluginsFolder + "/" + m_pluginName + "/" + m_pluginName + ".py";
        if (! QFileInfo(m_launcherPath).exists()) {
            Utility::DisplayStdErrorDialog("Installation Error: plugin launcher " + m_launcherPath + " does not exist");
            return;
        }
     } else {
        Utility::DisplayStdErrorDialog("Error: plugin engine " + m_engine + " is not supported (yet!)");
        return;
     }

    ui.setupUi(this);
    connectSignalsToSlots();
    ui.nameLbl->setText(m_pluginName);
    ui.statusLbl->setText("Status: ready");
    ui.progressBar->setRange(0,100);
    ui.progressBar->reset();
    ui.cancelButton->setEnabled(true);
    m_ready = true;
}


PluginRunner::~PluginRunner()
{
}

void PluginRunner::startPlugin() 
{
    QStringList args;
    // Utility::DisplayStdWarningDialog("Plugin Has Started");
    if (!m_ready) {
       Utility::DisplayStdErrorDialog("Error: plugin can not start");
       return;
    }
    ui.textEdit->clear();
    ui.textEdit->setOverwriteMode(true);
    ui.textEdit->setPlainText("");

    // uncomment these if debugging plugin errors

    // ui.textEdit->setPlainText("Launcher: " + m_launcherPath + "\n");
    // ui.textEdit->append("Book: " + m_bookRoot + "\n");
    // ui.textEdit->append("OutputDir: " + m_outputDir + "\n");
    // ui.textEdit->append("Type: " + m_pluginType + "\n");
    // ui.textEdit->append("Plugin Path: " + m_pluginPath + "\n");
    // ui.textEdit->append("Engine Path: " + m_enginePath + "\n");
  
    // prepare for the plugin by flushing all current changes to disk
    m_mainWindow->SaveTabData();
    m_book->GetFolderKeeper().SuspendWatchingResources();
    m_book->SaveAllResourcesToDisk();
    m_book->GetFolderKeeper().ResumeWatchingResources();
    ui.startButton->setEnabled(false);
    ui.okButton->setEnabled(false);
    ui.cancelButton->setEnabled(true);
    args << m_launcherPath << m_bookRoot << m_outputDir << m_pluginType << m_pluginPath;
    m_process.start(m_enginePath, args);
    ui.statusLbl->setText("Status: running");
    // this starts the infinite progress bar
    ui.progressBar->setRange(0,0);
}


void PluginRunner::processOutput() 
{
    QByteArray newbytedata = m_process.readAllStandardOutput();
    m_pluginOutput = m_pluginOutput + newbytedata ;
    // QString data = QString::fromUtf8(newbytedata);
    // ui.textEdit->append(data);
}


void PluginRunner::pluginFinished(int exitcode, QProcess::ExitStatus exitstatus) 
{
    if (exitstatus == QProcess::CrashExit) {
        ui.textEdit->append("Launcher process crashed");
    } 

    // launcher exiting properly does not mean target plugin succeeded or failed
    // we need to parse the response xml to find the true result of target plugin
    // if (exitcode != 0) {
    //    ui.textEdit->append("failed");
    // } else {
    //    ui.textEdit->append("success");
    // }

    ui.okButton->setEnabled(true);
    ui.cancelButton->setEnabled(false);
    // this stops the progress bar at full
    ui.progressBar->setRange(0,100);
    ui.progressBar->setValue(100);
    ui.statusLbl->setText("Status: finished");
    if (!processResultXML()) {
        return;
    }
    // everthing looks good so now make any necessary changes
    bool book_modified = false;
    m_book->GetFolderKeeper().SuspendWatchingResources();
    if (!m_filesToDelete.isEmpty()) {
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
    // this resets the progress bar
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
    // this resets the progress bar
    ui.progressBar->setRange(0,100);
    ui.progressBar->reset();
    ui.textEdit->append("Plugin cancelled");
    ui.statusLbl->setText("Status: cancelled");
    ui.cancelButton->setEnabled(false);
}

bool PluginRunner::processResultXML(){
    QXmlStreamReader reader(m_pluginOutput);
    reader.setNamespaceProcessing(false);
    while (! reader.atEnd()) {
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
                QXmlStreamAttributes attr = reader.attributes();
                info << attr.value("href").toString();
                info << attr.value("id").toString();
                info << attr.value("media-type").toString();
                QString fileinfo = info.join(SEP);
                if (reader.name() == "deleted") {
                    m_filesToDelete.append(fileinfo);
                } else if (reader.name() == "added") {
                    m_filesToAdd.append(fileinfo);
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


bool PluginRunner::deleteFiles(const QStringList & files)
{
    QList <Resource *> tabResources=m_mainWindow->m_TabManager.GetTabResources();
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
                m_mainWindow->m_TabManager.CloseTabForResource(*resource);
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
            // Should copy this epub someplace easy to access by user first
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
