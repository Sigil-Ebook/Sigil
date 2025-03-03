// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Modifications Copyright (C) 2025 Kevin B. Hendricks, Stratford, Ontario Canada

#include "Widgets/AlertBox.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QApplication>
#include <QList>
#include <QDebug>
#include <QGridLayout>
#include <QPushButton>
#include <QIcon>
#include <QTextDocument>
#include <QTextEdit>
#include <QMenu>
#include <QDialog>
#include <QFont>
#include <QFontMetrics>
#include <QClipboard>
#include <QAbstractButton>
#include <QAnyStringView>
#include <QPointer>
#include <QContextMenuEvent>
#include <QStyle>
#include <QStyleOptionButton>

class AlertBoxDetailsText : public QWidget
{
    Q_OBJECT
public:
    class TextEdit : public QTextEdit
    {
    public:
        TextEdit(QWidget *parent=nullptr) : QTextEdit(parent) { }
        void contextMenuEvent(QContextMenuEvent * e) override
        {
            if (QMenu *menu = createStandardContextMenu()) {
                menu->setAttribute(Qt::WA_DeleteOnClose);
                menu->popup(e->globalPos());
            }
        }
    };

    AlertBoxDetailsText(QWidget *parent=nullptr)
        : QWidget(parent)
        , copyAvailable(false)
    {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(QMargins());
        QFrame *line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);
        textEdit = new TextEdit();
        setMinimumHeight(200);
        setMaximumHeight(10000);
        setMinimumWidth(200);
        setMaximumWidth(10000);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        textEdit->setFocusPolicy(Qt::NoFocus);
        textEdit->setReadOnly(true);
        layout->addWidget(textEdit);
        setLayout(layout);

        connect(textEdit, &TextEdit::copyAvailable,
                this, &AlertBoxDetailsText::textCopyAvailable);
    }

    void setText(const QString &text) { textEdit->setPlainText(text); }
    QString text() const { return textEdit->toPlainText(); }

    bool copy()
    {
        if (!copyAvailable)
            return false;
        textEdit->copy();
        return true;
    }

    void selectAll()
    {
        textEdit->selectAll();
    }

private slots:
    void textCopyAvailable(bool available)
    {
        copyAvailable = available;
    }

private:
    bool copyAvailable;
    TextEdit *textEdit;
};



class DetailButton : public QPushButton
{
public:
    DetailButton(QWidget *parent) : QPushButton(AlertBox::tr("Show Details..."), parent)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    QString getlabel(bool show) const
    { QString curlbl = show ? AlertBox::tr("Show Details...") : AlertBox::tr("Hide Details...");
      return curlbl; }

    void setLabel(QString lbl)
    { setText(lbl); }

    QSize sizeHint() const override
    {
        ensurePolished();
        QStyleOptionButton opt;
        initStyleOption(&opt);
        const QFontMetrics fm = fontMetrics();
        opt.text = getlabel(true);
        QSize sz = fm.size(Qt::TextShowMnemonic, opt.text);
        QSize ret = style()->sizeFromContents(QStyle::CT_PushButton, &opt, sz, this);
        opt.text = getlabel(false);
        sz = fm.size(Qt::TextShowMnemonic, opt.text);
        ret = ret.expandedTo(style()->sizeFromContents(QStyle::CT_PushButton, &opt, sz, this));
#ifdef Q_OS_MAC
        // macOS needs a bit more room or the button border is lost
        return QSize(ret.width()+5, ret.height()+5);
#else
        return ret;
#endif
    }
};


Qt::TextFormat AlertBox::textFormat() const
{
    return m_label->textFormat();
}

void AlertBox::setTextFormat(Qt::TextFormat format)
{
    m_label->setTextFormat(format);
    m_label->setWordWrap(format == Qt::RichText
                    || (format == Qt::AutoText && Qt::mightBeRichText(m_label->text())));
    updateSize();
}

AlertBox::AlertBox(QWidget *parent)
    : QDialog(parent)
{
    init();
}

AlertBox::AlertBox(Icon icon, const QString &title, const QString &text,
                   QDialogButtonBox::StandardButtons buttons, QWidget *parent,
                   Qt::WindowFlags f)
: QDialog(parent)
{
    init(title, text);
    setIcon(icon);
    if (buttons != QDialogButtonBox::NoButton)
        setStandardButtons(buttons);
}


void AlertBox::init(const QString &title, const QString &text)
{

    m_label = new QLabel(text, this);
    m_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_iconLabel = new QLabel(this);
    m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_buttonBox = new QDialogButtonBox(this);
    // m_buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, this));
    connect(m_buttonBox, &QDialogButtonBox::clicked, this, &AlertBox::buttonClicked);

    setupLayout();
    if (!title.isEmpty() || !text.isEmpty()) {
        setWindowTitle(title);
        setText(text);
    }
    setModal(true);
#ifdef Q_OS_MAC
    QFont f = font();
    f.setBold(true);
    m_label->setFont(f);
#endif
    m_icon = AlertBox::NoIcon;
}


void AlertBox::addButton(QAbstractButton *button, QDialogButtonBox::ButtonRole role)
{
    if (!button)
        return;
    removeButton(button);
    if (button->text().isEmpty()) {
        qDebug() << "Cannot add" << button << "without title";
        return;
    }
    m_buttonBox->addButton(button, (QDialogButtonBox::ButtonRole)role);
    m_customButtonList.append(button);
}

QDialogButtonBox::StandardButton AlertBox::standardButtonForRole(QDialogButtonBox::ButtonRole role)
{
    switch (role) {
    case QDialogButtonBox::AcceptRole: return QDialogButtonBox::Ok;
    case QDialogButtonBox::RejectRole: return QDialogButtonBox::Cancel;
    case QDialogButtonBox::DestructiveRole: return QDialogButtonBox::Discard;
    case QDialogButtonBox::HelpRole: return QDialogButtonBox::Help;
    case QDialogButtonBox::ApplyRole: return QDialogButtonBox::Apply;
    case QDialogButtonBox::YesRole: return QDialogButtonBox::Yes;
    case QDialogButtonBox::NoRole: return QDialogButtonBox::No;
    case QDialogButtonBox::ResetRole: return QDialogButtonBox::Reset;
    default: return QDialogButtonBox::NoButton;
    }
}

QPushButton *AlertBox::addButton(const QString& text, QDialogButtonBox::ButtonRole role)
{
    QPushButton *pushButton = new QPushButton(text);
    addButton(pushButton, role);
    updateSize();
    return pushButton;
}

QPushButton *AlertBox::addButton(QDialogButtonBox::StandardButton button)
{
    QPushButton *pushButton = m_buttonBox->addButton((QDialogButtonBox::StandardButton)button);
    if (pushButton) return pushButton;
    return nullptr;
}

void AlertBox::removeButton(QAbstractButton *button)
{
    m_customButtonList.removeAll(button);
    if (m_defaultButton == button)
        m_defaultButton = nullptr;
    m_buttonBox->removeButton(button);
    updateSize();
}

void AlertBox::setStandardButtons(QDialogButtonBox::StandardButtons buttons)
{
    m_buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));

    QList<QAbstractButton *> buttonList = m_buttonBox->buttons();
    if (!buttonList.contains(m_defaultButton))
        m_defaultButton = nullptr;
    updateSize();
}

QDialogButtonBox::StandardButtons AlertBox::standardButtons() const
{
    return QDialogButtonBox::StandardButtons(int(m_buttonBox->standardButtons()));
}

QDialogButtonBox::StandardButton AlertBox::standardButton(QAbstractButton *button) const
{
    return (QDialogButtonBox::StandardButton)m_buttonBox->standardButton(button);
}

QAbstractButton *AlertBox::button(QDialogButtonBox::StandardButton which) const
{
    return m_buttonBox->button(QDialogButtonBox::StandardButton(which));
}


void AlertBox::setupLayout()
{
    delete layout();
    QGridLayout *grid = new QGridLayout(this);
    const bool hasIcon = !m_iconLabel->pixmap().isNull();

    if (hasIcon)
        grid->addWidget(m_iconLabel, 0, 0, 2, 1, Qt::AlignTop);
    m_iconLabel->setVisible(hasIcon);
#ifdef Q_OS_MAC
    QSpacerItem *indentSpacer = new QSpacerItem(14, 1, QSizePolicy::Fixed, QSizePolicy::Fixed);
#else
    QSpacerItem *indentSpacer = new QSpacerItem(hasIcon ? 7 : 15, 1, QSizePolicy::Fixed, QSizePolicy::Fixed);
#endif
    grid->addItem(indentSpacer, 0, hasIcon ? 1 : 0, 2, 1);
    grid->addWidget(m_label, 0, hasIcon ? 2 : 1, 1, 1);
#ifdef Q_OS_MAC
    grid->addWidget(m_buttonBox, grid->rowCount(), hasIcon ? 2 : 1, 1, 1);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setVerticalSpacing(8);
    grid->setHorizontalSpacing(0);
    setContentsMargins(24, 15, 24, 20);
    grid->setRowMinimumHeight(2, 6);
#else
    grid->addWidget(m_buttonBox, grid->rowCount(), 0, 1, grid->columnCount());
#endif
    if (m_detailsText) {
        int r = grid->rowCount();
        grid->addWidget(m_detailsText, grid->rowCount(), 0, 1, grid->columnCount());
        grid->setRowStretch(r, 100);
    }
    grid->setSizeConstraint(QLayout::SetNoConstraint);
    setLayout(grid);
    updateSize();
}

int AlertBox::layoutMinimumWidth()
{
    layout()->activate();
    return layout()->totalMinimumSize().width();
}

void AlertBox::updateSize()
{
    if (!isVisible()) return;

    const QSize screenSize = qApp->primaryScreen()->availableGeometry().size();
    int hardLimit = qMin(screenSize.width() - 480, 1000); // can never get bigger than this
    // on small screens allows the messagebox be the same size as the screen
    if (screenSize.width() <= 1024)
        hardLimit = screenSize.width();
#ifdef Q_OS_MAC
    int softLimit = qMin(screenSize.width()/2, 1000);
#else
    // note: ideally on windows, hard and soft limits but it breaks compat
    int softLimit = qMin(screenSize.width()/2, 1000);
#endif

    m_label->setWordWrap(false); // makes the label return min size
    int width = layoutMinimumWidth();

    if (width > softLimit || width > hardLimit) {
        m_label->setWordWrap(true);
        width = qMax(softLimit, layoutMinimumWidth());
    }

    QFontMetrics fm(QApplication::font("QMdiSubWindowTitleBar"));
    int windowTitleWidth = qMin(fm.horizontalAdvance(windowTitle()) + 50, hardLimit);
    if (windowTitleWidth > width)
        width = windowTitleWidth;

    layout()->activate();
    int height = (layout()->hasHeightForWidth())
        ? layout()->totalHeightForWidth(width)
        : layout()->totalMinimumSize().height();

    setMinimumSize(width, height);
    resize(width, height);
}



AlertBox::~AlertBox()
{
}


QString AlertBox::text() const
{
    return m_label->text();
}

void AlertBox::setText(const QString &text)
{
    m_label->setText(text);
    m_label->setWordWrap(m_label->textFormat() == Qt::RichText
        || (m_label->textFormat() == Qt::AutoText && Qt::mightBeRichText(text)));
    updateSize();
}


AlertBox::Icon AlertBox::icon() const
{
    return m_icon;
}

void AlertBox::setIcon(Icon icon)
{
    setIconPixmap(AlertBox::standardIcon((AlertBox::Icon)icon));
    m_icon = icon;
}

QPixmap AlertBox::iconPixmap() const
{
    return m_iconLabel->pixmap();
}

void AlertBox::setIconPixmap(const QPixmap &pixmap)
{
    m_iconLabel->setPixmap(pixmap);
    m_icon = NoIcon;
    setupLayout();
}



void AlertBox::keyPressEvent(QKeyEvent *e)
{
        if (e == QKeySequence::Copy) {
            if (m_detailsText && m_detailsText->isVisible() && m_detailsText->copy()) {
                e->setAccepted(true);
                return;
            }
        } else if (e == QKeySequence::SelectAll && m_detailsText && m_detailsText->isVisible()) {
            m_detailsText->selectAll();
            e->setAccepted(true);
            return;
        }

#if defined(Q_OS_WIN)
        if (e == QKeySequence::Copy) {
            const auto separator = "---------------------------\n";
            QString textToCopy;
            textToCopy += separator + windowTitle() + u'\n' + separator // title
                          + m_label->text() + u'\n' + separator;       // text
            if (m_detailsText)
                textToCopy += m_detailsText->text() + u'\n' + separator;
            QGuiApplication::clipboard()->setText(textToCopy);
            return;
        }
#endif // Q_OS_WIN

    QDialog::keyPressEvent(e);
}

void AlertBox::buttonClicked(QAbstractButton *button)
{
    if (m_detailsButton && m_detailsText && button == m_detailsButton) {
        QString lbl = m_detailsButton->getlabel(m_detailsText->isVisible());
        m_detailsButton->setLabel(lbl);
        m_detailsText->setHidden(!m_detailsText->isHidden());
        setupLayout();
        updateSize();
    } else {
        setClickedButton(button);
    }
}

void AlertBox::setClickedButton(QAbstractButton *button)
{
    m_buttonClicked = button;
    emit buttonWasClicked(m_buttonClicked);
    auto resultCode = execReturnCode(button);
    done(resultCode);
}


int AlertBox::execReturnCode(QAbstractButton *button)
{
    if (int standardButton = m_buttonBox->standardButton(button)) {
        // When using a AlertBox with standard buttons, the return
        // code is a StandardButton value indicating the standard button
        // that was clicked.
        return standardButton;
    } else {
        // When using AlertBox with custom buttons, the return code
        // is an opaque value, and the user is expected to use clickedButton()
        // to determine which button was clicked. We make sure to keep the opaque
        // value out of the QDialog::DialogCode range, so we can distinguish them.
        auto customButtonIndex = m_customButtonList.indexOf(button);
        if (customButtonIndex >= 0)
            return QDialog::DialogCode::Accepted + customButtonIndex + 1;
        else
            return customButtonIndex; // Not found, return -1
    }
}


void AlertBox::showEvent(QShowEvent *e)
{
    updateSize();
    QDialog::showEvent(e);
}


QString AlertBox::detailedText() const
{
    return m_detailsText ? m_detailsText->text() : QString();
}


void AlertBox::setDetailedText(const QString &text)
{
    if (text.isEmpty()) {
        if (m_detailsText) {
            m_detailsText->hide();
            m_detailsText->deleteLater();
        }
        m_detailsText = nullptr;
        removeButton(m_detailsButton);
        if (m_detailsButton) {
            m_detailsButton->hide();
            m_detailsButton->deleteLater();
        }
        m_detailsButton = nullptr;
    } else {
        if (!m_detailsText) {
            m_detailsText = new AlertBoxDetailsText(this);
            m_detailsText->hide();
        }
        if (!m_detailsButton) {
            m_detailsButton = new DetailButton(this);
            addButton(m_detailsButton, QDialogButtonBox::ActionRole);
        }
        m_detailsText->setText(text);
    }
    setupLayout();
}


void AlertBox::setWindowTitle(const QString &title)
{
    QDialog::setWindowTitle(title);
}


void AlertBox::setWindowModality(Qt::WindowModality windowModality)
{
    QDialog::setWindowModality(windowModality);

    if (parentWidget() && windowModality == Qt::WindowModal)
        setParent(parentWidget(), Qt::Sheet);
    else
        setParent(parentWidget(), Qt::Dialog);
}


QPixmap AlertBox::standardIcon(AlertBox::Icon icon, AlertBox* mb)
{
    QStyle *style = mb ? mb->style() : QApplication::style();
    int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, mb);
    QIcon tmpIcon;
    switch (icon) {
    case AlertBox::Information:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, nullptr, mb);
        break;
    case AlertBox::Warning:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, nullptr, mb);
        break;
    case AlertBox::Critical:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, nullptr, mb);
        break;
    case AlertBox::Question:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, nullptr, mb);
        break;
    default:
        break;
    }
    if (!tmpIcon.isNull()) {
        qreal dpr = mb ? mb->devicePixelRatio() : qApp->devicePixelRatio();
        return tmpIcon.pixmap(QSize(iconSize, iconSize), dpr);
    }
    return QPixmap();
}



QPixmap AlertBox::standardIcon(Icon icon)
{
    return standardIcon(icon, nullptr);
}

#include "moc_AlertBox.cpp"
#include "AlertBox.moc"
