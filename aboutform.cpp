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

#include "aboutform.hpp"
#include <poppler-version.h>
#include <QApplication>
#include <QHBoxLayout>
#include <QSettings>
#include <QShortcut>
#include <QTabWidget>
#include <QTextBrowser>


static const QString Version("2.1.2");


AboutForm::AboutForm(QWidget *parent) : QDialog(parent)
{
    QTextBrowser *aboutBrowser = new QTextBrowser;
    aboutBrowser->setReadOnly(true);
    aboutBrowser->setOpenExternalLinks(true);
    aboutBrowser->setHtml(tr(
    "<table border=0>"
    "<tr><td width=90%><b>%1</a> %2</b> by Mark Summerfield</td>"
    "<td rowspan=3><img align=right src=\":/icon.png\"></td></tr>"
    "<tr><td><tt>&lt;mark@qtrac.eu&gt;</tt>.</td></tr>"
    "<tr><td colspan=2>Copyright &copy; 2008-13 "
    "<a href=\"http://www.qtrac.eu\">Qtrac</a> Ltd. All rights reserved."
    "</td></tr>"
    "<tr><td colspan=2>Built with Qt %3 and Poppler %4.</td></tr>"
    "</table><hr>"
    "<p>This program compares the text or the visual appearance of "
    "each page in two PDF files."
    "<hr><p>If you like %1 you might like my books:<ul>"
    "<li><a href=\"http://www.qtrac.eu/gobook.html\">"
    "Programming in Go</a></li>"
    "<li><a href=\"http://www.qtrac.eu/aqpbook.html\">"
    "Advanced Qt Programming</a></li>"
    "<li><a href=\"http://www.qtrac.eu/py3book.html\">"
    "Programming in Python 3</a></li>"
    "<li><a href=\"http://www.qtrac.eu/pyqtbook.html\">"
    "Rapid GUI Programming with Python and Qt</a></li>"
    "</ul>"
    "I also provide training and consultancy in C++, Go, Python&nbsp;2, "
    "Python&nbsp;3, C++/Qt, and PyQt4.").arg(qApp->applicationName())
            .arg(Version).arg(qVersion()).arg(POPPLER_VERSION));
    QTextBrowser *contributorsBrowser = new QTextBrowser;
    contributorsBrowser->setReadOnly(true);
    contributorsBrowser->setHtml(tr("<table>"
    "<tr><td>&bull;</td><td bgcolor=lightyellow><i>Anonymous Company</i> "
    "&mdash; funded the addition of the margin exclusion "
    "functionality</td></tr>"
    "<tr><td>&bull;</td><td><b>David Paleino</b> &mdash; "
    "Debian packager</td></tr>"
    "<tr><td>&bull;</td><td><b>Dirk Loss</b> &mdash; creating "
    "Mac binaries</td></tr>"
    "<tr><td>&bull;</td><td>Florian Heiderich &mdash; suggested "
    "using composition modes for showing subtle differences</td></tr>"
    "<tr><td>&bull;</td><td><b>Jasmin Blanchette</b> &mdash; "
    "the original idea and subsequent suggestions</td></tr>"
    "<tr><td>&bull;</td><td>Liviu Andronic &mdash; suggested adding "
    "drag and drop</td></tr>"
    "<tr><td>&bull;</td><td>Paul Howarth &mdash; suggestions "
    "resulting in Characters mode</td></tr>"
    "<tr><td>&bull;</td><td bgcolor=\"#F0F0F0\"><i>Pavel Fric</i> &mdash; "
    "Czech translation</td></tr>"
    "<tr><td>&bull;</td><td bgcolor=\"#F0F0F0\"><i>Pierre-Alain "
    "Bandinelli</i>&mdash; French translation</td></tr>"
    "<tr><td>&bull;</td><td bgcolor=\"#F0F0F0\"><i>Rainer Krachten</i> "
    "&mdash; German translation and various suggestions</td></tr>"
    "<tr><td>&bull;</td><td>Rory Gordon &mdash; suggested adding "
    "drag and drop</td></tr>"
    "<tr><td>&bull;</td><td><b>Steven Lee</b> &mdash; creating "
    "Windows binaries</td></tr>"
    "</table>"));
    QTextBrowser *licenceBrowser = new QTextBrowser;
    licenceBrowser->setReadOnly(true);
    licenceBrowser->setHtml(tr(
    "This program is free software: you can redistribute it "
    "and/or modify it under the terms of the GNU General Public License "
    "as published by the Free Software Foundation, either version 2 of "
    "the License, or (at your option), any "
    "later version. This program is distributed in the hope that it will "
    "be useful, but WITHOUT ANY WARRANTY; without even the implied "
    "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
    "See the GNU General Public License (in file <tt>gpl-2.0.txt</tt>) "
    "for more details."));
    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(aboutBrowser, tr("&About"));
    tabWidget->addTab(contributorsBrowser, tr("&Contributors"));
    tabWidget->addTab(licenceBrowser, tr("&License"));
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(tabWidget);
    setLayout(layout);
    resize(480, 400);
    setWindowTitle(tr("%1 — About").arg(qApp->applicationName()));
}
