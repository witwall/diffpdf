#ifndef TEXTITEM_HPP
#define TEXTITEM_HPP
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

#include "generic.hpp"

#include <QList>
#include <QPainterPath>
#include <QRectF>
#include <QString>


struct TextItem
{
    TextItem(const QString &text, const QRectF &rect)
        : text(text), rect(rect) {}

    QRect toRect() const { return rect.toRect(); }

    QString text;
    QRectF rect;
};


class TextItems
{
public:
    TextItem at(const int index) const { return items.at(index); }
    void append(const TextItem &item) { items << item; }
    int count() const { return items.count(); }
    QStringList texts() const;
    QList<QRectF> rects() const;
    void columnZoneYxOrder(const int Width, const int ToleranceR,
            const int ToleranceY, const int Columns);
    void columnYxOrder(const int Width, const int ToleranceY,
                       const int Columns);
    const QList<QPainterPath> generateZones(const int Width,
            const int ToleranceR, const int ToleranceY,
            const int Columns) const;

    void debug(const int page, const int ToleranceY,
               const bool ComparingWords=true, const bool Yx=false);

private:
    QList<TextItem> items;
};


inline int normalizedY(const int y, const int ToleranceY);

const TextItems getWords(const TextBoxList &list);
const TextItems getCharacters(const TextBoxList &list);

#endif // TEXTITEM_HPP
