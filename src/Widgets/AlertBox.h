// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Modifications Copyright(C) 2025 Kevin B. Hendricks Stratford, Ontario Canada

#ifndef ALERTBOX_H
#define ALERTBOX_H

#include <QSize>
#include <QString>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QAbstractButton>

class AlertBoxDetailsText;
class DetailButton;

class AlertBox : public QDialog

{
    Q_OBJECT

public:

    enum Icon {
        // keep this in sync with QMessageDialogOptions::StandardIcon
        NoIcon = 0,
        Information = 1,
        Warning = 2,
        Critical = 3,
        Question = 4
    };


    AlertBox(QWidget *parent);

    AlertBox(Icon icon, const QString &title, const QString &text,
             QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::NoButton, QWidget *parent = nullptr,
            Qt::WindowFlags flags = Qt::Dialog);
    ~AlertBox();

    void addButton(QAbstractButton *button, QDialogButtonBox::ButtonRole role);
    QPushButton *addButton(const QString &text, QDialogButtonBox::ButtonRole role);
    QPushButton *addButton(QDialogButtonBox::StandardButton button);

    void removeButton(QAbstractButton *button);

    QList<QAbstractButton *> buttons() const;

    QDialogButtonBox::ButtonRole buttonRole(QAbstractButton *button) const;

    void setStandardButtons(QDialogButtonBox::StandardButtons buttons);
    QDialogButtonBox::StandardButtons standardButtons() const;
    QDialogButtonBox::StandardButton standardButton(QAbstractButton *button) const;
    QAbstractButton *button(QDialogButtonBox::StandardButton which) const;

    static QDialogButtonBox::StandardButton standardButtonForRole(QDialogButtonBox::ButtonRole role);

    QPushButton *defaultButton() const;
    void setDefaultButton(QPushButton *button);
    void setDefaultButton(QDialogButtonBox::StandardButton button);

    QAbstractButton *clickedButton() const;
    void setClickedButton(QAbstractButton *button);
    int execReturnCode(QAbstractButton *button);

    QString text() const;
    void setText(const QString &text);

    Icon icon() const;
    void setIcon(Icon);

    QPixmap iconPixmap() const;
    void setIconPixmap(const QPixmap &pixmap);

    Qt::TextFormat textFormat() const;
    void setTextFormat(Qt::TextFormat format);

    QString detailedText() const;
    void setDetailedText(const QString &text);

    void setWindowTitle(const QString &title);
    void setWindowModality(Qt::WindowModality windowModality);

    static QPixmap standardIcon(Icon icon);
    static QPixmap standardIcon(AlertBox::Icon icon, AlertBox *mb);

    void init(const QString &title = QString(), const QString &text = QString());
    void setupLayout();

    void updateSize();
    int layoutMinimumWidth();
    

public slots:
    void buttonClicked(QAbstractButton* button);

signals:
    void buttonWasClicked(QAbstractButton *button);

protected:
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *e);

private:
    QLabel*       m_label = nullptr;;
    QLabel*       m_iconLabel = nullptr;;
    AlertBox::Icon m_icon;
    QDialogButtonBox * m_buttonBox = nullptr;;
    DetailButton *m_detailsButton = nullptr;
    AlertBoxDetailsText * m_detailsText = nullptr;;
    QAbstractButton * m_buttonClicked = nullptr;;
    QList<QAbstractButton*> m_customButtonList;
    QAbstractButton * m_defaultButton = nullptr;;

};
#endif
