/************************************************************************
**
**  Copyright (C) 2018-2020  Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2019-2020  Doug Massay
**  Copyright (C) 2009-2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QStyleFactory>
#include <QtCore/QTextCodec>
#include <QtCore/QThreadPool>
#include <QtCore/QTranslator>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFontMetrics>

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
#include <QtWidgets/QPlainTextEdit>
static const QString WIN_CLIPBOARD_ERROR = "QClipboard::setMimeData: Failed to set data on clipboard";
static const int RETRY_DELAY_MS = 5;
#endif

#ifdef Q_OS_MAC
#include <QFileDialog>
#include <QKeySequence>
#include <QAction>
#include <QIcon>
#include "Dialogs/About.h"
#include "Dialogs/Preferences.h"
extern void disableWindowTabbing();
extern void removeMacosSpecificMenuItems();
#endif

// Creates a MainWindow instance depending
// on command line arguments
static MainWindow *GetMainWindow(const QStringList &arguments)
{
    // We use the first argument as the file to load after starting
    QString filepath;
    if (arguments.size() > 1 && Utility::IsFileReadable(arguments.at(1))) {
        filepath = arguments.at(1);
    }
    return new MainWindow(filepath);
}

#ifdef Q_OS_MAC

static void AboutDialog()
{
    About about;
    about.exec();
}

static void PreferencesDialog()
{
    Preferences prefers;
    prefers.exec();
}

static void AppExit()
{
    qApp->closeAllWindows();
    qApp->quit();
}

static void file_new()
{
    MainWindow *w = GetMainWindow(QStringList());
    w->show();
    w->activateWindow();
    QCoreApplication::processEvents();
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
		       &default_filter,
		       QFileDialog::DontUseNativeDialog
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
    QString context_file;
    QString qt_debug_message;

    switch (type) {
        // TODO: should go to a log
        case QtDebugMsg:
            qt_debug_message = QString("Debug: %1").arg(message.toLatin1().constData());
            fprintf(stderr, "Debug: %s\n", message.toLatin1().constData());
            break;
#if QT_VERSION >= 0x050600
        case QtInfoMsg:
            qt_debug_message = QString("Info: %1").arg(message.toLatin1().constData());
            fprintf(stderr, "Info: %s\n", message.toLatin1().constData());
            break;
#endif
        // TODO: should go to a log
        case QtWarningMsg:
            qt_debug_message = QString("Warning: %1").arg(message.toLatin1().constData());
            fprintf(stderr, "Warning: %s\n", message.toLatin1().constData());
            break;
        case QtCriticalMsg:
            qt_debug_message = QString("Critical: %1").arg(message.toLatin1().constData());
            error_message = QString(message.toLatin1().constData());
            if (context.file) context_file = QString(context.file);

	    
#ifdef Q_OS_WIN32
            // On Windows there is a known issue with the clipboard that results in some copy
            // operations in controls being intermittently blocked. Rather than presenting
            // the user with an error dialog, we should simply retry the operation.
            // Hopefully this will be fixed in a future Qt version (still broken as of 4.8.3).
            if (error_message.startsWith(WIN_CLIPBOARD_ERROR)) {
                QWidget *widget = QApplication::focusWidget();

                if (widget) {
                    QPlainTextEdit *textEdit = qobject_cast<QPlainTextEdit *>(widget);

                    if (textEdit) {
                        QTimer::singleShot(RETRY_DELAY_MS, textEdit, SLOT(copy()));
                        break;
                    }

                    // Same issue can happen on a QLineEdit / QComboBox
                    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget);

                    if (lineEdit) {
                        QTimer::singleShot(RETRY_DELAY_MS, lineEdit, SLOT(copy()));
                        break;
                    }

                    QComboBox *comboBox = qobject_cast<QComboBox *>(widget);

                    if (comboBox) {
                        QTimer::singleShot(RETRY_DELAY_MS, comboBox->lineEdit(), SLOT(copy()));
                        break;
                    }
                }
            }

#endif
            // screen out error messages from inspector / devtools
            if (!context_file.contains("devtools://devtools")) {
                Utility::DisplayExceptionErrorDialog(QString("Critical: %1").arg(error_message));
	    }
            break;

        case QtFatalMsg:
            Utility::DisplayExceptionErrorDialog(QString("Fatal: %1").arg(QString(message)));
            abort();
    }
    
    // qDebug() prints to SIGIL_DEBUG_LOGFILE environment variable.
    // User must have permissions to write to the location or no file will be created.
    QString sigil_log_file;
    sigil_log_file = Utility::GetEnvironmentVar("SIGIL_DEBUG_LOGFILE");
	if (!sigil_log_file.isEmpty()) {
            QFile outFile(sigil_log_file);
            outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
            QTextStream ts(&outFile);
            ts << qt_debug_message << endl;
	}
}


void VerifyPlugins()
{
    PluginDB *pdb = PluginDB::instance();
    pdb->load_plugins_from_disk();
}


void setupHighDPI()
{
    bool has_env_setting = false;
    QStringList env_vars;
    env_vars << "QT_ENABLE_HIGHDPI_SCALING" << "QT_SCALE_FACTOR_ROUNDING_POLICY"
             << "QT_AUTO_SCREEN_SCALE_FACTOR" << "QT_SCALE_FACTOR"
             << "QT_SCREEN_SCALE_FACTORS" << "QT_DEVICE_PIXEL_RATIO";
    foreach(QString v, env_vars) {
        if (!Utility::GetEnvironmentVar(v).isEmpty()) {
            has_env_setting = true;
            break;
        }
    }

    SettingsStore ss;
    int highdpi = ss.highDPI();
    if (highdpi == 1 || (highdpi == 0 && !has_env_setting)) {
        // Turning on Automatic High DPI scaling
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    } else if (highdpi == 2) {
        // Turning off Automatic High DPI scaling
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
        foreach(QString v, env_vars) {
            bool irrel = qunsetenv(v.toUtf8().constData());
        }
    }
}


QPalette getDarkPalette()
{
    // Dark palette for Sigil
    QPalette darkPalette;

    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::Disabled, QPalette::Window, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::WindowText, QColor(238, 238, 238));
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText,
                        QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipText, QColor(238, 238, 238));
    darkPalette.setColor(QPalette::Text, QColor(238, 238, 238));
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, QColor(238, 238, 238));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                        QColor(127, 127, 127));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(108, 180, 238));
    darkPalette.setColor(QPalette::LinkVisited, QColor(108, 180, 238));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::HighlightedText, QColor(238, 238, 238));
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                        QColor(127, 127, 127));

    return darkPalette;
}


// Application entry point
int main(int argc, char *argv[])
{
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    QT_REQUIRE_VERSION(argc, argv, "5.9.0");
#else
    QT_REQUIRE_VERSION(argc, argv, "5.12.3");
#endif

#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
// Unset platform theme plugins/styles environment variables immediately
// when forcing Sigil's own darkmode palette on Linux
if (!force_sigil_darkmode_palette.isEmpty()) {
    QStringList env_vars = {"QT_QPA_PLATFORMTHEME", "QT_STYLE_OVERRIDE"};
    foreach(QString v, env_vars) {
        bool irrel = qunsetenv(v.toUtf8().constData());
    }
}
#endif

#ifndef QT_DEBUG
    qInstallMessageHandler(MessageHandler);
#endif

    // Set application information for easier use of QSettings classes
    QCoreApplication::setOrganizationName("sigil-ebook");
    QCoreApplication::setOrganizationDomain("sigil-ebook.com");
    QCoreApplication::setApplicationName("sigil");
    QCoreApplication::setApplicationVersion(SIGIL_VERSION);

#ifndef Q_OS_MAC
    setupHighDPI();
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // many qtbugs related to mixing 32 and 64 bit qt apps when shader disk cache is used
    // Only use if using Qt5.9.0 or higher
#if QT_VERSION >= 0x050900
    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache);
#endif

    // Disable ? as Sigil does not use QWhatsThis
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QCoreApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

    // On recent processors with multiple cores this leads to over 40 threads at times
#if 0   
    // We prevent Qt from constantly creating and deleting threads.
    // Using a negative number forces the threads to stay around;
    // that way, we always have a steady number of threads ready to do work.
    QThreadPool::globalInstance()->setExpiryTimeout(-1);
#endif

    // QtWebEngine may need this
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    MainApplication app(argc, argv);

#ifdef Q_OS_MAC
    disableWindowTabbing();
    removeMacosSpecificMenuItems();
#endif


    // Install an event filter for the application
    // so we can catch OS X's file open events
    // This needs to be done upfront to prevent events from
    // being missed
    AppEventFilter *filter = new AppEventFilter(&app);
    app.installEventFilter(filter);

    // Set up embedded python integration first thing
    EmbeddedPython* epython = EmbeddedPython::instance();
    epython->addToPythonSysPath(epython->embeddedRoot());
    epython->addToPythonSysPath(PluginDB::launcherRoot() + "/python");

    try {

        // Specify the plugin folders
        // (language codecs and image loaders)
        app.addLibraryPath("codecs");
        app.addLibraryPath("iconengines");
        app.addLibraryPath("imageformats");

        QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
        SettingsStore settings;

        // Setup the qtbase_ translator and load the translation for the selected language
        QTranslator qtbaseTranslator;
        const QString qm_name_qtbase = QString("qtbase_%1").arg(settings.uiLanguage());
        // Run though all locations and stop once we find and are able to load
        // an appropriate Qt base translation.
        foreach(QString path, UILanguage::GetPossibleTranslationPaths()) {
            if (QDir(path).exists()) {
                if (qtbaseTranslator.load(qm_name_qtbase, path)) {
                    break;
                }
            }
        }
        app.installTranslator(&qtbaseTranslator);

        // Setup the Sigil translator and load the translation for the selected language
        QTranslator sigilTranslator;
        const QString qm_name = QString("sigil_%1").arg(settings.uiLanguage());
        // Run though all locations and stop once we find and are able to load
        // an appropriate translation.
        foreach(QString path, UILanguage::GetPossibleTranslationPaths()) {
            if (QDir(path).exists()) {
                if (sigilTranslator.load(qm_name, path)) {
                    break;
                }
            }
        }
        app.installTranslator(&sigilTranslator);

#ifndef Q_OS_MAC
#ifndef Q_OS_WIN32
        // Use platform themes/styles on Linux unless FORCE_SIGIL_DARKMODE_PALETTE is set
        if (!force_sigil_darkmode_palette.isEmpty()) {
            // Fusion style is fully dpi aware on Windows/Linux
            app.setStyle(QStyleFactory::create("fusion"));
            // qss stylesheet from resources
            QString dark_styles = Utility::ReadUnicodeTextFile(":/dark/win-dark-style.qss");
            app.setStyleSheet(dark_styles);
            app.setPalette(getDarkPalette());
        }
#else
        if (Utility::WindowsShouldUseDarkMode()) {
            // Fusion style is fully dpi aware on Windows/Linux
            app.setStyle(QStyleFactory::create("fusion"));
            // qss stylesheet from resources
            QString dark_styles = Utility::ReadUnicodeTextFile(":/dark/win-dark-style.qss");
            app.setStyleSheet(dark_styles);
            app.setPalette(getDarkPalette());
        }
#endif
#endif

        // Set ui font from preferences after dark theming
        QFont f = QFont(QApplication::font());
#ifdef Q_OS_WIN32
        if (f.family() == "MS Shell Dlg 2" && f.pointSize() == 8) {
            // Microsoft's recommended UI defaults
            f.setFamily("Segoe UI");
            f.setPointSize(9);
            QApplication::setFont(f);
        }
#elif defined(Q_OS_MAC)
        // Just in case
#else
        if (f.family() == "Sans Serif" && f.pointSize() == 9) {
            f.setPointSize(10);
            QApplication::setFont(f);
        }
#endif
        settings.setOriginalUIFont(f.toString());
        if (!settings.uiFont().isEmpty()) {
            QFont font;
            if (font.fromString(settings.uiFont()))
                QApplication::setFont(font);
        }
#ifndef Q_OS_MAC
        // redo on a timer to ensure in all cases
        if (!settings.uiFont().isEmpty()) {
            QFont font;
            if (font.fromString(settings.uiFont())) {
                QTimer::singleShot(0, [=]() {
                    QApplication::setFont(font);
                } );
            }
        }
#endif

        // drag and drop in main tab bar is too touchy and that can cause problems.
        // default drag distance limit is much too small especially for hpi displays
        // startDragDistance default is just 10 pixels
#ifdef Q_OS_MAC
        if (app.startDragDistance() < 30) app.setStartDragDistance(30);
#else
        QFontMetrics fm(app.font());
        int dragbase = fm.xHeight() * 2;
        int dragtweak = settings.uiDragDistanceTweak();
        // Use calculated base distance if tweak value not between -20 and 20px
        if (dragtweak >= -20 && dragtweak <= 20) {
            int newdrag = dragbase + dragtweak;
            if (newdrag < 10) {
                app.setStartDragDistance(10);  // 10px minimum
            } else if (newdrag > 60) {
                app.setStartDragDistance(60);  // 60px maximum
            } else {
                app.setStartDragDistance(newdrag);
            }
        } else {
            // Tweak value outside range. Use calculated distance.
            app.setStartDragDistance(dragbase);
        }
#endif
        // End of UI font stuff

        // Check for existing qt_styles.qss in Prefs dir and load it if present
        QString qt_stylesheet_path = Utility::DefinePrefsDir() + "/qt_styles.qss";
        QFileInfo QtStylesheetInfo(qt_stylesheet_path);
        if (QtStylesheetInfo.exists() && QtStylesheetInfo.isFile() && QtStylesheetInfo.isReadable()) {
            QString qtstyles = Utility::ReadUnicodeTextFile(qt_stylesheet_path);
            app.setStyleSheet(app.styleSheet().append(qtstyles));
        }

        // Qt's setCursorFlashTime(msecs) (or the docs) are broken
        // According to the docs, setting a negative value should disable cursor blinking 
        // but instead just forces it to look for PlatformSpecific Themeable Hints to get 
        // a value which for Mac OS X is hardcoded to 1000 ms
        // This was the only way I could get Qt to disable cursor blinking on a Mac if desired
        if (qEnvironmentVariableIsSet("SIGIL_DISABLE_CURSOR_BLINK")) {
            app.setCursorFlashTime(0);
        }
        // We set the window icon explicitly on Linux.
        // On Windows this is handled by the RC file,
        // and on Mac by the ICNS file.
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
        app.setWindowIcon(GetApplicationIcon());
#if QT_VERSION >= 0x050700
        // Wayland needs this clarified in order to propery assign the icon 
        app.setDesktopFileName(QStringLiteral("sigil.desktop"));
#endif
#endif
        // Needs to be created on the heap so that
        // the reply has time to return.
        UpdateChecker *checker = new UpdateChecker(&app);
        checker->CheckForUpdate();

        QStringList arguments = QCoreApplication::arguments();

#ifdef Q_OS_MAC
	// now process main app events so that any startup 
        // FileOpen event will be processed for macOS
	QCoreApplication::processEvents();

	QString filepath = filter->getInitialFilePath();

	// if one found append it to argv for processing as normal
	if ((arguments.size() == 1) && !filepath.isEmpty()) {
	    arguments << QFileInfo(filepath).absoluteFilePath();
	}

	if (filepath.isEmpty()) filter->setInitialFilePath(QString("placeholder"));
#endif

        if (arguments.contains("-t")) {
            std::cout  << TempFolder::GetPathToSigilScratchpad().toStdString() << std::endl;
            return 1;
        } else {
            // Normal startup

#ifdef Q_OS_MAC
            // Try to Work around QTBUG-62193 and QTBUG-65245 and others where menubar
	    // menu items are lost under File and Sigil menus and where
	    // Quit menu gets lost when deleting other windows first

            app.setQuitOnLastWindowClosed(false);

            // Create a viable Global MacOS QMenuBar
            QMenuBar *mac_bar = new QMenuBar(0);


            // Create the Application Menu
            QMenu *app_menu = new QMenu("Sigil");

            QIcon icon;

            // Quit
	    QAction* appquit_action = new QAction(QObject::tr("Quit"));
            appquit_action->setMenuRole(QAction::QuitRole);
            appquit_action->setShortcut(QKeySequence("Ctrl+Q"));
            icon = appquit_action->icon();
	    icon.addFile(QString::fromUtf8(":/main/process-stop_16px.png"));
	    icon.addFile(QString::fromUtf8(":/main/process-stop_22px.png"));
	    icon.addFile(QString::fromUtf8(":/main/process-stop_48px.png"));
            appquit_action->setIcon(icon);
            QObject::connect(appquit_action, &QAction::triggered, AppExit);
	    app_menu->addAction(appquit_action);

            // About
	    QAction* about_action = new QAction(QObject::tr("About"));
            about_action->setMenuRole(QAction::AboutRole);
            icon = about_action->icon();
	    icon.addFile(QString::fromUtf8(":/main/help-browser_16px.png"));
	    icon.addFile(QString::fromUtf8(":/main/help-browser_22px.png"));
	    icon.addFile(QString::fromUtf8(":/main/help-browser_48px.png"));
            about_action->setIcon(icon);
            QObject::connect(about_action, &QAction::triggered, AboutDialog);
	    app_menu->addAction(about_action);

            // Preferences
	    QAction* prefs_action = new QAction(QObject::tr("Preferences"));
            prefs_action->setMenuRole(QAction::PreferencesRole);
            QObject::connect(prefs_action, &QAction::triggered, PreferencesDialog);
	    app_menu->addAction(prefs_action);

            mac_bar->addMenu(app_menu);

            // now create a File Menu
            QMenu *file_menu = new QMenu("File");

            // New
            QAction * new_action = new QAction(QObject::tr("New"));
            new_action->setShortcut(QKeySequence("Ctrl+N"));
	    icon = new_action->icon();
	    icon.addFile(QString::fromUtf8(":/main/document-new_16px.png"));
	    icon.addFile(QString::fromUtf8(":/main/document-new_22px.png"));
	    icon.addFile(QString::fromUtf8(":/main/document-new_48px.png"));
	    new_action->setIcon(icon);
            QObject::connect(new_action, &QAction::triggered, file_new);
	    file_menu->addAction(new_action);

            // Open
	    QAction* open_action = new QAction(QObject::tr("Open"));
            open_action->setShortcut(QKeySequence("Ctrl+O"));
	    icon = open_action->icon();
	    icon.addFile(QString::fromUtf8(":/main/document-open_16px.png"));
	    icon.addFile(QString::fromUtf8(":/main/document-open_22px.png"));
	    icon.addFile(QString::fromUtf8(":/main/document-open_48px.png"));
	    open_action->setIcon(icon);
            QObject::connect(open_action, &QAction::triggered, file_open);
	    file_menu->addAction(open_action);

            // Quit - force add of a secondary quit menu to the file menu
	    QAction* quit_action = new QAction(QObject::tr("Quit"));
            quit_action->setMenuRole(QAction::NoRole);
            quit_action->setShortcut(QKeySequence("Ctrl+Q"));
            QObject::connect(quit_action, &QAction::triggered, qApp->quit);
	    file_menu->addAction(quit_action);

            mac_bar->addMenu(file_menu);
#endif

            VerifyPlugins();
            MainWindow *widget = GetMainWindow(arguments);
            widget->show();
            QApplication::setActiveWindow(widget);
            return app.exec();
        }
    } catch (std::exception &e) {
        Utility::DisplayExceptionErrorDialog(e.what());
        return 1;
    }
}
