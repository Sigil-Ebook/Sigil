/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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

#pragma once
#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H

#include <QtCore/QSettings>
#include <QtCore/QString>

/**
 * Singleton. Provides access for reading and writing user configurable
 * settings.
 */
class SettingsStore : public QSettings
{
    Q_OBJECT

    Q_PROPERTY(QString defaultMetadataLang READ defaultMetadataLang WRITE setDefaultMetadataLang NOTIFY settingsChanged)
    Q_PROPERTY(Qt::Orientation splitViewOrientation READ splitViewOrientation WRITE setSplitViewOrientation NOTIFY settingsChanged)
    Q_PROPERTY(bool splitViewOrder READ splitViewOrder WRITE setSplitViewOrder NOTIFY settingsChanged)
    Q_PROPERTY(float zoomImage READ zoomImage WRITE setZoomImage NOTIFY settingsChanged)
    Q_PROPERTY(float zoomText READ zoomText WRITE setZoomText NOTIFY settingsChanged)
    Q_PROPERTY(float zoomWeb READ zoomWeb WRITE setZoomWeb NOTIFY settingsChanged)
    Q_PROPERTY(QString dictionary READ dictionary WRITE setDictionary NOTIFY settingsChanged)
    Q_PROPERTY(QString renameTemplate READ renameTemplate WRITE setRenameTemplate NOTIFY settingsChanged)

public:
    /**
     * The accessor function to access the store.
     */
    static SettingsStore *instance();

    /**
     * The default langauge to use when creating new books.
     *
     * @return The language as a string.
     */
    QString defaultMetadataLang();
    /**
     * The orientation to use for the split view.
     *
     * @return The orientation.
     */
    Qt::Orientation splitViewOrientation();
    /**
     * The order to use for the split view (book view/code view).
     *
     * @return The order
     */
    bool splitViewOrder();

    /**
     * The zoom factor used by the component.
     *
     * @return The zoom factor.
     */
    float zoomImage();
    float zoomText();
    float zoomWeb();

    /**
     * The name of the dictionary to use for spell check.
     *
     * @return The dictionary name.
     */
    QString dictionary();

    /**
     * The template name for renaming selections in book browser
     *
     * @return The template name.
     */
    QString renameTemplate();

public slots:

    /**
     * Set the default language to use when creating new books.
     *
     * @param lang The language to set.
     */
    void setDefaultMetadataLang(const QString &lang);
    /**
     * Set the orientation of the split view.
     *
     * @param orientation The orientation to set.
     */
    void setSplitViewOrientation(Qt::Orientation orientation);
    /**
     * Set the order of the split view.
     *
     * @param order The order of book view/code view to set.
     */
    void setSplitViewOrder(bool order);

    /**
     * Set the zoom factor used by the component.
     *
     * @param zoom The zoom factor.
     */
    void setZoomImage(float zoom);
    void setZoomText(float zoom);
    void setZoomWeb(float zoom);

    /**
     * Set the name of the dictionary the user has selected.
     *
     * @param name The name of the dictionary.
     */
    void setDictionary(const QString &name);

    /**
     * Set the name of the dictionary the user has selected.
     *
     * @param name The name of the dictionary.
     */
    void setRenameTemplate(const QString &name);

signals:
    /**
     * Signals that settings have changed.
     *
     * This is not emitted after a set* function call. This is only emitted
     * by a call from triggerSettingsChanged. This is because we do not want
     * this signal to emit multiple times when a varity of settings are changed
     * via the preferences dialog.
     */
    void settingsChanged();

private:
    /**
     * Privateructor.
     */
    SettingsStore();

    /**
     * Ensures there is not open setting group which will cause the settings
     * This class implements in the wrong place.
     */
    void clearSettingsGroup();

    static SettingsStore *m_instance;
};

#endif // SETTINGSSTORE_H
