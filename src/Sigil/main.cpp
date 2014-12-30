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

#include "Misc/EmbeddedPython.h"
#include <iostream>
#include <XercesInit.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QTextCodec>
#include <QtCore/QThreadPool>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QXmlStreamReader>

#include "Misc/PluginDB.h"
#include "Misc/UILanguage.h"
#include "MainUI/MainApplication.h"
#include "MainUI/MainWindow.h"
#include "Misc/AppEventFilter.h"
#include "Misc/SettingsStore.h"
#include "Misc/TempFolder.h"
#include "Misc/UpdateChecker.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

#ifdef Q_OS_WIN32
# include <QtWidgets/QPlainTextEdit>
# include "ViewEditors/BookViewPreview.h"
static const QString WIN_CLIPBOARD_ERROR = "QClipboard::setMimeData: Failed to set data on clipboard";
static const int RETRY_DELAY_MS = 5;
#endif

#ifdef Q_OS_MAC
# include <QFileDialog>
# include <QKeySequence>
# include <QAction>
#endif

// Creates a MainWindow instance depending
// on command line arguments
static MainWindow *GetMainWindow(const QStringList &arguments)
{
    // We use the first argument
    // as the file to load after starting
    if (arguments.size() > 1 &&
        Utility::IsFileReadable(arguments.at(1))) {
        return new MainWindow(arguments.at(1));
    } else {
        return new MainWindow();
    }
}

#ifdef Q_OS_MAC
static void file_new()
{
    MainWindow *w = GetMainWindow(QStringList());
    w->show();
}

static void file_open()
{
    const QMap<QString, QString> load_filters = MainWindow::GetLoadFiltersMap();
    QStringList filters(load_filters.values());
    filters.removeDuplicates();
    QString filter_string = "";
    foreach(QString filter, filters) {
        filter_string += filter + ";;";
    }
    // "All Files (*.*)" is the default
    QString default_filter = load_filters.value("epub");
    QString filename = QFileDialog::getOpenFileName(0,
                       "Open File",
                       "~",
                       filter_string,
                       &default_filter
                                                   );

    if (!filename.isEmpty()) {
        MainWindow *w = GetMainWindow(QStringList() << "" << filename);
        w->show();
    }
}
#endif

#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
// Returns a QIcon with the Sigil "S" logo in various sizes
static QIcon GetApplicationIcon()
{
    QIcon app_icon;
    // This 16x16 one looks wrong for some reason
    //app_icon.addFile( ":/icon/app_icon_16.png", QSize( 16, 16 ) );
    app_icon.addFile(":/icon/app_icon_32.png",  QSize(32, 32));
    app_icon.addFile(":/icon/app_icon_48.png",  QSize(48, 48));
    app_icon.addFile(":/icon/app_icon_128.png", QSize(128, 128));
    app_icon.addFile(":/icon/app_icon_256.png", QSize(256, 256));
    app_icon.addFile(":/icon/app_icon_512.png", QSize(512, 512));
    return app_icon;
}
#endif


// The message handler installed to handle Qt messages
void MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QString error_message;

    switch (type) {
        // TODO: should go to a log
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s\n", message.toLatin1().constData());
            break;
        // TODO: should go to a log
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s\n", message.toLatin1().constData());
            break;
        case QtCriticalMsg:
            error_message = QString(message.toLatin1().constData());
#ifdef Q_OS_WIN32
            // On Windows there is a known issue with the clipboard that results in some copy
            // operations in controls being intermittently blocked. Rather than presenting
            // the user with an error dialog, we should simply retry the operation.
            // Hopefully this will be fixed in a future Qt version (still broken as of 4.8.3).
            if (error_message.startsWith(WIN_CLIPBOARD_ERROR)) {
                QWidget *widget = QApplication::focusWidget();

                if (widget) {
                    QPlainTextEdit *textEdit = dynamic_cast<QPlainTextEdit *>(widget);

                    if (textEdit) {
                        QTimer::singleShot(RETRY_DELAY_MS, textEdit, SLOT(copy()));
                        break;
                    }

                    // BV/PV copying is a little different, in that the focus widget is set to
                    // the parent editor (unlike CodeView's QPlainTextEdit).
                    BookViewPreview *bookViewPreview = dynamic_cast<BookViewPreview *>(widget);

                    if (bookViewPreview) {
                        QTimer::singleShot(RETRY_DELAY_MS, bookViewPreview, SLOT(copy()));
                        break;
                    }

                    // Same issue can happen on a QLineEdit / QComboBox
                    QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(widget);

                    if (lineEdit) {
                        QTimer::singleShot(RETRY_DELAY_MS, lineEdit, SLOT(copy()));
                        break;
                    }

                    QComboBox *comboBox = dynamic_cast<QComboBox *>(widget);

                    if (comboBox) {
                        QTimer::singleShot(RETRY_DELAY_MS, comboBox->lineEdit(), SLOT(copy()));
                        break;
                    }
                }
            }

#endif
            Utility::DisplayExceptionErrorDialog(QString("Critical: %1").arg(error_message));
            break;

        case QtFatalMsg:
            Utility::DisplayExceptionErrorDialog(QString("Fatal: %1").arg(QString(message)));
            abort();
    }
}


/**
 * Creates (or modifies, if it already exists) the Sigil temp folder so that it
 * can be read and modified by anyone.
 */
void CreateTempFolderWithCorrectPermissions()
{
    QString temp_path = TempFolder::GetPathToSigilScratchpad();
    QDir(temp_path).mkpath(temp_path);
    QFile::setPermissions(temp_path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                          QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                          QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
}

void VerifyPlugins()
{
    PluginDB *pdb = PluginDB::instance();
    pdb->load_plugins_from_disk();
}


// Application entry point
int main(int argc, char *argv[])
{
    QT_REQUIRE_VERSION(argc, argv, "5.0.0");
#ifndef QT_DEBUG
    qInstallMessageHandler(MessageHandler);
#endif
    MainApplication app(argc, argv);
    XercesExt::XercesInit init;
    EmbeddedPython* epython = EmbeddedPython::instance();
    epython->addToPythonSysPath(epython->embeddedRoot());

    try {
        // We prevent Qt from constantly creating and deleting threads.
        // Using a negative number forces the threads to stay around;
        // that way, we always have a steady number of threads ready to do work.
        QThreadPool::globalInstance()->setExpiryTimeout(-1);
        // Specify the plugin folders
        // (language codecs and image loaders)
        app.addLibraryPath("codecs");
        app.addLibraryPath("iconengines");
        app.addLibraryPath("imageformats");
        // Set application information for
        // easier use of QSettings classes
        QCoreApplication::setOrganizationName("sigil-ebook");
        QCoreApplication::setOrganizationDomain("sigil-ebook.com");
        QCoreApplication::setApplicationName("sigil");
        QCoreApplication::setApplicationVersion(SIGIL_VERSION);
        // Setup the translator and load the translation for the selected language
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
        QTranslator translator;
        SettingsStore settings;
        const QString qm_name = QString("sigil_%1").arg(settings.uiLanguage());
        // Run though all locations and stop once we find and are able to load
        // an appropriate translation.
        foreach(QString path, UILanguage::GetPossibleTranslationPaths()) {
            if (QDir(path).exists()) {
                if (translator.load(qm_name, path)) {
                    break;
                }
            }
        }
        app.installTranslator(&translator);
        // We set the window icon explicitly on Linux.
        // On Windows this is handled by the RC file,
        // and on Mac by the ICNS file.
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
        app.setWindowIcon(GetApplicationIcon());
#endif
        // On Unix systems, we make sure that the temp folder we
        // create is accessible by all users. On Windows, there's
        // a temp folder per user.
#ifndef Q_OS_WIN32
        CreateTempFolderWithCorrectPermissions();
#endif
        // Needs to be created on the heap so that
        // the reply has time to return.
        UpdateChecker *checker = new UpdateChecker(&app);
        checker->CheckForUpdate();
        // Install an event filter for the application
        // so we can catch OS X's file open events
        AppEventFilter *filter = new AppEventFilter(&app);
        app.installEventFilter(filter);
        const QStringList &arguments = QCoreApplication::arguments();

        if (arguments.contains("-t")) {
            std::cout  << TempFolder::GetPathToSigilScratchpad().toStdString() << std::endl;
            return 1;
        } else {
            // Normal startup
#ifdef Q_OS_MAC
            app.setQuitOnLastWindowClosed(false);

            QMenuBar *mac_menu = new QMenuBar(0);
            QMenu *file_menu = new QMenu("File");
            QAction *action;
            // New
            action = file_menu->addAction("New");
            action->setShortcut(QKeySequence("Ctrl+N"));
            QObject::connect(action, &QAction::triggered, file_new);
            // Open
            action = file_menu->addAction("Open");
            action->setShortcut(QKeySequence("Ctrl+O"));
            QObject::connect(action, &QAction::triggered, file_open);
            // Quit
            action = file_menu->addAction("Quit");
            action->setShortcut(QKeySequence("Ctrl+Q"));
            QObject::connect(action, &QAction::triggered, qApp->quit);

            mac_menu->addMenu(file_menu);
            mac_menu->show();
#endif
            VerifyPlugins();
            MainWindow *widget = GetMainWindow(arguments);
            widget->show();
            return app.exec();
        }
    } catch (ExceptionBase &exception) {
        Utility::DisplayExceptionErrorDialog(Utility::GetExceptionInfo(exception));
        return 1;
    }
}
