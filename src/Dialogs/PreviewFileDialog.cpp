#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QGridLayout>
#include <QTextDocument>
#include "Misc/MediaTypes.h"
#include "Dialogs/PreviewFileDialog.h"

PreviewFileDialog::PreviewFileDialog(
	QWidget* parent,
	const QString & caption,
	const QString & directory,
	const QString & filter,
    QString * selectedFilter,
    QFileDialog::Options options
) :
    QFileDialog(parent, caption, directory, filter)
{
    setOptions(options);
	setObjectName("PreviewFileDialog");
	QVBoxLayout* box = new QVBoxLayout(this);
	mpPreview = new QLabel(tr("Preview"), this);
    mpPreview->setFixedSize(300, 300);
	mpPreview->setAlignment(Qt::AlignCenter);
	mpPreview->setObjectName("labelPreview");
	box->addWidget(mpPreview);
	
	box->addStretch();
	
	// add to QFileDialog layout
	{
		QGridLayout *layout = (QGridLayout*)this->layout();
		layout->addLayout(box, 1, 3, 3, 1);
	}
    adjustSize();
    connect(this, SIGNAL(currentChanged(const QString&)), this, SLOT(OnCurrentChanged(const QString&)));
    connect(this, SIGNAL(fileSelected(const QString&)), this, SLOT(OnFileSelected(const QString&)));
    connect(this, SIGNAL(filesSelected(const QStringList&)), this, SLOT(OnFilesSelected(const QStringList&)));
}

void PreviewFileDialog::OnCurrentChanged(const QString & path)
{
    QString mt = MediaTypes::instance()->GetFileDataMimeType(path, "application/octet-stream");
    // qDebug() << "media type is: " << mt;
    if (mt.endsWith("+xml") || mt.endsWith("/xml")  || mt.endsWith("html") || mt.startsWith("text/")) {
        QFile file(path);
        if (!file.open(QFile::ReadOnly)) {
            mpPreview->setText("no preview available");
            return;
        }
        QTextStream in(&file);
        in.setAutoDetectUnicode(true);
        QString txtdata = in.readAll();
        file.close();
        txtdata.replace("\x0D\x0A", "\x0A").replace("\x0D", "\x0A");
        QTextDocument doc;
        doc.setPlainText(txtdata);
        QImage image(800, 600, QImage::Format_ARGB32); 
        image.fill(Qt::white);
        QPainter painter(&image);
        painter.setPen(Qt::black);
        doc.drawContents(&painter);
        QPixmap pixmap = QPixmap::fromImage(image);
	    mpPreview->setPixmap(pixmap.scaled(mpPreview->width(), mpPreview->height(),
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else if (mt.startsWith("image/")) {
	    QPixmap pixmap = QPixmap(path);
	    if (pixmap.isNull()) {
		    mpPreview->setText("not an image");
	    } else {
		    mpPreview->setPixmap(pixmap.scaled(mpPreview->width(), mpPreview->height(),
                                 Qt::KeepAspectRatio, Qt::SmoothTransformation));
	    }
    } else {
        mpPreview->setText("no preview available");
    }
}

void PreviewFileDialog::OnFileSelected(const QString& file)
{
    m_file_selected = file;
}

void PreviewFileDialog::OnFilesSelected(const QStringList& files)
{
    m_files_selected = files;
}
