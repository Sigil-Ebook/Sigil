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
        QString getFileSelected() { return m_file_selected; };

        static QString getOpenFileName(QWidget *parent = nullptr,
                                       const QString &caption = QString(),
                                       const QString &dir = QString(),
                                       const QString &filter = QString(),
                                       QString *selectedFilter = nullptr,
                                       Options options = Options());

        static QStringList getOpenFileNames(QWidget *parent = nullptr,
                                            const QString &caption = QString(),
                                            const QString &dir = QString(),
                                            const QString &filter = QString(),
                                            QString *selectedFilter = nullptr,
                                            Options options = Options());
    
        static QString getSaveFileName(QWidget *parent = nullptr,
                                       const QString &caption = QString(),
                                       const QString &dir = QString(),
                                       const QString &filter = QString(),
                                       QString *selectedFilter = nullptr,
                                       Options options = Options());

        static QString getExistingDirectory(QWidget *parent = nullptr,
                                            const QString &caption = QString(),
                                            const QString &dir = QString(),
                                            Options options = ShowDirsOnly);
                                                             
    protected slots:
        void OnCurrentChanged(const QString & path);
        void OnFileSelected(const QString& file);
        void OnFilesSelected(const QStringList& files);
        void OnThemeChanged();

    protected:
        QLabel* mpPreview;
        QString m_file_selected;
        QStringList m_files_selected;
        QString m_path;

};

#endif // PREVIEWFILEDIALOG_H
