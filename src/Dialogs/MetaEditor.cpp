/****************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks, Stratford, ON Canada
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
#include <QFileInfo>
#include <QTextStream>
#include <QDate>
#include <QShortcut>

#include "Dialogs/TreeModel.h"
#include "Dialogs/AddMetadata.h"
#include "Dialogs/MetaEditor.h"
#include "MainUI/MainWindow.h"
#include "Misc/Language.h"
#include "Misc/SettingsStore.h"
#include "Misc/PythonRoutines.h"

static const QString SETTINGS_GROUP = "meta_editor";

MetaEditor::MetaEditor(QWidget *parent)
  : QDialog(parent),
    m_mainWindow(qobject_cast<MainWindow *>(parent)),
    m_Relator(MarcRelators::instance()),
    m_RemoveRow(new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Delete),this, 0, 0, Qt::WidgetWithChildrenShortcut))
{
    setupUi(this);

    m_book = m_mainWindow->GetCurrentBook();
    m_version = m_book->GetConstOPF()->GetEpubVersion();
    m_opfdata = m_book->GetOPF()->GetText();

    QStringList headers;
    headers << tr("Name") << tr("Value");

    QString data = GetOPFMetadata();

    TreeModel *model = new TreeModel(headers, data);
    view->setModel(model);
    for (int column = 0; column < model->columnCount(); ++column)
        view->resizeColumnToContents(column);

    if (!isVisible()) {
        ReadSettings();
    }

    if (m_version.startsWith('3')) { 
        loadMetadataElements();
        loadMetadataProperties();
    } else {
        loadE2MetadataElements();
        loadE2MetadataProperties();
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
}


QString MetaEditor::GetOPFMetadata() {
    PythonRoutines pr;
    MetadataPieces mdp = pr.GetMetadataInPython(m_opfdata, m_version);
    QString data = mdp.data;
    m_otherxml = mdp.otherxml;
    m_metatag = mdp.metatag;
    m_idlist = mdp.idlist;
    return data;
}


QString MetaEditor::SetNewOPFMetadata(QString& data) 
{
    QString newopfdata = m_opfdata;
    MetadataPieces mdp;
    mdp.data = data;
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


const QHash<QString, DescriptiveInfo> &  MetaEditor::GetElementMap()
{
    if (m_version.startsWith('3')) {
        return m_ElementInfo;
    }
    return m_E2ElementInfo;
}

const QHash<QString, DescriptiveInfo> &  MetaEditor::GetPropertyMap()
{
    if (m_version.startsWith('3')) {
        return m_PropertyInfo;
    }
    return m_E2PropertyInfo;
}


void MetaEditor::selectElement()
{
    QStringList codes;
    {
         AddMetadata addelement(GetElementMap(), this);
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
            insertRow(code, lang);
        } else if (code == "dc:identifier-isbn") {
            QString content = "urn:isbn:" + tr("[ISBN here]");
            code = "dc:identifier";
            insertRow(code, content);
        } else if (code == "dc:identifier-issn") {
            QString content = "urn:issn:" + tr("[ISSN here]");
            code = "dc:identifier";
            insertRow(code, content);
        } else if (code == "dc:identifier-doi") {
            QString content = "urn:doi:" + tr("[DOI here]");
            code = "dc:identifier";
            insertRow(code, content);
        } else if (code == "dc:identifier-uuid") {
            QString content = "urn:uuid:" + tr("[UUID here]");
            code = "dc:identifier";
            insertRow(code, content);
        } else if ((code == "dc:date") || (code == "dcterms:issued") || (code == "dcterms:created")) {
            QString content = QDate::currentDate().toString(Qt::ISODate);
            insertRow(code, content);
        } else if (code == "dc:type") {
            QString content = "[dictionary,index]";
            insertRow(code, content);
        } else if (code == "dc:creator-aut") {
            code = "dc:creator";
            QString content = tr("[Author name here]");
            insertRow(code, content);
            insertChild(QString("role"),QString("aut"));
            insertChild(QString("scheme"),QString("marc:relators"));
        } else if (code == "dc:creator") {
            code = "dc:creator";
            QString content = tr("[Creator name here]");
            insertRow(code, content);
        } else if (code == "dc:contributor") {
            code = "dc:contributor";
            QString content = tr("[Contributor name here]");
            insertRow(code, content);
        } else {
            insertRow(code);
        }
    }
}

void MetaEditor::selectE2Element()
{
    QStringList codes;
    {
         AddMetadata addelement(GetElementMap(), this);
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
            insertRow(code, lang);
        } else if (code == "dc:identifier-isbn") {
            QString content = tr("[ISBN here]");
            code = "dc:identifier";
            insertRow(code, content);
            insertChild(QString("opf:scheme"), QString("ISBN"));
        } else if (code == "dc:identifier-issn") {
            QString content = tr("[ISSN here]");
            code = "dc:identifier";
            insertRow(code, content);
            insertChild(QString("opf:scheme"), QString("ISSN"));
        } else if (code == "dc:identifier-doi") {
            QString content = tr("[DOI here]");
            code = "dc:identifier";
            insertRow(code, content);
            insertChild(QString("opf:scheme"), QString("DOI"));
        } else if (code == "dc:identifier-uuid") {
            QString content = tr("[UUID here]");
            code = "dc:identifier";
            insertRow(code, content);
            insertChild(QString("opf:scheme"), QString("UUID"));
        } else if (code == "dc:identifier-custom") {
            QString content = tr("[Custom identifier here]");
            code = "dc:identifier";
            insertRow(code, content);
            insertChild(QString("opf:scheme"));
        } else if (code.startsWith("dc:date-")) {
            QStringList parts = code.split('-');
            QString dc_event = parts.at(1);
            code = "dc:date";
            QString content = QDate::currentDate().toString(Qt::ISODate);
            insertRow(code,content);
            insertChild(QString("opf:event"),dc_event);
        } else if (code == "dc:creator-aut") {
            code = "dc:creator";
            QString content = tr("[Author name here]");
            insertRow(code, content);
            insertChild(QString("opf:role"),QString("aut"));
        } else if (code == "dc:creator") {
            code = "dc:creator";
            QString content = tr("[Creator name here]");
            insertRow(code, content);
        } else if (code == "dc:contributor") {
            code = "dc:contributor";
            QString content = tr("[Contributor name here]");
            insertRow(code, content);
        } else {
            insertRow(code);
        }
    }
}


void MetaEditor::selectProperty()
{
    QStringList codes;
    {
        AddMetadata addproperty(GetPropertyMap(), this);
        if (addproperty.exec() == QDialog::Accepted) {
            codes = addproperty.GetSelectedEntries();
        }
    }
    foreach(QString code, codes) {
        if (code.startsWith("title-type:")) {
            QStringList parts = code.split(':');
            QString content = parts.at(1);
            code = parts.at(0);
            insertChild(code, content);
        } else if (code.startsWith("collection-type:")) {
            QStringList parts = code.split(':');
            QString content = parts.at(1);
            code = parts.at(0);
            insertChild(code, content);
        } else if (code.startsWith("dir:")) {
            QStringList parts = code.split(':');
            QString content = parts.at(1);
            code = parts.at(0);
            insertChild(code, content);
        } else if (code == "source-of") {
            QString content = "pagination";
            insertChild(code, content);
        } else if (code == "group-position") {
            QString content = "1";
            insertChild(code, content);
            insertChild(code, content);
        } else if (code == "display-seq") {
            QString content = "1";
            insertChild(code, content);
        } else if (code == "scheme") {
            insertChild(code);
        } else if ((code == "altlang") || (code == "xml:lang")) {
            QStringList langcodes;
            AddMetadata addvalue(Language::instance()->GetLangMap(), this);
            if (addvalue.exec() == QDialog::Accepted) {
                langcodes = addvalue.GetSelectedEntries();
            }
            QString lang= "en";
            if (!langcodes.isEmpty()) {
                lang = langcodes.at(0);
            }
            insertChild(code, lang);
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
            insertChild(code, role);
            code = "scheme";
            QString scheme = "marc:relators";
            insertChild(code, scheme);
        } else if (code == "identifier-type") {
            insertChild(code);
            code = "scheme";
            insertChild(code);
        } else {
            insertChild(code);
        }
    }
}

void MetaEditor::selectE2Property()
{
    QStringList codes;
    {
        AddMetadata addproperty(GetPropertyMap(), this);
        if (addproperty.exec() == QDialog::Accepted) {
            codes = addproperty.GetSelectedEntries();
        }
    }
    foreach(QString code, codes) {
        if (code == "opf:scheme") {
            insertChild(code);
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
            insertChild(code, lang);
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
            insertChild(code, role);
        } else {
            insertChild(code);
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

void MetaEditor::insertChild(QString code, QString contents)
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
    for (int column = 1; column < model->columnCount(index); ++column) {
        QModelIndex child = model->index(0, column, index);
        if (!contents.isEmpty()) {
            model->setData(child, QVariant(contents), Qt::EditRole);
        } else {
            model->setData(child, QVariant(tr("[Place value here]")), Qt::EditRole);
        }
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    view->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);
    updateActions();
}


void MetaEditor::insertRow(QString code, QString contents)
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
    for (int column = 1; column < model->columnCount(index.parent()); ++column) {
        QModelIndex nchild = model->index(index.row()+1, column, index.parent());
        if (!contents.isEmpty()) {
            model->setData(nchild, QVariant(contents), Qt::EditRole);
        } else {
            model->setData(nchild, QVariant(tr("[Your value here]")), Qt::EditRole);
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
         tr("Date: Publication") << "dc:date" << tr("The date of publication.") <<
         tr("Date: Creation") << "dcterms:created" << tr("The date of creation.") <<
         tr("Date: Issued") << "dcterms:issued" << tr("The date of modification.") <<
         tr("Date: Modification") << "dcterms:modfied" << tr("The date of modification.") <<
         tr("Type") << "dc:type" << tr("Used to indicate that the given EPUB Publication is of a specialized type..") <<
         tr("Format") << "dc:format" << tr("The media type or dimensions of the publication. Best practice is to use a value from a controlled vocabulary (e.g. MIME media types).") <<
         tr("Source") << "dc:source" << tr("Identifies the related resource(s) from which this EPUB Publication is derived.") <<
         tr("Language") << "dc:language" << tr("Specifies the language of the publication. Select from the dropdown menu") <<
         tr("Relation") << "dc:relation" << tr("A reference to a related resource. The recommended best practice is to identify the referenced resource by means of a string or number conforming to a formal identification system.") <<
         tr("Coverage") << "dc:coverage" << tr("The extent or scope of the content of the publication's content.") <<
         tr("Rights") << "dc:rights" << tr("Information about rights held in and over the publication. Rights information often encompasses Intellectual Property Rights (IPR), Copyright, and various Property Rights. If the Rights element is absent, no assumptions may be made about any rights held in or over the publication.") <<
         tr("Creator") << "dc:creator" << tr("Represents the name of a person, organization, etc. responsible for the creation of the content of an EPUB Publication. The role property can be attached to the element to indicate the function the creator played in the creation of the content.") <<
         tr("Contributor") << "dc:contributor" << tr("Represents the name of a person, organization, etc. that played a secondary role in the creation of the content of an EPUB Publication. The role property can be attached to the element to indicate the function the creator played in the creation of the content.") <<
         tr("Belongs to Collection") << "belongs-to-collection" << tr("Identifies the name of a collection to which the EPUB Publication belongs. An EPUB Publication may belong to one or more collections.") <<
         tr("Title") << "dc:title" << tr("A title of the publication.  A publication may have only one main title but may have numerous other title types.  These include main, subtitle, short, collection, edition, and expanded title types.") <<
         tr("Identifier: DOI") << "dc:identifier-doi" << tr("Digital Object Identifier associated with the given EPUB publication.") << 
         tr("Identifier: ISBN") << "dc:identifier-isbn" << tr("International Standard Book Number associated with the given EPUB publication.") <<
         tr("Identifier: ISSN") << "dc:identifier-issn" << tr("International Standard Serial Number associated with the given EPUB publication.") <<
         tr("Identifier: UUID") << "dc:identifier-uuid" << tr("A Universally Unique Idenitifier generated for this EPUB publication.") <<
         // tr("Identifier: Custom") << "dc:identifier-custom" << tr("A custom identifier based on a specified scheme") <<
         tr("Custom Element") << tr("[Custom element]") << tr("An empty metadata element you can modify.");

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
         tr("XML Language") << "xml:lang" << tr("Optional, language specifying attribute.  Uses same codes as dc:language. Not for use with dc:langauge, dc:date, or dc:identifier metadata elements.") <<
         tr("Text Direction: rtl") << "dir:rtl" << tr("Optional text direction attribute for this metadata item. right-to-left (rtl). Not for use with dc:language, dc:date, or dc:identifier metadata elements.") <<
         tr("Text Direction: ltr") << "dir:ltr" << tr("Optional text direction attribute for this metadata item. left-to-right (ltr). Not for use with dc:language, dc:date, or dc:identifier metadata elements.") <<
         tr("Title Type: main") << "title-type:main" << tr("Indicates the associated title is the main title of the publication.  Only one main title should exist.") <<
         tr("Title Type: subtitle") << "title-type:subtitle" << tr("Indicates that the associated title is a subtitle of the publication if one exists..") <<
         tr("Title Type: short") << "title-type:short" << tr("Indicates that the associated title is a shortened title of the publication if one exists.") <<
         tr("Title Type: collection") << "title-type:collection" << tr("Indicates that the associated title is the title of a collection that includes this publication belongs to, if one exists.") <<
         tr("Title Type: edition") << "title-type:edition" << tr("Indicates that the associated title is an edition title for this publications if one exists.") <<
         tr("Title Type: expanded") << "title-type:expanded" << tr("Indicates that the associated title is an expanded title for this publication if one exists.") <<
         tr("Alternate Script") << "alternate-script" << tr("Provides an alternate expression of the associated property value in a language and script identified by an alternate-language attribute.") <<
         tr("Alternate Language") << "altlang" << tr("Language code for the language used in the associated alternate-script property value.") <<
         tr("Collection Type: set") << "collection-type:set" << tr("Property used with belongs-to-collection. Indicates the form or nature of a collection. The value 'set' should be used for a finite collection of works that together constitute a single intellectual unit; typically issued together and able to be sold as a unit..") <<
         tr("Collection Type: series") << "collection-type:series" << tr("Property used with belongs-to-collection. Indicates the form or nature of a collection. The value 'series'' should be used for a sequence of related works that are formally identified as a group; typically open-ended with works issued individually over time.") <<
         tr("Display Sequence") << "display-seq" << tr("Indicates the numeric position in which to display the current property relative to identical metadata properties (e.g., to indicate the order in which to render multiple titles or multiple authors).") <<
         tr("File as") << "file-as" << tr("Provides the normalized form of the associated property for sorting. Typically used with author, creator, and contributor names.") <<
         tr("Group Position") << "group-position" << tr("Indicates the numeric position in which the EPUB Publication is ordered relative to other works belonging to the same group (whether all EPUB Publications or not).") <<
         tr("Identifier Type") << "identifier-type" << tr("Indicates the form or nature of an identifier. When the identifier-type value is drawn from a code list or other formal enumeration, the scheme attribute should be used to identify its source.") <<
         tr("Meta Authority") << "meta-auth" << tr("Identifies the party or authority responsible for an instance of package metadata.") <<
         tr("Role") << "role" << tr("Describes the nature of work performed by a creator or contributor (e.g., that the person is the author or editor of a work).  Typically used with the marc:relators scheme for a controlled vocabulary.") <<
         tr("Scheme") << "scheme" << tr("This attribute is typically added to dc:identifier, dc:source: dc:creator, or dc:contributor to indicate the controlled vocabulary system employed. (e.g. marc:relators to specify valid values for the role property.") <<
         tr("Source of Pagination") << "source-of" << tr("Indicates a unique aspect of an adapted source resource that has been retained in the given Rendition of the EPUB Publication. This specification defines the pagination value to indicate that the referenced source element is the source of the pagebreak properties defined in the content. This value should be set whenever pagination is included and the print source is known. Valid values: pagination.") <<
        tr("Custom Property") << tr("[Custom property/attribute]") << tr("An empty metadata property or attribute you can modify.");

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


// Loads the basic metadata types, names, and descriptions
void MetaEditor::loadE2MetadataElements()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_E2ElementInfo.isEmpty()) {
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
         tr("Identifier: Custom") << "dc:identifier-custom" << tr("A custom identifier based on a specified scheme") <<
         tr("Series") << "calibre:series" << tr("Series title or name (from calibre)") <<
         tr("Series Index") << "calibre:series_index" << tr("Index of this book in the series (from calibre)") <<
         tr("Title for Sorting") << "calibre:title_sort" << tr("Version of ebook title to use for sorting(from calibre)") <<
         tr("Custom Element") << tr("[Custom element]") << tr("An empty metadata element for you to modify");

    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo meta;
        meta.name = name;
        meta.description  = description;
        m_E2ElementInfo.insert(code, meta);
        m_E2ElementCode.insert(name, code);
    }
}

void MetaEditor::loadE2MetadataProperties()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_E2PropertyInfo.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
         tr("Id Attribute") << "id" << tr("Optional, typically short, unique identifier string used as an attribute in the Package (opf) document.") <<
         tr("XML Language") << "xml:lang" << tr("Optional, language specifying attribute.  Uses same codes as dc:language. Not for use with dc:language, dc:date, or dc:identifier metadata elements.") <<
         tr("File as") << "opf:file-as" << tr("Provides the normalized form of the associated property for sorting. Typically used with author, creator, and contributor names.") <<
         tr("Role") << "opf:role" << tr("Describes the nature of work performed by a creator or contributor (e.g., that the person is the author or editor of a work).  Typically used with the marc:relators scheme for a controlled vocabulary.") <<
         tr("Scheme") << "opf:scheme" << tr("This attribute is typically added to dc:identifier to indicate the type of identifier being used: DOI, ISBN, ISSN, or UUID.") <<
         tr("Event") << "opf:event" << tr("This attribute is typically added to dc:date elements to specify the date type: publication, creation, or modification.") <<
         tr("Custom Attribute") << tr("[Custom metadata property/attribute]") << tr("An empty metadata attribute you can modify.");

    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo minfo;
        minfo.name = name;
        minfo.description  = description;
        m_E2PropertyInfo.insert(code, minfo);
        m_E2PropertyCode.insert(name, code);
    }
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
