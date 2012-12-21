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

#include <QtCore/QByteArray>
#include <QtCore/QCryptographicHash>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "Misc/FontObfuscation.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

static int ADOBE_METHOD_NUM_BYTES = 1024;
static int IDPF_METHOD_NUM_BYTES  = 1040;

namespace
{

QByteArray IdpfKeyFromIdentifier(const QString &identifier)
{
    QString whitespace_free = QString(identifier)
                              .remove("\x20")
                              .remove("\x09")
                              .remove("\x0D")
                              .remove("\x0A");
    return QCryptographicHash::hash(whitespace_free.toLatin1(), QCryptographicHash::Sha1);
}


QByteArray AdobeKeyFromIdentifier(const QString &identifier)
{
    QString cruft_free = QString(identifier)
                         .remove("urn:uuid:")
                         .remove("-")
                         .remove(":");
    return QByteArray::fromHex(cruft_free.toLatin1());
}


void IdpfObfuscate(const QString &filepath, const QString &identifier)
{
    QFile file(filepath);

    if (!file.open(QFile::ReadWrite)) {
        return;
    }

    QByteArray contents = file.readAll();
    QByteArray key = IdpfKeyFromIdentifier(identifier);
    int key_size   = key.size();

    for (int i = 0; (i < IDPF_METHOD_NUM_BYTES) && (i < contents.size()); ++i) {
        contents[ i ] = contents[ i ] ^ key[ i % key_size ];
    }

    file.seek(0);
    file.write(contents);
}


void AdobeObfuscate(const QString &filepath, const QString &identifier)
{
    QFile file(filepath);

    if (!file.open(QFile::ReadWrite)) {
        return;
    }

    QByteArray contents = file.readAll();
    QByteArray key = AdobeKeyFromIdentifier(identifier);
    int key_size   = key.size();

    for (int i = 0; (i < ADOBE_METHOD_NUM_BYTES) && (i < contents.size()); ++i) {
        contents[ i ] = contents[ i ] ^ key[ i % key_size ];
    }

    file.seek(0);
    file.write(contents);
}

};


void FontObfuscation::ObfuscateFile(const QString &filepath,
                                    const QString &algorithm,
                                    const QString &identifier)
{
    if (!QFileInfo(filepath).exists() ||
        algorithm.isEmpty()             ||
        identifier.isEmpty()) {
        boost_throw(FontObfuscationError()
                    << errinfo_font_filepath(filepath.toStdString())
                    << errinfo_font_obfuscation_algorithm(algorithm.toStdString())
                    << errinfo_font_obfuscation_key(identifier.toStdString())
                   );
    }

    if (algorithm == ADOBE_FONT_ALGO_ID) {
        AdobeObfuscate(filepath, identifier);
    } else if (algorithm == IDPF_FONT_ALGO_ID) {
        IdpfObfuscate(filepath, identifier);
    } else {
        boost_throw(FontObfuscationError()
                    << errinfo_font_filepath(filepath.toStdString())
                    << errinfo_font_obfuscation_algorithm(algorithm.toStdString())
                    << errinfo_font_obfuscation_key(identifier.toStdString())
                   );
    }
}


