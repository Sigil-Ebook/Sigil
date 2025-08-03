#ifndef PREVIEWFILEDIALOG_H
#define PREVIEWFILEDIALOG_H

#include <QFileDialog>

class QLabel;

class PreviewFileDialog : public QFileDialog
{
		Q_OBJECT
	public:
		explicit PreviewFileDialog(
			QWidget* parent = 0,
			const QString & caption = QString(),
			const QString & directory = QString(),
			const QString & filter = QString(),
            QString * selectedFilter = nullptr,
            QFileDialog::Options options = Options()
		);

    QStringList getFilesSelected() { return m_files_selected; };
    QString     getFileSelected() { return m_file_selected; };

	protected slots:
		void OnCurrentChanged(const QString & path);
	    void OnFileSelected(const QString& file);
        void OnFilesSelected(const QStringList& files);

	protected:
		QLabel* mpPreview;
        QString m_file_selected;
        QStringList m_files_selected;

};

#endif // PREVIEWFILEDIALOG_H
