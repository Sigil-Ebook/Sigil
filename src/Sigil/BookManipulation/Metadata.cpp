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

#include <QtCore/QDate>
#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "BookManipulation/Metadata.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "Misc/Language.h"

static const QStringList EVENT_LIST           = QStringList() << "creation" << "publication" << "modification";
static const QStringList MODIFICATION_ALIASES = QStringList() << "modified" << "modification";
static const QStringList CREATION_ALIASES     = QStringList() << "created"  << "creation";
static const QStringList PUBLICATION_ALIASES  = QStringList() << "issued"   << "published" << "publication";
static const QStringList SCHEME_LIST          = QStringList() << "ISBN" << "ISSN" << "DOI";

QMutex Metadata::s_AccessMutex;
Metadata *Metadata::m_Instance = NULL;

Metadata &Metadata::Instance()
{
    // We use a static local variable
    // to hold our singleton instance; using a pointer member
    // variable creates problems with object destruction;
    QMutexLocker locker(&s_AccessMutex);

    if (!m_Instance) {
        static Metadata meta;
        m_Instance = &meta;
    }

    return *m_Instance;
}

QString Metadata::GetName(QString code)
{
    QString name = "";

    // Codes are unique between basic/relator
    if (m_Basic.contains(code)) {
        name = m_Basic[ code ].name;
    } else if (m_Relators.contains(code)) {
        name = m_Relators[ code ].name;
    }

    return name;
}

QString Metadata::GetCode(QString name)
{
    QString code = "";

    // Names are sufficiently unique between basic/relator
    // Except Publisher which is handled as an exception elsewhere
    if (m_BasicFullNames.contains(name)) {
        code = m_BasicFullNames[ name ];
    } else if (m_RelatorFullNames.contains(name)) {
        code = m_RelatorFullNames[ name ];
    }

    return code;
}

QString Metadata::GetText(QString text)
{
    if (m_Text.contains(text)) {
        return m_Text.value(text);
    }

    return text;
}

bool Metadata::IsRelator(QString code)
{
    return m_Relators.contains(code);
}

const QHash< QString, Metadata::MetaInfo > &Metadata::GetRelatorMap()
{
    return m_Relators;
}


const QHash< QString, Metadata::MetaInfo > &Metadata::GetBasicMetaMap()
{
    return m_Basic;
}


Metadata::MetaElement Metadata::MapToBookMetadata(const xc::DOMElement &element)
{
    Metadata::MetaElement meta;
    QString element_name = XhtmlDoc::GetNodeName(element);

    if (element_name == "meta") {
        meta.name  = XtoQ(element.getAttribute(QtoX("name")));
        meta.value = XtoQ(element.getAttribute(QtoX("content")));
        meta.attributes[ "scheme" ] = XtoQ(element.getAttribute(QtoX("scheme")));
        meta.attributes[ "id" ] = XtoQ(element.getAttribute(QtoX("id")));

        if ((!meta.name.isEmpty()) && (!meta.value.toString().isEmpty())) {
            return MapToBookMetadata(meta , false);
        }
    } else {
        meta.attributes = XhtmlDoc::GetNodeAttributes(element);
        meta.name = element_name;
        QString element_text = XtoQ(element.getTextContent());
        meta.value = element_text;

        if (!element_text.isEmpty()) {
            return MapToBookMetadata(meta , true);
        }
    }

    return meta;
}


// Maps Dublic Core metadata to internal book meta format
Metadata::MetaElement Metadata::MapToBookMetadata(const Metadata::MetaElement &meta, bool is_dc_element)
{
    QString name = meta.name.toLower();

    if (!is_dc_element &&
        !name.startsWith("dc.") &&
        !name.startsWith("dcterms.")) {
        return FreeFormMetadata(meta);
    }

    // Dublin Core
    // Transform HTML based Dublin Core to OPF style meta element
    MetaElement working_copy_meta = is_dc_element ? meta : HtmlToOpfDC(meta);
    name = working_copy_meta.name.toLower();

    if ((name == "creator") || (name == "contributor")) {
        return CreateContribMetadata(working_copy_meta);
    }

    if (name == "date") {
        return DateMetadata(working_copy_meta);
    }

    if (name == "identifier") {
        return IdentifierMetadata(working_copy_meta);
    }

    QString value = meta.value.toString();

    if (name == "language") {
        value = Language::instance()->GetLanguageName(value);
        // fall through
    }

    MetaElement book_meta;

    if ((!name.isEmpty()) && (!value.isEmpty())) {
        book_meta.name = name;
        book_meta.value = value;
    }

    return book_meta;
}


Metadata::Metadata()
{
    LoadBasicMetadata();
    LoadRelatorCodes();
    LoadText();
}

void Metadata::LoadText()
{
    m_Text[ "creator" ] = tr("Creator");
    m_Text[ "contributor" ] = tr("Contributor");
    m_Text[ "date" ] = tr("Date");
    m_Text[ "identifier" ] = tr("Identifier");
}


// Loads the basic metadata types, names, and descriptions
void Metadata::LoadBasicMetadata()
{
    // If the basic metadata has already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_Basic.isEmpty()) {
        return;
    }

    // These descriptions are standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher)
    // Abbreviations are not translated.
    QStringList data;
    data <<
         tr("Subject") << "subject" << tr("An arbitrary phrase or keyword describing the subject in question. Use multiple 'subject' elements if needed.") <<
         tr("Description") << "description" << tr("Description of the publication's content.") <<
         tr("Publisher") << "publisher" << tr("An entity responsible for making the publication available.") <<
         tr("Date: Publication") << "publication" << tr("The date of publication.") <<
         tr("Date: Creation") << "creation" << tr("The date of creation.") <<
         tr("Date: Modification") << "modification" << tr("The date of modification.") <<
         tr("Date (custom)") << "customdate" << tr("Enter your own event name in the File As column, e.g. updated.") <<
         tr("Type") << "type" << tr("The nature or genre of the content of the resource.") <<
         tr("Format") << "format" << tr("The media type or dimensions of the publication. Best practice is to use a value from a controlled vocabulary (e.g. MIME media types).") <<
         tr("Source") << "source" << tr("A reference to a resource from which the present publication is derived.") <<
         tr("Language") << "language" << tr("An optional extra language of the publication.  Use a value from the Language drop down menu.  For example use 'English' instead of the language code 'en'.") <<
         tr("Relation") << "relation" << tr("A reference to a related resource. The recommended best practice is to identify the referenced resource by means of a string or number conforming to a formal identification system.") <<
         tr("Coverage") << "coverage" << tr("The extent or scope of the content of the publication's content.") <<
         tr("Rights") << "rights" << tr("Information about rights held in and over the publication. Rights information often encompasses Intellectual Property Rights (IPR), Copyright, and various Property Rights. If the Rights element is absent, no assumptions may be made about any rights held in or over the publication.") <<
         tr("Title") << "title" << tr("An optional extra title of the publication in addition to the main title already entered.") <<
         tr("Identifier") + ": DOI"   << "DOI" << tr("Digital Object Identifier") <<
         tr("Identifier") + ": ISBN"  << "ISBN" << tr("International Standard Book Number") <<
         tr("Identifier") + ": ISSN"  << "ISSN" << tr("International Standard Serial Number") <<
         tr("Identifier (custom)") << "customidentifier" << tr("Enter your own custom identifier name in the File As column, e.g. stocknumber");

    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        MetaInfo meta;
        meta.name = name;
        meta.description  = description;
        m_Basic.insert(code, meta);
        m_BasicFullNames.insert(name, code);
    }
}

// Loads the relator codes, names, and descriptions
// A Relator (one who relates a story) identifies who a creator/contributor is
void Metadata::LoadRelatorCodes()
{
    // If the relator codes have already been loaded
    // by a previous Meta Editor, then don't load them again
    if (!m_Relators.isEmpty()) {
        return;
    }

    // These descriptions fixed, standard EPUB descriptions and should not be changed.
    // Names and codes must be unique between basic and advanced (except Publisher).
    QStringList data;
    data <<
         tr("Actor") << "act" << tr("Use for a person or organization who principally exhibits acting skills in a musical or dramatic presentation or entertainment.") <<
         tr("Adapter") << "adp" << tr("Use for a person or organization who 1) reworks a musical composition, usually for a different medium, or 2) rewrites novels or stories for motion pictures or other audiovisual medium.") <<
         tr("Analyst") << "anl" << tr("Use for a person or organization that reviews, examines and interprets data or information in a specific area.") <<
         tr("Animator") << "anm" << tr("Use for a person or organization who draws the two-dimensional figures, manipulates the three dimensional objects and/or also programs the computer to move objects and images for the purpose of animated film processing. Animation cameras, stands, celluloid screens, transparencies and inks are some of the tools of the animator.") <<
         tr("Annotator") << "ann" << tr("Use for a person who writes manuscript annotations on a printed item.") <<
         tr("Applicant") << "app" << tr("Use for a person or organization responsible for the submission of an application or who is named as eligible for the results of the processing of the application (e.g., bestowing of rights, reward, title, position).") <<
         tr("Architect") << "arc" << tr("Use for a person or organization who designs structures or oversees their construction.") <<
         tr("Arranger") << "arr" << tr("Use for a person or organization who transcribes a musical composition, usually for a different medium from that of the original; in an arrangement the musical substance remains essentially unchanged.") <<
         tr("Art copyist") << "acp" << tr("Use for a person (e.g., a painter or sculptor) who makes copies of works of visual art.") <<
         tr("Artist") << "art" << tr("Use for a person (e.g., a painter) or organization who conceives, and perhaps also implements, an original graphic design or work of art, if specific codes (e.g., [egr], [etr]) are not desired. For book illustrators, prefer Illustrator [ill]. ") <<
         tr("Artistic director") << "ard" << tr("Use for a person responsible for controlling the development of the artistic style of an entire production, including the choice of works to be presented and selection of senior production staff.") <<
         tr("Assignee") << "asg" << tr("Use for a person or organization to whom a license for printing or publishing has been transferred.") <<
         tr("Associated name") << "asn" << tr("Use for a person or organization associated with or found in an item or collection, which cannot be determined to be that of a Former owner [fmo] or other designated relator indicative of provenance.") <<
         tr("Attributed name") << "att" << tr("Use for an author, artist, etc., relating him/her to a work for which there is or once was substantial authority for designating that person as author, creator, etc. of the work. ") <<
         tr("Auctioneer") << "auc" << tr("Use for a person or organization in charge of the estimation and public auctioning of goods, particularly books, artistic works, etc.") <<
         tr("Author") << "aut" << tr("Use for a person or organization chiefly responsible for the intellectual or artistic content of a work, usually printed text. This term may also be used when more than one person or body bears such responsibility. ") <<
         tr("Author in quotations or text extracts") << "aqt" << tr("Use for a person or organization whose work is largely quoted or extracted in works to which he or she did not contribute directly. Such quotations are found particularly in exhibition catalogs, collections of photographs, etc.") <<
         tr("Author of afterword, colophon, etc.") << "aft" << tr("Use for a person or organization responsible for an afterword, postface, colophon, etc. but who is not the chief author of a work.") <<
         tr("Author of dialog") << "aud" << tr("Use for a person or organization responsible for the dialog or spoken commentary for a screenplay or sound recording.") <<
         tr("Author of introduction, etc.") << "aui" << tr("Use for a person or organization responsible for an introduction, preface, foreword, or other critical introductory matter, but who is not the chief author.") <<
         tr("Author of screenplay, etc.") << "aus" << tr("Use for a person or organization responsible for a motion picture screenplay, dialog, spoken commentary, etc.") <<
         tr("Bibliographic antecedent") << "ant" << tr("Use for a person or organization responsible for a work upon which the work represented by the catalog record is based. This may be appropriate for adaptations, sequels, continuations, indexes, etc.") <<
         tr("Binder") << "bnd" << tr("Use for a person or organization responsible for the binding of printed or manuscript materials.") <<
         tr("Binding designer") << "bdd" << tr("Use for a person or organization responsible for the binding design of a book, including the type of binding, the type of materials used, and any decorative aspects of the binding. ") <<
         tr("Book designer") << "bkd" << tr("Use for a person or organization responsible for the entire graphic design of a book, including arrangement of type and illustration, choice of materials, and process used. ") <<
         tr("Book producer") << "bkp" << tr("Use for a person or organization responsible for the production of books and other print media, if specific codes (e.g., [bkd], [egr], [tyd], [prt]) are not desired. ") <<
         tr("Bookjacket designer") << "bjd" << tr("Use for a person or organization responsible for the design of flexible covers designed for or published with a book, including the type of materials used, and any decorative aspects of the bookjacket. ") <<
         tr("Bookplate designer") << "bpd" << tr("Use for a person or organization responsible for the design of a book owner's identification label that is most commonly pasted to the inside front cover of a book. ") <<
         tr("Bookseller") << "bsl" << tr("Use for a person or organization who makes books and other bibliographic materials available for purchase. Interest in the materials is primarily lucrative.") <<
         tr("Calligrapher") << "cll" << tr("Use for a person or organization who writes in an artistic hand, usually as a copyist and or engrosser.") <<
         tr("Cartographer") << "ctg" << tr("Use for a person or organization responsible for the creation of maps and other cartographic materials.") <<
         tr("Censor") << "cns" << tr("Use for a censor, bowdlerizer, expurgator, etc., official or private. ") <<
         tr("Choreographer") << "chr" << tr("Use for a person or organization who composes or arranges dances or other movements (e.g., \"master of swords\") for a musical or dramatic presentation or entertainment.") <<
         tr("Cinematographer") << "cng" << tr("Use for a person or organization who is in charge of the images captured for a motion picture film. The cinematographer works under the supervision of a director, and may also be referred to as director of photography. Do not confuse with videographer.") <<
         tr("Client") << "cli" << tr("Use for a person or organization for whom another person or organization is acting.") <<
         tr("Collaborator") << "clb" << tr("Use for a person or organization that takes a limited part in the elaboration of a work of another person or organization that brings complements (e.g., appendices, notes) to the work.") <<
         tr("Collector") << "col" << tr("Use for a person or organization who has brought together material from various sources that has been arranged, described, and cataloged as a collection. A collector is neither the creator of the material nor a person to whom manuscripts in the collection may have been addressed.") <<
         tr("Collotyper") << "clt" << tr("Use for a person or organization responsible for the production of photographic prints from film or other colloid that has ink-receptive and ink-repellent surfaces.") <<
         tr("Commentator") << "cmm" << tr("Use for a person or organization who provides interpretation, analysis, or a discussion of the subject matter on a recording, motion picture, or other audiovisual medium.") <<
         tr("Commentator for written text") << "cwt" << tr("Use for a person or organization responsible for the commentary or explanatory notes about a text. For the writer of manuscript annotations in a printed book, use Annotator [ann].") <<
         tr("Compiler") << "com" << tr("Use for a person or organization who produces a work or publication by selecting and putting together material from the works of various persons or bodies.") <<
         tr("Complainant") << "cpl" << tr("Use for the party who applies to the courts for redress, usually in an equity proceeding.") <<
         tr("Complainant-appellant") << "cpt" << tr("Use for a complainant who takes an appeal from one court or jurisdiction to another to reverse the judgment, usually in an equity proceeding.") <<
         tr("Complainant-appellee") << "cpe" << tr("Use for a complainant against whom an appeal is taken from one court or jurisdiction to another to reverse the judgment, usually in an equity proceeding.") <<
         tr("Composer") << "cmp" << tr("Use for a person or organization who creates a musical work, usually a piece of music in manuscript or printed form.") <<
         tr("Compositor") << "cmt" << tr("Use for a person or organization responsible for the creation of metal slug, or molds made of other materials, used to produce the text and images in printed matter. ") <<
         tr("Conceptor") << "ccp" << tr("Use for a person or organization responsible for the original idea on which a work is based, this includes the scientific author of an audio-visual item and the conceptor of an advertisement.") <<
         tr("Conductor") << "cnd" << tr("Use for a person who directs a performing group (orchestra, chorus, opera, etc.) in a musical or dramatic presentation or entertainment.") <<
         tr("Consultant") << "csl" << tr("Use for a person or organization relevant to a resource, who is called upon for professional advice or services in a specialized field of knowledge or training.") <<
         tr("Consultant to a project") << "csp" << tr("Use for a person or organization relevant to a resource, who is engaged specifically to provide an intellectual overview of a strategic or operational task and by analysis, specification, or instruction, to create or propose a cost-effective course of action or solution.") <<
         tr("Contestant") << "cos" << tr("Use for the party who opposes, resists, or disputes, in a court of law, a claim, decision, result, etc.") <<
         tr("Contestant-appellant") << "cot" << tr("Use for a contestant who takes an appeal from one court of law or jurisdiction to another to reverse the judgment.") <<
         tr("Contestant-appellee") << "coe" << tr("Use for a contestant against whom an appeal is taken from one court of law or jurisdiction to another to reverse the judgment.") <<
         tr("Contestee") << "cts" << tr("Use for the party defending a claim, decision, result, etc. being opposed, resisted, or disputed in a court of law.") <<
         tr("Contestee-appellant") << "ctt" << tr("Use for a contestee who takes an appeal from one court or jurisdiction to another to reverse the judgment.") <<
         tr("Contestee-appellee") << "cte" << tr("Use for a contestee against whom an appeal is taken from one court or jurisdiction to another to reverse the judgment.") <<
         tr("Contractor") << "ctr" << tr("Use for a person or organization relevant to a resource, who enters into a contract with another person or organization to perform a specific task.") <<
         tr("Contributor") << "ctb" << tr("Use for a person or organization one whose work has been contributed to a larger work, such as an anthology, serial publication, or other compilation of individual works. Do not use if the sole function in relation to a work is as author, editor, compiler or translator.") <<
         tr("Copyright claimant") << "cpc" << tr("Use for a person or organization listed as a copyright owner at the time of registration. Copyright can be granted or later transferred to another person or organization, at which time the claimant becomes the copyright holder.") <<
         tr("Copyright holder") << "cph" << tr("Use for a person or organization to whom copy and legal rights have been granted or transferred for the intellectual content of a work. The copyright holder, although not necessarily the creator of the work, usually has the exclusive right to benefit financially from the sale and use of the work to which the associated copyright protection applies.") <<
         tr("Corrector") << "crr" << tr("Use for a person or organization who is a corrector of manuscripts, such as the scriptorium official who corrected the work of a scribe. For printed matter, use Proofreader.") <<
         tr("Correspondent") << "crp" << tr("Use for a person or organization who was either the writer or recipient of a letter or other communication.") <<
         tr("Costume designer") << "cst" << tr("Use for a person or organization who designs or makes costumes, fixes hair, etc., for a musical or dramatic presentation or entertainment.") <<
         tr("Cover designer") << "cov" << tr("Use for a person or organization responsible for the graphic design of a book cover, album cover, slipcase, box, container, etc. For a person or organization responsible for the graphic design of an entire book, use Book designer; for book jackets, use Bookjacket designer.") <<
         tr("Creator") << "cre" << tr("Use for a person or organization responsible for the intellectual or artistic content of a work.") <<
         tr("Curator of an exhibition") << "cur" << tr("Use for a person or organization responsible for conceiving and organizing an exhibition.") <<
         tr("Dancer") << "dnc" << tr("Use for a person or organization who principally exhibits dancing skills in a musical or dramatic presentation or entertainment.") <<
         tr("Data contributor") << "dtc" << tr("Use for a person or organization that submits data for inclusion in a database or other collection of data.") <<
         tr("Data manager") << "dtm" << tr("Use for a person or organization responsible for managing databases or other data sources.") <<
         tr("Dedicatee") << "dte" << tr("Use for a person or organization to whom a book, manuscript, etc., is dedicated (not the recipient of a gift).") <<
         tr("Dedicator") << "dto" << tr("Use for the author of a dedication, which may be a formal statement or in epistolary or verse form.") <<
         tr("Defendant") << "dfd" << tr("Use for the party defending or denying allegations made in a suit and against whom relief or recovery is sought in the courts, usually in a legal action.") <<
         tr("Defendant-appellant") << "dft" << tr("Use for a defendant who takes an appeal from one court or jurisdiction to another to reverse the judgment, usually in a legal action.") <<
         tr("Defendant-appellee") << "dfe" << tr("Use for a defendant against whom an appeal is taken from one court or jurisdiction to another to reverse the judgment, usually in a legal action.") <<
         tr("Degree grantor") << "dgg" << tr("Use for the organization granting a degree for which the thesis or dissertation described was presented.") <<
         tr("Delineator") << "dln" << tr("Use for a person or organization executing technical drawings from others' designs.") <<
         tr("Depicted") << "dpc" << tr("Use for an entity depicted or portrayed in a work, particularly in a work of art.") <<
         tr("Depositor") << "dpt" << tr("Use for a person or organization placing material in the physical custody of a library or repository without transferring the legal title.") <<
         tr("Designer") << "dsr" << tr("Use for a person or organization responsible for the design if more specific codes (e.g., [bkd], [tyd]) are not desired.") <<
         tr("Director") << "drt" << tr("Use for a person or organization who is responsible for the general management of a work or who supervises the production of a performance for stage, screen, or sound recording.") <<
         tr("Dissertant") << "dis" << tr("Use for a person who presents a thesis for a university or higher-level educational degree.") <<
         tr("Distributor") << "dst" << tr("Use for a person or organization that has exclusive or shared marketing rights for an item.") <<
         tr("Donor") << "dnr" << tr("Use for a person or organization who is the donor of a book, manuscript, etc., to its present owner. Donors to previous owners are designated as Former owner [fmo] or Inscriber [ins].") <<
         tr("Draftsman") << "drm" << tr("Use for a person or organization who prepares artistic or technical drawings. ") <<
         tr("Dubious author") << "dub" << tr("Use for a person or organization to which authorship has been dubiously or incorrectly ascribed.") <<
         tr("Editor") << "edt" << tr("Use for a person or organization who prepares for publication a work not primarily his/her own, such as by elucidating text, adding introductory or other critical matter, or technically directing an editorial staff.") <<
         tr("Electrician") << "elg" << tr("Use for a person responsible for setting up a lighting rig and focusing the lights for a production, and running the lighting at a performance.") <<
         tr("Electrotyper") << "elt" << tr("Use for a person or organization who creates a duplicate printing surface by pressure molding and electrodepositing of metal that is then backed up with lead for printing.") <<
         tr("Engineer") << "eng" << tr("Use for a person or organization that is responsible for technical planning and design, particularly with construction.") <<
         tr("Engraver") << "egr" << tr("Use for a person or organization who cuts letters, figures, etc. on a surface, such as a wooden or metal plate, for printing.") <<
         tr("Etcher") << "etr" << tr("Use for a person or organization who produces text or images for printing by subjecting metal, glass, or some other surface to acid or the corrosive action of some other substance.") <<
         tr("Expert") << "exp" << tr("Use for a person or organization in charge of the description and appraisal of the value of goods, particularly rare items, works of art, etc. ") <<
         tr("Facsimilist") << "fac" << tr("Use for a person or organization that executed the facsimile. ") <<
         tr("Field director") << "fld" << tr("Use for a person or organization that manages or supervises the work done to collect raw data or do research in an actual setting or environment (typically applies to the natural and social sciences).") <<
         tr("Film editor") << "flm" << tr("Use for a person or organization who is an editor of a motion picture film. This term is used regardless of the medium upon which the motion picture is produced or manufactured (e.g., acetate film, video tape). ") <<
         tr("First party") << "fpy" << tr("Use for a person or organization who is identified as the only party or the party of the first part. In the case of transfer of right, this is the assignor, transferor, licensor, grantor, etc. Multiple parties can be named jointly as the first party") <<
         tr("Forger") << "frg" << tr("Use for a person or organization who makes or imitates something of value or importance, especially with the intent to defraud. ") <<
         tr("Former owner") << "fmo" << tr("Use for a person or organization who owned an item at any time in the past. Includes those to whom the material was once presented. A person or organization giving the item to the present owner is designated as Donor [dnr]") <<
         tr("Funder") << "fnd" << tr("Use for a person or organization that furnished financial support for the production of the work.") <<
         tr("Geographic information specialist") << "gis" << tr("Use for a person responsible for geographic information system (GIS) development and integration with global positioning system data.") <<
         tr("Honoree") << "hnr" << tr("Use for a person or organization in memory or honor of whom a book, manuscript, etc. is donated. ") <<
         tr("Host") << "hst" << tr("Use for a person who is invited or regularly leads a program (often broadcast) that includes other guests, performers, etc. (e.g., talk show host).") <<
         tr("Illuminator") << "ilu" << tr("Use for a person or organization responsible for the decoration of a work (especially manuscript material) with precious metals or color, usually with elaborate designs and motifs.") <<
         tr("Illustrator") << "ill" << tr("Use for a person or organization who conceives, and perhaps also implements, a design or illustration, usually to accompany a written text.") <<
         tr("Inscriber") << "ins" << tr("Use for a person who signs a presentation statement.") <<
         tr("Instrumentalist") << "itr" << tr("Use for a person or organization who principally plays an instrument in a musical or dramatic presentation or entertainment.") <<
         tr("Interviewee") << "ive" << tr("Use for a person or organization who is interviewed at a consultation or meeting, usually by a reporter, pollster, or some other information gathering agent.") <<
         tr("Interviewer") << "ivr" << tr("Use for a person or organization who acts as a reporter, pollster, or other information gathering agent in a consultation or meeting involving one or more individuals.") <<
         tr("Inventor") << "inv" << tr("Use for a person or organization who first produces a particular useful item, or develops a new process for obtaining a known item or result.") <<
         tr("Laboratory") << "lbr" << tr("Use for an institution that provides scientific analyses of material samples.") <<
         tr("Laboratory director") << "ldr" << tr("Use for a person or organization that manages or supervises work done in a controlled setting or environment. ") <<
         tr("Landscape architect") << "lsa" << tr("Use for a person or organization whose work involves coordinating the arrangement of existing and proposed land features and structures.") <<
         tr("Lead") << "led" << tr("Use to indicate that a person or organization takes primary responsibility for a particular activity or endeavor. Use with another relator term or code to show the greater importance this person or organization has regarding that particular role. If more than one relator is assigned to a heading, use the Lead relator only if it applies to all the relators.") <<
         tr("Lender") << "len" << tr("Use for a person or organization permitting the temporary use of a book, manuscript, etc., such as for photocopying or microfilming.") <<
         tr("Libelant") << "lil" << tr("Use for the party who files a libel in an ecclesiastical or admiralty case.") <<
         tr("Libelant-appellant") << "lit" << tr("Use for a libelant who takes an appeal from one ecclesiastical court or admiralty to another to reverse the judgment.") <<
         tr("Libelant-appellee") << "lie" << tr("Use for a libelant against whom an appeal is taken from one ecclesiastical court or admiralty to another to reverse the judgment.") <<
         tr("Libelee") << "lel" << tr("Use for a party against whom a libel has been filed in an ecclesiastical court or admiralty.") <<
         tr("Libelee-appellant") << "let" << tr("Use for a libelee who takes an appeal from one ecclesiastical court or admiralty to another to reverse the judgment.") <<
         tr("Libelee-appellee") << "lee" << tr("Use for a libelee against whom an appeal is taken from one ecclesiastical court or admiralty to another to reverse the judgment.") <<
         tr("Librettist") << "lbt" << tr("Use for a person or organization who is a writer of the text of an opera, oratorio, etc.") <<
         tr("Licensee") << "lse" << tr("Use for a person or organization who is an original recipient of the right to print or publish.") <<
         tr("Licensor") << "lso" << tr("Use for person or organization who is a signer of the license, imprimatur, etc. ") <<
         tr("Lighting designer") << "lgd" << tr("Use for a person or organization who designs the lighting scheme for a theatrical presentation, entertainment, motion picture, etc.") <<
         tr("Lithographer") << "ltg" << tr("Use for a person or organization who prepares the stone or plate for lithographic printing, including a graphic artist creating a design directly on the surface from which printing will be done.") <<
         tr("Lyricist") << "lyr" << tr("Use for a person or organization who is the a writer of the text of a song.") <<
         tr("Manufacturer") << "mfr" << tr("Use for a person or organization that makes an artifactual work (an object made or modified by one or more persons). Examples of artifactual works include vases, cannons or pieces of furniture.") <<
         tr("Markup editor") << "mrk" << tr("Use for a person or organization performing the coding of SGML, HTML, or XML markup of metadata, text, etc.") <<
         tr("Metadata contact") << "mdc" << tr("Use for a person or organization primarily responsible for compiling and maintaining the original description of a metadata set (e.g., geospatial metadata set).") <<
         tr("Metal-engraver") << "mte" << tr("Use for a person or organization responsible for decorations, illustrations, letters, etc. cut on a metal surface for printing or decoration.") <<
         tr("Moderator") << "mod" << tr("Use for a person who leads a program (often broadcast) where topics are discussed, usually with participation of experts in fields related to the discussion.") <<
         tr("Monitor") << "mon" << tr("Use for a person or organization that supervises compliance with the contract and is responsible for the report and controls its distribution. Sometimes referred to as the grantee, or controlling agency.") <<
         tr("Music copyist") << "mcp" << tr("Use for a person who transcribes or copies musical notation") <<
         tr("Musical director") << "msd" << tr("Use for a person responsible for basic music decisions about a production, including coordinating the work of the composer, the sound editor, and sound mixers, selecting musicians, and organizing and/or conducting sound for rehearsals and performances.") <<
         tr("Musician") << "mus" << tr("Use for a person or organization who performs music or contributes to the musical content of a work when it is not possible or desirable to identify the function more precisely.") <<
         tr("Narrator") << "nrt" << tr("Use for a person who is a speaker relating the particulars of an act, occurrence, or course of events.") <<
         tr("Opponent") << "opn" << tr("Use for a person or organization responsible for opposing a thesis or dissertation.") <<
         tr("Organizer of meeting") << "orm" << tr("Use for a person or organization responsible for organizing a meeting for which an item is the report or proceedings.") <<
         tr("Originator") << "org" << tr("Use for a person or organization performing the work, i.e., the name of a person or organization associated with the intellectual content of the work. This category does not include the publisher or personal affiliation, or sponsor except where it is also the corporate author.") <<
         tr("Other") << "oth" << tr("Use for relator codes from other lists which have no equivalent in the MARC list or for terms which have not been assigned a code.") <<
         tr("Owner") << "own" << tr("Use for a person or organization that currently owns an item or collection.") <<
         tr("Papermaker") << "ppm" << tr("Use for a person or organization responsible for the production of paper, usually from wood, cloth, or other fibrous material.") <<
         tr("Patent applicant") << "pta" << tr("Use for a person or organization that applied for a patent.") <<
         tr("Patent holder") << "pth" << tr("Use for a person or organization that was granted the patent referred to by the item. ") <<
         tr("Patron") << "pat" << tr("Use for a person or organization responsible for commissioning a work. Usually a patron uses his or her means or influence to support the work of artists, writers, etc. This includes those who commission and pay for individual works.") <<
         tr("Performer") << "prf" << tr("Use for a person or organization who exhibits musical or acting skills in a musical or dramatic presentation or entertainment, if specific codes for those functions ([act], [dnc], [itr], [voc], etc.) are not used. If specific codes are used, [prf] is used for a person whose principal skill is not known or specified.") <<
         tr("Permitting agency") << "pma" << tr("Use for an authority (usually a government agency) that issues permits under which work is accomplished.") <<
         tr("Photographer") << "pht" << tr("Use for a person or organization responsible for taking photographs, whether they are used in their original form or as reproductions.") <<
         tr("Plaintiff") << "ptf" << tr("Use for the party who complains or sues in court in a personal action, usually in a legal proceeding.") <<
         tr("Plaintiff-appellant") << "ptt" << tr("Use for a plaintiff who takes an appeal from one court or jurisdiction to another to reverse the judgment, usually in a legal proceeding.") <<
         tr("Plaintiff-appellee") << "pte" << tr("Use for a plaintiff against whom an appeal is taken from one court or jurisdiction to another to reverse the judgment, usually in a legal proceeding.") <<
         tr("Platemaker") << "plt" << tr("Use for a person or organization responsible for the production of plates, usually for the production of printed images and/or text.") <<
         tr("Printer") << "prt" << tr("Use for a person or organization who prints texts, whether from type or plates.") <<
         tr("Printer of plates") << "pop" << tr("Use for a person or organization who prints illustrations from plates. ") <<
         tr("Printmaker") << "prm" << tr("Use for a person or organization who makes a relief, intaglio, or planographic printing surface.") <<
         tr("Process contact") << "prc" << tr("Use for a person or organization primarily responsible for performing or initiating a process, such as is done with the collection of metadata sets.") <<
         tr("Producer") << "pro" << tr("Use for a person or organization responsible for the making of a motion picture, including business aspects, management of the productions, and the commercial success of the work.") <<
         tr("Production manager") << "pmn" << tr("Use for a person responsible for all technical and business matters in a production.") <<
         tr("Production personnel") << "prd" << tr("Use for a person or organization associated with the production (props, lighting, special effects, etc.) of a musical or dramatic presentation or entertainment.") <<
         tr("Programmer") << "prg" << tr("Use for a person or organization responsible for the creation and/or maintenance of computer program design documents, source code, and machine-executable digital files and supporting documentation.") <<
         tr("Project director") << "pdr" << tr("Use for a person or organization with primary responsibility for all essential aspects of a project, or that manages a very large project that demands senior level responsibility, or that has overall responsibility for managing projects, or provides overall direction to a project manager.") <<
         tr("Proofreader") << "pfr" << tr("Use for a person who corrects printed matter. For manuscripts, use Corrector [crr].") <<
         tr("Publisher") << "pbl" << tr("Use for a person or organization that makes printed matter, often text, but also printed music, artwork, etc. available to the public.") <<
         tr("Publishing director") << "pbd" << tr("Use for a person or organization who presides over the elaboration of a collective work to ensure its coherence or continuity. This includes editors-in-chief, literary editors, editors of series, etc.") <<
         tr("Puppeteer") << "ppt" << tr("Use for a person or organization who manipulates, controls, or directs puppets or marionettes in a musical or dramatic presentation or entertainment.") <<
         tr("Recipient") << "rcp" << tr("Use for a person or organization to whom correspondence is addressed.") <<
         tr("Recording engineer") << "rce" << tr("Use for a person or organization who supervises the technical aspects of a sound or video recording session.") <<
         tr("Redactor") << "red" << tr("Use for a person or organization who writes or develops the framework for an item without being intellectually responsible for its content.") <<
         tr("Renderer") << "ren" << tr("Use for a person or organization who prepares drawings of architectural designs (i.e., renderings) in accurate, representational perspective to show what the project will look like when completed.") <<
         tr("Reporter") << "rpt" << tr("Use for a person or organization who writes or presents reports of news or current events on air or in print.") <<
         tr("Repository") << "rps" << tr("Use for an agency that hosts data or material culture objects and provides services to promote long term, consistent and shared use of those data or objects.") <<
         tr("Research team head") << "rth" << tr("Use for a person who directed or managed a research project.") <<
         tr("Research team member") << "rtm" << tr("Use for a person who participated in a research project but whose role did not involve direction or management of it.") <<
         tr("Researcher") << "res" << tr("Use for a person or organization responsible for performing research. ") <<
         tr("Respondent") << "rsp" << tr("Use for the party who makes an answer to the courts pursuant to an application for redress, usually in an equity proceeding.") <<
         tr("Respondent-appellant") << "rst" << tr("Use for a respondent who takes an appeal from one court or jurisdiction to another to reverse the judgment, usually in an equity proceeding.") <<
         tr("Respondent-appellee") << "rse" << tr("Use for a respondent against whom an appeal is taken from one court or jurisdiction to another to reverse the judgment, usually in an equity proceeding.") <<
         tr("Responsible party") << "rpy" << tr("Use for a person or organization legally responsible for the content of the published material.") <<
         tr("Restager") << "rsg" << tr("Use for a person or organization, other than the original choreographer or director, responsible for restaging a choreographic or dramatic work and who contributes minimal new content.") <<
         tr("Reviewer") << "rev" << tr("Use for a person or organization responsible for the review of a book, motion picture, performance, etc.") <<
         tr("Rubricator") << "rbr" << tr("Use for a person or organization responsible for parts of a work, often headings or opening parts of a manuscript, that appear in a distinctive color, usually red.") <<
         tr("Scenarist") << "sce" << tr("Use for a person or organization who is the author of a motion picture screenplay.") <<
         tr("Scientific advisor") << "sad" << tr("Use for a person or organization who brings scientific, pedagogical, or historical competence to the conception and realization on a work, particularly in the case of audio-visual items.") <<
         tr("Scribe") << "scr" << tr("Use for a person who is an amanuensis and for a writer of manuscripts proper. For a person who makes pen-facsimiles, use Facsimilist [fac].") <<
         tr("Sculptor") << "scl" << tr("Use for a person or organization who models or carves figures that are three-dimensional representations.") <<
         tr("Second party") << "spy" << tr("Use for a person or organization who is identified as the party of the second part. In the case of transfer of right, this is the assignee, transferee, licensee, grantee, etc. Multiple parties can be named jointly as the second party.") <<
         tr("Secretary") << "sec" << tr("Use for a person or organization who is a recorder, redactor, or other person responsible for expressing the views of a organization.") <<
         tr("Set designer") << "std" << tr("Use for a person or organization who translates the rough sketches of the art director into actual architectural structures for a theatrical presentation, entertainment, motion picture, etc. Set designers draw the detailed guides and specifications for building the set.") <<
         tr("Signer") << "sgn" << tr("Use for a person whose signature appears without a presentation or other statement indicative of provenance. When there is a presentation statement, use Inscriber [ins].") <<
         tr("Singer") << "sng" << tr("Use for a person or organization who uses his/her/their voice with or without instrumental accompaniment to produce music. A performance may or may not include actual words.") <<
         tr("Sound designer") << "sds" << tr("Use for a person who produces and reproduces the sound score (both live and recorded), the installation of microphones, the setting of sound levels, and the coordination of sources of sound for a production.") <<
         tr("Speaker") << "spk" << tr("Use for a person who participates in a program (often broadcast) and makes a formalized contribution or presentation generally prepared in advance.") <<
         tr("Sponsor") << "spn" << tr("Use for a person or organization that issued a contract or under the auspices of which a work has been written, printed, published, etc.") <<
         tr("Stage manager") << "stm" << tr("Use for a person who is in charge of everything that occurs on a performance stage, and who acts as chief of all crews and assistant to a director during rehearsals.") <<
         tr("Standards body") << "stn" << tr("Use for an organization responsible for the development or enforcement of a standard.") <<
         tr("Stereotyper") << "str" << tr("Use for a person or organization who creates a new plate for printing by molding or copying another printing surface.") <<
         tr("Storyteller") << "stl" << tr("Use for a person relaying a story with creative and/or theatrical interpretation.") <<
         tr("Supporting host") << "sht" << tr("Use for a person or organization that supports (by allocating facilities, staff, or other resources) a project, program, meeting, event, data objects, material culture objects, or other entities capable of support. ") <<
         tr("Surveyor") << "srv" << tr("Use for a person or organization who does measurements of tracts of land, etc. to determine location, forms, and boundaries.") <<
         tr("Teacher") << "tch" << tr("Use for a person who, in the context of a resource, gives instruction in an intellectual subject or demonstrates while teaching physical skills. ") <<
         tr("Technical director") << "tcd" << tr("Use for a person who is ultimately in charge of scenery, props, lights and sound for a production.") <<
         tr("Thesis advisor") << "ths" << tr("Use for a person under whose supervision a degree candidate develops and presents a thesis, mémoire, or text of a dissertation. ") <<
         tr("Transcriber") << "trc" << tr("Use for a person who prepares a handwritten or typewritten copy from original material, including from dictated or orally recorded material. For makers of pen-facsimiles, use Facsimilist [fac].") <<
         tr("Translator") << "trl" << tr("Use for a person or organization who renders a text from one language into another, or from an older form of a language into the modern form.") <<
         tr("Type designer") << "tyd" << tr("Use for a person or organization who designed the type face used in a particular item. ") <<
         tr("Typographer") << "tyg" << tr("Use for a person or organization primarily responsible for choice and arrangement of type used in an item. If the typographer is also responsible for other aspects of the graphic design of a book (e.g., Book designer [bkd]), codes for both functions may be needed.") <<
         tr("Videographer") << "vdg" << tr("Use for a person or organization in charge of a video production, e.g. the video recording of a stage production as opposed to a commercial motion picture. The videographer may be the camera operator or may supervise one or more camera operators. Do not confuse with cinematographer.") <<
         tr("Vocalist") << "voc" << tr("Use for a person or organization who principally exhibits singing skills in a musical or dramatic presentation or entertainment.") <<
         tr("Witness") << "wit" << tr("Use for a person who verifies the truthfulness of an event or action. ") <<
         tr("Wood-engraver") << "wde" << tr("Use for a person or organization who makes prints by cutting the image in relief on the end-grain of a wood block.") <<
         tr("Woodcutter") << "wdc" << tr("Use for a person or organization who makes prints by cutting the image in relief on the plank side of a wood block.") <<
         tr("Writer of accompanying material") << "wam" << tr("Use for a person or organization who writes significant material which accompanies a sound recording or other audiovisual material.");

    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        MetaInfo meta;
        meta.name = name;
        meta.description  = description;
        m_Relators.insert(code, meta);
        m_RelatorFullNames.insert(name, code);
    }
}


// Converts HTML sourced Dublin Core metadata to OPF style metadata
Metadata::MetaElement Metadata::HtmlToOpfDC(const Metadata::MetaElement &meta)
{
    // Dublin Core from html file with the original 15 element namespace or
    // expanded DCTerms namespace. Allows qualifiers as refinements
    // prefix.name[.refinement]
    QStringList fields = QString(meta.name.toLower() + "..").split(".");
    QString name       = fields[ 1 ];
    QString refinement = fields[ 2 ];
    QString dc_event;

    if (MODIFICATION_ALIASES.contains(name) || MODIFICATION_ALIASES.contains(refinement)) {
        name     = "date";
        dc_event = "modification";
    } else if (CREATION_ALIASES.contains(name) || CREATION_ALIASES.contains(refinement)) {
        name     = "date";
        dc_event = "creation";
    } else if (PUBLICATION_ALIASES.contains(name) || PUBLICATION_ALIASES.contains(refinement)) {
        name     = "date";
        dc_event = "publication";
    }

    QString role   = (name == "creator") || (name == "contributor") ? refinement : QString();
    QString scheme = meta.attributes.value("scheme");

    if ((name == "identifier") && (scheme.isEmpty())) {
        scheme = refinement;
    }

    if (!scheme.isEmpty()) {
        if (SCHEME_LIST.contains(scheme, Qt::CaseInsensitive)) {
            scheme = SCHEME_LIST.filter(scheme, Qt::CaseInsensitive)[ 0 ];
        }
    }

    MetaElement opf_meta;
    opf_meta.name  = name;
    opf_meta.value = meta.value;

    if (!scheme.isEmpty()) {
        opf_meta.attributes[ "scheme" ] = scheme;
    }

    if (!dc_event.isEmpty()) {
        opf_meta.attributes[ "event" ] = dc_event;
    }

    if (!role.isEmpty()) {
        opf_meta.attributes[ "role" ] = role;
    }

    return opf_meta;
}


// Converts free form metadata into internal book metadata
Metadata::MetaElement Metadata::FreeFormMetadata(const Metadata::MetaElement &meta)
{
    // non - dublin core meta info from html file, if this maps to
    // one of the metadata basic fields used internally pass it through
    // i.e. Author, Title, Publisher, Rights/CopyRight, EISBN/ISBN
    QString name = meta.name.toLower();

    // Remap commonly used meta values to match internal names
    if (name == "copyright") {
        name = "rights";
    } else if (name  == "eisbn") {
        name = "ISBN";
    } else if (name  == "issn") {
        name = "ISSN";
    } else if (name == "doi") {
        name = "DOI";
    }

    MetaElement book_meta;
    book_meta.name  = name;
    book_meta.value = meta.value;
    return book_meta;
}


// Converts dc:creator and dc:contributor metadata to book internal metadata
Metadata::MetaElement Metadata::CreateContribMetadata(const Metadata::MetaElement &meta)
{
    QString role    = meta.attributes.value("role", "aut");

    // Some epub exporters set incorrect opf:role attributes
    // and we need to handle that. Otherwise, Sigil bugs out on export.
    // Since we can't tell what the role is, just guess author.
    if (!IsRelator(role)) {
        role = "aut";
    }

    MetaElement book_meta;
    book_meta.name  = role;
    book_meta.value = meta.value.toString();
    book_meta.file_as = meta.attributes.value("file-as");
    // Save contributor or creator
    book_meta.role_type = meta.name.toLower();
    return book_meta;
}


// Converts dc:date metadata to book internal metadata
Metadata::MetaElement Metadata::DateMetadata(const Metadata::MetaElement &meta)
{
    QString dc_event = meta.attributes.value("event");
    // Dates are in YYYY[-MM[-DD]] format
    QStringList date_parts = meta.value.toString().split("-", QString::SkipEmptyParts);

    if (date_parts.count() < 1) {
        date_parts.append(QString::number(QDate::currentDate().year()));
    }

    if (date_parts.count() < 2) {
        date_parts.append("01");
    }

    if (date_parts.count() < 3) {
        date_parts.append("01");
    }

    QVariant value = QDate(date_parts[ 0 ].toInt(),
                           date_parts[ 1 ].toInt(),
                           date_parts[ 2 ].toInt());
    MetaElement book_meta;
    book_meta.name  = meta.name;
    book_meta.value = value;
    book_meta.file_as = dc_event;
    return book_meta;
}


// Converts dc:identifier metadata to book internal metadata
Metadata::MetaElement Metadata::IdentifierMetadata(const Metadata::MetaElement &meta)
{
    QString scheme = meta.attributes.value("scheme");
    QString id = meta.attributes.value("id");
    MetaElement book_meta;

    // Ignore any identifiers with an id as id can't be edited in dialog
    // And skip the uuid identifier in case it made it through without an id
    if (id.isEmpty() && scheme.toLower() != "uuid") {
        book_meta.name = meta.name;
        book_meta.value = meta.value;
        book_meta.file_as = scheme;
    }

    return book_meta;
}

