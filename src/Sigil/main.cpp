/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#include "Utility.h"
#include "MainWindow.h"
#include <QtGui/QApplication>
#include "UpdateChecker.h"

// Creates a MainWindow instance depending
// on command line arguments
static MainWindow* GetMainWindow()
{
    // We use the first argument
    // as the file to load after starting
    QStringList arguments = QCoreApplication::arguments();

    if (    arguments.size() > 1 &&
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


// Returns a QIcon with the Sigil "S" logo in various sizes
static QIcon GetApplicationIcon()
{
    QIcon app_icon;

    // This 16x16 one looks wrong for some reason
    //app_icon.addFile( ":/icon/app_icon_16.png", QSize( 16, 16 ) );
    app_icon.addFile( ":/icon/app_icon_32.png",     QSize( 32, 32 )     );
    app_icon.addFile( ":/icon/app_icon_48.png",     QSize( 48, 48 )     );
    app_icon.addFile( ":/icon/app_icon_128.png",    QSize( 128, 128 )   );
    app_icon.addFile( ":/icon/app_icon_256.png",    QSize( 256, 256 )   );
    app_icon.addFile( ":/icon/app_icon_512.png",    QSize( 512, 512 )   );

    return app_icon;
}


// Application entry point
int main( int argc, char *argv[] )
{
    QT_REQUIRE_VERSION( argc, argv, "4.5.0" );

    QApplication app( argc, argv );

    // Specify the plugin folders 
    // (language codecs and image loaders)
    app.addLibraryPath( "codecs" );
    app.addLibraryPath( "iconengines" );
    app.addLibraryPath( "imageformats" );

    // Set application information for easier use
    // of QSettings classes
    QCoreApplication::setOrganizationName( "Strahinja Markovic" );
    QCoreApplication::setApplicationName( "Sigil" );

    // We set the window icon explicitly on Linux.
    // On Windows this is handled by the RC file,
    // and on Mac by the ICNS file.
 #ifdef Q_WS_X11
    app.setWindowIcon( GetApplicationIcon() );
 #endif

    // We write the full path to Sigil's executable
    // in a file in the home folder for calibre interoperability
#ifdef Q_WS_WIN
    QString location_file = QDir::homePath() + WIN_PATH_SUFFIX + "/sigil-location.txt";
#else
    QString location_file = QDir::homePath() + NIX_PATH_SUFFIX + "/sigil-location.txt";
#endif

    Utility::WriteUnicodeTextFile( QCoreApplication::applicationFilePath(), location_file );

    // Needs to be created on the heap so that
    // the reply has time to return.
    UpdateChecker *checker = new UpdateChecker( &app );
    checker->CheckForUpdate();

    MainWindow *widget = GetMainWindow();    
    widget->show();
    
    return app.exec();
}





