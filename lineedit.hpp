#ifndef LINEEDIT_HPP
#define LINEEDIT_HPP
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

#include <QLineEdit>


class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget *parent=0);

signals:
    void filenamesDropped(const QStringList &filenames);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};

#endif // LINEEDIT_HPP
