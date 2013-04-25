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
#include <QMimeData>
#include <QRectF>
#include <QPainter>
#include <QPen>
#include <QPixmapCache>
#include <QUrl>

const QSize SwatchSize(24, 24);


void scaleRect(int dpi, QRectF *rect)
{
    if (dpi == POINTS_PER_INCH)
        return;
    qreal x1;
    qreal y1;
    qreal x2;
    qreal y2;
    rect->getCoords(&x1, &y1, &x2, &y2);
    qreal factor = dpi / static_cast<qreal>(POINTS_PER_INCH);
    x1 *= factor;
    y1 *= factor;
    x2 *= factor;
    y2 *= factor;
    rect->setCoords(x1, y1, x2, y2);
}


int pointValueForPixelOffset(const double dpi, int px)
{
    const double inches = px / dpi;
    return qRound(inches * POINTS_PER_INCH);
}


int pixelOffsetForPointValue(const double dpi, int pt)
{
    const double inches = pt / static_cast<double>(POINTS_PER_INCH);
    return qRound(inches * dpi);
}


QRectF rectForMargins(const int width, const int height, const int top,
        const int bottom, const int left, const int right)
{
    return QRectF(QPointF(left, top),
                  QPointF(width - right, height - bottom));
}


Ranges unorderedRange(int end, int start)
{
    Q_ASSERT(end >= start);
    Ranges ranges;
    while (start < end)
        ranges.insert(start++);
    return ranges;
}


QPixmap colorSwatch(const QColor &color)
{
    QString key = QString("COLORSWATCH:%1").arg(color.name());
    QPixmap pixmap(SwatchSize);
#if QT_VERSION >= 0x040600
    if (!QPixmapCache::find(key, &pixmap)) {
#else
    if (!QPixmapCache::find(key, pixmap)) {
#endif
        pixmap.fill(Qt::transparent);
        {
            QPainter painter(&pixmap);
            painter.setRenderHints(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            painter.drawEllipse(0, 0, SwatchSize.width(),
                                SwatchSize.height());
        }
        QPixmapCache::insert(key, pixmap);
    }
    return pixmap;
}


QPixmap brushSwatch(const Qt::BrushStyle style, const QColor &color)
{
    QString key = QString("BRUSHSTYLESWATCH:%1:%2:%3")
        .arg(static_cast<int>(style)).arg(color.name()).arg(color.alpha());
    QPixmap pixmap(SwatchSize);
#if QT_VERSION >= 0x040600
    if (!QPixmapCache::find(key, &pixmap)) {
#else
    if (!QPixmapCache::find(key, pixmap)) {
#endif
        pixmap.fill(Qt::transparent);
        {
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(color, style));
            painter.drawRect(0, 0, SwatchSize.width(),
                             SwatchSize.height());
        }
        QPixmapCache::insert(key, pixmap);
    }
    return pixmap;
}


QPixmap penStyleSwatch(const Qt::PenStyle style, const QColor &color)
{
    QString key = QString("PENSTYLESWATCH:%1:%2")
        .arg(static_cast<int>(style)).arg(color.name());
    QPixmap pixmap(SwatchSize);
#if QT_VERSION >= 0x040600
    if (!QPixmapCache::find(key, &pixmap)) {
#else
    if (!QPixmapCache::find(key, pixmap)) {
#endif
        pixmap.fill(Qt::transparent);
        {
            QPainter painter(&pixmap);
            QPen pen(style);
            pen.setColor(color);
            pen.setWidth(2);
            painter.setPen(pen);
            const int Y = SwatchSize.height() / 2;
            painter.drawLine(0, Y, SwatchSize.width(), Y);
        }
        QPixmapCache::insert(key, pixmap);
    }
    return pixmap;
}


const TextBoxList getTextBoxes(PdfPage page, const QRectF &rect)
{
    TextBoxList boxes;
    foreach (Poppler::TextBox *box, page->textList()) {
        PdfTextBox box_ptr(box);
        if (rect.isEmpty() || rect.contains(box_ptr->boundingBox()))
            boxes.append(box_ptr);
    }
    return boxes;
}


const QString strippedFilename(const QString &filename)
{
    const QString FilePrefix("file://");
    QString filename_ = filename;
    if (filename_.startsWith(FilePrefix))
        filename_ = filename_.mid(FilePrefix.length());
#ifdef Q_WS_WIN
    if (filename_.startsWith("/"))
        filename_ = filename_.mid(1);
#endif
    return filename_.trimmed();
}


const QStringList droppedFilenames(const QMimeData *mimeData)
{
    QStringList filenames;
    QString text = mimeData->text();
    if (!text.isEmpty()) {
        filenames = text.split("\n");
        for (int i = 0; i < filenames.count(); ++i)
            filenames[i] = strippedFilename(filenames.at(i));
    }
    else {
        foreach (const QUrl &url, mimeData->urls())
            filenames << strippedFilename(url.toString());
    }
    return filenames;
}

// Returns a copy of pageRect reduced if necessary to have the same
// aspect ratio as pixmapSize.
const QRect resizeRect(const QRect &pageRect, const QSize &pixmapSize)
{
    double ratio = pixmapSize.width() /
            static_cast<double>(pixmapSize.height());
    double height = pageRect.height();
    double width = ratio * height;
    if (width > pageRect.width()) {
        width = pageRect.width();
        height = width / ratio;
    }
    QRect rect(pageRect);
    rect.setWidth(width);
    rect.setHeight(height);
    return rect;
}
