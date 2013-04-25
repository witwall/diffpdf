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
#include "optionsform.hpp"
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>


OptionsForm::OptionsForm(QPen *pen, QBrush *brush, qreal *ruleWidth,
        bool *showToolTips,  bool *combineTextHighlighting,
        int *cacheSize, int *alpha, int *squareSize, QWidget *parent)
    : QDialog(parent), m_pen(pen), m_brush(brush), m_ruleWidth(ruleWidth),
      m_showToolTips(showToolTips),
      m_combineTextHighlighting(combineTextHighlighting),
      m_cacheSize(cacheSize), m_alpha(alpha), m_squareSize(squareSize)
{
    this->pen = *m_pen;
    this->brush = *m_brush;

    createWidgets();
    createLayout();
    createConnections();

    updateSwatches();
    updateUi();
    setWindowTitle(tr("DiffPDF — Options"));
}


void OptionsForm::createWidgets()
{
    colorComboBox = new QComboBox;
    foreach (const QString &name, QColor::colorNames()) {
        QColor color(name);
        colorComboBox->addItem(colorSwatch(color), name, color);
    }
    colorComboBox->setCurrentIndex(colorComboBox->findData(pen.color()));

    QColor color = pen.color();
    color.setAlphaF(*m_alpha / 100.0);

    brushStyleComboBox = new QComboBox;
    typedef QPair<QString, Qt::BrushStyle> BrushPair;
    foreach (const BrushPair &pair, QList<BrushPair>()
            << qMakePair(tr("No Brush"), Qt::NoBrush)
            << qMakePair(tr("Solid"), Qt::SolidPattern)
            << qMakePair(tr("Dense #1"), Qt::Dense1Pattern)
            << qMakePair(tr("Dense #2"), Qt::Dense2Pattern)
            << qMakePair(tr("Dense #3"), Qt::Dense3Pattern)
            << qMakePair(tr("Dense #4"), Qt::Dense4Pattern)
            << qMakePair(tr("Dense #5"), Qt::Dense5Pattern)
            << qMakePair(tr("Dense #6"), Qt::Dense6Pattern)
            << qMakePair(tr("Horizontal"), Qt::HorPattern)
            << qMakePair(tr("Vertical"), Qt::VerPattern)
            << qMakePair(tr("Cross"), Qt::CrossPattern)
            << qMakePair(tr("Diagonal /"), Qt::BDiagPattern)
            << qMakePair(tr("Diagonal \\"), Qt::FDiagPattern)
            << qMakePair(tr("Diagonal Cross"), Qt::DiagCrossPattern))
        brushStyleComboBox->addItem(brushSwatch(pair.second, color),
                                                pair.first, pair.second);
    brushStyleComboBox->setCurrentIndex(brushStyleComboBox->findData(
                brush.style()));

    penStyleComboBox = new QComboBox;
    typedef QPair<QString, Qt::PenStyle> PenPair;
    foreach (const PenPair &pair, QList<PenPair>()
            << qMakePair(tr("No Pen"), Qt::NoPen)
            << qMakePair(tr("Solid"), Qt::SolidLine)
            << qMakePair(tr("Dashed"), Qt::DashLine)
            << qMakePair(tr("Dotted"), Qt::DotLine)
            << qMakePair(tr("Dash-Dotted"), Qt::DashDotLine)
            << qMakePair(tr("Dash-Dot-Dotted"), Qt::DashDotDotLine))
        penStyleComboBox->addItem(penStyleSwatch(pair.second, color),
                                  pair.first, pair.second);
    penStyleComboBox->setCurrentIndex(penStyleComboBox->findData(
                pen.style()));

    alphaSpinBox = new QSpinBox;
    alphaSpinBox->setRange(1, 100);
    alphaSpinBox->setValue(*m_alpha);
    alphaSpinBox->setSuffix(tr(" %"));
    alphaSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    alphaSpinBox->setToolTip(tr("<p>How opaque the highlighting "
            "color is. The default is 13%"));

    squareSizeSpinBox = new QSpinBox;
    squareSizeSpinBox->setRange(2, 40);
    squareSizeSpinBox->setValue(*m_squareSize);
    squareSizeSpinBox->setSuffix(tr(" px"));
    squareSizeSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    squareSizeSpinBox->setToolTip(tr("<p>The size of the "
            "highlighting squares in Appearance mode. Small values are "
            "more expensive to compute. Large values give coarse "
            "comparisons. The default is 10 px"));

    ruleWidthSpinBox = new QDoubleSpinBox;
    ruleWidthSpinBox->setRange(0.0, 10.0);
    ruleWidthSpinBox->setDecimals(2);
    ruleWidthSpinBox->setSingleStep(0.25);
    ruleWidthSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    ruleWidthSpinBox->setSpecialValueText(tr("No Rules"));
    ruleWidthSpinBox->setValue(*m_ruleWidth);

    showToolTipsCheckBox = new QCheckBox(tr("Show &Tooltips in "
                                            "the Main Window"));
    showToolTipsCheckBox->setChecked(*m_showToolTips);

    combineTextHighlightingCheckBox = new QCheckBox(
            tr("Combine Highlighting in &Text Modes"));
    combineTextHighlightingCheckBox->setChecked(
            *m_combineTextHighlighting);

    cacheSizeSpinBox = new QSpinBox;
    cacheSizeSpinBox->setRange(1, 100);
    cacheSizeSpinBox->setValue(*m_cacheSize);
    cacheSizeSpinBox->setSuffix(tr(" MB"));
    cacheSizeSpinBox->setToolTip(tr("<p>The cache is used to store pages "
            "already seen to make flipping back and forth as fast as "
            "possible. The bigger the cache the more pages that can be "
            "stored."));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|
                                     QDialogButtonBox::Cancel);
    tabWidget = new QTabWidget;
}


void OptionsForm::createLayout()
{
    QFormLayout *generalLayout = new QFormLayout;
    generalLayout->addRow(showToolTipsCheckBox);
    generalLayout->addRow(tr("&Rule width:"), ruleWidthSpinBox);
    QWidget *widget = new QWidget;
    widget->setLayout(generalLayout);
    tabWidget->addTab(widget, tr("&General"));

    QFormLayout *highlightingLayout = new QFormLayout;
    highlightingLayout->addRow(tr("&Base Color:"), colorComboBox);
    highlightingLayout->addRow(tr("O&utline:"), penStyleComboBox);
    highlightingLayout->addRow(tr("&Fill:"), brushStyleComboBox);
    highlightingLayout->addRow(tr("F&ill Opacity:"), alphaSpinBox);
    highlightingLayout->addRow(combineTextHighlightingCheckBox);
    widget = new QWidget;
    widget->setToolTip(tr("<p>The outline and fill are used to highlight "
            "differences using a semi-transparent version of the base "
            "color. The margin rules are painted using the base color  "
            "and indicate where changes are. Set the rule width to 0.0 "
            "to switch the rules off. If combining highlighting is "
            "checked it will try to merge the highlighting of adjacent "
            "text differences."));
    widget->setLayout(highlightingLayout);
    tabWidget->addTab(widget, tr("&Highlighting"));

    QFormLayout *performanceLayout = new QFormLayout;
    performanceLayout->addRow(tr("&Square Size:"), squareSizeSpinBox);
    performanceLayout->addRow(tr("C&ache Size:"), cacheSizeSpinBox);
    widget = new QWidget;
    widget->setLayout(performanceLayout);
    tabWidget->addTab(widget, tr("&Performance"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);
}


void OptionsForm::createConnections()
{
    connect(colorComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateColor(int)));
    connect(penStyleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updatePenStyle(int)));
    connect(penStyleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateUi()));
    connect(brushStyleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateBrushStyle(int)));
    connect(brushStyleComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateUi()));
    connect(alphaSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateSwatches()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


void OptionsForm::updateColor(int index)
{
    QColor color = colorComboBox->itemData(index).value<QColor>();
    brush.setColor(color);
    pen.setColor(color);
    updateSwatches();
}


void OptionsForm::updatePenStyle(int index)
{
    pen.setStyle(static_cast<Qt::PenStyle>(
                 penStyleComboBox->itemData(index).toInt()));
}


void OptionsForm::updateBrushStyle(int index)
{
    brush.setStyle(static_cast<Qt::BrushStyle>(
                   brushStyleComboBox->itemData(index).toInt()));
}


void OptionsForm::updateSwatches()
{
    QColor color = colorComboBox->itemData(
            colorComboBox->currentIndex()).value<QColor>();
    color.setAlphaF(alphaSpinBox->value() / 100.0);
    for (int i = 0; i < brushStyleComboBox->count(); ++i)
        brushStyleComboBox->setItemIcon(i, brushSwatch(
                static_cast<Qt::BrushStyle>(
                    brushStyleComboBox->itemData(i).toInt()), color));
    for (int i = 0; i < penStyleComboBox->count(); ++i)
        penStyleComboBox->setItemIcon(i, penStyleSwatch(
                static_cast<Qt::PenStyle>(
                    penStyleComboBox->itemData(i).toInt()), color));
}


void OptionsForm::updateUi()
{
    Qt::BrushStyle brushStyle = static_cast<Qt::BrushStyle>(
            brushStyleComboBox->itemData(
                    brushStyleComboBox->currentIndex()).toInt());
    Qt::PenStyle penStyle = static_cast<Qt::PenStyle>(
            penStyleComboBox->itemData(
                    penStyleComboBox->currentIndex()).toInt());
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
            !(brushStyle == Qt::NoBrush && penStyle == Qt::NoPen));
}


void OptionsForm::accept()
{
    *m_pen = pen;
    *m_brush = brush;
    *m_ruleWidth = ruleWidthSpinBox->value();
    *m_showToolTips = showToolTipsCheckBox->isChecked();
    *m_combineTextHighlighting =
            combineTextHighlightingCheckBox->isChecked();
    *m_cacheSize = cacheSizeSpinBox->value();
    *m_alpha = alphaSpinBox->value();
    *m_squareSize = squareSizeSpinBox->value();
    QDialog::accept();
}
