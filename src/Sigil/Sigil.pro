win32{
	TEMPLATE = vcapp
	LIBEXT = lib
	LIBPREF =
}

unix{
	TEMPLATE = app
	LIBEXT = a
	LIBPREF = lib
}

QT += xml svg webkit
CONFIG += qt debug_and_release
DEFINES += QT_XML_LIB QT_SVG_LIB

CONFIG(debug, debug|release) {
	LIBS += "../../lib/$${LIBPREF}tidyLibd.$${LIBEXT}"
	LIBS += "../../lib/$${LIBPREF}ZipArchived.$${LIBEXT}"	
}

# Release build
else{
	LIBS += "../../lib/$${LIBPREF}tidyLib.$$LIBEXT"
	LIBS += "../../lib/$${LIBPREF}ZipArchive.$${LIBEXT}"
}

INCLUDEPATH += "../ZipArchive";"../tidyLib"

HEADERS += 	AddMetadata.h \
			About.h \
			Book.h \
			BookNormalization.h \
			CleanSource.h \
			CodeViewEditor.h \
			constants.h \
			ExportEPUB.h \
			Exporter.h \
			ExporterFactory.h \
			ExportSGF.h \
			FolderKeeper.h \
            Headings.h \			
			ImportEPUB.h \
			Importer.h \
			ImporterFactory.h \
			ImportHTML.h \
			ImportTXT.h \
			ImportSGF.h \
			LineNumberArea.h \
			MainWindow.h \
			Metadata.h \
			MetaEditor.h \
			NCXWriter.h \
			OPFWriter.h \
			SigilMarkup.h \
			stdafx.h \
			TOCEditor.h \
			Utility.h \			
			XHTMLHighlighter.h \
			XMLWriter.h \

SOURCES += 	AddMetadata.cpp \
			About.cpp \
			Book.cpp \
			BookNormalization.cpp \
			CleanSource.cpp \
			CodeViewEditor.cpp \
			ExportEPUB.cpp \
			ExporterFactory.cpp \
			ExportSGF.cpp \
			FolderKeeper.cpp \
            Headings.cpp \			
			ImportEPUB.cpp \
			ImporterFactory.cpp \
			ImportHTML.cpp \
			ImportTXT.cpp \
			ImportSGF.cpp \
			main.cpp \
			LineNumberArea.cpp \
			MainWindow.cpp \
			Metadata.cpp \
			MetaEditor.cpp \
			NCXWriter.cpp \
			OPFWriter.cpp \
			SigilMarkup.cpp \
			stdafx.cpp \
			TOCEditor.cpp \
			Utility.cpp \			
			XHTMLHighlighter.cpp \
			XMLWriter.cpp \
			
FORMS += 	Form_Files/main.ui \
			Form_Files/MetaEditor.ui \
			Form_Files/AddMetadata.ui \
			Form_Files/About.ui \
			Form_Files/TOCEditor.ui  			

RESOURCES += 	Resource_Files/main/main.qrc \
				Resource_Files/About/About.qrc
				
RC_FILE = Resource_Files/icon/icon.rc
ICON = Resource_Files/icon/Sigil.icns

PRECOMPILED_HEADER = stdafx.h
PRECOMPILED_SOURCE = stdafx.cpp


