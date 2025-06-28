/************************************************************************
 **
 **  Copyright (C) 2025 Kevin B. Hendricks, Stratford Ontario Canada
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
#include <QString>
#include <QList>
#include <QDialog>
#include <QFontMetrics>
#include <QWidget>
#include <QScrollBar>
#include <QClipboard>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QTextCursor>
#include <QScrollBar>
#include <QTextBlock>
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

#include "Widgets/SourceEditor.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/SearchUtils.h"
#include "MainUI/MainApplication.h"
#include "Dialogs/PythonFunctionEditor.h"
#include "sigil_constants.h"

static const QString SETTINGS_GROUP = "python_editor";

static const QString EMPTY_REPLACE_FUNCTION = "def replace(match, number, file_name, metadata, data):\n\tif match:\n\t\treturn match.group(0)";

PythonFunctionEditor::PythonFunctionEditor(QMap<QString,QVariant>& func, const QString& functionName, QWidget *parent)
    : QDialog(parent),
      m_funcmap(func),
      m_editor(new SourceEditor(this)),
      m_initialFunctionName(functionName),
      m_layout(new QVBoxLayout(this))
{
    // setAttribute(Qt::WA_DeleteOnClose,true);

    // handle the layout manually
    QGridLayout * gl = new QGridLayout();
    m_lbl = new QLabel(tr("Function"), this);
    m_lbl->setMaximumWidth(120);
    m_lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_cb = new QComboBox(this);
    m_cb->setEditable(false);
    m_cb->addItems(m_funcmap.keys());
    m_butnew = new QPushButton(tr("New"));
    m_butnew->setMaximumWidth(120);   
    m_butdel = new QPushButton(tr("Delete"));
    m_butdel->setMaximumWidth(120);   
    m_butuse = new QPushButton(tr("Use"));
    m_butuse->setMaximumWidth(120);   
    gl->addWidget(m_lbl, 1, 0);
    gl->addWidget(m_cb, 1, 1);
    gl->addWidget(m_butnew, 1, 2);
    gl->addWidget(m_butdel, 1, 3);
    gl->addWidget(m_butuse, 1, 4);
    gl->setRowStretch(2,1);
    m_layout->addLayout(gl);
    m_layout->addWidget(m_editor);

    // need fixed width font for diff to show properly
    QFont tf = m_editor->font();
    tf.setFamily("Courier New");
    tf.setStyleHint(QFont::TypeWriter);
    m_editor->setFont(tf);
    m_editor->setTabStopDistance(QFontMetricsF(m_editor->font()).horizontalAdvance(' ') * 4);
    setWindowTitle(tr("Python Function Replace"));
    ReadSettings();
    m_cb->blockSignals(true);
    if (!m_initialFunctionName.isEmpty() && m_funcmap.contains(m_initialFunctionName)) {
        m_cb->setCurrentText(m_initialFunctionName);
    }
    m_cb->blockSignals(false);
    LoadEditor();
    m_hightype = SourceEditor::Highlight_PYTHON;
    m_editor->DoHighlightDocument(m_hightype);
    connectSignalsToSlots();
}

PythonFunctionEditor::~PythonFunctionEditor()
{
    QString fullfilepath = Utility::DefinePrefsDir() + "/" + SIGIL_FUNCTION_REPLACE_JSON_FILE;
    SearchUtils::WriteFuncDicttoJSONFile(fullfilepath, m_funcmap);
    WriteSettings();
}


QSize PythonFunctionEditor::sizeHint()
{
    return QSize(500, 500);
}


void PythonFunctionEditor::createFunction()
{
    bool ok;
    QString fn = QInputDialog::getText(this, tr("Input Name"),
                                       tr("Input Function Name"),
                                       QLineEdit::Normal,
                                       QString(), &ok);
    if (ok) {
        if (fn.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Name cannot be empty"));
        } else if (m_funcmap.keys().contains(fn)) {
            QMessageBox::warning(this, tr("Warning"), tr("Name cannot be duplicate"));
		} else {
            m_funcmap[fn] = QVariant(EMPTY_REPLACE_FUNCTION);
            m_cb->addItem(fn);
            m_cb->setCurrentText(fn);
        }
    }
}

void PythonFunctionEditor::deleteFunction()
{
    m_safe_to_save = false;
    // qDebug() << "in deleteFunction";
    QString fn = m_cb->currentText();
    QMessageBox::StandardButton button_pressed;
    button_pressed = Utility::warning(this, tr("Sigil"),
                                      tr("Are you sure you want to delete the function?:") + "\n" + fn,
                                      QMessageBox::Ok | QMessageBox::Cancel);
    if (button_pressed == QMessageBox::Ok) {
        int idx = m_cb->currentIndex();
        if (idx != -1) {
            m_cb->removeItem(idx);
            m_funcmap.remove(fn);
        }
    }
    m_safe_to_save = true;
}

void PythonFunctionEditor::useFunction()
{
    QString fn = m_cb->currentText();
    if (!fn.isEmpty()) {
        fn = "\\F<" + fn + ">";
        emit UseFunctionRequest(fn);
    }
    // now close which will write everything to json
    // QString fullfilepath = Utility::DefinePrefsDir() + "/" + SIGIL_FUNCTION_REPLACE_JSON_FILE;
    // SearchUtils::WriteFuncDicttoJSONFile(fullfilepath, m_funcmap);
    close();
}

void PythonFunctionEditor::loadFunctionToEdit(const QString& fn)
{
    m_safe_to_save = false;    
    // qDebug() << "in loadFunctionToEdit " << fn;    
    if (!fn.isEmpty()) {
        LoadEditor();
    } else {
        m_editor->clear();
    }
    m_safe_to_save = true;
}


void PythonFunctionEditor::saveFunction()
{
    if (!m_safe_to_save) return;
    // qDebug() << "in saveFunction";
    QString fn = m_cb->currentText();
    if (!fn.isEmpty()) {
        QString data = m_editor->toPlainText();
        m_funcmap[fn] = QVariant(data);
    }
}

// This is needed on all platforms to force the 
// syntax highlighting to start from scratch
// when dark to light mode is switched dynamically
void PythonFunctionEditor::ReloadEditor()
{
    // This will force the SourceEditor to delete its current XHTMLHighlighter and 
    // if needed install a freshly created one.  This is all because of a bug 
    // in QSyntaxHighlighting that can not detect color format changes alone 
    // if the exact same ranges were previously formatted
    m_editor->Refresh(m_hightype);
}

void PythonFunctionEditor::LoadEditor()
{
    QString fn = m_cb->currentText();
    QString data = m_funcmap[fn].toString();
    m_editor->setPlainText(data);
    // set cursor to the top and start scroll there
    QTextCursor tc = m_editor->textCursor();
    tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor,1);
    m_editor->setTextCursor(tc);
    m_editor->GetVerticalScrollBar()->setValue(0);
}

void PythonFunctionEditor::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    } else {
        resize(sizeHint());
    }
    settings.endGroup();
}

void PythonFunctionEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

int PythonFunctionEditor::exec()
{
    return QDialog::exec();
}

// should cover both escape key use and using x to close the runner dialog
void PythonFunctionEditor::reject()
{
    QDialog::reject();
}

void PythonFunctionEditor::connectSignalsToSlots()
{
    // this should now work correctly on all platforms
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    connect(mainApplication, SIGNAL(applicationPaletteChanged()), this, SLOT(ReloadEditor()));

    connect(m_butnew, SIGNAL(clicked()), this, SLOT(createFunction()));
    connect(m_butdel, SIGNAL(clicked()), this, SLOT(deleteFunction()));
    connect(m_butuse, SIGNAL(clicked()), this, SLOT(useFunction()));
    connect(m_cb, SIGNAL(currentTextChanged(const QString&)), this, SLOT(loadFunctionToEdit(const QString&)));
    connect(m_editor, SIGNAL(textChanged()), this, SLOT(saveFunction()));

}
