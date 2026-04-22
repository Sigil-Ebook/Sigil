#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "Dialogs/ImageResizeDialog.h"

ImageResizeDialog::ImageResizeDialog(int initialW, int initialH, QWidget *parent)
    : QDialog(parent), ratio((double)initialW / initialH) {
    setWindowTitle("Resize Image");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Width Input
    QHBoxLayout *wLayout = new QHBoxLayout();
    wLayout->addWidget(new QLabel("Width:"));
    widthSpin = new QSpinBox();
    widthSpin->setRange(1, 10000);
    widthSpin->setValue(initialW);
    wLayout->addWidget(widthSpin);
    mainLayout->addLayout(wLayout);

    // Height Input
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(new QLabel("Height:"));
    heightSpin = new QSpinBox();
    heightSpin->setRange(1, 10000);
    heightSpin->setValue(initialH);
    hLayout->addWidget(heightSpin);
    mainLayout->addLayout(hLayout);

    // Aspect Ratio Checkbox
    aspectCheck = new QCheckBox("Keep Aspect Ratio");
    aspectCheck->setChecked(true);
    mainLayout->addWidget(aspectCheck);

    // Standard Buttons (OK/Cancel)
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("OK");
    QPushButton *cancelBtn = new QPushButton("Cancel");
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    // Connections
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(widthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageResizeDialog::onWidthChanged);
    connect(heightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageResizeDialog::onHeightChanged);
}

void ImageResizeDialog::onWidthChanged(int w) {
    if (aspectCheck->isChecked() && !isSyncing) {
        isSyncing = true;
        heightSpin->setValue(qRound(w / ratio));
        isSyncing = false;
    }
}

void ImageResizeDialog::onHeightChanged(int h) {
    if (aspectCheck->isChecked() && !isSyncing) {
        isSyncing = true;
        widthSpin->setValue(qRound(h * ratio));
        isSyncing = false;
    }
}

