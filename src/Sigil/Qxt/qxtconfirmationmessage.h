/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#ifndef QXTCONFIRMATIONMESSAGE_H
#define QXTCONFIRMATIONMESSAGE_H

#include <QtCore/QSettings>
#include <QtGui/QMessageBox>

#define QXT_STATIC

#include "qxtglobal.h"

class QxtConfirmationMessagePrivate;

class QXT_GUI_EXPORT QxtConfirmationMessage : public QMessageBox
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtConfirmationMessage)
    Q_PROPERTY(QString confirmationText READ confirmationText WRITE setConfirmationText)
    Q_PROPERTY(QString overrideSettingsApplication READ overrideSettingsApplication WRITE setOverrideSettingsApplication)
    Q_PROPERTY(QString overrideSettingsKey READ overrideSettingsKey WRITE setOverrideSettingsKey)
    Q_PROPERTY(QString overrideSettingsOrganization READ overrideSettingsOrganization WRITE setOverrideSettingsOrganization)
    Q_PROPERTY(bool rememberOnReject READ rememberOnReject WRITE setRememberOnReject)

public:
    explicit QxtConfirmationMessage(QWidget* parent = 0);
    virtual ~QxtConfirmationMessage();

    QxtConfirmationMessage(QMessageBox::Icon icon,
                           const QString& title, const QString& text, const QString& confirmation = QString(),
                           QMessageBox::StandardButtons buttons = QMessageBox::NoButton, QWidget* parent = 0,
                           Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    static QMessageBox::StandardButton confirm(QWidget* parent,
            const QString& title, const QString& text, const QString& confirmation = QString(),
            QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QString confirmationText() const;
    void setConfirmationText(const QString& confirmation);

    QString overrideSettingsApplication() const;
    void setOverrideSettingsApplication(const QString& application);

    QString overrideSettingsKey() const;
    void setOverrideSettingsKey(const QString& key);

    QString overrideSettingsOrganization() const;
    void setOverrideSettingsOrganization(const QString& organization);

    bool rememberOnReject() const;
    void setRememberOnReject(bool remember);

    static QSettings::Format settingsFormat();
    static void setSettingsFormat(QSettings::Format format);

    static QSettings::Scope settingsScope();
    static void setSettingsScope(QSettings::Scope scope);

    static QString settingsPath();
    static void setSettingsPath(const QString& path);

public Q_SLOTS:
    int exec();
    void reset();
    virtual void done(int result);
};

#endif // QXTCONFIRMATIONMESSAGE_H
