/****************************************************************************
**
**  Copyright (C) 2016-2022 Kevin B. Hendricks, Stratford, ON Canada
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

#include "EmbedPython/PythonRoutines.h"

#include <QString>
#include <QChar>
#include <QString>
#include <QFileInfo>
#include <QTextStream>
#include <QDate>
#include <QShortcut>
#include <QInputDialog>
#include <QDebug>

#include "Dialogs/TreeModel.h"
#include "Dialogs/AddMetadata.h"
#include "MainUI/MainWindow.h"
#include "Misc/Language.h"
#include "Misc/MarcRelators.h"
#include "Misc/SettingsStore.h"
#include "Dialogs/MetaEditorItemDelegate.h"
#include "Dialogs/MetaEditor.h"


static const QString SETTINGS_GROUP = "meta_editor";
static const QString _IN = "  ";
static const QString _GS = QString(QChar(29)); // Ascii Group Separator
static const QString _RS = QString(QChar(30)); // Ascii Record Separator
static const QString _US = QString(QChar(31)); // Ascii Unit Separator


MetaEditor::MetaEditor(QWidget *parent)
  : QDialog(parent),
    m_mainWindow(qobject_cast<MainWindow *>(parent)),
    m_RemoveRow(new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Delete),this, 0, 0, Qt::WidgetWithChildrenShortcut)),
    m_cbDelegate(new MetaEditorItemDelegate())
{
    setupUi(this);

    view->setTextElideMode(Qt::ElideNone);
    view->setUniformRowHeights(false);
    view->setWordWrap(true);
    m_book = m_mainWindow->GetCurrentBook();
    m_version = m_book->GetConstOPF()->GetEpubVersion();
    m_opfdata = m_book->GetOPF()->GetText();

    if (m_version.startsWith('3')) { 
        loadMetadataElements();
        loadMetadataProperties();
        loadMetadataXProperties();
        loadChoices();
    } else {
        loadE2MetadataElements();
        loadE2MetadataProperties();
        loadE2MetadataXProperties();
        loadE2Choices();
    }

    // pass choice sets to Delegate
    m_cbDelegate->setChoices(m_Choices);
    
    QStringList headers;
    headers << tr("Name") << tr("Value");

    // takes metadata from opf and translates it to human readable form
    // but stores original code as tooltips
    QString data = GetOPFMetadata();

    TreeModel *model = new TreeModel(headers, data);
    view->setModel(model);
    for (int column = 0; column < model->columnCount(); ++column) {
        if (column != 1) {
            view->resizeColumnToContents(column);
        } else {
            view->setColumnWidth(column,300);
        }
    }
    
    // need to assign delegate for values stored in column 1 only
    // Delegate is used to add custom combobox pulldowns and textedit
    // to specific metadata items stored in the tree
    view->setItemDelegateForColumn(1, m_cbDelegate);
    
    if (!isVisible()) {
        ReadSettings();
    }

    connect(view->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &,
                                    const QItemSelection &)),
            this, SLOT(updateActions()));

    connect(delButton, SIGNAL(clicked()), this, SLOT(removeRow()));
    connect(tbMoveUp, SIGNAL(clicked()), this, SLOT(moveRowUp()));
    connect(tbMoveDown, SIGNAL(clicked()), this, SLOT(moveRowDown()));
    connect(m_RemoveRow, SIGNAL(activated()), this, SLOT(removeRow()));

    if (m_version.startsWith('3')) {
        connect(addMetaButton, SIGNAL(clicked()), this, SLOT(selectElement()));
        connect(addPropButton, SIGNAL(clicked()), this, SLOT(selectProperty()));
    } else {
        connect(addMetaButton, SIGNAL(clicked()), this, SLOT(selectE2Element()));
        connect(addPropButton, SIGNAL(clicked()), this, SLOT(selectE2Property()));
    }

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveData()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    updateActions();
}


MetaEditor::~MetaEditor()
{
    m_RemoveRow->deleteLater();
    if (m_cbDelegate) {
        delete m_cbDelegate;
        m_cbDelegate = 0;
    }
}


// utility routine
QStringList MetaEditor::buildChoices(const QStringList& opts)
{
    QStringList  choices;
    foreach(QString aval, opts) {
        choices << PName(aval) + _GS + aval;
    }
    choices.sort();
    return choices;
}

void MetaEditor::loadChoices()
{
    const QStringList TXTDIR = QStringList() << "dir:rtl" << "dir:ltr";
    
    const QStringList COLLECT = QStringList() << "collection-type:set" << "collection-type:series";
    
    const QStringList TITLES = QStringList() << "title-type:main" << "title-type:subtitle" <<
        "title-type:short" << "title-type:collection" << "title-type:edition" << "title-type:expanded";
    
    QString cat;
    cat = PName("title-type");
    m_Choices[cat] = buildChoices(TITLES);

    cat = PName("dir");
    m_Choices[cat] = buildChoices(TXTDIR);

    cat = PName("collection-type");
    m_Choices[cat] = buildChoices(COLLECT);

    cat = PName("role");
    QStringList rolenames = MarcRelators::instance()->GetSortedNames();
    QStringList  rchoices;
    foreach(QString aval, rolenames) {
        rchoices << aval + _GS + RCode(aval);
    }
    rchoices.sort();
    m_Choices[cat] = rchoices;

    QStringList langnames = Language::instance()->GetSortedPrimaryLanguageNames();
    QStringList  lchoices;
    foreach(QString aval, langnames) {
        lchoices << aval + _GS + LCode(aval);
    }
    lchoices.sort();

    cat = EName("dc:language");
    m_Choices[cat] = lchoices;

    cat = PName("xml:lang");
    m_Choices[cat] = lchoices;

    //  handle as special case
    cat = EName("dc:description");
    m_Choices[cat] = QStringList();
}


void MetaEditor::loadE2Choices()
{
    const QStringList EVENTS = QStringList() << "opf:event-publication" << "opf:event-creation" <<
        "opf:event-modification";

    const QStringList SCHEMES = QStringList() << "marc:relators" << "DOI" << "ISBN" << "ISSN" << "UUID" << "AMAZON";

    QString cat;
    cat = PName("opf:event");
    m_Choices[cat] = buildChoices(EVENTS);

    cat = PName("opf:scheme");
    m_Choices[cat] = buildChoices(SCHEMES);

    cat = PName("opf:role");
    QStringList rolenames = MarcRelators::instance()->GetSortedNames();
    QStringList  rchoices;
    foreach(QString aval, rolenames) {
        rchoices << aval + _GS + RCode(aval);
    }
    rchoices.sort();
    m_Choices[cat] = rchoices;

    QStringList langnames = Language::instance()->GetSortedPrimaryLanguageNames();
    QStringList  lchoices;
    foreach(QString aval, langnames) {
        lchoices << aval + _GS + LCode(aval);
    }
    lchoices.sort();

    cat = EName("dc:language");
    m_Choices[cat] = lchoices;

    cat = PName("xml:lang");
    m_Choices[cat] = lchoices;

    // handle as special case
    cat = EName("dc:description");
    m_Choices[cat] = QStringList();
}


QString MetaEditor::GetOPFMetadata() {
    PythonRoutines pr;
    MetadataPieces mdp = pr.GetMetadataInPython(m_opfdata, m_version);
    QString adata = mdp.data;
    m_otherxml = mdp.otherxml;
    m_metatag = mdp.metatag;
    m_idlist = mdp.idlist;
    // Translate to Human Readable Form
    // and set original codes as tooltips
    QStringList nlist;
    QStringList dlist = adata.split(_RS);
    foreach(QString rc, dlist) {
        if (rc.isEmpty()) continue;
        if (rc.startsWith(_IN)) {
            // treat as property value attribute pair
            QStringList parts = rc.trimmed().split(_US);
            QString tvalue = parts.at(1);
            QString value = parts.at(1);
            if (parts.at(0) == "opf:role") value = RName(value);
            if (parts.at(0) == "opf:scheme") value = PName(value);
            if (parts.at(0) == "opf:file-as") tvalue = "";
            if (parts.at(0) == "opf:event") value = PName("opf:event-" + value);
            if (parts.at(0) == "role") value = RName(value);
            if (parts.at(0) == "title-type") value = PName("title-type:" + value);
            if (parts.at(0) == "collection-type") value = PName("collection-type:" + value);
            if (parts.at(0) == "xml:lang") value = LName(value);
            if (parts.at(0) == "altlang") value = LName(value);
            if (parts.at(0) == "source-of") value = PName("source-of:" + value);
            if (parts.at(0) == "dir") value = PName("dir:" + value);
            QString prop = PName(parts.at(0)) + _GS + parts.at(0);
            value = value + _GS + tvalue;
            nlist.append(_IN + prop + _US + value + _RS);
        } else {
            // treat as element with content
            QStringList parts = rc.split(_US);
            QString elem = EName(parts.at(0)) + _GS + parts.at(0);
            QString value = parts.at(1);
            QString valtip = "";
            if (parts.at(0) == "dc:language") {
                valtip = parts.at(1);
                value = LName(value);
            }
            value = value + _GS + valtip;
            nlist.append(elem + _US + value + _RS);
        }
    }
    QString ndata = nlist.join("");
    return ndata;
}


QString MetaEditor::SetNewOPFMetadata(QString& data) 
{
    QString newopfdata = m_opfdata;
    MetadataPieces mdp;
    // Translate from Human Readable Form
    QStringList dlist = data.split(_RS);
    QStringList nlist;
    foreach(QString rc, dlist) {
        if (rc.startsWith(_IN)) {
            // treat as property value attribute pair
            QStringList parts = rc.trimmed().split(_US);
            QString prop = PCode(parts.at(0));
            QString value = parts.at(1);
            if (prop == "opf:role") value = RCode(value);
            if (prop == "opf:scheme") value = PCode(value);
            if (prop == "opf:event") {
                value = PCode(value);
                // strip off "opf:event-"
                value = value.mid(10);
            }
            if (prop == "dir") {
                value = PCode(value);
                // strip off "dir:""
                value = value.mid(4);
            }
            if (prop == "role") value = RCode(value);
            if (prop == "title-type") {
                value = PCode(value);
                // strip off "title-type:"
                value = value.mid(11);
            }
            if (prop == "collection-type") {
                value = PCode(value);
                // strip off "collection-type:"
                value = value.mid(16);
            }
            if (prop == "xml:lang") value = LCode(value);
            if (prop == "altlang") value = LCode(value);
            if (prop == "source-of") {
                value = PCode(value);
                // strip off "source-of:"
                value = value.mid(10);
            }
            nlist.append(_IN + prop + _US + value + _RS);
        } else {
            // treat as element with content
            QStringList parts = rc.split(_US);
            QString elem = ECode(parts.at(0));
            QString value = parts.at(1);
            if (elem == "dc:language") value = LCode(value);
            nlist.append(elem + _US + value + _RS);
        }
    }
    mdp.data = nlist.join("");
    mdp.otherxml = m_otherxml;
    mdp.metatag = m_metatag;
    mdp.idlist = m_idlist;
    PythonRoutines pr;
    QString results = pr.SetNewMetadataInPython(mdp, m_opfdata, m_version);
    if (!results.isEmpty()) {
        newopfdata = results;
    }
    return newopfdata;
}

//Quick Utility Conversion from Element Code to Name
const QString MetaEditor::EName(const QString& code)
{
    if (m_ElementInfo.contains(code)) return m_ElementInfo[code].name;
    return code;
}

//Quick Utility Conversion from Element Name to Code
const QString MetaEditor::ECode  (const QString& name)
{
    if (m_ElementCode.contains(name)) return m_ElementCode[name];
    return name;
}


//Quick Utility Conversion from Property Code to Name
const QString MetaEditor::PName(const QString& code)
{
    if (m_PropertyInfo.contains(code)) return m_PropertyInfo[code].name;
    if (m_XPropertyInfo.contains(code)) return m_XPropertyInfo[code].name;
    return code;
}

//Quick Utility Conversion from Property Name to Code
const QString MetaEditor::PCode  (const QString& name)
{
    if (m_PropertyCode.contains(name)) return m_PropertyCode[name];
    if (m_XPropertyCode.contains(name)) return m_XPropertyCode[name];
    return name;
}

const QString MetaEditor::LName  (const QString& code) { return Language::instance()->GetLanguageName(code); }
const QString MetaEditor::LCode  (const QString& name) { return Language::instance()->GetLanguageCode(name); }

const QString MetaEditor::RName  (const QString& code) { return MarcRelators::instance()->GetName(code);     }
const QString MetaEditor::RCode  (const QString& name) { return MarcRelators::instance()->GetCode(name);     }





void MetaEditor::selectElement()
{
    QStringList codes;
    {
         AddMetadata addelement(m_ElementInfo, this);
         if (addelement.exec() == QDialog::Accepted) {
            codes = addelement.GetSelectedEntries();
         }
    }
    foreach(QString code, codes) {
        if (code == "dc:language") {
            QStringList langcodes;
            AddMetadata addvalue(Language::instance()->GetLangMap(), this);
            if (addvalue.exec() == QDialog::Accepted) {
                 langcodes = addvalue.GetSelectedEntries();
            }
            QString lang = "en";
            if (!langcodes.isEmpty()) {
                lang = langcodes.at(0);
            }
            insertRow(EName(code), code, LName(lang), lang);
        } else if (code == "dc:identifier-isbn") {
            QString content = "urn:isbn:" + tr("[ISBN here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
        } else if (code == "dc:identifier-issn") {
            QString content = "urn:issn:" + tr("[ISSN here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
        } else if (code == "dc:identifier-doi") {
            QString content = "urn:doi:" + tr("[DOI here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
        } else if (code == "dc:identifier-uuid") {
            QString content = "urn:uuid:" + tr("[UUID here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
        } else if (code == "dc:identifier-amazon") {
            QString content = "urn:AMAZON:" + tr("[Amazon ASIN here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
        } else if ((code == "dc:date") || (code == "dcterms:created")) {
            QString content = QDate::currentDate().toString(Qt::ISODate);
            insertRow(EName(code), code, content, "");
        } else if (code == "dcterms:modified") {
            QDateTime zt(QDateTime::currentDateTime());
            zt.setTimeSpec(Qt::UTC);
            QString content = zt.toString(Qt::ISODate);
            insertRow(EName(code), code, content, "");
        } else if (code == "dc:type") {
            QString content = "[dictionary,index]";
            insertRow(EName(code), code, content, "");
        } else if (code == "dc:creator-aut") {
            code = "dc:creator";
            QString content = tr("[Author name here]");
            insertRow(EName(code), code, content, "");
            insertChild(PName("role"),"role", RName("aut"), "aut");
            insertChild(PName("scheme"),"scheme","marc:relators", "");
        } else if (code == "dc:creator") {
            code = "dc:creator";
            QString content = tr("[Creator name here]");
            insertRow(EName(code), code, content, "");
        } else if (code == "dc:contributor") {
            code = "dc:contributor";
            QString content = tr("[Contributor name here]");
            insertRow(EName(code), code, content, "");
        } else if (code == "meta") {
            code = "meta";
            QString content = tr("[Value here]");
            insertRow(EName(code), code, content, "");
            insertChild(PName("property"),"property", "[name]", "");
        } else if (code == "custom-element") {
            QString custom_element = getInput(tr("Custom Element"),tr("Custom Element"), tr("[Custom element]"));
            QString content = tr("[Value here]");
            insertRow(custom_element, custom_element, content, "");
        } else {
            insertRow(EName(code), code, "", "");
        }
    }
}

void MetaEditor::selectE2Element()
{
    QStringList codes;
    {
         AddMetadata addelement(m_ElementInfo, this);
         if (addelement.exec() == QDialog::Accepted) {
            codes = addelement.GetSelectedEntries();
         }
    }
    foreach(QString code, codes) {
        if (code == "dc:language") {
            QStringList langcodes;
            AddMetadata addvalue(Language::instance()->GetLangMap(), this);
            if (addvalue.exec() == QDialog::Accepted) {
                 langcodes = addvalue.GetSelectedEntries();
            }
            QString lang = "en";
            if (!langcodes.isEmpty()) {
                lang = langcodes.at(0);
            }
            insertRow(EName(code), code, LName(lang), lang);
        } else if (code == "dc:identifier-isbn") {
            QString content = tr("[ISBN here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
            insertChild(PName("opf:scheme"), "opf:scheme", "ISBN", "");
        } else if (code == "dc:identifier-issn") {
            QString content = tr("[ISSN here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
            insertChild(PName("opf:scheme"), "opf:scheme", "ISSN", "");
        } else if (code == "dc:identifier-doi") {
            QString content = tr("[DOI here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
            insertChild(PName("opf:scheme"), "opf:scheme", "DOI", "");
        } else if (code == "dc:identifier-uuid") {
            QString content = tr("[UUID here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
            insertChild(PName("opf:scheme"), "opf:scheme", "UUID", "");
        } else if (code == "dc:identifier-amazon") {
            QString content = tr("[Amazon ASIN here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
            insertChild(PName("opf:scheme"), "opf:scheme", "AMAZON", "");
        } else if (code == "dc:identifier-custom") {
            QString content = tr("[Custom identifier here]");
            code = "dc:identifier";
            insertRow(EName(code), code, content, "");
            insertChild(PName("opf:scheme"), "opf:scheme", "", "");
        } else if (code.startsWith("dc:date-")) {
            QStringList parts = code.split('-');
            QString event = parts.at(1);
            code = "dc:date";
            QString content = QDate::currentDate().toString(Qt::ISODate);
            insertRow(EName(code),code, content, "");
            insertChild(PName("opf:event"),"opf:event", PName("opf:event-"+event), event);
        } else if (code == "dc:creator-aut") {
            code = "dc:creator";
            QString content = tr("[Author name here]");
            insertRow(EName(code), code, content, "");
            insertChild(PName("opf:role"),"opf:role", RName("aut"), "aut");
        } else if (code == "dc:creator") {
            code = "dc:creator";
            QString content = tr("[Creator name here]");
            insertRow(EName(code), code, content, "");
        } else if (code == "dc:contributor") {
            code = "dc:contributor";
            QString content = tr("[Contributor name here]");
            insertRow(EName(code), code, content, "");
        } else if (code == "custom-element") {
            QString custom_element = getInput(tr("Custom Element"),tr("Custom Element"), tr("[Custom element]"));
            QString content = tr("[Value here]");
            insertRow(custom_element, custom_element, content, "");
        } else {
            insertRow(EName(code), code, "", "");
        }
    }
}


void MetaEditor::selectProperty()
{
    QStringList codes;
    {
        AddMetadata addproperty(m_PropertyInfo, this);
        if (addproperty.exec() == QDialog::Accepted) {
            codes = addproperty.GetSelectedEntries();
        }
    }
    foreach(QString code, codes) {
        if (code.startsWith("title-type:")) {
            QString content = PName(code);
            code = code.mid(11); //strip "title-type:"
            insertChild(PName("title-type"),"title-type", content, code);
        } else if (code.startsWith("collection-type:")) {
            QString content = PName(code);
            code = code.mid(16); //strip "collection-type:"
            insertChild(PName("collection-type"), "collection-type", content, code);
        } else if (code.startsWith("dir:")) {
            QString content = PName(code);
            code = code.mid(4); // strip "dir:"
            insertChild(PName("dir"),"dir", content, code);
        } else if (code == "source-of") {
            QString content = PName(code);
            insertChild(PName("source-of"), "source-of", content, "pagination");
        } else if (code == "group-position") {
            QString content = "1";
            insertChild(PName(code), code, content, "");
        } else if (code == "display-seq") {
            QString content = "1";
            insertChild(PName(code), code, content, "");
        } else if (code == "scheme") {
                insertChild(PName(code), code, "", "");
        } else if (code == "alternate-script") {
                insertChild(PName(code), code, "", "");
                QStringList langcodes;
                AddMetadata addvalue(Language::instance()->GetLangMap(), this);
                if (addvalue.exec() == QDialog::Accepted) {
                    langcodes = addvalue.GetSelectedEntries();
                }
                QString lang= "en";
                if (!langcodes.isEmpty()) {
                    lang = langcodes.at(0);
                }
                code = "altlang";
                insertChild(PName(code), code, LName(lang), lang);
        } else if ((code == "xml:lang") || (code == "altlang")) {
            QStringList langcodes;
            AddMetadata addvalue(Language::instance()->GetLangMap(), this);
            if (addvalue.exec() == QDialog::Accepted) {
                langcodes = addvalue.GetSelectedEntries();
            }
            QString lang= "en";
            if (!langcodes.isEmpty()) {
                lang = langcodes.at(0);
            }
            insertChild(PName(code), code, LName(lang), lang);
        } else if (code == "role") {
            QStringList rolecodes;
            AddMetadata addrole(MarcRelators::instance()->GetCodeMap(), this);
            if (addrole.exec() == QDialog::Accepted) {
                rolecodes = addrole.GetSelectedEntries();
            }
            QString role = "aut";
            if (!rolecodes.isEmpty()) {
                role = rolecodes.at(0);
            }
            insertChild(PName(code), code, RName(role), role);
            code = "scheme";
            QString scheme = "marc:relators";
            insertChild(PName(code), code, scheme, "");
        } else if (code == "identifier-type") {
            insertChild(PName(code), code, tr("[Insert identifier type here]"), "");
            code = "scheme";
            insertChild(PName(code), code, tr("[Insert scheme type here]"), "");
        } else if (code == "custom-property") {
            QString custom_property = getInput(tr("Custom Property"),tr("Custom Property"), tr("[Custom property/attribute]"));
            QString content = tr("[Value here]");
            insertChild(custom_property, custom_property, content, "");
        } else {
            insertChild(PName(code), code, "", "");
        }
    }
}

void MetaEditor::selectE2Property()
{
    QStringList codes;
    {
        AddMetadata addproperty(m_PropertyInfo, this);
        if (addproperty.exec() == QDialog::Accepted) {
            codes = addproperty.GetSelectedEntries();
        }
    }
    foreach(QString code, codes) {
        if (code == "opf:scheme") {
            insertChild(PName(code), code, "", "");
        } else if (code == "xml:lang") {
            QStringList langcodes;
            AddMetadata addvalue(Language::instance()->GetLangMap(), this);
            if (addvalue.exec() == QDialog::Accepted) {
                langcodes = addvalue.GetSelectedEntries();
            }
            QString lang= "en";
            if (!langcodes.isEmpty()) {
                lang = langcodes.at(0);
            }
            insertChild(PName(code), code, LName(lang), lang);
        } else if (code == "opf:role") {
            QStringList rolecodes;
            AddMetadata addrole(MarcRelators::instance()->GetCodeMap(), this);
            if (addrole.exec() == QDialog::Accepted) {
                rolecodes = addrole.GetSelectedEntries();
            }
            QString role = "aut";
            if (!rolecodes.isEmpty()) {
                role = rolecodes.at(0);
            }
            insertChild(PName(code), code, RName(role), role);
        } else if (code == "custom-property") {
            QString custom_property = getInput(tr("Custom Attribute"),tr("Custom Attribute"), tr("[Custom metadata property/attribute]"));
            QString content = tr("[Value here]");
            insertChild(custom_property, custom_property, content, "");
        } else {
            insertChild(PName(code), code, "", "");
        }
    }
}


void MetaEditor::saveData()
{
    WriteSettings();

    TreeModel *model = qobject_cast<TreeModel *>(view->model());
    QString data = model->getAllModelData();

    QString newopfdata = SetNewOPFMetadata(data);
    m_book->GetOPF()->SetText(newopfdata);
    QDialog::accept();
}

void MetaEditor::reject()
{
    WriteSettings();
    QDialog::reject();
}

// Used to insert a property for a parent metadata element
void MetaEditor::insertChild(const QString& code, const QString& tip, const QString& contents, const QString& vtip)
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();

    // restrict children to be a grandchild of the root item
    // and make sure you are in column 0 when inserting a child
    if (index.parent() != QModelIndex()) {
        index = index.parent();
    }
    int row = index.row();
    index = index.sibling(row,0);

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    QModelIndex child = model->index(0, 0, index);
    model->setData(child, QVariant(code), Qt::EditRole);
    model->setData(child, QVariant(tip), Qt::ToolTipRole);
    for (int column = 1; column < model->columnCount(index); ++column) {
        QModelIndex child = model->index(0, column, index);
        if (!contents.isEmpty()) {
            model->setData(child, QVariant(contents), Qt::EditRole);
            model->setData(child, QVariant(vtip), Qt::ToolTipRole);
        } else {
            model->setData(child, QVariant(tr("[Place value here]")), Qt::EditRole);
            model->setData(child, QVariant(""), Qt::ToolTipRole);
        }
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    view->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);
    updateActions();
}


// used to insert a new metadata element into the tree
void MetaEditor::insertRow(const QString& code, const QString& tip, const QString& contents, const QString& vtip)
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();

    // force all row insertions to be children of the root item
    while(index.parent() != QModelIndex()) {
        index = index.parent();
    }

    if (!model->insertRow(index.row()+1, index.parent()))
        return;


    updateActions();

    QModelIndex child = model->index(index.row()+1, 0, index.parent());
    model->setData(child, QVariant(code), Qt::EditRole);
    model->setData(child, QVariant(tip), Qt::ToolTipRole);
    for (int column = 1; column < model->columnCount(index.parent()); ++column) {
        QModelIndex nchild = model->index(index.row()+1, column, index.parent());
        if (!contents.isEmpty()) {
            model->setData(nchild, QVariant(contents), Qt::EditRole);
            model->setData(nchild, QVariant(vtip), Qt::ToolTipRole);
        } else {
            model->setData(nchild, QVariant(tr("[Your value here]")), Qt::EditRole);
            model->setData(nchild, QVariant(""), Qt::ToolTipRole);
        }
    }

    // force newly inserted row to be the currently selected item so that any
    // follow-on insertChild calls use this as their parent.
    view->selectionModel()->setCurrentIndex(child, QItemSelectionModel::ClearAndSelect);
    updateActions();
}


void MetaEditor::removeRow()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();
    if (model->removeRow(index.row(), index.parent()))
        updateActions();
}


void MetaEditor::moveRowUp()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    TreeModel *model = qobject_cast<TreeModel *>(view->model());
    if (model->moveRowUp(index.row(), index.parent()))
        updateActions();
}


void MetaEditor::moveRowDown()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    TreeModel *model = qobject_cast<TreeModel *>(view->model());
    if (model->moveRowDown(index.row(), index.parent()))
        updateActions();
}


void MetaEditor::updateActions()
{
    bool hasSelection = !view->selectionModel()->selection().isEmpty();
    delButton->setEnabled(hasSelection);

    bool hasCurrent = view->selectionModel()->currentIndex().isValid();

    if (hasCurrent) {
        view->closePersistentEditor(view->selectionModel()->currentIndex());
    }
}

void MetaEditor::loadMetadataElements()
{

    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_ElementInfo.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
         tr("Author") << "dc:creator-aut" << tr("Represents a primary author of the book or publication") <<
         tr("Subject") << "dc:subject" << tr("An arbitrary phrase or keyword describing the subject in question. Use multiple 'subject' elements if needed.") <<
         tr("Description") << "dc:description" << tr("Description of the publication's content.") <<
         tr("Publisher") << "dc:publisher" << tr("An entity responsible for making the publication available.") <<
         tr("Date Published") << "dc:date" << tr("The date of publication.") <<
         tr("Date Created") << "dcterms:created" << tr("The date of creation.") <<
         tr("Date Modified") << "dcterms:modified" << tr("The date of modification.") <<
         tr("Type") << "dc:type" << tr("Used to indicate that the given EPUB Publication is of a specialized type..") <<
         tr("Format") << "dc:format" << tr("The media type or dimensions of the publication. Best practice is to use a value from a controlled vocabulary (e.g. MIME media types).") <<
         tr("Source") << "dc:source" << tr("Identifies the related resource(s) from which this EPUB Publication is derived.") <<
         tr("Language") << "dc:language" << tr("Specifies the language of the publication. Select from the dropdown menu") <<
         tr("Related To") << "dc:relation" << tr("A reference to a related resource. The recommended best practice is to identify the referenced resource by means of a string or number conforming to a formal identification system.") <<
         tr("Coverage") << "dc:coverage" << tr("The extent or scope of the content of the publication's content.") <<
         tr("Rights") << "dc:rights" << tr("Information about rights held in and over the publication. Rights information often encompasses Intellectual Property Rights (IPR), Copyright, and various Property Rights. If the Rights element is absent, no assumptions may be made about any rights held in or over the publication.") <<
         tr("Creator") << "dc:creator" << tr("Represents the name of a person, organization, etc. responsible for the creation of the content of an EPUB Publication. The Role property can be attached to the element to indicate the function the creator played in the creation of the content.") <<
         tr("Contributor") << "dc:contributor" << tr("Represents the name of a person, organization, etc. that played a secondary role in the creation of the content of an EPUB Publication. The Role property can be attached to the element to indicate the function the creator played in the creation of the content.") <<
         tr("Belongs to a Collection") << "belongs-to-collection" << tr("Identifies the name of a collection to which the EPUB Publication belongs. An EPUB Publication may belong to one or more collections.") <<
         tr("Title") << "dc:title" << tr("A title of the publication.  A publication may have only one main title but may have numerous other title types.  These include main, subtitle, short, collection, edition, and expanded title types.") <<
         tr("Identifier: DOI") << "dc:identifier-doi" << tr("Digital Object Identifier associated with this publication.") << 
         tr("Identifier: ISBN") << "dc:identifier-isbn" << tr("International Standard Book Number associated with this publication.") <<
         tr("Identifier: ISSN") << "dc:identifier-issn" << tr("International Standard Serial Number associated with this publication.") <<
         tr("Identifier: UUID") << "dc:identifier-uuid" << tr("A Universally Unique Identifier generated for this publication.") <<
         tr("Identifier: ASIN") << "dc:identifier-amazon" << tr("An Amazon Standard Identification Number associated with this publication.") <<
         tr("Identifier: Custom") << "dc:identifier-custom" << tr("A custom identifier based on a specified scheme") <<
         tr("Custom Element") << "custom-element" << tr("An empty metadata element you can modify.")  << 
         tr("Meta Element (primary)") << "meta" << tr("An empty primary metadata element you can modify.");
    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo minfo;
        minfo.name = name;
        minfo.description  = description;
        m_ElementInfo.insert(code, minfo);
        m_ElementCode.insert(name, code);
    }
}

void MetaEditor::loadMetadataProperties()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_PropertyInfo.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
         tr("Id Attribute") << "id" << tr("Optional, typically short, unique identifier string used as an attribute in the Package (opf) document.") <<
         tr("XML Language") << "xml:lang" << tr("Optional, language specifying attribute.  Uses same codes as Language. Not for use with Language, Date, or Identifier metadata elements.") <<
         tr("Uses Right To Left Text") << "dir:rtl" << tr("Optional text direction attribute for this metadata item. right-to-left (rtl). Not for use with dc:language, dc:date, or dc:identifier metadata elements.") <<
         tr("Uses Left to Right Text") << "dir:ltr" << tr("Optional text direction attribute for this metadata item. left-to-right (ltr). Not for use with dc:language, dc:date, or dc:identifier metadata elements.") <<
         tr("Title: Main Title") << "title-type:main" << tr("Indicates the associated title is the main title of the publication.  Only one main title should exist.") <<
         tr("Title: Subtitle") << "title-type:subtitle" << tr("Indicates that the associated title is a subtitle of the publication if one exists..") <<
         tr("Title: Short Title") << "title-type:short" << tr("Indicates that the associated title is a shortened title of the publication if one exists.") <<
         tr("Title: Collection Title") << "title-type:collection" << tr("Indicates that the associated title is the title of a collection that includes this publication belongs to, if one exists.") <<
         tr("Title: Edition Title") << "title-type:edition" << tr("Indicates that the associated title is an edition title for this publications if one exists.") <<
         tr("Title: Expanded Title") << "title-type:expanded" << tr("Indicates that the associated title is an expanded title for this publication if one exists.") <<
         tr("Alternate Script") << "alternate-script" << tr("Provides an alternate expression of the associated property value in a language and script identified by an xml:lang attribute.") <<
         tr("Alternate Language") << "altlang" << tr("Language code for the language used in the associated alternate-script property value.") <<
         tr("Collection is a Set") << "collection-type:set" << tr("Property used with belongs-to-collection. Indicates the form or nature of a collection. The value 'set' should be used for a finite collection of works that together constitute a single intellectual unit; typically issued together and able to be sold as a unit..") <<
         tr("Collection is a Series") << "collection-type:series" << tr("Property used with belongs-to-collection. Indicates the form or nature of a collection. The value 'series'' should be used for a sequence of related works that are formally identified as a group; typically open-ended with works issued individually over time.") <<
         tr("Display Sequence") << "display-seq" << tr("Indicates the numeric position in which to display the current property relative to identical metadata properties (e.g., to indicate the order in which to render multiple titles or multiple authors).") <<
         tr("File As") << "file-as" << tr("Provides the normalized form of the associated property for sorting. Typically used with author, creator, and contributor names.") <<
         tr("Position In Group") << "group-position" << tr("Indicates the numeric position in which the EPUB Publication is ordered relative to other works belonging to the same group (whether all EPUB Publications or not).") <<
         tr("Identifier Type") << "identifier-type" << tr("Indicates the form or nature of an identifier. When the identifier-type value is drawn from a code list or other formal enumeration, the scheme attribute should be used to identify its source.") <<
         tr("Role") << "role" << tr("Describes the nature of work performed by a creator or contributor (e.g., that the person is the author or editor of a work).  Typically used with the marc:relators scheme for a controlled vocabulary.") <<
         tr("Scheme") << "scheme" << tr("This attribute is typically added to Identifier, Source, Creator, or Contributors to indicate the controlled vocabulary system employed. (e.g. marc:relators to specify valid values for the role property.") <<
         tr("Source of Pagination") << "source-of" << tr("Indicates a unique aspect of an adapted source resource that has been retained in the given Rendition of the EPUB Publication. This specification defines the pagination value to indicate that the referenced source element is the source of the pagebreak properties defined in the content. This value should be set whenever pagination is included and the print source is known. Valid values: pagination.") <<
        tr("Custom Property") << "custom-property" << tr("An empty metadata property or attribute you can modify.");

    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo minfo;
        minfo.name = name;
        minfo.description  = description;
        m_PropertyInfo.insert(code, minfo);
        m_PropertyCode.insert(name, code);
    }
}


void MetaEditor::loadMetadataXProperties()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_XPropertyInfo.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
        tr("Text Direction") << "dir" << tr("Optional text direction attribute for this metadata item.") <<
        tr("Title Type") << "title-type" << tr("Indicates the kind or type of the title") <<
        tr("Collection Type") << "collection-type" << tr("Property used with belongs-to-collection. Indicates the form or nature of a collection.") <<
        tr("Source of") << "source-of" << tr("Indicates a unique aspect of an adapted source resource that has been retained in the given Rendition of the EPUB Publication.");
    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo minfo;
        minfo.name = name;
        minfo.description  = description;
        m_XPropertyInfo.insert(code, minfo);
        m_XPropertyCode.insert(name, code);
    }
}


void MetaEditor::loadE2MetadataXProperties()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_XPropertyInfo.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
        tr("Published") << "opf:event-published" << tr("Event Type is Published.") <<
        tr("Publication") << "opf:event-publication" << tr("Event Type is Publication.") <<
        tr("Creation") << "opf:event-creation" << tr("Event Type is Creation.") <<
        tr("Modification") << "opf:event-modification" << tr("Event Type is Modification.") <<
        tr("Digital Object Identifier")   << "DOI" << tr("Identifier Scheme: Digital Object Identifier") <<
        tr("International Standard Book Number")  << "ISBN" << tr("Identifier Scheme: International Standard Book Number") <<
        tr("International Standard Serial Number") << "ISSN"  << tr("Identifier Scheme: International Standard Serial Number") <<
        tr("Universally Unique Identifier") << "UUID"  << tr("Identifier Scheme: Universally Unique Identifier") <<
        tr("Amazon Unique Identifier") <<  "AMAZON" << tr("Identifier Scheme: Amazon Unique Identifier");
    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo minfo;
        minfo.name = name;
        minfo.description  = description;
        m_XPropertyInfo.insert(code, minfo);
        m_XPropertyCode.insert(name, code);
    }
}


// Loads the basic metadata types, names, and descriptions
void MetaEditor::loadE2MetadataElements()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_ElementInfo.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
         tr("Author") << "dc:creator-aut" << tr("Represents a primary author of the book or publication") <<
         tr("Title") << "dc:title" << tr("The main title of the epub publication.  Only one title may exist.") <<
         tr("Creator") << "dc:creator" << tr("Represents the name of a person, organization, etc. responsible for the creation of the content of an EPUB Publication. The attributes opf:role, opf:scheme and opf:file-as can be attached to the element to indicate the function the creator played in the creation of the content.") <<
         tr("Contributor") << "dc:contributor" << tr("Represents the name of a person, organization, etc. that played a secondary role in the creation of the content of an EPUB Publication'") <<
         tr("Subject") << "dc:subject" << tr("An arbitrary phrase or keyword describing the subject in question. Use multiple 'subject' elements if needed.") <<
         tr("Description") << "dc:description" << tr("Description of the publication's content.") <<
         tr("Publisher") << "dc:publisher" << tr("An entity responsible for making the publication available.") <<
         tr("Date") << "dc:date" << tr("A date associated with this epub, typically refined by event type information") <<
         tr("Date: Publication") << "dc:date-publication" << tr("The date of publication.") <<
         tr("Date: Creation") << "dc:date-creation" << tr("The date of creation.") <<
         tr("Date: Modification") << "dc:date-modification" << tr("The date of modification.") <<
         tr("Type") << "dc:type" << tr("The nature or genre of the content of the resource.") <<
         tr("Format") << "dc:format" << tr("The media type or dimensions of the publication. Best practice is to use a value from a controlled vocabulary (e.g. MIME media types).") <<
         tr("Source") << "dc:source" << tr("A reference to a resource from which the present publication is derived.") <<
         tr("Language") << "dc:language" << tr("A language used in the publication. Choose a RFC5646 value.") <<
         tr("Relation") << "dc:relation" << tr("A reference to a related resource. The recommended best practice is to identify the referenced resource by means of a string or number conforming to a formal identification system.") <<
         tr("Coverage") << "dc:coverage" << tr("The extent or scope of the content of the publication's content.") <<
         tr("Rights") << "dc:rights" << tr("Information about rights held in and over the publication. Rights information often encompasses Intellectual Property Rights (IPR), Copyright, and various Property Rights. If the Rights element is absent, no assumptions may be made about any rights held in or over the publication.") <<
         tr("Identifier") + ": DOI"   << "dc:identifier-doi" << tr("Digital Object Identifier") <<
         tr("Identifier") + ": ISBN"  << "dc:identifier-isbn" << tr("International Standard Book Number") <<
         tr("Identifier") + ": ISSN"  << "dc:identifier-issn" << tr("International Standard Serial Number") <<
         tr("Identifier") + ": UUID"  << "dc:identifier-uuid" << tr("Universally Unique Identifier") <<
         tr("Identifier") + ": ASIN"  << "dc:identifier-amazon" << tr("Amazon Standard Identification Number") <<
         tr("Identifier: Custom") << "dc:identifier-custom" << tr("A custom identifier") <<
         tr("Series") << "calibre:series" << tr("Series title or name (from calibre)") <<
         tr("Series Index") << "calibre:series_index" << tr("Index of this book in the series (from calibre)") <<
         tr("Title for Sorting") << "calibre:title_sort" << tr("Version of ebook title to use for sorting (from calibre)") <<
         tr("Custom Element") << "custom-element" << tr("An empty element for you to modify");

    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo meta;
        meta.name = name;
        meta.description  = description;
        m_ElementInfo.insert(code, meta);
        m_ElementCode.insert(name, code);
    }
}

void MetaEditor::loadE2MetadataProperties()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_PropertyInfo.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
         tr("Id Attribute") << "id" << tr("Optional, typically short, unique identifier string used as an attribute in the Package (opf) document.") <<
         tr("XML Language") << "xml:lang" << tr("Optional, language specifying attribute.  Uses same codes as dc:language. Not for use with dc:language, dc:date, or dc:identifier metadata elements.") <<
         tr("File As") << "opf:file-as" << tr("Provides the normalized form of the associated property for sorting. Typically used with author, creator, and contributor names.") <<
         tr("Role") << "opf:role" << tr("Describes the nature of work performed by a creator or contributor (e.g., that the person is the author or editor of a work).  Typically used with the marc:relators scheme for a controlled vocabulary.") <<
         tr("Scheme") << "opf:scheme" << tr("This attribute is typically added to dc:identifier to indicate the type of identifier being used: DOI, ISBN, ISSN, UUID, or AMAZON.") <<
         tr("Event") << "opf:event" << tr("This attribute is typically added to dc:date elements to specify the date type: publication, creation, or modification.") <<
         tr("Custom Attribute") << "custom-property" << tr("An empty metadata attribute you can modify.");

    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo minfo;
        minfo.name = name;
        minfo.description  = description;
        m_PropertyInfo.insert(code, minfo);
        m_PropertyCode.insert(name, code);
    }
}


QString MetaEditor::getInput(const QString& title, const QString& prompt, const QString& initvalue)
{
    QString result;
    QInputDialog dinput;
    dinput.setWindowTitle(title);
    dinput.setLabelText(prompt);
    dinput.setTextValue(initvalue);
    if (dinput.exec()) {
        result = dinput.textValue();
    }
    return result;
}


void MetaEditor::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    settings.endGroup();
}


void MetaEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}
