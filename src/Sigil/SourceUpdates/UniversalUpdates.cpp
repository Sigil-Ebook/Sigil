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

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <QtCore/QtCore>
#include <QtCore/QFutureSynchronizer>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XercesCppUse.h"
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

using boost::make_tuple;
using boost::shared_ptr;
using boost::tie;
using boost::tuple;

QStringList UniversalUpdates::PerformUniversalUpdates(bool resources_already_loaded,
        const QList< Resource * > &resources,
        const QHash< QString, QString > &updates)
{
    QHash< QString, QString > html_updates;
    QHash< QString, QString > css_updates;
    QHash< QString, QString > xml_updates;
    tie(html_updates, css_updates, xml_updates) = SeparateHtmlCssXmlUpdates(updates);
    QList< HTMLResource * > html_resources;
    QList< CSSResource * > css_resources;
    OPFResource *opf_resource = NULL;
    NCXResource *ncx_resource = NULL;
    int num_files = resources.count();

    for (int i = 0; i < num_files; ++i) {
        Resource *resource = resources.at(i);

        if (resource->Type() == Resource::HTMLResourceType) {
            html_resources.append(qobject_cast< HTMLResource * >(resource));
        } else if (resource->Type() == Resource::CSSResourceType) {
            css_resources.append(qobject_cast< CSSResource * >(resource));
        } else if (resource->Type() == Resource::OPFResourceType) {
            opf_resource = qobject_cast< OPFResource * >(resource);
        } else if (resource->Type() == Resource::NCXResourceType) {
            ncx_resource = qobject_cast< NCXResource * >(resource);
        }
    }

    QFutureSynchronizer<void> sync;
    QFuture< QString > html_future;
    QFuture< void > css_future;

    if (resources_already_loaded) {
        html_future = QtConcurrent::mapped(html_resources, boost::bind(UpdateOneHTMLFile, _1, html_updates, css_updates));
        css_future = QtConcurrent::map(css_resources,  boost::bind(UpdateOneCSSFile,  _1, css_updates));
    } else {
        html_future = QtConcurrent::mapped(html_resources, boost::bind(LoadAndUpdateOneHTMLFile, _1, html_updates, css_updates));
        css_future = QtConcurrent::map(css_resources,  boost::bind(LoadAndUpdateOneCSSFile,  _1, css_updates));
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


tuple < QHash< QString, QString >,
      QHash< QString, QString >,
      QHash< QString, QString > >
      UniversalUpdates::SeparateHtmlCssXmlUpdates(const QHash< QString, QString > &updates)
{
    QHash< QString, QString > html_updates = updates;
    QHash< QString, QString > css_updates;
    QHash< QString, QString > xml_updates;
    QList< QString > keys = updates.keys();
    int num_keys = keys.count();

    for (int i = 0; i < num_keys; ++i) {
        QString key_path = keys.at(i);
        QString extension = QFileInfo(key_path).suffix().toLower();
        // The OPF and NCX files are in the OEBPS folder along with the content folders.
        // This means that the "../" prefix is unnecessary and wrong.
        xml_updates[ key_path ] = QString(html_updates.value(key_path)).remove(QRegExp("^../"));

        // Font file updates are CSS updates, not HTML updates
        if (FONT_EXTENSIONS.contains(extension)) {
            css_updates[ key_path ] = html_updates.value(key_path);
            html_updates.remove(key_path);
        } else if (extension == "css") {
            // Needed for CSS updates because of @import rules
            css_updates[ key_path ] = html_updates.value(key_path);
        } else if (IMAGE_EXTENSIONS.contains(extension)) {
            // Needed for CSS updates because of background-image rules
            css_updates[ key_path ] = html_updates.value(key_path);
        }
    }

    return make_tuple(html_updates, css_updates, xml_updates);
}


QString UniversalUpdates::UpdateOneHTMLFile(HTMLResource *html_resource,
        const QHash< QString, QString > &html_updates,
        const QHash< QString, QString > &css_updates)
{
    if (!html_resource) {
        return QString();
    }

    try {
        QWriteLocker locker(&html_resource->GetLock());
        shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
        shared_ptr<xc::DOMDocument> u = PerformHTMLUpdates(*d.get(), html_updates, css_updates)();
        html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(*u.get()));
        return QString();
    } catch (const ErrorBuildingDOM &) {
        // It would be great if we could just let this exception bubble up,
        // but we can't since QtConcurrent doesn't let exceptions cross threads.
        // So we just leave the old source in the resource.
        return QString(QObject::tr("Invalid HTML file: %1")).arg(html_resource->Filename());
    }
}


void UniversalUpdates::UpdateOneCSSFile(CSSResource *css_resource,
                                        const QHash< QString, QString > &css_updates)
{
    if (!css_resource) {
        return;
    }

    QWriteLocker locker(&css_resource->GetLock());
    const QString &source = css_resource->GetText();
    css_resource->SetText(PerformCSSUpdates(source, css_updates)());
}

QString UniversalUpdates::LoadAndUpdateOneHTMLFile(HTMLResource *html_resource,
        const QHash< QString, QString > &html_updates,
        const QHash< QString, QString > &css_updates)
{
    SettingsStore ss;
    QString source;

    if (!html_resource) {
        return QString();
    }

    try {
        source = XhtmlDoc::ResolveCustomEntities(HTMLEncodingResolver::ReadHTMLFile(html_resource->GetFullPath()));

        if (ss.cleanOn() & CLEANON_OPEN) {
            source = CleanSource::Clean(source);
        }
        XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource(source);
        if (error.line != -1) {
            throw QObject::tr("Not well formed, Cannot perform html updates");
        }

        html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(*PerformHTMLUpdates(source, html_updates, css_updates)().get()));
        return QString();
    } catch (const ErrorBuildingDOM &) {
        // It would be great if we could just let this exception bubble up,
        // but we can't since QtConcurrent doesn't let exceptions cross threads.
        // So we just leave the old source in the resource.
        html_resource->SetText(source);
        return QString(QObject::tr("Invalid HTML file: %1")).arg(html_resource->Filename());
    } catch (const QString &err) {
        html_resource->SetText(source);
        return QString("%1: %2").arg(err).arg(html_resource->Filename());
    }
}


void UniversalUpdates::LoadAndUpdateOneCSSFile(CSSResource *css_resource,
        const QHash< QString, QString > &css_updates)
{
    if (!css_resource) {
        return;
    }

    const QString &source = Utility::ReadUnicodeTextFile(css_resource->GetFullPath());
    css_resource->SetText(PerformCSSUpdates(source, css_updates)());
}


QString UniversalUpdates::UpdateOPFFile(OPFResource *opf_resource,
                                        const QHash< QString, QString > &xml_updates)
{
    if (!opf_resource) {
        return QString();
    }

    QWriteLocker locker(&opf_resource->GetLock());
    const QString &source = opf_resource->GetText();

    try {
        shared_ptr< xc::DOMDocument > document = PerformOPFUpdates(source, xml_updates)();
        opf_resource->SetText(XhtmlDoc::GetDomDocumentAsString(*document.get()));
        return QString();
    } catch (const ErrorBuildingDOM &) {
        // It would be great if we could just let this exception bubble up,
        // but we can't since QtConcurrent doesn't let exceptions cross threads.
        // So we just leave the old source in the resource.
        return QString(QObject::tr("Invalid OPF file: %1")).arg(opf_resource->Filename());
    }
}


QString UniversalUpdates::UpdateNCXFile(NCXResource *ncx_resource,
                                        const QHash< QString, QString > &xml_updates)
{
    if (!ncx_resource) {
        return QString();
    }

    QWriteLocker locker(&ncx_resource->GetLock());
    const QString &source = ncx_resource->GetText();

    try {
        shared_ptr< xc::DOMDocument > document = PerformNCXUpdates(source, xml_updates)();
        ncx_resource->SetText(CleanSource::PrettifyDOCTYPEHeader(XhtmlDoc::GetDomDocumentAsString(*document.get())));
        return QString();
    } catch (const ErrorBuildingDOM &) {
        // It would be great if we could just let this exception bubble up,
        // but we can't since QtConcurrent doesn't let exceptions cross threads.
        // So we just leave the old source in the resource.
        return QString(QObject::tr("Invalid NCX file: %1")).arg(ncx_resource->Filename());
    }
}
