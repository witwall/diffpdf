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

#include "generic.hpp"
#include "lineedit.hpp"
#include <QDragEnterEvent>
#include <QDragEnterEvent>


LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent)
{
    setAcceptDrops(true);
}

void LineEdit::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasFormat("text/plain") ||
        mimeData->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}


void LineEdit::dropEvent(QDropEvent *event)
{
    QStringList filenames = droppedFilenames(event->mimeData());
    if (!filenames.isEmpty())
        emit filenamesDropped(filenames);
    event->acceptProposedAction();
}
