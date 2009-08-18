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

int main( int argc, char *argv[] )
{
    QT_REQUIRE_VERSION( argc, argv, "4.5.0" );

    QApplication app( argc, argv );

    // Specify the plugin folders 
    // (language codecs and image loaders)
    app.addLibraryPath( "codecs" );
    app.addLibraryPath( "iconengines" );
    app.addLibraryPath( "imageformats" );

    // We write the full path to Sigil's executable
    // in a file in the home folder for calibre interoperability
#ifdef Q_WS_WIN
    QString location_file = QDir::homePath() + WIN_PATH_SUFFIX + "/sigil-location.txt";
#else
    QString location_file = QDir::homePath() + NIX_PATH_SUFFIX + "/sigil-location.txt";
#endif

    Utility::WriteUnicodeTextFile( QCoreApplication::applicationFilePath(), location_file );
	
    MainWindow *widget = NULL;
    
    // We use the first argument
    // as the file to load after starting
	QStringList arguments = QCoreApplication::arguments();
	
    if (    arguments.size() > 1 &&
            Utility::IsFileReadable( arguments.at( 1 ) ) 
        )
    {
        widget = new MainWindow( arguments.at( 1 ) );
    }
	
    else
    {
        widget = new MainWindow();
    }
    
    widget->show();
    
    return app.exec();
}


