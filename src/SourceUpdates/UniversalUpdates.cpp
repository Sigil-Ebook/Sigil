/************************************************************************
**
**  Copyright (C) 2015  Kevin B. Hendricks  Stratford, ON Canada
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

#include <memory>
#include <functional>

#include <QtCore/QtCore>
#include <QtCore/QFutureSynchronizer>
#include <QtConcurrent/QtConcurrent>
#include <QRegularExpression>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/HTMLEncodingResolver.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "SourceUpdates/PerformHTMLUpdates.h"
#include "SourceUpdates/PerformCSSUpdates.h"
#include "SourceUpdates/PerformNCXUpdates.h"
#include "SourceUpdates/PerformOPFUpdates.h"
#include "SourceUpdates/UniversalUpdates.h"

#define NON_WELL_FORMED_MESSAGE "Cannot perform HTML updates since the file is not well formed"

QStringList UniversalUpdates::PerformUniversalUpdates(bool resources_already_loaded,
        const QList<Resource *> &resources,
        const QHash<QString, QString> &updates,
        const QList<XMLResource *> &non_well_formed)
{
    QStringList updatekeys = updates.keys();
    QHash<QString, QString> html_updates;
    QHash<QString, QString> css_updates;
    QHash<QString, QString> xml_updates;
    std::tie(html_updates, css_updates, xml_updates) = SeparateHtmlCssXmlUpdates(updates);
    QList<HTMLResource *> html_resources;
    QList<CSSResource *> css_resources;
    OPFResource *opf_resource = NULL;
    NCXResource *ncx_resource = NULL;
    int num_files = resources.count();

    for (int i = 0; i < num_files; ++i) {
        Resource *resource = resources.at(i);

        if (resource->Type() == Resource::HTMLResourceType) {
            html_resources.append(qobject_cast<HTMLResource *>(resource));
        } else if (resource->Type() == Resource::CSSResourceType) {
            css_resources.append(qobject_cast<CSSResource *>(resource));
        } else if (resource->Type() == Resource::OPFResourceType) {
            opf_resource = qobject_cast<OPFResource *>(resource);
        } else if (resource->Type() == Resource::NCXResourceType) {
            ncx_resource = qobject_cast<NCXResource *>(resource);
        }
    }

    QFutureSynchronizer<void> sync;
    QFuture<QString> html_future;
    QFuture<void> css_future;

    if (resources_already_loaded) {
        html_future = QtConcurrent::mapped(html_resources, std::bind(UpdateOneHTMLFile, std::placeholders::_1, html_updates, css_updates));
        css_future = QtConcurrent::map(css_resources,  std::bind(UpdateOneCSSFile,  std::placeholders::_1, css_updates));
    } else {
        html_future = QtConcurrent::mapped(html_resources, std::bind(LoadAndUpdateOneHTMLFile, std::placeholders::_1, html_updates, css_updates, non_well_formed));
        css_future = QtConcurrent::map(css_resources,  std::bind(LoadAndUpdateOneCSSFile,  std::placeholders::_1, css_updates));
    }

    sync.addFuture(html_future);
    sync.addFuture(css_future);
    // We can't schedule these with QtConcurrent because they
    // will (indirectly) call QTextDocument::setPlainText, and if
    // a tab is open for the ncx/opf, then an event needs to be sent
    // to the tab widget. Events can't cross threads, and we crash.
    const QString ncx_result = UpdateNCXFile(ncx_resource, xml_updates);
    const QString opf_result = UpdateOPFFile(opf_resource, xml_updates);
    sync.waitForFinished();
    // Now assemble our list of errors if any.
    QStringList load_update_errors;

    for (int i = 0; i < html_future.results().count(); i++) {
        const QString html_error = html_future.resultAt(i);

        if (!html_error.isEmpty()) {
            load_update_errors.append(html_error);
        }
    }

    if (!ncx_result.isEmpty()) {
        load_update_errors.append(ncx_result);
    }

    if (!opf_result.isEmpty()) {
        load_update_errors.append(opf_result);
    }

    return load_update_errors;
}


std::tuple <QHash<QString, QString>,
      QHash<QString, QString>,
      QHash<QString, QString>>
      UniversalUpdates::SeparateHtmlCssXmlUpdates(const QHash<QString, QString> &updates)
{
    QHash<QString, QString> html_updates = updates;
    QHash<QString, QString> css_updates;
    QHash<QString, QString> xml_updates;
    QList<QString> keys = updates.keys();
    int num_keys = keys.count();

    for (int i = 0; i < num_keys; ++i) {
        QString key_path = keys.at(i);
        QString extension = QFileInfo(key_path).suffix().toLower();
        // The OPF and NCX files are in the OEBPS folder along with the content folders.
        // This means that the "../" prefix is unnecessary and wrong.

        xml_updates[ key_path ] = QString(html_updates.value(key_path)).remove(QRegularExpression("^../"));

        // Font file updates are CSS updates, not HTML updates
        // Actually with SVG font-face-uri tag and epub3 this is no longer true
        // They can appear in both html and css
        if (FONT_EXTENSIONS.contains(extension)) {
            css_updates[ key_path ] = html_updates.value(key_path);
            // html_updates.remove(key_path);
        } else if (extension == "css") {
            // Needed for CSS updates because of @import rules
            css_updates[ key_path ] = html_updates.value(key_path);
        } else if (IMAGE_EXTENSIONS.contains(extension)) {
            // Needed for CSS updates because of background-image rules
            css_updates[ key_path ] = html_updates.value(key_path);
        }
    }
    return std::make_tuple(html_updates, css_updates, xml_updates);
}


QString UniversalUpdates::UpdateOneHTMLFile(HTMLResource *html_resource,
        const QHash<QString, QString> &html_updates,
        const QHash<QString, QString> &css_updates)
{
    if (!html_resource) {
        return QString();
    }

    try {
        QWriteLocker locker(&html_resource->GetLock());
        QString currentpath = html_resource->GetCurrentBookRelPath();
        QString version = html_resource->GetEpubVersion();
        QString source = html_resource->GetText();
        QString newsource = source;
        newsource = PerformHTMLUpdates(newsource, html_updates, css_updates, currentpath, version)();
        html_resource->SetText(newsource);
        html_resource->SetCurrentBookRelPath("");
        return QString();
    } catch (ErrorBuildingDOM) {
        // It would be great if we could just let this exception bubble up,
        // but we can't since QtConcurrent doesn't let exceptions cross threads.
        // So we just leave the old source in the resource.
        return QString(QObject::tr("Invalid HTML file: %1")).arg(html_resource->Filename());
    }
}


void UniversalUpdates::UpdateOneCSSFile(CSSResource *css_resource,
                                        const QHash<QString, QString> &css_updates)
{
    if (!css_resource) {
        return;
    }

    QWriteLocker locker(&css_resource->GetLock());
    const QString &source = css_resource->GetText();
    css_resource->SetText(PerformCSSUpdates(source, css_updates)());
}

QString UniversalUpdates::LoadAndUpdateOneHTMLFile(HTMLResource *html_resource,
        const QHash<QString, QString> &html_updates,
        const QHash<QString, QString> &css_updates,
        const QList<XMLResource *> &non_well_formed)
{
    SettingsStore ss;
    QString source;

    if (!html_resource) {
        return QString();
    }

    QString currentpath = html_resource->GetCurrentBookRelPath();
    QString version = html_resource->GetEpubVersion();

    // non_well_formed will only be set if the user has chosen not to have
    // the file auto fixed.
    if (non_well_formed.contains(html_resource)) {
        return QString("%1: %2").arg(NON_WELL_FORMED_MESSAGE).arg(html_resource->Filename());
    }

    try {
        source = XhtmlDoc::ResolveCustomEntities(html_resource->GetText());
        source = CleanSource::CharToEntity(source);

        if (ss.cleanOn() & CLEANON_OPEN) {
            source = CleanSource::Mend(source, version);
        }
        // Even though well formed checks might have already run we need to double check because cleaning might
        // have tried to fix and may have failed or the user may have said to skip cleanning.
        if (!XhtmlDoc::IsDataWellFormed(source)) {
            throw QObject::tr(NON_WELL_FORMED_MESSAGE);
        }

        source = PerformHTMLUpdates(source, html_updates, css_updates, currentpath, version)();
        html_resource->SetCurrentBookRelPath("");
        // For files that are valid we need to do a second clean becasue PerformHTMLUpdates) will remove
        // the formatting.
        if (ss.cleanOn() & CLEANON_OPEN) {
            source = CleanSource::Mend(source, version);
        }
        html_resource->SetText(source);
        return QString();
    } catch (ErrorBuildingDOM) {
        // It would be great if we could just let this exception bubble up,
        // but we can't since QtConcurrent doesn't let exceptions cross threads.
        // So we just leave the old source in the resource.
        return QString(QObject::tr("Invalid HTML file: %1")).arg(html_resource->Filename());
    } catch (QString err) {
        return QString("%1: %2").arg(err).arg(html_resource->Filename());
    } catch (...) {
        return QString("Cannot perform HTML updates there was an unrecoverable error: %1").arg(html_resource->Filename());
    }
}


void UniversalUpdates::LoadAndUpdateOneCSSFile(CSSResource *css_resource,
        const QHash<QString, QString> &css_updates)
{
    if (!css_resource) {
        return;
    }

    const QString &source = Utility::ReadUnicodeTextFile(css_resource->GetFullPath());
    css_resource->SetText(PerformCSSUpdates(source, css_updates)());
    css_resource->SaveToDisk();
}


QString UniversalUpdates::UpdateOPFFile(OPFResource *opf_resource,
                                        const QHash<QString, QString> &xml_updates)
{
    if (!opf_resource) {
        return QString();
    }

    QWriteLocker locker(&opf_resource->GetLock());
    const QString &source = opf_resource->GetText();
    QString currentpath = opf_resource->GetCurrentBookRelPath();
    try {
        QString newsource = PerformOPFUpdates(source, xml_updates, currentpath)();
        opf_resource->SetText(newsource);
        opf_resource->SetCurrentBookRelPath("");
        return QString();
    } catch (ErrorBuildingDOM) {
        // It would be great if we could just let this exception bubble up,
        // but we can't since QtConcurrent doesn't let exceptions cross threads.
        // So we just leave the old source in the resource.
        return QString(QObject::tr("Invalid OPF file: %1")).arg(opf_resource->Filename());
    }
}


QString UniversalUpdates::UpdateNCXFile(NCXResource *ncx_resource,
                                        const QHash<QString, QString> &xml_updates)
{
    if (!ncx_resource) {
        return QString();
    }

    QWriteLocker locker(&ncx_resource->GetLock());
    const QString &source = ncx_resource->GetText();
    QString currentpath = ncx_resource->GetCurrentBookRelPath();

    try {
        QString newsource = PerformNCXUpdates(source, xml_updates, currentpath)();
        ncx_resource->SetText(CleanSource::PrettifyDOCTYPEHeader(newsource));
        ncx_resource->SetCurrentBookRelPath("");
        return QString();
    } catch (ErrorBuildingDOM) {
        // It would be great if we could just let this exception bubble up,
        // but we can't since QtConcurrent doesn't let exceptions cross threads.
        // So we just leave the old source in the resource.
        return QString(QObject::tr("Invalid NCX file: %1")).arg(ncx_resource->Filename());
    }
}
