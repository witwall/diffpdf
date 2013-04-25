/*
    Copyright © 2008-13 Qtrac Ltd. All rights reserved.
    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 2 of
    the License, or (at your option) any later version. This program is
    distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
    for more details.
*/

#include "saveform.hpp"
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QImageWriter>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>


SaveForm::SaveForm(const QString &path, QString *filename, bool *saveAll,
        SavePages *savePages, QWidget *parent)
    : QDialog(parent), m_path(path), m_filename(filename),
      m_saveAll(saveAll), m_savePages(savePages)
{
    createWidgets();
    createLayout();
    createConnections();

    updateUi();
    setWindowTitle(tr("DiffPDF — Save As"));
}


void SaveForm::createWidgets()
{
    saveGroupBox = new QGroupBox(tr("Pairs"));
    saveCurrentRadioButton = new QRadioButton(tr("Current &Page Pair"));
    saveAllRadioButton = new QRadioButton(tr("&All Page Pairs"));
    if (*m_saveAll)
        saveAllRadioButton->setChecked(true);
    else
        saveCurrentRadioButton->setChecked(true);
    pagesGroupBox = new QGroupBox(tr("Pages"));
    leftPagesRadioButton = new QRadioButton(tr("File #&1's Pages"));
    rightPagesRadioButton = new QRadioButton(tr("File #&2's Pages"));
    bothPagesRadioButton = new QRadioButton(tr("&Both Files' Pages"));
    if (*m_savePages == SaveLeftPages)
        leftPagesRadioButton->setChecked(true);
    else if (*m_savePages == SaveRightPages)
        rightPagesRadioButton->setChecked(true);
    else // m_savePages == SaveBothPages
        bothPagesRadioButton->setChecked(true);
    filenameLabel = new QLabel(tr("&Filename:"));
    filenameLineEdit = new QLineEdit;
    filenameLineEdit->setText(*m_filename);
    filenameLabel->setBuddy(filenameLineEdit);
    chooseFileButton = new QPushButton(tr("B&rowse..."));
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|
                                     QDialogButtonBox::Cancel);
}


void SaveForm::createLayout()
{
    QHBoxLayout *saveLayout = new QHBoxLayout;
    saveLayout->addWidget(saveCurrentRadioButton);
    saveLayout->addWidget(saveAllRadioButton);
    saveLayout->addStretch();
    saveGroupBox->setLayout(saveLayout);
    QHBoxLayout *pagesLayout = new QHBoxLayout;
    pagesLayout->addWidget(leftPagesRadioButton);
    pagesLayout->addWidget(rightPagesRadioButton);
    pagesLayout->addWidget(bothPagesRadioButton);
    pagesGroupBox->setLayout(pagesLayout);
    QHBoxLayout *filenameLayout = new QHBoxLayout;
    filenameLayout->addWidget(filenameLabel);
    filenameLayout->addWidget(filenameLineEdit);
    filenameLayout->addWidget(chooseFileButton);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(saveGroupBox);
    layout->addWidget(pagesGroupBox);
    layout->addLayout(filenameLayout);
    layout->addStretch();
    layout->addWidget(buttonBox);
    setLayout(layout);
}


void SaveForm::createConnections()
{
    connect(chooseFileButton, SIGNAL(clicked()), this, SLOT(chooseFile()));
    connect(filenameLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(updateUi()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


void SaveForm::updateUi()
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
            !filenameLineEdit->text().isEmpty());
}


void SaveForm::chooseFile()
{
    QList<QByteArray> formats = QImageWriter::supportedImageFormats();
    QStringList suffixes;
    foreach (const QByteArray &format, formats)
        suffixes << "*." + QString(format.toLower());
    QString filename = QFileDialog::getSaveFileName(this,
            tr("DiffPDF — Browse"), m_path,
            tr("PDF files (*.pdf);;Image files (%1)")
            .arg(suffixes.join(" ")));
    if (!filename.isEmpty()) {
        filenameLineEdit->setText(filename);
        buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    }
}


void SaveForm::accept()
{
    *m_filename = filenameLineEdit->text();
    if (!m_filename->contains("."))
        *m_filename += ".pdf";
    *m_saveAll = saveAllRadioButton->isChecked();
    if (leftPagesRadioButton->isChecked())
        *m_savePages = SaveLeftPages;
    else if (rightPagesRadioButton->isChecked())
        *m_savePages = SaveRightPages;
    else // bothPagesRadioButton->isChecked()
        *m_savePages = SaveBothPages;
    QDialog::accept();
}
