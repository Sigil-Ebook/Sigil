#ifndef IMAGERESIZEDIALOG_H
#define IMAGERESIZEDIALOG_H
#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>

class ImageResizeDialog : public QDialog {
    Q_OBJECT
public:
    explicit ImageResizeDialog(int initialW, int initialH, QWidget *parent = nullptr);
    int getWidth() const { return widthSpin->value(); }
    int getHeight() const { return heightSpin->value(); }

private slots:
    void onWidthChanged(int w);
    void onHeightChanged(int h);

private:
    QSpinBox *widthSpin;
    QSpinBox *heightSpin;
    QCheckBox *aspectCheck;
    double ratio;
    bool isSyncing = false; // Prevents infinite recursion between signals
};

#endif // IMAGERESIZEDIALOG_H
