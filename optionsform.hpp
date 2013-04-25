#ifndef OPTIONSFORM_HPP
#define OPTIONSFORM_HPP
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

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QSpinBox;
class QTabWidget;


class OptionsForm : public QDialog
{
    Q_OBJECT

public:
    OptionsForm(QPen *pen, QBrush *brush, qreal *ruleWidth,
            bool *showToolTips, bool *combineTextHighlighting,
            int *cacheSize, int *alpha, int *squareSize,
            QWidget *parent=0);

private slots:
    void updateColor(int index);
    void updateBrushStyle(int index);
    void updatePenStyle(int index);
    void updateSwatches();
    void updateUi();
    void accept();

private:
    void createWidgets();
    void createLayout();
    void createConnections();

    QTabWidget *tabWidget;
    QComboBox *colorComboBox;
    QComboBox *brushStyleComboBox;
    QComboBox *penStyleComboBox;
    QSpinBox *alphaSpinBox;
    QSpinBox *squareSizeSpinBox;
    QDoubleSpinBox *ruleWidthSpinBox;
    QCheckBox *showToolTipsCheckBox;
    QCheckBox *combineTextHighlightingCheckBox;
    QSpinBox *cacheSizeSpinBox;
    QDialogButtonBox *buttonBox;

    QPen *m_pen;
    QBrush *m_brush;
    qreal *m_ruleWidth;
    bool *m_showToolTips;
    bool *m_combineTextHighlighting;
    int *m_cacheSize;
    int *m_alpha;
    int *m_squareSize;
    QPen pen;
    QBrush brush;
};

#endif // OPTIONSFORM_HPP


