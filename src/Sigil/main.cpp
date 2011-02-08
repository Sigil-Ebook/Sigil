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

#include "stdafx.h"
#include "Misc/Utility.h"
#include "MainUI/MainWindow.h"
#include <QtGui/QApplication>
#include "Misc/UpdateChecker.h"
#include "Misc/AppEventFilter.h"
#include "BookManipulation/Book.h"
#include "Importers/ImporterFactory.h"
#include "Exporters/ExporterFactory.h"
#include "BookManipulation/BookNormalization.h"
#include <XercesInit.h>
#include <iostream>


// Creates a MainWindow instance depending
// on command line arguments
static MainWindow* GetMainWindow( const QStringList &arguments )
{
    // We use the first argument
    // as the file to load after starting

    if ( arguments.size() > 1 &&
         Utility::IsFileReadable( arguments.at( 1 ) )
       )
    {
        return new MainWindow( arguments.at( 1 ) );
    }

    else
    {
        return new MainWindow();
    }
}


#ifdef Q_WS_X11
// Returns a QIcon with the Sigil "S" logo in various sizes
static QIcon GetApplicationIcon()
{
    QIcon app_icon;

    // This 16x16 one looks wrong for some reason
    //app_icon.addFile( ":/icon/app_icon_16.png", QSize( 16, 16 ) );
    app_icon.addFile( ":/icon/app_icon_32.png",  QSize( 32, 32 )   );
    app_icon.addFile( ":/icon/app_icon_48.png",  QSize( 48, 48 )   );
    app_icon.addFile( ":/icon/app_icon_128.png", QSize( 128, 128 ) );
    app_icon.addFile( ":/icon/app_icon_256.png", QSize( 256, 256 ) );
    app_icon.addFile( ":/icon/app_icon_512.png", QSize( 512, 512 ) );

    return app_icon;
}
#endif


// The message handler installed to handle Qt messages
void MessageHandler( QtMsgType type, const char *message )
{
    switch (type) 
    {
        // TODO: should go to a log
        case QtDebugMsg:
 
            fprintf( stderr, "Debug: %s\n", message );
            break;

        // TODO: should go to a log
        case QtWarningMsg:

            fprintf( stderr, "Warning: %s\n", message );
            break;

        case QtCriticalMsg:

            Utility::DisplayStdErrorDialog( "Critical: " + QString( message ) );
            break;

        case QtFatalMsg:

            Utility::DisplayStdErrorDialog( "Fatal: " + QString( message ) );
            abort();
    }
}


// Used for an undocumented, unsupported *-to-epub
// console conversion. USE AT YOUR OWN PERIL!
static bool QuickConvert( const QStringList &arguments )
{
    if ( arguments.count() != 4 )

        return false;

    // Hm... no text is printed to the console
    // for a QApplication...
    if ( !QFileInfo( arguments.at( 1 ) ).isAbsolute() )
    {
        std::cout << "ERROR: The input file path is not an absolute path." << std::endl;
        return false;
    }

    if ( !QFileInfo( arguments.at( 3 ) ).isAbsolute() )
    {
        std::cout << "ERROR: The output file path is not an absolute path." << std::endl;
        return false;
    }

    if ( !QFileInfo( arguments.at( 1 ) ).isReadable() )
    {
        std::cout << "ERROR: The input file cannot be read." << std::endl;
        return false;
    }

    QSharedPointer< Book > book = ImporterFactory().GetImporter( arguments.at( 1 ) ).GetBook();
    BookNormalization::Normalize( book );
    ExporterFactory().GetExporter( arguments.at( 3 ), book ).WriteBook();

    return true;
}

/**
 * Creates (or modifies, if it already exists) the Sigil temp folder so that it
 * can be read and modified by anyone.
 */
void CreateTempFolderWithCorrectPermissions()
{
    QString temp_path = Utility::GetPathToSigilScratchpad();
    QDir( temp_path ).mkpath( temp_path );

    QFile::setPermissions( temp_path, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                      QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                                      QFile::ReadOther | QFile::WriteOther | QFile::ExeOther );
}


// Application entry point
int main( int argc, char *argv[] )
{
    QT_REQUIRE_VERSION( argc, argv, "4.7.0" );

#ifndef QT_DEBUG
    qInstallMsgHandler( MessageHandler );
#endif
    
    QApplication app( argc, argv );
    XercesExt::XercesInit init;

    try
    {
        // We prevent Qt from constantly creating and deleting threads.
        // Using a negative number forces the threads to stay around;
        // that way, we always have a steady number of threads ready to do work.
        QThreadPool::globalInstance()->setExpiryTimeout( -1 );

        // Specify the plugin folders 
        // (language codecs and image loaders)
        app.addLibraryPath( "codecs" );
        app.addLibraryPath( "iconengines" );
        app.addLibraryPath( "imageformats" );

        // Set application information for
        // easier use of QSettings classes
        QCoreApplication::setOrganizationName( "Strahinja Markovic" );
        QCoreApplication::setApplicationName( "Sigil" );

        // We set the window icon explicitly on Linux.
        // On Windows this is handled by the RC file,
        // and on Mac by the ICNS file.
    #ifdef Q_WS_X11
        app.setWindowIcon( GetApplicationIcon() );
    #endif

        // On Unix systems, we make sure that the temp folder we
        // create is accessible by all users. On Windows, there's
        // a temp folder per user.
    #ifndef Q_WS_WIN
        CreateTempFolderWithCorrectPermissions();
    #endif

        // Needs to be created on the heap so that
        // the reply has time to return.
        UpdateChecker *checker = new UpdateChecker( &app );
        checker->CheckForUpdate();
        
        // Install an event filter for the application
        // so we can catch OS X's file open events
        AppEventFilter *filter = new AppEventFilter( &app );
        app.installEventFilter( filter );

        const QStringList &arguments = QCoreApplication::arguments();

        // Normal startup
        if ( arguments.count() != 4 )
        {
            MainWindow *widget = GetMainWindow( arguments );    
            widget->show();

            return app.exec();
        }

        // Used for an undocumented, unsupported *-to-epub
        // console conversion. USE AT YOUR OWN PERIL!
        else
        {
            return QuickConvert( arguments );
        }
    }
    
    catch ( ExceptionBase &exception )
    {
        Utility::DisplayStdErrorDialog( Utility::GetExceptionInfo( exception ) );
        return 1;
    }
}





