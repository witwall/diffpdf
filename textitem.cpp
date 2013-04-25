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

#include "textitem.hpp"

#include <QDir>
#include <QFile>
#include <QTextStream>


QStringList TextItems::texts() const
{
    QStringList list;
    foreach (const TextItem &item, items)
        list << item.text;
    return list;
}


QList<QRectF> TextItems::rects() const
{
    QList<QRectF> list;
    foreach (const TextItem &item, items)
        list << item.rect;
    return list;
}

struct Key
{
    Key(const int a, const int b, const int c) : a(a), b(b), c(c) {}

    bool operator<(const Key &other) const {
        if (a != other.a) return a < other.a;
        if (b != other.b) return b < other.b;
        return c < other.c;
    }

    int a, b, c;
};


void TextItems::columnZoneYxOrder(const int Width, const int ToleranceR,
        const int ToleranceY, const int Columns)
{
    // Phase #1: Sort all the texts into (column, y, x) order
    columnYxOrder(Width, ToleranceY, Columns);
    // Phase #2: Sort all the texts into (column, zone, y, x) order
    QList<QPainterPath> zones = generateZones(Width, ToleranceR,
                                              ToleranceY, Columns);
    QMap<Key, TextItem> itemForZoneYx;
    foreach (const TextItem &item, items) {
        const QRectF rect = item.rect.adjusted(-ToleranceR, -ToleranceR,
                                               ToleranceR, ToleranceR);
        const int y = normalizedY(static_cast<int>(item.rect.y()),
                                  ToleranceY);
        for (int i = 0; i < zones.count(); ++i) {
            if (zones.at(i).intersects(rect)) {
                itemForZoneYx.insert(Key(i, y, item.rect.x()), item);
                break;
            }
        }
    }
    items = itemForZoneYx.values();
}


void TextItems::columnYxOrder(const int Width, const int ToleranceY,
                              const int Columns)
{
    // Phase #1: Sort all the texts into (column, y, x) order
    const int Span = Width / Columns;
    QMap<Key, TextItem> itemForColumnYx;
    foreach (const TextItem &item, items) {
        const QRect &rect = item.toRect();
        const int Column = ((Columns == 1) ? 0
            : (rect.width() > Span) ? Columns : rect.right() / Span);
        const int y = normalizedY(static_cast<int>(rect.y()), ToleranceY);
        itemForColumnYx.insert(Key(Column, y, rect.x()), item);
    }
    items = itemForColumnYx.values();
}


const QList<QPainterPath> TextItems::generateZones(const int Width,
        const int ToleranceR, const int ToleranceY,
        const int Columns) const
{ // Assumes that items are already in column, y, x order!
    // Phase #1: Generate the zones
    QList<QPainterPath> zones;
    foreach (const TextItem &item, items) {
        if (zones.isEmpty()) { // First word becomes first zone
            QPainterPath zone;
            zone.addRect(item.rect);
            zones << zone;
        } else { // Add to an existing zone within tolerance or a new one
            const QRectF tolerantRect = item.rect.adjusted(-ToleranceR,
                    -ToleranceR, ToleranceR, ToleranceR);
            bool found = false;
            for (int i = 0; i < zones.count(); ++i) {
                QPainterPath zone = zones.at(i);
                if (zone.intersects(tolerantRect)) {
                    zone.addRect(item.rect);
                    zones[i] = zone;
                    found = true;
                    break;
                }
            }
            if (!found) {
                QPainterPath zone;
                zone.addRect(item.rect);
                zones << zone;
            }
        }
    }

    // Phase #2: Order the zones by (column, y, x)
    const int Span = Width / Columns;
    QMultiMap<Key, QPainterPath> zonesForColumn;
    foreach (const QPainterPath &zone, zones) {
        const QRect &rect = zone.boundingRect().toRect();
        const int Column = ((Columns == 1) ? 0
            : (rect.width() > Span) ? Columns : rect.right() / Span);
        const int y = normalizedY(static_cast<int>(rect.y()), ToleranceY);
        zonesForColumn.insertMulti(Key(Column, y, rect.x()), zone);
    }
    return zonesForColumn.values();
}


void TextItems::debug(const int page, const int ToleranceY,
        const bool ComparingWords, const bool Yx)
{
    QString filename = QDir::tempPath() + QString("/page%1.txt").arg(page);
    int count = 0;
    while (QFile::exists(filename)) {
        filename = QDir::tempPath() + QString("/page%1-%2.txt").arg(page)
                                              .arg(++count);
    }
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        qDebug("%s", qPrintable(QString("cannot write debug file: %1")
                                .arg(file.errorString())));
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "Page #" << page << ": "
        << (ComparingWords ? "Words" : "Characters") << " mode\n";
    for (int i = 0; i < items.count(); ++i) {
        const TextItem &item = items.at(i);
        const QRect rect = item.toRect();
        out << item.text;
        if (!ComparingWords)
            out << QString(" %1").arg(item.text.at(0).unicode(), 4, 16,
                                      QChar('0'));
        if (Yx) {
            const int y = normalizedY(static_cast<int>(item.rect.y()),
                                      ToleranceY);
            out << QString(" (%1, %2)").arg(y).arg(rect.x());
        }
        out << "\n";
    }
    qDebug("%s", qPrintable(QString("wrote '%1'").arg(filename)));
}


inline int normalizedY(const int y, const int ToleranceY)
{
    if (ToleranceY == 0)
        return y;
    const int remainder = y % ToleranceY;
    return (remainder == 0) ? y : y + ToleranceY - remainder;
}


const TextItems getWords(const TextBoxList &list)
{
    TextItems items;
    foreach (const PdfTextBox &box, list) {
        QString word = box->text().trimmed();
        for (int i = 0; i < word.length(); ++i)
            word[i] = canonicalizedCharacter(word[i]);
        // DON'T DO: if (!word.isEmpty()) words << word;
        // since it can mess up highlighting.
        items.append(TextItem(word, box->boundingBox()));
    }
    return items;
}


const TextItems getCharacters(const TextBoxList &list)
{
    TextItems items;
    foreach (const PdfTextBox &box, list) {
        const QString word = box->text();
        int limit = word.count() - 1;
        for (int i = limit; i >= 0; --i)
            if (!word[i].isSpace())
                break;
        for (int i = 0; i <= limit; ++i) {
            items.append(TextItem(QString(canonicalizedCharacter(word[i])),
                                  box->charBoundingBox(i)));
        }
    }
    return items;
}
