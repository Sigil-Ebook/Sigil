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
#include <stdafx.h>
#include "qxtconfirmationmessage.h"

static const QLatin1String DEFAULT_ORGANIZATION("QxtGui");
static const QLatin1String DEFAULT_APPLICATION("QxtConfirmationMessage");

class QxtConfirmationMessagePrivate : public QxtPrivate<QxtConfirmationMessage>
{
public:
    QXT_DECLARE_PUBLIC(QxtConfirmationMessage)
    void init(const QString& message = QString());

    QString key() const;
    QString applicationName() const;
    QString organizationName() const;

    int showAgain();
    void doNotShowAgain(int result);
    void reset();

    bool remember;
    QCheckBox* confirm;
    QString overrideApp;
    QString overrideKey;
    QString overrideOrg;

    static QString path;
    static QSettings::Scope scope;
    static QSettings::Format format;
};

QString QxtConfirmationMessagePrivate::path;
QSettings::Scope QxtConfirmationMessagePrivate::scope = QSettings::UserScope;
QSettings::Format QxtConfirmationMessagePrivate::format = QSettings::NativeFormat;

void QxtConfirmationMessagePrivate::init(const QString& message)
{
    remember = false;
    confirm = new QCheckBox(&qxt_p());
    if (!message.isNull())
        confirm->setText(message);
    else
        confirm->setText(QxtConfirmationMessage::tr("Do not show again."));

    QGridLayout* grid = qobject_cast<QGridLayout*>(qxt_p().layout());
    QDialogButtonBox* buttons = qFindChild<QDialogButtonBox*>(&qxt_p());
    if (grid && buttons)
    {
        const int idx = grid->indexOf(buttons);
        int row, column, rowSpan, columnSpan = 0;
        grid->getItemPosition(idx, &row, &column, &rowSpan, &columnSpan);
        QLayoutItem* buttonsItem = grid->takeAt(idx);
        grid->addWidget(confirm, row, column, rowSpan, columnSpan, Qt::AlignLeft | Qt::AlignTop);
        grid->addItem(buttonsItem, ++row, column, rowSpan, columnSpan);
    }
}

QString QxtConfirmationMessagePrivate::key() const
{
    QString value = overrideKey;
    if (value.isEmpty())
    {
        const QString all = qxt_p().windowTitle() + qxt_p().text() + qxt_p().informativeText();
        const QByteArray data = all.toLocal8Bit();
        value = QString::number(qChecksum(data.constData(), data.length()));
    }
    return value;
}

QString QxtConfirmationMessagePrivate::applicationName() const
{
    QString name = overrideApp;
    if (name.isEmpty())
        name = QCoreApplication::applicationName();
    if (name.isEmpty())
        name = DEFAULT_APPLICATION;
    return name;
}

QString QxtConfirmationMessagePrivate::organizationName() const
{
    QString name = overrideOrg;
    if (name.isEmpty())
        name = QCoreApplication::organizationName();
    if (name.isEmpty())
        name = DEFAULT_ORGANIZATION;
    return name;
}

int QxtConfirmationMessagePrivate::showAgain()
{
    QSettings settings(format, scope, organizationName(), applicationName());
    if (!path.isEmpty())
        settings.beginGroup(path);
    return settings.value(key(), -1).toInt();
}

void QxtConfirmationMessagePrivate::doNotShowAgain(int result)
{
    QSettings settings(format, scope, organizationName(), applicationName());
    if (!path.isEmpty())
        settings.beginGroup(path);
    settings.setValue(key(), result);
}

void QxtConfirmationMessagePrivate::reset()
{
    QSettings settings(format, scope, organizationName(), applicationName());
    if (!path.isEmpty())
        settings.beginGroup(path);
    settings.remove(key());
}

/*!
    \class QxtConfirmationMessage
    \inmodule QxtGui
    \brief The QxtConfirmationMessage class provides a confirmation message.

    QxtConfirmationMessage is a confirmation message with checkable
    \bold {"Do not show again."} option. A checked and accepted confirmation
    message is no more shown until reseted.

    Example usage:
    \code
    void MainWindow::closeEvent(QCloseEvent* event)
    {
        static const QString text(tr("Are you sure you want to quit?"));
        if (QxtConfirmationMessage::confirm(this, tr("Confirm"), text) == QMessageBox::No)
            event->ignore();
    }
    \endcode

    \image qxtconfirmationmessage.png "QxtConfirmationMessage in action."

    \bold {Note:} QCoreApplication::organizationName and QCoreApplication::applicationName
    are used for storing settings. In case these properties are empty, \bold "QxtGui" and
    \bold "QxtConfirmationMessage" are used, respectively.
 */

/*!
    Constructs a new QxtConfirmationMessage with \a parent.
 */
QxtConfirmationMessage::QxtConfirmationMessage(QWidget* parent)
        : QMessageBox(parent)
{
    QXT_INIT_PRIVATE(QxtConfirmationMessage);
    qxt_d().init();
}

/*!
    Constructs a new QxtConfirmationMessage with \a icon, \a title, \a text, \a confirmation, \a buttons, \a parent and \a flags.
 */
QxtConfirmationMessage::QxtConfirmationMessage(QMessageBox::Icon icon, const QString& title, const QString& text, const QString& confirmation,
        QMessageBox::StandardButtons buttons, QWidget* parent, Qt::WindowFlags flags)
        : QMessageBox(icon, title, text, buttons, parent, flags)
{
    QXT_INIT_PRIVATE(QxtConfirmationMessage);
    qxt_d().init(confirmation);
}

/*!
    Destructs the confirmation message.
 */
QxtConfirmationMessage::~QxtConfirmationMessage()
{
}

/*!
    Opens an confirmation message box with the specified \a title, \a text and \a confirmation.
    The standard \a buttons are added to the message box. \a defaultButton specifies 
    the button used when Enter is pressed. \a defaultButton must refer to a button that
    was given in \a buttons. If \a defaultButton is QMessageBox::NoButton, QMessageBox
    chooses a suitable default automatically.

    Returns the identity of the standard button that was clicked.
    If Esc was pressed instead, the escape button is returned.

    If \a parent is \c 0, the message box is an application modal dialog box.
    If \a parent is a widget, the message box is window modal relative to \a parent.
 */
QMessageBox::StandardButton QxtConfirmationMessage::confirm(QWidget* parent,
        const QString& title, const QString& text, const QString& confirmation,
        QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QxtConfirmationMessage msgBox(QMessageBox::NoIcon, title, text, confirmation, QMessageBox::NoButton, parent);
    QDialogButtonBox* buttonBox = qFindChild<QDialogButtonBox*>(&msgBox);
    Q_ASSERT(buttonBox != 0);

    uint mask = QMessageBox::FirstButton;
    while (mask <= QMessageBox::LastButton)
    {
        uint sb = buttons & mask;
        mask <<= 1;
        if (!sb)
            continue;
        QPushButton* button = msgBox.addButton((QMessageBox::StandardButton)sb);
        // Choose the first accept role as the default
        if (msgBox.defaultButton())
            continue;
        if ((defaultButton == QMessageBox::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
                || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton)))
            msgBox.setDefaultButton(button);
    }
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

/*!
    \property QxtConfirmationMessage::confirmationText
    \brief the confirmation text

    The default value is \bold {"Do not show again."}
 */
QString QxtConfirmationMessage::confirmationText() const
{
    return qxt_d().confirm->text();
}

void QxtConfirmationMessage::setConfirmationText(const QString& confirmation)
{
    qxt_d().confirm->setText(confirmation);
}

/*!
    \property QxtConfirmationMessage::overrideSettingsApplication
    \brief the override application name used for settings

    QCoreApplication::applicationName is used when no \bold overrideSettingsApplication
    has been set. The application name falls back to \bold "QxtConfirmationMessage"
    when no QCoreApplication::applicationName has been set.

    The default value is an empty string.
 */
QString QxtConfirmationMessage::overrideSettingsApplication() const
{
    return qxt_d().overrideApp;
}

void QxtConfirmationMessage::setOverrideSettingsApplication(const QString& application)
{
    qxt_d().overrideApp = application;
}

/*!
    \property QxtConfirmationMessage::overrideSettingsKey
    \brief the override key used for settings

    When no \bold overrideSettingsKey has been set, the key is calculated with
    qChecksum() based on title, text and confirmation message.

    The default value is an empty string.
 */
QString QxtConfirmationMessage::overrideSettingsKey() const
{
    return qxt_d().overrideKey;
}

void QxtConfirmationMessage::setOverrideSettingsKey(const QString& key)
{
    qxt_d().overrideKey = key;
}

/*!
    \property QxtConfirmationMessage::overrideSettingsOrganization
    \brief the override organization name used for settings

    QCoreApplication::organizationName is used when no \bold overrideSettingsOrganization
    has been set. The organization name falls back to \bold "QxtGui" when no
    QCoreApplication::organizationName has been set.

    The default value is an empty string.
 */
QString QxtConfirmationMessage::overrideSettingsOrganization() const
{
    return qxt_d().overrideOrg;
}

void QxtConfirmationMessage::setOverrideSettingsOrganization(const QString& organization)
{
    qxt_d().overrideOrg = organization;
}

/*!
    \property QxtConfirmationMessage::rememberOnReject
    \brief whether \bold {"Do not show again."} option is stored even
    if the message box is rejected (eg. user presses Cancel).

    The default value is \c false.
 */
bool QxtConfirmationMessage::rememberOnReject() const
{
    return qxt_d().remember;
}

void QxtConfirmationMessage::setRememberOnReject(bool remember)
{
    qxt_d().remember = remember;
}

/*!
    Returns The format used for storing settings.

    The default value is QSettings::NativeFormat.
 */
QSettings::Format QxtConfirmationMessage::settingsFormat()
{
    return QxtConfirmationMessagePrivate::format;
}

/*!
    Sets the \a format used for storing settings.
 */
void QxtConfirmationMessage::setSettingsFormat(QSettings::Format format)
{
    QxtConfirmationMessagePrivate::format = format;
}

/*!
    Returns The scope used for storing settings.

    The default value is QSettings::UserScope.
 */
QSettings::Scope QxtConfirmationMessage::settingsScope()
{
    return QxtConfirmationMessagePrivate::scope;
}

/*!
    Sets the \a scope used for storing settings.
 */
void QxtConfirmationMessage::setSettingsScope(QSettings::Scope scope)
{
    QxtConfirmationMessagePrivate::scope = scope;
}

/*!
    Returns the path used for storing settings.

    The default value is an empty string.
 */
QString QxtConfirmationMessage::settingsPath()
{
    return QxtConfirmationMessagePrivate::path;
}

/*!
    Sets the \a path used for storing settings.
 */
void QxtConfirmationMessage::setSettingsPath(const QString& path)
{
    QxtConfirmationMessagePrivate::path = path;
}

/*!
    Shows the confirmation message if necessary. The confirmation message is not
    shown in case \bold {"Do not show again."} has been checked while the same
    confirmation message was earlierly accepted.

    A confirmation message is identified by the combination of title,
    QMessageBox::text and optional QMessageBox::informativeText.

    A clicked button with role QDialogButtonBox::AcceptRole or
    QDialogButtonBox::YesRole is considered as "accepted".

    \warning This function does not reimplement but shadows QMessageBox::exec().

    \sa QWidget::windowTitle, QMessageBox::text, QMessageBox::informativeText
 */
int QxtConfirmationMessage::exec()
{
    int res = qxt_d().showAgain();
    if (res == -1)
        res = QMessageBox::exec();
    return res;
}

/*!
    \reimp
 */
void QxtConfirmationMessage::done(int result)
{
    QDialogButtonBox* buttons = qFindChild<QDialogButtonBox*>(this);
    Q_ASSERT(buttons != 0);

    int role = buttons->buttonRole(clickedButton());
    if (qxt_d().confirm->isChecked() &&
            (qxt_d().remember || role != QDialogButtonBox::RejectRole))
    {
        qxt_d().doNotShowAgain(result);
    }
    QMessageBox::done(result);
}

/*!
    Resets this instance of QxtConfirmationMessage. A reseted confirmation
    message is shown again until user checks \bold {"Do not show again."} and
    accepts the confirmation message.
 */
void QxtConfirmationMessage::reset()
{
    qxt_d().reset();
}
