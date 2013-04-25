#ifndef SAVEFORM_HPP
#define SAVEFORM_HPP
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

#include <QBrush>
#include <QDialog>
#include <QPen>

class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

enum SavePages {SaveLeftPages, SaveRightPages, SaveBothPages};

class SaveForm : public QDialog
{
    Q_OBJECT

public:
    SaveForm(const QString &path, QString *filename, bool *saveAll,
             SavePages *savePages, QWidget *parent=0);

private slots:
    void accept();
    void updateUi();
    void chooseFile();

private:
    void createWidgets();
    void createLayout();
    void createConnections();

    QGroupBox *saveGroupBox;
    QRadioButton *saveCurrentRadioButton;
    QRadioButton *saveAllRadioButton;
    QGroupBox *pagesGroupBox;
    QRadioButton *leftPagesRadioButton;
    QRadioButton *rightPagesRadioButton;
    QRadioButton *bothPagesRadioButton;
    QLabel *filenameLabel;
    QLineEdit *filenameLineEdit;
    QPushButton *chooseFileButton;
    QDialogButtonBox *buttonBox;

    const QString m_path;
    QString *m_filename;
    bool *m_saveAll;
    SavePages *m_savePages;
};

#endif // SAVEFORM_HPP
