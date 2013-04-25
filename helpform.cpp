/*
    Copyright © 2011-13 Qtrac Ltd. All rights reserved.
    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 2 of
    the License, or (at your option) any later version. This program is
    distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
    for more details.
*/

#include "helpform.hpp"
#include <poppler-version.h>
#include <QApplication>
#include <QFile>
#include <QKeySequence>
#include <QSettings>
#include <QShortcut>
#include <QTextBrowser>
#include <QTextStream>


HelpForm::HelpForm(const QString &language, QWidget *parent)
    : QMainWindow(parent)
{
    QTextBrowser *viewer = new QTextBrowser;
    viewer->setOpenExternalLinks(true);
    QString filename = QString(":/help_%1.html").arg(language);
    if (!QFile::exists(filename))
        filename = ":/help.html";
    QFile file(filename);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream in(&file);
    in.setCodec("UTF-8");
    viewer->setHtml(in.readAll());
    viewer->setReadOnly(true);
    setCentralWidget(viewer);
    (void) new QShortcut(QKeySequence("Escape"), this, SLOT(close()));
    QPoint pos(0, 0);
    if (parent)
        pos = parent->pos();
    move(pos);
    resize(640, 480);
    QSettings settings;
    restoreGeometry(settings.value("HelpForm/Geometry").toByteArray());
    setWindowTitle(tr("%1 — Help").arg(qApp->applicationName()));
}


void HelpForm::closeEvent(QCloseEvent *)
{
    QSettings settings;
    settings.setValue("HelpForm/Geometry", saveGeometry());
}
