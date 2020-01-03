/************************************************************************
**
**  Copyright (C) 2019   Kevin B. Hendricks, Stratford, Ontario Canada
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
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringList>
#include <QtCore/QStringRef>
#include <QtCore/QTextStream>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>
#include <QtCore/QUuid>
#include <QtWidgets/QMainWindow>
#include <QTextEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFile>
#include <QFileInfo>
#include <QCollator>
#include <QDebug>

#include "sigil_constants.h"
#include "sigil_exception.h"
#include "Misc/QCodePage437Codec.h"
#include "Misc/SettingsStore.h"
#include "Misc/SleepFunctions.h"
#include "MainUI/MainApplication.h"

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
        return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
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
        qDebug() << "Registry Value = " << s.value("AppsUseLightTheme");
        return s.value("AppsUseLightTheme") == 0;
    }
    return false;
}

bool Utility::WindowsShouldUseDarkMode()
{
    QString override(GetEnvironmentVar("SIGIL_USES_DARK_MODE"));
    if (IsWindowsSysDarkMode() || (!override.isEmpty() && override == "1")) {
        return true;
    }
    return false;
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
    return string.midRef(start_index, end_index - start_index);
}
// Replace the first occurrence of string "before"
// with string "after" in string "string"
QString Utility::ReplaceFirst(const QString &before, const QString &after, const QString &string)
{
    int start_index = string.indexOf(before);
    int end_index   = start_index + before.length();
    return Substring(0, start_index, string) + after + Substring(end_index, string.length(), string);
}


QStringList Utility::GetAbsolutePathsToFolderDescendantFiles(const QString &fullfolderpath)
{
    QDir folder(fullfolderpath);
    QStringList files;
    foreach(QFileInfo file, folder.entryInfoList()) {
        if ((file.fileName() != ".") && (file.fileName() != "..")) {
            // If it's a file, add it to the list
            if (file.isFile()) {
                files.append(Utility::URLEncodePath(file.absoluteFilePath()));
            }
            // Else it's a directory, so
            // we add all files from that dir
            else {
                files.append(GetAbsolutePathsToFolderDescendantFiles(file.absoluteFilePath()));
            }
        }
    }
    return files;
}


// Copies every file and folder in the source folder
// to the destination folder; the paths to the folders are submitted;
// the destination folder needs to be created in advance
void Utility::CopyFiles(const QString &fullfolderpath_source, const QString &fullfolderpath_destination)
{
    QDir folder_source(fullfolderpath_source);
    QDir folder_destination(fullfolderpath_destination);
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
                result = QFile::remove(info.absoluteFilePath());
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
    in.setCodec("UTF-8");
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
    out.setCodec("UTF-8");
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
    QString newtext(text);
    return newtext.toHtmlEscaped();
}

QString Utility::URLEncodePath(const QString &path)
{
    QString newpath = path;
    QUrl href = QUrl(newpath);
    QString scheme = href.scheme();
    if (!scheme.isEmpty()) {
        scheme = scheme + "://";
        newpath.remove(0, scheme.length());
    }

    // some very poorly written software uses xml escaping of the 
    // "&" instead of url encoding when building hrefs
    // So run xmldecode first to convert them to normal characters before 
    // url encoding them
    newpath = DecodeXML(newpath);
    QByteArray encoded_url = QUrl::toPercentEncoding(newpath, QByteArray("/#"));
    return scheme + QString::fromUtf8(encoded_url.constData(), encoded_url.count());
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
    message_box.setStandardButtons(QMessageBox::Close);
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

#if 0
// brute force method
QString Utility::longestCommonPath(const QStringList& filepaths, const QString& sep)
{
    // handle special cases
    if (filepaths.isEmpty()) return QString();
    if (filepaths.length() == 1) return QFileInfo(filepaths.at(0)).absolutePath() + sep;
    
    // split each path into its component segments
    QList<QStringList> fpaths;
    int minlen = -1;
    foreach(QString apath, filepaths) {
        QStringList segs = apath.split(sep);
        int n = segs.length();
        if (minlen == -1) minlen = n;
        if (n < minlen) minlen = n;
        fpaths.append(segs);
    }

    // now build up the results
    QStringList res;
    int numpaths = fpaths.length();
    for(int i=0; i < minlen; i++) {
        QString aseg = fpaths.at(0).at(i);
        bool amatch = true;
        int j = 1;
        while(amatch && j < numpaths) {
            amatch = (aseg == fpaths.at(j).at(i));
            j++;
        }
        if (amatch) {
            res << aseg;
        } else {
            break;
        }
    }
    if (res.isEmpty()) return "";
    return res.join(sep) + sep;
}

#else

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

#endif


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

    QStringList dsegs = dest.split(sep, QString::KeepEmptyParts);
    QStringList ssegs = start.split(sep, QString::KeepEmptyParts);
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

std::pair<QString, QString> Utility::parseHREF(const QString &relative_href)
{
    QString fragment;
    QString attpath = relative_href;
    int fragpos = attpath.lastIndexOf("#");
    // fragment will include any # if one exists
    if (fragpos != -1) {
        fragment = attpath.mid(fragpos, -1);
        attpath = attpath.mid(0, fragpos);
    }
    if (attpath.startsWith("./")) attpath = attpath.mid(2,-1);
    return std::make_pair(attpath, fragment);
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
    for(int j=0; j < vec.size(); j++) {
        sortedlst << vec[j].second;
    }
    return sortedlst;
}

QStringList Utility::LocaleAwareSort(QStringList &names)
{
  SettingsStore ss;
  QLocale uiLocale(ss.uiLanguage());
  QCollator uiCollator(uiLocale);
  uiCollator.setCaseSensitivity(Qt::CaseInsensitive);
  // use uiCollator.compare(s1, s2)
  std::sort(names.begin(), names.end(), uiCollator);
  return names;
}
