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

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUrl>
#include <QDebug>

#include "Misc/SettingsStore.h"
#include "Misc/UpdateChecker.h"
#include "sigil_constants.h"

static const QString DOWNLOAD_PAGE_LOCATION  = "http://sigil-ebook.com/get";
static const QString UPDATE_XML_LOCATION     = "https://raw.githubusercontent.com/Sigil-Ebook/Sigil/master/version.xml";
static const QString LAST_ONLINE_VERSION_KEY = "last_online_version";
static const QString LAST_CHECK_TIME_KEY     = "last_check_time";
static const QString SETTINGS_GROUP          = "updatechecker";

// Delta is six hours
static const int SECONDS_BETWEEN_CHECKS      = 60 * 60 * 6 ;


UpdateChecker::UpdateChecker(QObject *parent)
    :
    QObject(parent)
{
}

void UpdateChecker::CheckForUpdate()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The default time is one always longer than the check interval
    QDateTime default_time    = QDateTime::currentDateTime().addSecs(- SECONDS_BETWEEN_CHECKS - 1);
    QDateTime last_check_time = settings.value(LAST_CHECK_TIME_KEY, default_time).toDateTime();
    QString last_online_version    = settings.value(LAST_ONLINE_VERSION_KEY, QString()).toString();

    // We want to check for a new version
    // no sooner than every six hours
    if (last_check_time.secsTo(QDateTime::currentDateTime()) > SECONDS_BETWEEN_CHECKS) {
        settings.setValue(LAST_CHECK_TIME_KEY, QDateTime::currentDateTime());

        int rv = 0;
        QString error_traceback;
        QList<QVariant> args;
        args.append(QVariant(UPDATE_XML_LOCATION));

        EmbeddedPython * epython  = EmbeddedPython::instance();
        QVariant res = epython->runInPython( QString("updatechecker"),
                                         QString("check_for_updates"),
                                         args,
                                         &rv,
                                         error_traceback);    
        if (rv != 0) {
             qDebug() << QString("error in updatechecker check_for_updates: ") + QString::number(rv) + error_traceback;
            return;
        }

        QString current_online_version = res.toString();
        qDebug() << current_online_version;

        if (current_online_version.isEmpty()) {
            return;
        }

        bool is_newer = IsOnlineVersionNewer(SIGIL_VERSION, current_online_version);
        // The message box is displayed only if the online version is newer
        // and only if the user hasn't been informed about this release before
        if (is_newer && (current_online_version != last_online_version)) {
            QMessageBox::StandardButton button_clicked;
            button_clicked = QMessageBox::question(
                                 0,
                                 QObject::tr("Sigil"),
                                 QObject::tr("<p>A newer version of Sigil is available, version <b>%1</b>.<br/>"
                                             "<p>Would you like to go to the download page?</p>")
                                 .arg(current_online_version),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::Yes);

            if (button_clicked == QMessageBox::Yes) {
                QDesktopServices::openUrl(QUrl(DOWNLOAD_PAGE_LOCATION));
            }
        }

        // Store the current online version as the last one checked
        settings.setValue(LAST_ONLINE_VERSION_KEY, current_online_version);
        settings.endGroup();
    }
}


bool UpdateChecker::IsOnlineVersionNewer(const QString &current_version_string,
        const QString &online_version_string)
{
    if (current_version_string.isEmpty() || online_version_string.isEmpty()) {
        return false;
    }

    QRegularExpression current_version_numbers(VERSION_NUMBERS);
    QRegularExpressionMatch current_version_numbers_match = current_version_numbers.match(current_version_string);
    if (!current_version_numbers_match.hasMatch()) {
        return false;
    }

    QRegularExpression online_version_numbers(VERSION_NUMBERS);
    QRegularExpressionMatch online_version_numbers_match = online_version_numbers.match(online_version_string);
    if (!online_version_numbers_match.hasMatch()) {
        return false;
    }

    // This code assumes three digits per field,
    // which should be way more than enough.
    int current_version = current_version_numbers_match.captured(1).toInt() * 1000000 +
                          current_version_numbers_match.captured(2).toInt() * 1000 +
                          current_version_numbers_match.captured(3).toInt();
    int online_version  = online_version_numbers_match.captured(1).toInt() * 1000000 +
                          online_version_numbers_match.captured(2).toInt() * 1000 +
                          online_version_numbers_match.captured(3).toInt();
    return online_version > current_version;
}
