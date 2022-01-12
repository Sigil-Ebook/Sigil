/************************************************************************
**
**  Copyright (C) 2015-2022 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2016-2020 Doug Massay
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#ifdef _WIN32
#define NOMINMAX
#endif

#include "unzip.h"

#ifdef _WIN32
#include "iowin32.h"
#endif

#include <stdio.h>
#include <time.h>
#include <string>

#include <utility>
#include <vector>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>
#include <QStringRef>
#include <QTextStream>
#include <QtGlobal>
#include <QUrl>
#include <QUuid>
#include <QMainWindow>
#include <QTextEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QCollator>
#include <QMenu>
#include <QSet>
#include <QVector>
#include <QDebug>

#include "sigil_constants.h"
#include "sigil_exception.h"
#include "Misc/QCodePage437Codec.h"
#include "Misc/SettingsStore.h"
#include "Misc/SleepFunctions.h"
#include "MainUI/MainApplication.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #define QT_ENUM_SKIPEMPTYPARTS Qt::SkipEmptyParts
    #define QT_ENUM_KEEPEMPTYPARTS Qt::KeepEmptyParts
#else
    #define QT_ENUM_SKIPEMPTYPARTS QString::SkipEmptyParts
    #define QT_ENUM_KEEPEMPTYPARTS QString::KeepEmptyParts
#endif

static const QString URL_SAFE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-/~";

static const QString DARK_STYLE =
    "<style>:root { background-color: %1; color: %2; } ::-webkit-scrollbar { display: none; }</style>"
    "<link rel=\"stylesheet\" type=\"text/css\" href=\"%3\" />";

#ifndef MAX_PATH
// Set Max length to 256 because that's the max path size on many systems.
#define MAX_PATH 256
#endif
// This is the same read buffer size used by Java and Perl.
#define BUFF_SIZE 8192

static QCodePage437Codec *cp437 = 0;

// Subclass QMessageBox for our StdWarningDialog to make any Details Resizable
class SigilMessageBox: public QMessageBox
{
    public:
        SigilMessageBox(QWidget* parent) : QMessageBox(parent) 
        {
            setSizeGripEnabled(true);
        }
    private:
        virtual void resizeEvent(QResizeEvent * e) {
            QMessageBox::resizeEvent(e);
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            if (QWidget *textEdit = findChild<QTextEdit *>()) {
                textEdit->setMaximumHeight(QWIDGETSIZE_MAX);
            }
        }
};

#include "Misc/Utility.h"


// Define the user preferences location to be used
QString Utility::DefinePrefsDir()
{
    // If the SIGIL_PREFS_DIR environment variable override exists; use it.
    // It's up to the user to provide a directory they have permission to write to.
    if (!SIGIL_PREFS_DIR.isEmpty()) {
        return SIGIL_PREFS_DIR;
    } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
        return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#endif
    }
}

bool Utility::IsDarkMode()
{
#ifdef Q_OS_MAC
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    return mainApplication->isDarkMode();
#else
    // Windows, Linux and Other platforms
    QPalette app_palette = qApp->palette();
    bool isdark = app_palette.color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    return isdark;
#endif
}

bool Utility::IsWindowsSysDarkMode()
{
    QSettings s("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    if (s.status() == QSettings::NoError) {
        // qDebug() << "Registry Value = " << s.value("AppsUseLightTheme");
        return s.value("AppsUseLightTheme") == 0;
    }
    return false;
}

bool Utility::WindowsShouldUseDarkMode()
{
    QString override(GetEnvironmentVar("SIGIL_USES_DARK_MODE"));
    if (override.isEmpty()) {
        //Env var unset - use system registry setting.
        return IsWindowsSysDarkMode();
    }
    // Otherwise use the env var: anything other than "0" is true.
    return (override == "0" ? false : true);
}

#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
// Return correct path(s) for Linux hunspell dictionaries
QStringList Utility::LinuxHunspellDictionaryDirs()
{
    QStringList paths;
    // prefer the directory specified by the env var SIGIL_DICTIONARIES above all else.
    if (!hunspell_dicts_override.isEmpty()) {
        // Handle multiple colon-delimited paths
        foreach (QString s, hunspell_dicts_override.split(":")) {
            paths << s.trimmed();
        }
    }
    // else use the env var runtime overridden 'share/sigil/hunspell_dictionaries/' location.
    else if (!sigil_extra_root.isEmpty()) {
        paths.append(sigil_extra_root + "/hunspell_dictionaries/");
    }
    // Bundled dicts were not installed use standard system dictionary location.
    else if (!dicts_are_bundled) {
        paths.append("/usr/share/hunspell");
        // Add additional hunspell dictionary directories. Provided at compile
        // time via the cmake option EXTRA_DICT_DIRS (colon separated list).
        if (!extra_dict_dirs.isEmpty()) {
            foreach (QString s, extra_dict_dirs.split(":")) {
                paths << s.trimmed();
            }
        }
    }
    else {
        // else use the standard build time 'share/sigil/hunspell_dictionaries/'location.
        paths.append(sigil_share_root + "/hunspell_dictionaries/");
    }
    return paths;
}
#endif


// Uses QUuid to generate a random UUID but also removes
// the curly braces that QUuid::createUuid() adds
QString Utility::CreateUUID()
{
    return QUuid::createUuid().toString().remove("{").remove("}");
}


// Convert the casing of the text, returning the result.
QString Utility::ChangeCase(const QString &text, const Utility::Casing &casing)
{
    if (text.isEmpty()) {
        return text;
    }

    switch (casing) {
        case Utility::Casing_Lowercase: {
            return text.toLower();
        }

        case Utility::Casing_Uppercase: {
            return text.toUpper();
        }

        case Utility::Casing_Titlecase: {
            // This is a super crude algorithm, could be replaced by something more clever.
            QString new_text = text.toLower();
            // Skip past any leading spaces
            int i = 0;

            while (i < text.length() && new_text.at(i).isSpace()) {
                i++;
            }

            while (i < text.length()) {
                if (i == 0 || new_text.at(i - 1).isSpace()) {
                    new_text.replace(i, 1, new_text.at(i).toUpper());
                }

                i++;
            }

            return new_text;
        }

        case Utility::Casing_Capitalize: {
            // This is a super crude algorithm, could be replaced by something more clever.
            QString new_text = text.toLower();
            // Skip past any leading spaces
            int i = 0;

            while (i < text.length() && new_text.at(i).isSpace()) {
                i++;
            }

            if (i < text.length()) {
                new_text.replace(i, 1, new_text.at(i).toUpper());
            }

            return new_text;
        }

        default:
            return text;
    }
}


// Returns true if the string is mixed case, false otherwise.
// For instance, "test" and "TEST" return false, "teSt" returns true.
// If the string is empty, returns false.
bool Utility::IsMixedCase(const QString &string)
{
    if (string.isEmpty() || string.length() == 1) {
        return false;
    }

    bool first_char_lower = string[ 0 ].isLower();

    for (int i = 1; i < string.length(); ++i) {
        if (string[ i ].isLower() != first_char_lower) {
            return true;
        }
    }

    return false;
}

// Returns a substring from a specified QStringRef;
// the characters included are in the interval:
// [ start_index, end_index >
QString Utility::Substring(int start_index, int end_index, const QStringRef &string)
{
    return string.mid(start_index, end_index - start_index).toString();
}


// Returns a substring of a specified string;
// the characters included are in the interval:
// [ start_index, end_index >
QString Utility::Substring(int start_index, int end_index, const QString &string)
{
    return string.mid(start_index, end_index - start_index);

}

// Returns a substring of a specified string;
// the characters included are in the interval:
// [ start_index, end_index >
QStringRef Utility::SubstringRef(int start_index, int end_index, const QString &string)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return string.midRef(start_index, end_index - start_index);
#else
    return QStringRef(&string, start_index, end_index - start_index);
#endif
}

// Replace the first occurrence of string "before"
// with string "after" in string "string"
QString Utility::ReplaceFirst(const QString &before, const QString &after, const QString &string)
{
    int start_index = string.indexOf(before);
    int end_index   = start_index + before.length();
    return Substring(0, start_index, string) + after + Substring(end_index, string.length(), string);
}


// Copies every file and folder in the source folder
// to the destination folder; the paths to the folders are submitted;
// the destination folder needs to be created in advance
void Utility::CopyFiles(const QString &fullfolderpath_source, const QString &fullfolderpath_destination)
{
    QDir folder_source(fullfolderpath_source);
    QDir folder_destination(fullfolderpath_destination);
    folder_source.setFilter(QDir::AllDirs |
                            QDir::Files |
                            QDir::NoDotAndDotDot |
                            QDir::NoSymLinks |
                            QDir::Hidden);
    // Erase all the files in this folder
    foreach(QFileInfo file, folder_source.entryInfoList()) {
        if ((file.fileName() != ".") && (file.fileName() != "..")) {
            // If it's a file, copy it
            if (file.isFile()) {
                QString destination = fullfolderpath_destination + "/" + file.fileName();
                bool success = QFile::copy(file.absoluteFilePath(), destination);

                if (!success) {
                    std::string msg = file.absoluteFilePath().toStdString() + ": " + destination.toStdString();
                    throw(CannotCopyFile(msg));
                }
            }
            // Else it's a directory, copy everything in it
            // to a new folder of the same name in the destination folder
            else {
                folder_destination.mkpath(file.fileName());
                CopyFiles(file.absoluteFilePath(), fullfolderpath_destination + "/" + file.fileName());
            }
        }
    }
}



//
//   Delete a directory along with all of its contents.
//
//   \param dirName Path of directory to remove.
//   \return true on success; false on error.
//
bool Utility::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            } else {
                result = SDeleteFile(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}



// Deletes the specified file if it exists
bool Utility::SDeleteFile(const QString &fullfilepath)
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if (!QFileInfo(fullfilepath).exists()) {
        return false;
    }

    QFile file(fullfilepath);
    bool deleted = file.remove();
    // Some multiple file deletion operations fail on Windows, so we try once more.
    if (!deleted) {
        qApp->processEvents();
        SleepFunctions::msleep(100);
        deleted = file.remove();
    }
    return deleted;
}


// Copies File from full Inpath to full OutPath with overwrite if needed
bool Utility::ForceCopyFile(const QString &fullinpath, const QString &fulloutpath)
{
    if (!QFileInfo(fullinpath).exists()) {
        return false;
    }
    if (QFileInfo::exists(fulloutpath)) {
        Utility::SDeleteFile(fulloutpath);
    }
    return QFile::copy(fullinpath, fulloutpath);
}


// Needed to add the S to this routine name to prevent collisions on Windows
// We had to do the same thing for DeleteFile earlier
bool Utility::SMoveFile(const QString &oldfilepath, const QString &newfilepath)
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if (!QFileInfo(oldfilepath).exists()) {
        return false;
    }

    // check if these are identical files on the file system
    // and if so no copy and delete sequence is needed
    if (QFileInfo(oldfilepath) == QFileInfo(newfilepath)) {
        return true;
    }

    // Ensure that the newfilepath doesn't already exist but due to case insenstive file systems
    // check if we are actually moving to an identical path with a different case.
    if (QFileInfo(newfilepath).exists() && QFileInfo(oldfilepath) != QFileInfo(newfilepath)) {
        return false;
    }

    // copy file from old file path to new file path
    bool success = QFile::copy(oldfilepath, newfilepath);
    // if and only if copy succeeds then delete old file 
    if (success) {
        Utility::SDeleteFile(oldfilepath);
    }
    return success;
}


bool Utility::RenameFile(const QString &oldfilepath, const QString &newfilepath)
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if (!QFileInfo(oldfilepath).exists()) {
        return false;
    }

    // Ensure that the newfilepath doesn't already exist but due to case insenstive file systems
    // check if we are actually renaming to an identical path with a different case.
    if (QFileInfo(newfilepath).exists() && QFileInfo(oldfilepath) != QFileInfo(newfilepath)) {
        return false;
    }

    // On case insensitive file systems, QFile::rename fails when the new name is the
    // same (case insensitive) to the old one. This is workaround for that issue.
    int ret = -1;
#if defined(Q_OS_WIN32)
    ret = _wrename(Utility::QStringToStdWString(oldfilepath).data(), Utility::QStringToStdWString(newfilepath).data());
#else
    ret = rename(oldfilepath.toUtf8().data(), newfilepath.toUtf8().data());
#endif

    if (ret == 0) {
        return true;
    }

    return false;
}


QString Utility::GetTemporaryFileNameWithExtension(const QString &extension)
{
    SettingsStore ss;
    QString temp_path = ss.tempFolderHome();
    if (temp_path == "<SIGIL_DEFAULT_TEMP_HOME>") {
        temp_path = QDir::tempPath();
    }
    return temp_path +  "/sigil_" + Utility::CreateUUID() + extension;
}


// Returns true if the file can be read;
// shows an error dialog if it can't
// with a message elaborating what's wrong
bool Utility::IsFileReadable(const QString &fullfilepath)
{
    // Qt has <QFileInfo>.exists() and <QFileInfo>.isReadable()
    // functions, but then we would have to create our own error
    // message for each of those situations (and more). Trying to
    // actually open the file enables us to retrieve the exact
    // reason preventing us from reading the file in an error string.
    QFile file(fullfilepath);

    // Check if we can open the file
    if (!file.open(QFile::ReadOnly)) {
        Utility::DisplayStdErrorDialog(
            QObject::tr("Cannot read file %1:\n%2.")
            .arg(fullfilepath)
            .arg(file.errorString())
        );
        return false;
    }

    file.close();
    return true;
}


// Reads the text file specified with the full file path;
// text needs to be in UTF-8 or UTF-16; if the file cannot
// be read, an error dialog is shown and an empty string returned
QString Utility::ReadUnicodeTextFile(const QString &fullfilepath)
{
    // TODO: throw an exception instead of
    // returning an empty string
    QFile file(fullfilepath);

    // Check if we can open the file
    if (!file.open(QFile::ReadOnly)) {
        std::string msg = fullfilepath.toStdString() + ": " + file.errorString().toStdString();
        throw(CannotOpenFile(msg));
    }

    QTextStream in(&file);
    // Input should be UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif
    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode(true);
    return ConvertLineEndings(in.readAll());
}


// Writes the provided text variable to the specified
// file; if the file exists, it is truncated
void Utility::WriteUnicodeTextFile(const QString &text, const QString &fullfilepath)
{
    QFile file(fullfilepath);

    if (!file.open(QIODevice::WriteOnly |
                   QIODevice::Truncate  |
                   QIODevice::Text
                  )
       ) {
        std::string msg = file.fileName().toStdString() + ": " + file.errorString().toStdString();
        throw(CannotOpenFile(msg));
    }

    QTextStream out(&file);
    // We ALWAYS output in UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif
    out << text;
}


// Converts Mac and Windows style line endings to Unix style
// line endings that are expected throughout the Qt framework
QString Utility::ConvertLineEndings(const QString &text)
{
    QString newtext(text);
    return newtext.replace("\x0D\x0A", "\x0A").replace("\x0D", "\x0A");
}


// Decodes XML escaped string to normal text                                                                                
// &amp; -> "&"    &apos; -> "'"  &quot; -> "\""   &lt; -> "<"  &gt; -> ">"
QString Utility::DecodeXML(const QString &text)
{
    QString newtext(text);
    newtext.replace("&apos;", "'");
    newtext.replace("&quot;", "\"");
    newtext.replace("&lt;", "<");
    newtext.replace("&gt;", ">");
    newtext.replace("&amp;", "&");
    return newtext;
}

QString Utility::EncodeXML(const QString &text)
{
    QString newtext = Utility::DecodeXML(text);
    return newtext.toHtmlEscaped();
}



// From the IRI spec rfc3987
// iunreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~" / ucschar
// 
//    ucschar        = %xA0-D7FF / %xF900-FDCF / %xFDF0-FFEF
//                   / %x10000-1FFFD / %x20000-2FFFD / %x30000-3FFFD
//                   / %x40000-4FFFD / %x50000-5FFFD / %x60000-6FFFD
//                   / %x70000-7FFFD / %x80000-8FFFD / %x90000-9FFFD
//                   / %xA0000-AFFFD / %xB0000-BFFFD / %xC0000-CFFFD
//                   / %xD0000-DFFFD / %xE1000-EFFFD
// But currently nothing *after* the 0x30000 plane is even defined

bool Utility::NeedToPercentEncode(uint32_t cp)
{
    // sequence matters for both correctness and speed
    if (cp < 128) {
        if (URL_SAFE.contains(QChar(cp))) return false;
        return true;
    }
    if (cp < 0xA0) return true;
    if (cp <= 0xD7FF) return false;
    if (cp < 0xF900) return true;
    if (cp <= 0xFDCF) return false;
    if (cp < 0xFDF0) return true;
    if (cp <= 0xFFEF) return false;
    if (cp < 0x10000) return true;
    if (cp <= 0x1FFFD) return false;
    if (cp < 0x20000) return true;
    if (cp <= 0x2FFFD) return false;
    if (cp < 0x30000) return true;
    if (cp <= 0x3FFFD) return false;
    return true;
}

// this is meant to work on paths, not paths and fragments and schemes
// therefore do not leave # chars unencoded
QString Utility::URLEncodePath(const QString &path)
{
    // some very poorly written software uses xml escaping of the 
    // "&" instead of url encoding when building hrefs
    // So run xmldecode first to convert them to normal characters before 
    // url encoding them
    QString newpath = DecodeXML(path);

    // then undo any existing url encoding
    newpath = URLDecodePath(newpath);

    QString result = "";
    QVector<uint32_t> codepoints = newpath.toUcs4();
    for (int i = 0; i < codepoints.size(); i++) {
        uint32_t cp = codepoints.at(i);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QString s = QString::fromUcs4(&cp, 1);
#else
        QString s = QString::fromUcs4(reinterpret_cast<char32_t *>(&cp), 1);
#endif
        if (NeedToPercentEncode(cp)) {
            QByteArray b = s.toUtf8();
            for (int j = 0; j < b.size(); j++) {
                uint8_t bval = b.at(j);
                QString val = QString::number(bval,16);
                val = val.toUpper();
                if (val.size() == 1) val.prepend("0");
                val.prepend("%");
                result.append(val);
            }
        } else {
            result.append(s);
        }
    }
    // qDebug() << "In Utility URLEncodePath: " << result;
    // Previously was:
    // encoded_url = QUrl::toPercentEncoding(newpath, QByteArray("/"), QByteArray("#"));
    // encoded_path = scheme + QString::fromUtf8(encoded_url.constData(), encoded_url.count());
    return result;
}


QString Utility::URLDecodePath(const QString &path)
{
    QString apath(path);
    // some very poorly written software uses xml-escape on hrefs
    // instead of properly url encoding them, so look for the
    // the "&" character which should *not* exist if properly
    // url encoded and if found try to xml decode them first
    apath = DecodeXML(apath);
    return QUrl::fromPercentEncoding(apath.toUtf8());
}


void Utility::DisplayExceptionErrorDialog(const QString &error_info)
{
    QMessageBox message_box(QApplication::activeWindow());
    message_box.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    message_box.setModal(true);
    message_box.setIcon(QMessageBox::Critical);
    message_box.setWindowTitle("Sigil");
    // Spaces are added to the end because otherwise the dialog is too small.
    message_box.setText(QObject::tr("Sigil has encountered a problem.") % "                                                                                                       ");
    message_box.setInformativeText(QObject::tr("Sigil may need to close."));
    message_box.setStandardButtons(QMessageBox::Close);
    QStringList detailed_text;
    detailed_text << "Error info: "    + error_info
                  << "Sigil version: " + QString(SIGIL_FULL_VERSION)
                  << "Runtime Qt: "    + QString(qVersion())
                  << "Compiled Qt: "   + QString(QT_VERSION_STR)
                  << "System: "        + QSysInfo::prettyProductName()
                  << "Architecture: "  + QSysInfo::currentCpuArchitecture();

    message_box.setDetailedText(detailed_text.join("\n"));
    message_box.exec();
}


void Utility::DisplayStdErrorDialog(const QString &error_message, const QString &detailed_text)
{
    QMessageBox message_box(QApplication::activeWindow());
    message_box.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    message_box.setModal(true);
    message_box.setIcon(QMessageBox::Critical);
    message_box.setWindowTitle("Sigil");
    message_box.setText(error_message);

    if (!detailed_text.isEmpty()) {
        message_box.setDetailedText(detailed_text);
    }

    message_box.setStandardButtons(QMessageBox::Close);
    message_box.exec();
}


void Utility::DisplayStdWarningDialog(const QString &warning_message, const QString &detailed_text)
{
    SigilMessageBox message_box(QApplication::activeWindow());
    message_box.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    message_box.setModal(true);
    message_box.setIcon(QMessageBox::Warning);
    message_box.setWindowTitle("Sigil");
    message_box.setText(warning_message);
    message_box.setTextFormat(Qt::RichText);

    if (!detailed_text.isEmpty()) {
        message_box.setDetailedText(detailed_text);
    }
    message_box.setStandardButtons(QMessageBox::Ok);
    message_box.exec();
}

// Returns a value for the environment variable name passed;
// if the env var isn't set, it returns an empty string
QString Utility::GetEnvironmentVar(const QString &variable_name)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    // The only time this might fall down is on Linux when an
    // environment variable holds bytedata. Don't use this
    // utility function for retrieval if that's the case.
    return qEnvironmentVariable(variable_name.toUtf8().constData(), "").trimmed();
#else
    // This will typically only be used on older Qts on Linux
    return QProcessEnvironment::systemEnvironment().value(variable_name, "").trimmed();
#endif
}


// Returns the same number, but rounded to one decimal place
float Utility::RoundToOneDecimal(float number)
{
    return QString::number(number, 'f', 1).toFloat();
}


QWidget *Utility::GetMainWindow()
{
    QWidget *parent_window = QApplication::activeWindow();
    while (parent_window && !(qobject_cast<QMainWindow *>(parent_window))) {
        parent_window = parent_window->parentWidget();
    }

    return parent_window;
}


QString Utility::getSpellingSafeText(const QString &raw_text)
{
    // There is currently a problem with Hunspell if we attempt to pass
    // words with smart apostrophes from the CodeView encoding.
    // Hunspell dictionaries typically store their main wordlist using
    // the dumb apostrophe variants only to save space and speed checking
    QString text(raw_text);
    text.replace(QChar(0x00ad),"");
    return text.replace(QChar(0x2019),QChar(0x27));
}


bool Utility::has_non_ascii_chars(const QString &str)
{
    QRegularExpression not_ascii("[^\\x00-\\x7F]");
    QRegularExpressionMatch mo = not_ascii.match(str);
    return mo.hasMatch();
}

bool Utility::use_filename_warning(const QString &filename)
{
    if (has_non_ascii_chars(filename)) {
        return QMessageBox::Apply == QMessageBox::warning(QApplication::activeWindow(),
                tr("Sigil"),
                tr("The requested file name contains non-ASCII characters. "
                   "You should only use ASCII characters in filenames. "
                   "Using non-ASCII characters can prevent the EPUB from working "
                   "with some readers.\n\n"
                   "Continue using the requested filename?"),
                QMessageBox::Cancel|QMessageBox::Apply);
    }
    return true;
}

#if defined(Q_OS_WIN32)
std::wstring Utility::QStringToStdWString(const QString &str)
{
    return std::wstring((const wchar_t *)str.utf16());
}

QString Utility::stdWStringToQString(const std::wstring &str)
{
    return QString::fromUtf16((const ushort *)str.c_str());
}
#endif


bool Utility::UnZip(const QString &zippath, const QString &destpath)
{
    int res = 0;
    QDir dir(destpath);
    if (!cp437) {
        cp437 = new QCodePage437Codec();
    }
#ifdef Q_OS_WIN32
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64W(&ffunc);
    unzFile zfile = unzOpen2_64(Utility::QStringToStdWString(QDir::toNativeSeparators(zippath)).c_str(), &ffunc);
#else
    unzFile zfile = unzOpen64(QDir::toNativeSeparators(zippath).toUtf8().constData());
#endif

    if ((zfile == NULL) || (!IsFileReadable(zippath)) || (!dir.exists())) {
        return false;
    }

    res = unzGoToFirstFile(zfile);

    if (res == UNZ_OK) {
        do {
            // Get the name of the file in the archive.
            char file_name[MAX_PATH] = {0};
            unz_file_info64 file_info;
            unzGetCurrentFileInfo64(zfile, &file_info, file_name, MAX_PATH, NULL, 0, NULL, 0);
            QString qfile_name;
            QString cp437_file_name;
            qfile_name = QString::fromUtf8(file_name);
            if (!(file_info.flag & (1<<11))) {
                // General purpose bit 11 says the filename is utf-8 encoded. If not set then
                // IBM 437 encoding might be used.
                cp437_file_name = cp437->toUnicode(file_name);
            }

            // If there is no file name then we can't do anything with it.
            if (!qfile_name.isEmpty()) {

                // for security reasons against maliciously crafted zip archives
                // we need the file path to always be inside the target folder 
                // and not outside, so we will remove all illegal backslashes
                // and all relative upward paths segments "/../" from the zip's local 
                // file name/path before prepending the target folder to create 
                // the final path

                QString original_path = qfile_name;
                bool evil_or_corrupt_epub = false;

                if (qfile_name.contains("\\")) evil_or_corrupt_epub = true; 
                qfile_name = "/" + qfile_name.replace("\\","");

                if (qfile_name.contains("/../")) evil_or_corrupt_epub = true;
                qfile_name = qfile_name.replace("/../","/");
    
                while(qfile_name.startsWith("/")) { 
                    qfile_name = qfile_name.remove(0,1);
                }
                    
                if (cp437_file_name.contains("\\")) evil_or_corrupt_epub = true; 
                cp437_file_name = "/" + cp437_file_name.replace("\\","");

                if (cp437_file_name.contains("/../")) evil_or_corrupt_epub = true;
                cp437_file_name = cp437_file_name.replace("/../","/");
    
                while(cp437_file_name.startsWith("/")) { 
                  cp437_file_name = cp437_file_name.remove(0,1);
                }

                if (evil_or_corrupt_epub) {
                    unzCloseCurrentFile(zfile);
                    unzClose(zfile);
                    // throw (UNZIPLoadParseError(QString(QObject::tr("Possible evil or corrupt zip file name: %1")).arg(original_path).toStdString()));
                    return false;
                }

                // We use the dir object to create the path in the temporary directory.
                // Unfortunately, we need a dir ojbect to do this as it's not a static function.
                // Full file path in the temporary directory.
                QString file_path = destpath + "/" + qfile_name;
                QFileInfo qfile_info(file_path);

                // Is this entry a directory?
                if (file_info.uncompressed_size == 0 && qfile_name.endsWith('/')) {
                    dir.mkpath(qfile_name);
                    continue;
                } else {
                    if (!qfile_info.path().isEmpty()) dir.mkpath(qfile_info.path());
                }

                // Open the file entry in the archive for reading.
                if (unzOpenCurrentFile(zfile) != UNZ_OK) {
                    unzClose(zfile);
                    return false;
                }

                // Open the file on disk to write the entry in the archive to.
                QFile entry(file_path);

                if (!entry.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    unzCloseCurrentFile(zfile);
                    unzClose(zfile);
                    return false;
                }

                // Buffered reading and writing.
                char buff[BUFF_SIZE] = {0};
                int read = 0;

                while ((read = unzReadCurrentFile(zfile, buff, BUFF_SIZE)) > 0) {
                    entry.write(buff, read);
                }

                entry.close();

                // Read errors are marked by a negative read amount.
                if (read < 0) {
                    unzCloseCurrentFile(zfile);
                    unzClose(zfile);
                    return false;
                }

                // The file was read but the CRC did not match.
                // We don't check the read file size vs the uncompressed file size
                // because if they're different there should be a CRC error.
                if (unzCloseCurrentFile(zfile) == UNZ_CRCERROR) {
                    unzClose(zfile);
                    return false;
                }

                if (!cp437_file_name.isEmpty() && cp437_file_name != qfile_name) {
                    QString cp437_file_path = destpath + "/" + cp437_file_name;
                    QFile::copy(file_path, cp437_file_path);
                }
            }
        } while ((res = unzGoToNextFile(zfile)) == UNZ_OK);
    }

    if (res != UNZ_END_OF_LIST_OF_FILE) {
        unzClose(zfile);
        return false;
    }

    unzClose(zfile);
    return true;
}

QStringList Utility::ZipInspect(const QString &zippath)
{
    QStringList filelist;
    int res = 0;

    if (!cp437) {
        cp437 = new QCodePage437Codec();
    }
#ifdef Q_OS_WIN32
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64W(&ffunc);
    unzFile zfile = unzOpen2_64(Utility::QStringToStdWString(QDir::toNativeSeparators(zippath)).c_str(), &ffunc);
#else
    unzFile zfile = unzOpen64(QDir::toNativeSeparators(zippath).toUtf8().constData());
#endif

    if ((zfile == NULL) || (!IsFileReadable(zippath))) {
        return filelist;
    }
    res = unzGoToFirstFile(zfile);
    if (res == UNZ_OK) {
        do {
            // Get the name of the file in the archive.
            char file_name[MAX_PATH] = {0};
            unz_file_info64 file_info;
            unzGetCurrentFileInfo64(zfile, &file_info, file_name, MAX_PATH, NULL, 0, NULL, 0);
            QString qfile_name;
            QString cp437_file_name;
            qfile_name = QString::fromUtf8(file_name);
            if (!(file_info.flag & (1<<11))) {
                cp437_file_name = cp437->toUnicode(file_name);
            }

            // If there is no file name then we can't do anything with it.
            if (!qfile_name.isEmpty()) {
                if (!cp437_file_name.isEmpty() && cp437_file_name != qfile_name) {
                    filelist.append(cp437_file_name);
                } else {
                    filelist.append(qfile_name);
                }
            }
        } while ((res = unzGoToNextFile(zfile)) == UNZ_OK);
    }
    unzClose(zfile);
    return filelist;
}

// some utilities for working with absolute and book relative paths

QString Utility::longestCommonPath(const QStringList& filepaths, const QString& sep)
{
    if (filepaths.isEmpty()) return QString();
    if (filepaths.length() == 1) return QFileInfo(filepaths.at(0)).absolutePath() + sep;
    QStringList fpaths(filepaths);
    fpaths.sort();
    const QStringList segs1 = fpaths.first().split(sep);
    const QStringList segs2 = fpaths.last().split(sep);
    QStringList res;
    int i = 0;
    while((i < segs1.length()) && (i < segs2.length()) && (segs1.at(i) == segs2.at(i))) {
        res.append(segs1.at(i));
        i++;
    }
    if (res.length() == 0) return sep;
    return res.join(sep) + sep;
}


// works with absolute paths and book (internal to epub) paths
QString Utility::resolveRelativeSegmentsInFilePath(const QString& file_path, const QString &sep)
{
    const QStringList segs = file_path.split(sep);
    QStringList res;
    for (int i = 0; i < segs.length(); i++) {
        // FIXME skip empty segments but not at the front when windows
        if (segs.at(i) == ".") continue;
        if (segs.at(i) == "..") {
            if (!res.isEmpty()) {
                res.removeLast();
            } else {
                qDebug() << "Error resolving relative path segments";
                qDebug() << "original file path: " << file_path;
            }
        } else {
            res << segs.at(i);
        }
    }
    return res.join(sep);
}


// Generate relative path to destination from starting directory path
// Both paths should be cannonical
QString Utility::relativePath(const QString & destination, const QString & start_dir)
{
    QString dest(destination);
    QString start(start_dir);

    // first handle the special case
    if (start_dir.isEmpty()) return destination;

    QChar sep = '/';

    // remove any trailing path separators from both paths
    while (dest.endsWith(sep)) dest.chop(1);
    while (start.endsWith(sep)) start.chop(1);

    QStringList dsegs = dest.split(sep, QT_ENUM_KEEPEMPTYPARTS);
    QStringList ssegs = start.split(sep, QT_ENUM_KEEPEMPTYPARTS);
    QStringList res;
    int i = 0;
    int nd = dsegs.size();
    int ns = ssegs.size();
    // skip over starting common path segments in both paths 
    while (i < ns && i < nd && (dsegs.at(i) == ssegs.at(i))) {
        i++;
    }
    // now "move up" for each remaining path segment in the starting directory
    int p = i;
    while (p < ns) {
        res.append("..");
        p++;
    }
    // And append the remaining path segments from the destination 
    p = i;
    while(p < nd) {
        res.append(dsegs.at(p));
        p++;
    }
    return res.join(sep);
}

// dest_relpath is the relative path to the destination file
// start_folder is the *book path* (path internal to the epub) to the starting folder
QString Utility::buildBookPath(const QString& dest_relpath, const QString& start_folder)
{
    QString bookpath(start_folder);
    while (bookpath.endsWith("/")) bookpath.chop(1);
    if (!bookpath.isEmpty()) { 
        bookpath = bookpath + "/" + dest_relpath;
    } else {
        bookpath = dest_relpath;
    }
    bookpath = resolveRelativeSegmentsInFilePath(bookpath, "/");
    return bookpath;
}

// no ending path separator
QString Utility::startingDir(const QString &file_bookpath)
{
    QString start_dir(file_bookpath);
    int pos = start_dir.lastIndexOf('/');
    if (pos > -1) { 
        start_dir = start_dir.left(pos);
    } else {
        start_dir = "";
    }
    return start_dir;
}

// This is the equivalent of Resource.cpp's GetRelativePathFromResource but using book paths
QString Utility::buildRelativePath(const QString &from_file_bkpath, const QString & to_file_bkpath)
{
    // handle special case of "from" and "to" being identical
    if (from_file_bkpath == to_file_bkpath) return "";

    // convert start_file_bkpath to start_dir by stripping off existing filename component
    return relativePath(to_file_bkpath, startingDir(from_file_bkpath));
}

// return fully decoded path and fragment (if any) from a raw relative href string.
// any fragment will start with '#', use QUrl to handle parsing and Percent Decoding
std::pair<QString, QString> Utility::parseRelativeHREF(const QString &relative_href)
{
    QUrl href(relative_href);
    Q_ASSERT(href.isRelative());
    Q_ASSERT(!href.hasQuery());
    QString attpath = href.path();
    QString fragment = href.fragment();
    // fragment will include any # if fragment exists
    if (relative_href.indexOf("#") != -1) {
        fragment = "#" + fragment;
    }
    if (attpath.startsWith("./")) attpath = attpath.mid(2,-1);
    return std::make_pair(attpath, fragment);
}

// return a url encoded string for given decoded path and fragment (if any)
// Note: Any fragment will start with a "#" ! to allow links to root as just "#"
QString Utility::buildRelativeHREF(const QString &apath, const QString &afrag)
{
    QString newhref = URLEncodePath(apath);
    QString id = afrag;
    if (!id.isEmpty()) {
        if (id.startsWith("#")) {
            id = id.mid(1, -1);
        } else {
            qDebug() << "Warning: buildRelativeHREF has fragment that does not start with #" << afrag;
        }
        // technically fragments should be percent encoded if needed
        id = URLEncodePath(id);
        newhref = newhref + "#" + id;
    }
    return newhref;
}


bool Utility::sort_string_pairs_by_first(const std::pair<QString, QString> &a, const std::pair<QString, QString> &b)
{
    return (QString::localeAwareCompare(a.first, b.first) < 0);
}


bool Utility::sort_pair_in_reverse(const std::pair<int,QString> &a, const std::pair<int,QString> &b)
{
    return (a.first > b.first);
}

QStringList Utility::sortByCounts(const QStringList &folderlst, const QList<int> &countlst)
{
    std::vector< std::pair<int , QString> > vec;
    int i = 0;
    foreach(QString afolder, folderlst) {
        vec.push_back(std::make_pair(countlst.at(i++), afolder));
    }
    std::sort(vec.begin(), vec.end(), sort_pair_in_reverse);
    QStringList sortedlst;
    for(unsigned int j=0; j < vec.size(); j++) {
        sortedlst << vec[j].second;
    }
    return sortedlst;
}

QStringList Utility::LocaleAwareSort(const QStringList &names)
{
    SettingsStore ss;
    QStringList nlist(names);
    QLocale uiLocale(ss.uiLanguage());
    QCollator uiCollator(uiLocale);
    uiCollator.setCaseSensitivity(Qt::CaseInsensitive);
    // use uiCollator.compare(s1, s2)
    std::sort(nlist.begin(), nlist.end(), uiCollator);
    return nlist;
}



QString Utility::AddDarkCSS(const QString &html)
{
    QString text = html;
    int endheadpos = text.indexOf("</head>");
    if (endheadpos == -1) return text;
    QPalette pal = qApp->palette();
    QString back = pal.color(QPalette::Base).name();
    QString fore = pal.color(QPalette::Text).name();
#ifdef Q_OS_MAC
    // on macOS the Base role is used for the background not the Window role
    QString dark_css_url = "qrc:///dark/mac_dark_scrollbar.css";
#elif defined(Q_OS_WIN32)
    QString dark_css_url = "qrc:///dark/win_dark_scrollbar.css";
#else
    QString dark_css_url = "qrc:///dark/lin_dark_scrollbar.css";
#endif
    QString inject_dark_style = DARK_STYLE.arg(back).arg(fore).arg(dark_css_url);
    // qDebug() << "Injecting dark style: ";
    text.insert(endheadpos, inject_dark_style);
    return text;
}


QColor Utility::WebViewBackgroundColor(bool followpref)
{
    QColor back_color = Qt::white;
    if (IsDarkMode()) {
        if (followpref) {
            SettingsStore ss;
            if (!ss.previewDark()) {
                return back_color;    
            }
        }
        QPalette pal = qApp->palette();
        back_color = pal.color(QPalette::Base);
    }
    return back_color; 
}

QBrush Utility::ValidationResultBrush(const Val_Msg_Type &valres)
{
    if (Utility::IsDarkMode()) {
        switch (valres) {
            case Utility::INFO_BRUSH: {
               return QBrush(QColor(114, 165, 212)); 
            }
            case Utility::WARNING_BRUSH: {
                return QBrush(QColor(212, 165, 114));
            }
            case Utility::ERROR_BRUSH: {
                return QBrush(QColor(222, 94, 94));
            }
            default:
                QPalette pal = qApp->palette();
                return QBrush(pal.color(QPalette::Text));
        }
    } else {
        switch (valres) {
            case Utility::INFO_BRUSH: {
                return QBrush(QColor(224, 255, 255));;
            }
            case Utility::WARNING_BRUSH: {
                return QBrush(QColor(255, 255, 230));
            }
            case Utility::ERROR_BRUSH: {
                return QBrush(QColor(255, 230, 230));
            }
            default:
                QPalette pal = qApp->palette();
                return QBrush(pal.color(QPalette::Window));
        }
    }
}


QString Utility::createCSVLine(const QStringList &data)
{
    QStringList csvline;
    foreach(QString val, data) {
        bool need_quotes = val.contains(',');
        QString cval = "";
        if (need_quotes) cval.append('"');
        foreach(QChar c, val) {
            if (c == '"') cval.append('"');
            cval.append(c);
        }
        if (need_quotes) cval.append('"');
        csvline.append(cval);
    }
    return csvline.join(',');
}


QStringList Utility::parseCSVLine(const QString &data)
{
    auto unquote_val = [](const QString &av) {
        QString nv(av);
        if (nv.startsWith('"')) nv = nv.mid(1);
        if (nv.endsWith('"')) nv = nv.mid(0, nv.length()-1);
        return nv;
    };
    
    bool in_quote = false;
    QStringList vals;
    QString v;
    int n = data.size();
    int i = 0;
    while(i < n) {
        QChar c = data.at(i);
        if (!in_quote) {
            if (c == ',') {
                vals.append(unquote_val(v.trimmed()));
                v = "";
            } else  {
                v.append(c);
                if (c == '"') in_quote = true;
            }
        } else {
            v.append(c);
            if (c == '"') {
                if ((i+1 < n) && (data.at(i+1) == '"')) { 
                    i++;
                } else {
                    in_quote = false;
                }
            }
        }
        i++;
    }
    if (!v.isEmpty()) vals.append(unquote_val(v.trimmed()));
    return vals;
}


QString Utility::GenerateUniqueId(const QString &id, const QSet<QString>& Used)
{
    int cnt = 1;
    QString new_id = id + "_" + QString::number(cnt);
    while (Used.contains(new_id)) {
        cnt++;
        new_id = id + "_" + QString::number(cnt);
    }
    return new_id;
}

 
const quint32 CRC32_TAB[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
 
QString Utility::FileCRC32(const QString& filePath)
{
	QFile file(filePath);
	char buf[BUFF_SIZE];
	quint32 crc32 = 0;
	qint64 n = 0;
 
	if(!file.open(QIODevice::ReadOnly)) return "";
 
	crc32 = 0xffffffff;
	while((n = file.read(buf, 1)) > 0) {
		for(qint64 i = 0; i < n; i++) {
			crc32 = (crc32 >> 8) ^ CRC32_TAB[((crc32 ^ buf[i]) & 0xff)];
		}
	}
	crc32 ^= 0xffffffff;

	file.close();
	return QString("%1").arg(crc32, 8, 16, QLatin1Char('0'));
}
