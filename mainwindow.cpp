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
#include "aboutform.hpp"
#include "generic.hpp"
#include "helpform.hpp"
#include "label.hpp"
#include "lineedit.hpp"
#include "optionsform.hpp"
#include "mainwindow.hpp"
#include "sequence_matcher.hpp"
#include "textitem.hpp"
#ifdef DEBUG
#include <QtDebug>
#endif
#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QEvent>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPixmapCache>
#include <QPlainTextEdit>
#include <QPrinter>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>


MainWindow::MainWindow(const Debug debug,
        const InitialComparisonMode comparisonMode,
        const QString &filename1, const QString &filename2,
        const QString &language, QWidget *parent)
    : QMainWindow(parent),
      controlDockArea(Qt::RightDockWidgetArea),
      actionDockArea(Qt::RightDockWidgetArea),
      marginsDockArea(Qt::RightDockWidgetArea),
      zoningDockArea(Qt::RightDockWidgetArea),
      logDockArea(Qt::RightDockWidgetArea), cancel(false),
      saveAll(true), savePages(SaveBothPages), language(language),
      debug(debug), aboutForm(0), helpForm(0)
{
    currentPath = QDir::homePath();
    QSettings settings;
    pen.setStyle(Qt::NoPen);
    pen.setColor(Qt::red);
    pen = settings.value("Outline", pen).value<QPen>();
    brush.setColor(pen.color());
    brush.setStyle(Qt::SolidPattern);
    brush = settings.value("Fill", brush).value<QBrush>();
    showToolTips = settings.value("ShowToolTips", true).toBool();
    combineTextHighlighting = settings.value("CombineTextHighlighting",
            true).toBool();
    QPixmapCache::setCacheLimit(1000 *
            qBound(1, settings.value("CacheSizeMB", 25).toInt(), 100));

    createWidgets(filename1, filename2);
    createCentralArea();
    createDockWidgets();
    createConnections();

    restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
    restoreState(settings.value("MainWindow/State").toByteArray());
    controlDockLocationChanged(static_cast<Qt::DockWidgetArea>(
                settings.value("MainWindow/ControlDockArea",
                        static_cast<int>(controlDockArea)).toInt()));
    actionDockLocationChanged(static_cast<Qt::DockWidgetArea>(
                settings.value("MainWindow/ActionDockArea",
                        static_cast<int>(actionDockArea)).toInt()));
    zoningDockLocationChanged(static_cast<Qt::DockWidgetArea>(
                settings.value("MainWindow/ZoningDockArea",
                        static_cast<int>(zoningDockArea)).toInt()));
    marginsDockLocationChanged(static_cast<Qt::DockWidgetArea>(
                settings.value("MainWindow/MarginsDockArea",
                        static_cast<int>(marginsDockArea)).toInt()));
    controlDockWidget->resize(controlDockWidget->minimumSizeHint());
    actionDockWidget->resize(actionDockWidget->minimumSizeHint());
    zoningDockWidget->resize(zoningDockWidget->minimumSizeHint());
    marginsDockWidget->resize(marginsDockWidget->minimumSizeHint());
    //logDockWidget->resize(logDockWidget->minimumSizeHint());

    setWindowTitle(tr("DiffPDF"));
    setWindowIcon(QIcon(":/icon.png"));
    compareComboBox->setCurrentIndex(comparisonMode);
    QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection,
            Q_ARG(QString, filename1),
            Q_ARG(QString, filename2));
}


void MainWindow::createWidgets(const QString &filename1,
                               const QString &filename2)
{
    setFile1Button = new QPushButton(tr("File #&1..."));
    setFile1Button->setToolTip(tr("<p>Choose the first (left hand) file "
                "to be compared."));
    filename1LineEdit = new LineEdit;
    filename1LineEdit->setToolTip(tr("The first (left hand) file."));
    filename1LineEdit->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    filename1LineEdit->setMinimumWidth(100);
    filename1LineEdit->setText(filename1);
    setFile2Button = new QPushButton(tr("File #&2..."));
    setFile2Button->setToolTip(tr("<p>Choose the second (right hand) file "
                "to be compared."));
    filename2LineEdit = new LineEdit;
    filename2LineEdit->setToolTip(tr("The second (right hand) file."));
    filename2LineEdit->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    filename2LineEdit->setMinimumWidth(100);
    filename2LineEdit->setText(filename2);
    comparePages1Label = new QLabel(tr("Pa&ges:"));
    pages1LineEdit = new QLineEdit;
    comparePages1Label->setBuddy(pages1LineEdit);
    pages1LineEdit->setToolTip(tr("<p>Pages can be specified using ranges "
                "such as 1-10, and multiple ranges can be used, e.g., "
                "1-10, 12-15, 20, 22, 35-39. This makes it "
                "straighforward to compare similar documents where one "
                "has one or more additional pages.<p>For example, if "
                "file1.pdf has pages 1-30 and file2.pdf has pages 1-31 "
                "with the extra page being page 14, the two page ranges "
                "would be set to 1-30 for file1.pdf and 1-13, 15-31 for "
                "file2.pdf."));
    comparePages2Label = new QLabel(tr("&Pages:"));
    pages2LineEdit = new QLineEdit;
    comparePages2Label->setBuddy(pages2LineEdit);
    pages2LineEdit->setToolTip(pages1LineEdit->toolTip());
    compareButton = new QPushButton(tr("&Compare"));
    compareButton->setEnabled(false);
    compareButton->setToolTip(tr("<p>Click to compare (or re-compare) "
                "the documents&mdash;or to cancel a comparison that's "
                "in progress."));
    compareComboBox = new QComboBox;
    compareComboBox->addItems(QStringList() << tr("Appearance")
            << tr("Characters") << tr("Words"));
    compareComboBox->setToolTip(
            tr("<p>If the <b>Words</b> comparison "
               "mode is chosen, then each page's text is compared "
               "word by word (best for alphabetic languages like "
               "English). "
               "If the <b>Characters</b> comparison mode is chosen, "
               "then each page's text is compared character by "
               "character (best for logographic languages like Chinese "
               "and Japanese). "
               "If the <b>Appearance</b> comparison mode is chosen "
               "then each page's visual appearance is compared. "
               "Comparing appearance can be slow for large documents "
               "and can also produce false positives&mdash;but is "
               "absolutely precise."));
    compareLabel = new QLabel(tr("Co&mpare:"));
    compareLabel->setBuddy(compareComboBox);
    viewDiffLabel = new QLabel(tr("&View:"));
    viewDiffLabel->setToolTip(tr("<p>Shows each pair of pages which "
                "are different. The comparison is textual unless the "
                "<b>Appearance</b> comparison mode is chosen, in "
                "which case the comparison is done visually. "
                "Visual differences can occur if a paragraph is "
                "formated differently or if an embedded diagram or "
                "image has changed."));
    viewDiffComboBox = new QComboBox;
    viewDiffComboBox->addItem(tr("(Not viewing)"));
    viewDiffLabel->setBuddy(viewDiffComboBox);
    viewDiffComboBox->setToolTip(viewDiffLabel->toolTip());
    showLabel = new QLabel(tr("S&how:"));
    showLabel->setToolTip(tr("<p>In show <b>Highlighting</b> mode the "
                "pages are shown side by side with their differences "
                "highlighted. All the other modes are composition "
                "modes which show the first PDF as-is and the "
                "composition (blend) of the two PDFs."));
    showComboBox = new QComboBox;
    showComboBox->addItem(tr("%1Highlighting%2")
            .arg(QChar(0xAB)).arg(QChar(0xBB)), -1);
    showComboBox->addItem(tr("Not Src Xor Dest"),
            QPainter::RasterOp_NotSourceXorDestination);
    showComboBox->addItem(tr("Difference"),
            QPainter::CompositionMode_Difference);
    showComboBox->addItem(tr("Exclusion"),
            QPainter::CompositionMode_Exclusion);
    showComboBox->addItem(tr("Src Xor Dest"),
            QPainter::RasterOp_SourceXorDestination);
    showLabel->setBuddy(showComboBox);
    showComboBox->setToolTip(showLabel->toolTip());
    previousButton = new QPushButton(tr("Previo&us"));
    previousButton->setToolTip(
            "<p>Navigate to the previous pair of pages.");
#if QT_VERSION >= 0x040600
    previousButton->setIcon(QIcon(":/left.png"));
#endif
    nextButton = new QPushButton(tr("Ne&xt"));
    nextButton->setToolTip("<p>Navigate to the next pair of pages.");
#if QT_VERSION >= 0x040600
    nextButton->setIcon(QIcon(":/right.png"));
#endif
    zoomLabel = new QLabel(tr("&Zoom:"));
    zoomLabel->setToolTip(tr("<p>Determines the scale at which the "
                "pages are shown."));
    zoomSpinBox = new QSpinBox;
    zoomLabel->setBuddy(zoomSpinBox);
    zoomSpinBox->setRange(20, 800);
    zoomSpinBox->setSuffix(tr(" %"));
    zoomSpinBox->setSingleStep(5);
    QSettings settings;
    zoomSpinBox->setValue(settings.value("Zoom", 100).toInt());
    zoomSpinBox->setToolTip(zoomLabel->toolTip());
    zoningGroupBox = new QGroupBox(tr("Zo&ning"));
    zoningGroupBox->setToolTip(tr("<p>Zoning is a computationally "
                "expensive experimental mode that can reduce or "
                "eliminate false positives particularly for pages "
                "that have tables or that mix alphabetic and "
                "logographic languages&mdash;it can also increase "
                "false positives! Zoning only applies to text "
                "comparisons."));
    zoningGroupBox->setCheckable(true);
    zoningGroupBox->setChecked(false);
    columnsLabel = new QLabel(tr("Co&lumns:"));
    columnsSpinBox = new QSpinBox;
    columnsSpinBox->setRange(1, 16);
    columnsSpinBox->setValue(settings.value("Columns", 1).toInt());
    columnsSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    columnsSpinBox->setToolTip(tr("<p>Use this to tell DiffPDF how "
                "many columns the page has; this should improve the "
                "zoning."));
    columnsLabel->setBuddy(columnsSpinBox);
    toleranceRLabel = new QLabel(tr("Tolerance/&R:"));
    toleranceRSpinBox = new QSpinBox;
    toleranceRSpinBox->setRange(4, 144);
    toleranceRSpinBox->setValue(settings.value("Tolerance/R", 8).toInt());
    toleranceRSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    toleranceRSpinBox->setToolTip(tr("<p>This is the maximum distance  "
                "between text (word) rectangles for the rectangles to "
                "appear in the same zone."));
    toleranceRLabel->setBuddy(toleranceRSpinBox);
    toleranceYLabel = new QLabel(tr("Tolerance/&Y:"));
    toleranceYSpinBox = new QSpinBox;
    toleranceYSpinBox->setRange(0, 32);
    toleranceYSpinBox->setValue(settings.value("Tolerance/Y", 10).toInt());
    toleranceYSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    toleranceYSpinBox->setToolTip(tr("<p>Text position <i>y</i> "
                "coordinates are rounded to the nearest Tolerance/Y "
                "value when zoning."));
    toleranceYLabel->setBuddy(toleranceYSpinBox);
    showZonesCheckBox = new QCheckBox(tr("Sho&w Zones"));
    showZonesCheckBox->setToolTip(tr("<p>This shows the zones that are "
                "being used and may be helpful when adjusting "
                "tolerances. (Its original purpose was for debugging.)"));
    marginsGroupBox = new QGroupBox(tr("&Exclude Margins"));
    marginsGroupBox->setToolTip(tr("<p>If this is checked, anything "
                "outside non-zero margins is ignored when comparing."));
    marginsGroupBox->setCheckable(true);
    marginsGroupBox->setChecked(settings.value("Margins/Exclude",
                false).toBool());
    topMarginLabel = new QLabel(tr("&Top:"));
    topMarginSpinBox = new QSpinBox;
    topMarginSpinBox->setSuffix(" pt");
    topMarginSpinBox->setValue(settings.value("Margins/Top", 0).toInt());
    topMarginSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    topMarginSpinBox->setToolTip(tr("<p>The top margin in points. "
                "Anything above this will be ignored."));
    topMarginLabel->setBuddy(topMarginSpinBox);
    bottomMarginLabel = new QLabel(tr("&Bottom:"));
    bottomMarginSpinBox = new QSpinBox;
    bottomMarginSpinBox->setSuffix(" pt");
    bottomMarginSpinBox->setValue(settings.value("Margins/Bottom", 0)
            .toInt());
    bottomMarginSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    bottomMarginSpinBox->setToolTip(tr("<p>The bottom margin in points. "
                "Anything below this will be ignored."));
    bottomMarginLabel->setBuddy(bottomMarginSpinBox);
    leftMarginLabel = new QLabel(tr("Le&ft:"));
    leftMarginSpinBox = new QSpinBox;
    leftMarginSpinBox->setSuffix(" pt");
    leftMarginSpinBox->setValue(settings.value("Margins/Left", 0).toInt());
    leftMarginSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    leftMarginSpinBox->setToolTip(tr("<p>The left margin in points. "
                "Anything left of this will be ignored."));
    leftMarginLabel->setBuddy(leftMarginSpinBox);
    rightMarginLabel = new QLabel(tr("R&ight:"));
    rightMarginSpinBox = new QSpinBox;
    rightMarginSpinBox->setSuffix(" pt");
    rightMarginSpinBox->setValue(settings.value("Margins/Right", 0)
            .toInt());
    rightMarginSpinBox->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
    rightMarginSpinBox->setToolTip(tr("<p>The right margin in points. "
                "Anything right of this will be ignored."));
    rightMarginLabel->setBuddy(rightMarginSpinBox);
    statusLabel = new QLabel(tr("Choose files..."));
    statusLabel->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
    statusLabel->setMaximumHeight(statusLabel->minimumSizeHint().height());
    optionsButton = new QPushButton(tr("&Options..."));
    optionsButton->setToolTip(tr("Click to customize the application."));
    saveButton = new QPushButton(tr("&Save As..."));
    saveButton->setToolTip(tr("Save the differences."));
    helpButton = new QPushButton(tr("Help"));
    helpButton->setShortcut(tr("F1"));
    helpButton->setToolTip(tr("Click for basic help."));
    aboutButton = new QPushButton(tr("&About"));
    aboutButton->setToolTip(tr("Click for copyright and credits."));
    quitButton = new QPushButton(tr("&Quit"));
    quitButton->setToolTip(tr("Click to terminate the application."));
    page1Label = new Label;
    page1Label->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    page1Label->setToolTip(tr("<p>Shows the first (left hand) document's "
                "page that corresponds to the page shown in the "
                "View Difference combobox."));
    page2Label = new Label;
    page2Label->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    page2Label->setToolTip(tr("<p>Shows the second (right hand) "
                "document's page that corresponds to the page shown in "
                "the View Difference combobox."));
    logEdit = new QPlainTextEdit;

    QList<QWidget*> widgets;
    widgets << setFile1Button << filename1LineEdit << pages1LineEdit
            << page1Label << setFile2Button << filename2LineEdit
            << pages2LineEdit << page2Label << compareButton
            << compareComboBox << viewDiffLabel << viewDiffComboBox
            << showLabel << showComboBox << zoomLabel << zoomSpinBox
            << optionsButton << zoningGroupBox << columnsLabel
            << columnsSpinBox << toleranceRLabel << toleranceRSpinBox
            << toleranceYLabel << toleranceYSpinBox << marginsGroupBox
            << topMarginLabel << topMarginSpinBox << bottomMarginSpinBox
            << bottomMarginLabel << leftMarginLabel << leftMarginSpinBox
            << rightMarginLabel << rightMarginSpinBox << saveButton
            << helpButton << aboutButton << quitButton << logEdit
            << previousButton << nextButton << showZonesCheckBox;
    foreach (QWidget *widget, widgets)
        if (!widget->toolTip().isEmpty())
            widget->installEventFilter(this);
}


void MainWindow::createCentralArea()
{
    QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(setFile1Button);
    topLeftLayout->addWidget(filename1LineEdit, 3);
    topLeftLayout->addWidget(comparePages1Label);
    topLeftLayout->addWidget(pages1LineEdit, 2);
    area1 = new QScrollArea;
    area1->setWidget(page1Label);
    area1->setWidgetResizable(true);
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(topLeftLayout);
    leftLayout->addWidget(area1, 1);
    QWidget *leftWidget = new QWidget;
    leftWidget->setLayout(leftLayout);

    QHBoxLayout *topRightLayout = new QHBoxLayout;
    topRightLayout->addWidget(setFile2Button);
    topRightLayout->addWidget(filename2LineEdit, 3);
    topRightLayout->addWidget(comparePages2Label);
    topRightLayout->addWidget(pages2LineEdit, 2);
    area2 = new QScrollArea;
    area2->setWidget(page2Label);
    area2->setWidgetResizable(true);
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addLayout(topRightLayout);
    rightLayout->addWidget(area2, 1);
    QWidget *rightWidget = new QWidget;
    rightWidget->setLayout(rightLayout);

    splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    QSettings settings;
    splitter->restoreState(settings.value("MainWindow/ViewSplitter")
            .toByteArray());

    setCentralWidget(splitter);
}


void MainWindow::createDockWidgets()
{
    setTabPosition(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea,
            QTabWidget::North);
    setTabPosition(Qt::TopDockWidgetArea|Qt::BottomDockWidgetArea,
            QTabWidget::West);
    QDockWidget::DockWidgetFeatures features =
            QDockWidget::DockWidgetMovable|
            QDockWidget::DockWidgetFloatable;

    controlDockWidget = new QDockWidget(tr("Controls"), this);
    controlDockWidget->setObjectName("Controls");
    controlDockWidget->setFeatures(features);
    controlLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    compareLayout = new QHBoxLayout;
    compareLayout->addWidget(compareLabel);
    compareLayout->addWidget(compareComboBox, 1);
    controlLayout->addLayout(compareLayout);
    QHBoxLayout *viewLayout = new QHBoxLayout;
    viewLayout->addWidget(viewDiffLabel);
    viewLayout->addWidget(viewDiffComboBox, 1);
    controlLayout->addLayout(viewLayout);
    QHBoxLayout *showLayout = new QHBoxLayout;
    showLayout->addWidget(showLabel);
    showLayout->addWidget(showComboBox, 1);
    controlLayout->addLayout(showLayout);
    QHBoxLayout *navigationLayout = new QHBoxLayout;
    navigationLayout->addWidget(previousButton);
    navigationLayout->addWidget(nextButton);
    controlLayout->addLayout(navigationLayout);
    QHBoxLayout *zoomLayout = new QHBoxLayout;
    zoomLayout->addWidget(zoomLabel);
    zoomLayout->addWidget(zoomSpinBox);
    controlLayout->addLayout(zoomLayout);
    controlLayout->addWidget(statusLabel);
    controlLayout->addStretch();
    QWidget *widget = new QWidget;
    widget->setLayout(controlLayout);
    controlDockWidget->setWidget(widget);
    addDockWidget(controlDockArea, controlDockWidget);

    actionDockWidget = new QDockWidget(tr("Actions"), this);
    actionDockWidget->setObjectName("Actions");
    actionDockWidget->setFeatures(features);
    actionLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    actionLayout->addWidget(compareButton);
    QHBoxLayout *optionLayout = new QHBoxLayout;
    optionLayout->addWidget(optionsButton);
    optionLayout->addWidget(saveButton);
    actionLayout->addLayout(optionLayout);
    QHBoxLayout *helpLayout = new QHBoxLayout;
    helpLayout->addWidget(helpButton);
    helpLayout->addWidget(aboutButton);
    actionLayout->addLayout(helpLayout);
    actionLayout->addWidget(quitButton);
    actionLayout->addStretch();
    widget = new QWidget;
    widget->setLayout(actionLayout);
    actionDockWidget->setWidget(widget);
    addDockWidget(actionDockArea, actionDockWidget);

    marginsDockWidget = new QDockWidget(tr("Margins"), this);
    marginsDockWidget->setObjectName("Margins");
    marginsDockWidget->setFeatures(features|
                                  QDockWidget::DockWidgetClosable);
    marginsLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QHBoxLayout *topMarginLayout = new QHBoxLayout;
    topMarginLayout->addWidget(topMarginLabel);
    topMarginLayout->addWidget(topMarginSpinBox);
    marginsLayout->addLayout(topMarginLayout);
    QHBoxLayout *bottomMarginLayout = new QHBoxLayout;
    bottomMarginLayout->addWidget(bottomMarginLabel);
    bottomMarginLayout->addWidget(bottomMarginSpinBox);
    marginsLayout->addLayout(bottomMarginLayout);
    QHBoxLayout *leftMarginLayout = new QHBoxLayout;
    leftMarginLayout->addWidget(leftMarginLabel);
    leftMarginLayout->addWidget(leftMarginSpinBox);
    marginsLayout->addLayout(leftMarginLayout);
    QHBoxLayout *rightMarginLayout = new QHBoxLayout;
    rightMarginLayout->addWidget(rightMarginLabel);
    rightMarginLayout->addWidget(rightMarginSpinBox);
    marginsLayout->addLayout(rightMarginLayout);
    marginsLayout->addStretch();
    marginsGroupBox->setLayout(marginsLayout);
    marginsDockWidget->setWidget(marginsGroupBox);
    addDockWidget(marginsDockArea, marginsDockWidget);

    zoningDockWidget = new QDockWidget(tr("Zoning"), this);
    zoningDockWidget->setObjectName("Zoning");
    zoningDockWidget->setFeatures(features|
                                  QDockWidget::DockWidgetClosable);
    zoningLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QHBoxLayout *columnLayout = new QHBoxLayout;
    columnLayout->addWidget(columnsLabel);
    columnLayout->addWidget(columnsSpinBox);
    zoningLayout->addLayout(columnLayout);
    QHBoxLayout *toleranceLayout1 = new QHBoxLayout;
    toleranceLayout1->addWidget(toleranceRLabel);
    toleranceLayout1->addWidget(toleranceRSpinBox);
    zoningLayout->addLayout(toleranceLayout1);
    QHBoxLayout *toleranceLayout2 = new QHBoxLayout;
    toleranceLayout2->addWidget(toleranceYLabel);
    toleranceLayout2->addWidget(toleranceYSpinBox);
    zoningLayout->addLayout(toleranceLayout2);
    zoningLayout->addWidget(showZonesCheckBox);
    zoningLayout->addStretch();
    zoningGroupBox->setLayout(zoningLayout);
    zoningDockWidget->setWidget(zoningGroupBox);
    addDockWidget(zoningDockArea, zoningDockWidget);

    logDockWidget = new QDockWidget(tr("Log"), this);
    logDockWidget->setObjectName("Log");
    logDockWidget->setFeatures(features|QDockWidget::DockWidgetClosable);
    logDockWidget->setWidget(logEdit);
    addDockWidget(Qt::RightDockWidgetArea, logDockWidget);

    tabifyDockWidget(marginsDockWidget, controlDockWidget);
    tabifyDockWidget(logDockWidget, zoningDockWidget);
    tabifyDockWidget(zoningDockWidget, actionDockWidget);
}


void MainWindow::createConnections()
{
    connect(area1->verticalScrollBar(), SIGNAL(valueChanged(int)),
            area2->verticalScrollBar(), SLOT(setValue(int)));
    connect(area2->verticalScrollBar(), SIGNAL(valueChanged(int)),
            area1->verticalScrollBar(), SLOT(setValue(int)));
    connect(area1->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            area2->horizontalScrollBar(), SLOT(setValue(int)));
    connect(area2->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            area1->horizontalScrollBar(), SLOT(setValue(int)));

    connect(filename1LineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(updateUi()));
    connect(filename1LineEdit,
            SIGNAL(filenamesDropped(const QStringList&)),
            this, SLOT(setFiles1(const QStringList&)));
    connect(filename2LineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(updateUi()));
    connect(filename2LineEdit,
            SIGNAL(filenamesDropped(const QStringList&)),
            this, SLOT(setFiles2(const QStringList&)));

    connect(page1Label, SIGNAL(filenamesDropped(const QStringList&)),
            this, SLOT(setFiles1(const QStringList&)));
    connect(page2Label, SIGNAL(filenamesDropped(const QStringList&)),
            this, SLOT(setFiles2(const QStringList&)));

    connect(compareComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateUi()));
    connect(compareComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateViews()));

    connect(viewDiffComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateViews(int)));
    connect(viewDiffComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateUi()));
    connect(showComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateViews()));
    connect(previousButton, SIGNAL(clicked()),
            this, SLOT(previousPages()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextPages()));
    connect(setFile1Button, SIGNAL(clicked()), this, SLOT(setFile1()));
    connect(setFile2Button, SIGNAL(clicked()), this, SLOT(setFile2()));
    connect(compareButton, SIGNAL(clicked()), this, SLOT(compare()));
    connect(zoomSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(zoningGroupBox, SIGNAL(toggled(bool)),
            this, SLOT(updateUi()));
    connect(zoningGroupBox, SIGNAL(toggled(bool)),
            this, SLOT(updateViews()));
    connect(columnsSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(toleranceRSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(toleranceYSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(showZonesCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(updateViews()));
    connect(marginsGroupBox, SIGNAL(toggled(bool)),
            this, SLOT(updateUi()));
    connect(marginsGroupBox, SIGNAL(toggled(bool)),
            this, SLOT(updateViews()));
    connect(leftMarginSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(rightMarginSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(topMarginSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(bottomMarginSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(updateViews()));
    connect(page1Label, SIGNAL(clicked(const QPoint&)),
            this, SLOT(setAMargin(const QPoint&)));
    connect(page2Label, SIGNAL(clicked(const QPoint&)),
            this, SLOT(setAMargin(const QPoint&)));

    connect(optionsButton, SIGNAL(clicked()), this, SLOT(options()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
    connect(helpButton, SIGNAL(clicked()), this, SLOT(help()));
    connect(aboutButton, SIGNAL(clicked()), this, SLOT(about()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(controlDockWidget,
            SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(controlDockLocationChanged(Qt::DockWidgetArea)));
    connect(actionDockWidget,
            SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(actionDockLocationChanged(Qt::DockWidgetArea)));
    connect(zoningDockWidget,
            SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(zoningDockLocationChanged(Qt::DockWidgetArea)));
    connect(marginsDockWidget,
            SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(marginsDockLocationChanged(Qt::DockWidgetArea)));
    connect(controlDockWidget, SIGNAL(topLevelChanged(bool)),
            this, SLOT(controlTopLevelChanged(bool)));
    connect(actionDockWidget, SIGNAL(topLevelChanged(bool)),
            this, SLOT(actionTopLevelChanged(bool)));
    connect(zoningDockWidget, SIGNAL(topLevelChanged(bool)),
            this, SLOT(zoningTopLevelChanged(bool)));
    connect(marginsDockWidget, SIGNAL(topLevelChanged(bool)),
            this, SLOT(marginsTopLevelChanged(bool)));
    connect(logDockWidget, SIGNAL(topLevelChanged(bool)),
            this, SLOT(logTopLevelChanged(bool)));
}


void MainWindow::initialize(const QString &filename1,
                            const QString &filename2)
{
    if (!filename1.isEmpty()) {
        setFile1(filename1);
        setFile2Button->setFocus();
        if (!filename2.isEmpty()) {
            setFile2(filename2);
            compare();
        }
    }
    else
        updateUi();
}


void MainWindow::updateUi()
{
    compareButton->setEnabled(!filename1LineEdit->text().isEmpty() &&
                              !filename2LineEdit->text().isEmpty());
    saveButton->setEnabled(viewDiffComboBox->count() > 1);
    if (!showZonesCheckBox->isEnabled())
        showZonesCheckBox->setChecked(false);
    if (compareComboBox->currentIndex() != CompareAppearance)
        showComboBox->setCurrentIndex(0);
    showComboBox->setEnabled(compareComboBox->currentIndex() ==
                             CompareAppearance);
    QPushButton *button = qobject_cast<QPushButton*>(focusWidget());
    bool enableNavigationButton = (button == previousButton ||
                                   button == nextButton);
    previousButton->setEnabled(viewDiffComboBox->count() > 1 &&
            viewDiffComboBox->currentIndex() > 0);
    nextButton->setEnabled(viewDiffComboBox->count() > 1 &&
            viewDiffComboBox->currentIndex() + 1 <
            viewDiffComboBox->count());
    if (enableNavigationButton && !(previousButton->isEnabled() &&
                                    nextButton->isEnabled())) {
        if (previousButton->isEnabled())
            previousButton->setFocus();
        else
            nextButton->setFocus();
    }
    if (marginsGroupBox->isChecked()) {
        page1Label->setCursor(Qt::PointingHandCursor);
        page2Label->setCursor(Qt::PointingHandCursor);
    }
    else {
        page1Label->setCursor(Qt::ArrowCursor);
        page2Label->setCursor(Qt::ArrowCursor);
    }
}


void MainWindow::controlDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::TopDockWidgetArea ||
        area == Qt::BottomDockWidgetArea) {
        controlLayout->setDirection(QBoxLayout::LeftToRight);
    }
    else {
        controlLayout->setDirection(QBoxLayout::TopToBottom);
    }
    controlDockArea = area;
}


void MainWindow::actionDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::TopDockWidgetArea ||
        area == Qt::BottomDockWidgetArea)
        actionLayout->setDirection(QBoxLayout::LeftToRight);
    else
        actionLayout->setDirection(QBoxLayout::TopToBottom);
    actionDockArea = area;
}


void MainWindow::zoningDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::TopDockWidgetArea ||
        area == Qt::BottomDockWidgetArea)
        zoningLayout->setDirection(QBoxLayout::LeftToRight);
    else
        zoningLayout->setDirection(QBoxLayout::TopToBottom);
    zoningDockArea = area;
}


void MainWindow::marginsDockLocationChanged(Qt::DockWidgetArea area)
{
    if (area == Qt::TopDockWidgetArea ||
        area == Qt::BottomDockWidgetArea)
        marginsLayout->setDirection(QBoxLayout::LeftToRight);
    else
        marginsLayout->setDirection(QBoxLayout::TopToBottom);
    marginsDockArea = area;
}


void MainWindow::controlTopLevelChanged(bool floating)
{
    controlLayout->setDirection(floating ? QBoxLayout::TopToBottom
                                         : QBoxLayout::LeftToRight);
    if (QWidget *widget = static_cast<QWidget*>(controlLayout->parent()))
        widget->setFixedSize(floating ? widget->minimumSizeHint()
                : QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    controlDockWidget->setWindowTitle(floating ? tr("DiffPDF — Controls")
                                               : tr("Controls"));
}


void MainWindow::actionTopLevelChanged(bool floating)
{
    actionLayout->setDirection(floating ? QBoxLayout::TopToBottom
                                        : QBoxLayout::LeftToRight);
    if (QWidget *widget = static_cast<QWidget*>(actionLayout->parent()))
        widget->setFixedSize(floating ? widget->minimumSizeHint()
                : QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    actionDockWidget->setWindowTitle(floating ? tr("DiffPDF — Actions")
                                              : tr("Actions"));
}


void MainWindow::zoningTopLevelChanged(bool floating)
{
    zoningLayout->setDirection(floating ? QBoxLayout::TopToBottom
                                        : QBoxLayout::LeftToRight);
    if (QWidget *widget = static_cast<QWidget*>(zoningLayout->parent()))
        widget->setFixedSize(floating ? widget->minimumSizeHint()
                : QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    zoningDockWidget->setWindowTitle(floating ? tr("DiffPDF — Zoning")
                                              : tr("Zoning"));
}


void MainWindow::marginsTopLevelChanged(bool floating)
{
    marginsLayout->setDirection(floating ? QBoxLayout::TopToBottom
                                        : QBoxLayout::LeftToRight);
    if (QWidget *widget = static_cast<QWidget*>(marginsLayout->parent()))
        widget->setFixedSize(floating ? widget->minimumSizeHint()
                : QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    marginsDockWidget->setWindowTitle(floating ? tr("DiffPDF — Margins")
                                              : tr("Margins"));
}


void MainWindow::logTopLevelChanged(bool floating)
{
    logDockWidget->setWindowTitle(floating ? tr("DiffPDF — Log")
                                           : tr("Log"));
}


void MainWindow::previousPages()
{
    int i = viewDiffComboBox->currentIndex();
    if (i > 0)
        viewDiffComboBox->setCurrentIndex(i - 1);
}


void MainWindow::nextPages()
{
    int i = viewDiffComboBox->currentIndex();
    if (i + 1 < viewDiffComboBox->count())
        viewDiffComboBox->setCurrentIndex(i + 1);
}


void MainWindow::updateViews(int index)
{
    if (index == 0) {
        page1Label->clear();
        page2Label->clear();
        return;
    }
    else if (index == -1)
        index = viewDiffComboBox->currentIndex();
    PagePair pair = viewDiffComboBox->itemData(index).value<PagePair>();
    if (pair.isNull())
        return;

    QString filename1 = filename1LineEdit->text();
    PdfDocument pdf1 = getPdf(filename1);
    if (!pdf1)
        return;
    PdfPage page1(pdf1->page(pair.left));
    if (!page1)
        return;

    QString filename2 = filename2LineEdit->text();
    PdfDocument pdf2 = getPdf(filename2);
    if (!pdf2)
        return;
    PdfPage page2(pdf2->page(pair.right));
    if (!page2)
        return;

    const QPair<QString, QString> keys = cacheKeys(index, pair);
    const QPair<QPixmap, QPixmap> pixmaps = populatePixmaps(pdf1, page1,
            pdf2, page2, pair.hasVisualDifference, keys.first,
            keys.second);
    page1Label->setPixmap(pixmaps.first);
    page2Label->setPixmap(pixmaps.second);
    if (showZonesCheckBox->isChecked())
        showZones();
    if (marginsGroupBox->isChecked())
        showMargins();
}


const QPair<QString, QString> MainWindow::cacheKeys(const int index,
        const PagePair &pair) const
{
    int comparisonMode;
    if (compareComboBox->currentIndex() == CompareAppearance)
        comparisonMode = showComboBox->currentIndex();
    else
        comparisonMode = -compareComboBox->currentIndex();
    QString zoning;
    if (zoningGroupBox->isChecked())
        zoning = QString("%1:%2:%3").arg(columnsSpinBox->value())
                .arg(toleranceRSpinBox->value())
                .arg(toleranceYSpinBox->value());
    QString margins;
    if (marginsGroupBox->isChecked())
        margins = QString("%1:%2:%3:%4").arg(topMarginSpinBox->value())
                .arg(bottomMarginSpinBox->value())
                .arg(leftMarginSpinBox->value())
                .arg(rightMarginSpinBox->value());
    const QString key = QString("%1:%2:%3:%4:%5").arg(index)
            .arg(zoomSpinBox->value()).arg(comparisonMode).arg(zoning)
            .arg(margins);
    const QString key1 = QString("1:%1:%2:%3").arg(key).arg(pair.left)
            .arg(filename1LineEdit->text());
    const QString key2 = QString("2:%1:%2:%3").arg(key).arg(pair.right)
            .arg(filename2LineEdit->text());
    return qMakePair(key1, key2);
}


const QPair<QPixmap, QPixmap> MainWindow::populatePixmaps(
        const PdfDocument &pdf1, const PdfPage &page1,
        const PdfDocument &pdf2, const PdfPage &page2,
        bool hasVisualDifference, const QString &key1,
        const QString &key2)
{
    QPixmap pixmap1;
    QPixmap pixmap2;
#if QT_VERSION >= 0x040600
    if (!QPixmapCache::find(key1, &pixmap1) ||
        !QPixmapCache::find(key2, &pixmap2)) {
#else
    if (!QPixmapCache::find(key1, pixmap1) ||
        !QPixmapCache::find(key2, pixmap2)) {
#endif
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        const int DPI = static_cast<int>(POINTS_PER_INCH *
                (zoomSpinBox->value() / 100.0));
        const bool compareText = compareComboBox->currentIndex() !=
                                 CompareAppearance;
        QImage plainImage1;
        QImage plainImage2;
        if (hasVisualDifference || !compareText) {
            plainImage1 = page1->renderToImage(DPI, DPI);
            plainImage2 = page2->renderToImage(DPI, DPI);
        }
        pdf1->setRenderHint(Poppler::Document::Antialiasing);
        pdf1->setRenderHint(Poppler::Document::TextAntialiasing);
        pdf2->setRenderHint(Poppler::Document::Antialiasing);
        pdf2->setRenderHint(Poppler::Document::TextAntialiasing);
        QImage image1 = page1->renderToImage(DPI, DPI);
        QImage image2 = page2->renderToImage(DPI, DPI);

        if (compareComboBox->currentIndex() != CompareAppearance ||
            showComboBox->currentIndex() == 0) {
            QPainterPath highlighted1;
            QPainterPath highlighted2;
            if (hasVisualDifference || !compareText)
                computeVisualHighlights(&highlighted1, &highlighted2,
                        plainImage1, plainImage2);
            else
                computeTextHighlights(&highlighted1, &highlighted2, page1,
                        page2, DPI);
            if (!highlighted1.isEmpty())
                paintOnImage(highlighted1, &image1);
            if (!highlighted2.isEmpty())
                paintOnImage(highlighted2, &image2);
            if (highlighted1.isEmpty() && highlighted2.isEmpty()) {
                QFont font("Helvetica", 14);
                font.setOverline(true);
                font.setUnderline(true);
                highlighted1.addText(DPI / 4, DPI / 4, font,
                    tr("DiffPDF: False Positive"));
                paintOnImage(highlighted1, &image1);
            }
            pixmap1 = QPixmap::fromImage(image1);
            pixmap2 = QPixmap::fromImage(image2);
        } else {
            pixmap1 = QPixmap::fromImage(image1);
            QImage composed(image1.size(), image1.format());
            QPainter painter(&composed);
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.fillRect(composed.rect(), Qt::transparent);
            painter.setCompositionMode(
                    QPainter::CompositionMode_SourceOver);
            painter.drawImage(0, 0, image1);
            painter.setCompositionMode(
                    static_cast<QPainter::CompositionMode>(
                        showComboBox->itemData(
                            showComboBox->currentIndex()).toInt()));
            painter.drawImage(0, 0, image2);
            painter.setCompositionMode(
                    QPainter::CompositionMode_DestinationOver);
            painter.fillRect(composed.rect(), Qt::white);
            painter.end();
            pixmap2 = QPixmap::fromImage(composed);
        }
        QPixmapCache::insert(key1, pixmap1);
        QPixmapCache::insert(key2, pixmap2);
        QApplication::restoreOverrideCursor();
    }
    return qMakePair(pixmap1, pixmap2);
}


void MainWindow::computeTextHighlights(QPainterPath *highlighted1,
        QPainterPath *highlighted2, const PdfPage &page1,
        const PdfPage &page2, const int DPI)
{
    const bool ComparingWords = compareComboBox->currentIndex() ==
                                CompareWords;
    QRectF rect1;
    QRectF rect2;
    QSettings settings;
    const int OVERLAP = settings.value("Overlap", 5).toInt();
    const bool COMBINE = settings.value("CombineTextHighlighting", true)
            .toBool();
    QRectF rect;
    if (marginsGroupBox->isChecked())
        rect = pointRectForMargins(page1->pageSize());
    const TextBoxList list1 = getTextBoxes(page1, rect);
    const TextBoxList list2 = getTextBoxes(page2, rect);
    TextItems items1 = ComparingWords ? getWords(list1)
                                      : getCharacters(list1);
    TextItems items2 = ComparingWords ? getWords(list2)
                                      : getCharacters(list2);
    const int ToleranceY = toleranceYSpinBox->value();
    if (zoningGroupBox->isChecked()) {
        const int ToleranceR = toleranceRSpinBox->value();
        const int Columns = columnsSpinBox->value();
        items1.columnZoneYxOrder(page1->pageSize().width(), ToleranceR,
                                 ToleranceY, Columns);
        items2.columnZoneYxOrder(page2->pageSize().width(), ToleranceR,
                                 ToleranceY, Columns);
    }

    if (debug >= DebugShowTexts) {
        const bool Yx = debug == DebugShowTextsAndYX;
        items1.debug(1, ToleranceY, ComparingWords, Yx);
        items2.debug(2, ToleranceY, ComparingWords, Yx);
    }

    SequenceMatcher matcher(items1.texts(), items2.texts());
    RangesPair rangesPair = computeRanges(&matcher);
    rangesPair = invertRanges(rangesPair.first, items1.count(),
                              rangesPair.second, items2.count());

    foreach (int index, rangesPair.first)
        addHighlighting(&rect1, highlighted1, items1.at(index).rect,
                        OVERLAP, DPI, COMBINE);
    if (!rect1.isNull() && !rangesPair.first.isEmpty())
        highlighted1->addRect(rect1);
    foreach (int index, rangesPair.second)
        addHighlighting(&rect2, highlighted2, items2.at(index).rect,
                        OVERLAP, DPI, COMBINE);
    if (!rect2.isNull() && !rangesPair.second.isEmpty())
        highlighted2->addRect(rect2);
}


void MainWindow::addHighlighting(QRectF *bigRect,
        QPainterPath *highlighted, const QRectF wordOrCharRect,
        const int OVERLAP, const int DPI, const bool COMBINE)
{
    QRectF rect = wordOrCharRect;
    scaleRect(DPI, &rect);
    if (COMBINE && rect.adjusted(-OVERLAP, -OVERLAP, OVERLAP, OVERLAP)
        .intersects(*bigRect))
        *bigRect = bigRect->united(rect);
    else {
        highlighted->addRect(*bigRect);
        *bigRect = rect;
    }
}


void MainWindow::computeVisualHighlights(QPainterPath *highlighted1,
        QPainterPath *highlighted2, const QImage &plainImage1,
        const QImage &plainImage2)
{
    QSettings settings;
    const int SQUARE_SIZE = settings.value("SquareSize", 10).toInt();
    QRect box;
    if (marginsGroupBox->isChecked())
        box = pixelRectForMargins(plainImage1.size());
    QRect target;
    for (int x = 0; x < plainImage1.width(); x += SQUARE_SIZE) {
        for (int y = 0; y < plainImage1.height(); y += SQUARE_SIZE) {
            const QRect rect(x, y, SQUARE_SIZE, SQUARE_SIZE);
            if (!box.isEmpty() && !box.contains(rect))
                continue;
            QImage temp1 = plainImage1.copy(rect);
            QImage temp2 = plainImage2.copy(rect);
            if (temp1 != temp2) {
                if (rect.adjusted(-1, -1, 1, 1).intersects(target))
                    target = target.united(rect);
                else {
                    highlighted1->addRect(target);
                    highlighted2->addRect(target);
                    target = rect;
                }
            }
        }
    }
    if (!target.isNull()) {
        highlighted1->addRect(target);
        highlighted2->addRect(target);
    }
}


QRect MainWindow::pixelRectForMargins(const QSize &size)
{
    const int DPI = static_cast<int>(POINTS_PER_INCH *
                (zoomSpinBox->value() / 100.0));
    int top = pixelOffsetForPointValue(DPI, topMarginSpinBox->value());
    int left = pixelOffsetForPointValue(DPI, leftMarginSpinBox->value());
    int right = pixelOffsetForPointValue(DPI, rightMarginSpinBox->value());
    int bottom = pixelOffsetForPointValue(DPI,
            bottomMarginSpinBox->value());
    return QRect(QPoint(left, top),
                 QPoint(size.width() - right, size.height() - bottom));
}


void MainWindow::paintOnImage(const QPainterPath &path, QImage *image)
{
    QPen pen_(pen);
    QBrush brush_(brush);
    QColor color = pen.color();
    QSettings settings;
    const qreal Alpha = settings.value("Opacity", 13).toInt() / 100.0;
    color.setAlphaF(Alpha);
    pen_.setColor(color);
    brush_.setColor(color);

    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen_);
    painter.setBrush(brush_);

    const int SQUARE_SIZE = settings.value("SquareSize", 10).toInt();
    const double RULE_WIDTH = settings.value("RuleWidth", 1.5).toDouble();
    QRectF rect = path.boundingRect();
    if (rect.width() < SQUARE_SIZE && rect.height() < SQUARE_SIZE) {
        rect.setHeight(SQUARE_SIZE);
        rect.setWidth(SQUARE_SIZE);
        painter.drawRect(rect);
        if (!qFuzzyCompare(RULE_WIDTH, 0.0)) {
            painter.setPen(QPen(pen.color()));
            painter.drawRect(0, rect.y(), RULE_WIDTH, rect.height());
        }
    }
    else {
        QPainterPath path_(path);
        path_.setFillRule(Qt::WindingFill);
        painter.drawPath(path_);
        if (!qFuzzyCompare(RULE_WIDTH, 0.0)) {
            painter.setPen(QPen(pen.color()));
            QList<QPolygonF> polygons = path_.toFillPolygons();
            foreach (const QPolygonF &polygon, polygons) {
                const QRectF rect = polygon.boundingRect();
                painter.drawRect(0, rect.y(), RULE_WIDTH, rect.height());
            }
        }
    }
    painter.end();
}


void MainWindow::closeEvent(QCloseEvent*)
{
    QSettings settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/State", saveState());
    settings.setValue("MainWindow/ControlDockArea",
                      static_cast<int>(controlDockArea));
    settings.setValue("MainWindow/ActionDockArea",
                      static_cast<int>(actionDockArea));
    settings.setValue("MainWindow/ZoningDockArea",
                      static_cast<int>(zoningDockArea));
    settings.setValue("MainWindow/MarginsDockArea",
                      static_cast<int>(marginsDockArea));
    settings.setValue("MainWindow/LogDockArea",
                      static_cast<int>(logDockArea));
    settings.setValue("MainWindow/ViewSplitter", splitter->saveState());
    settings.setValue("ShowToolTips", showToolTips);
    settings.setValue("CombineTextHighlighting", combineTextHighlighting);
    settings.setValue("Zoom", zoomSpinBox->value());
    settings.setValue("Columns", columnsSpinBox->value());
    settings.setValue("Tolerance/R", toleranceRSpinBox->value());
    settings.setValue("Tolerance/Y", toleranceYSpinBox->value());
    settings.setValue("Outline", pen);
    settings.setValue("Fill", brush);
    settings.setValue("InitialComparisonMode",
                      compareComboBox->currentIndex());
    settings.setValue("Margins/Exclude", marginsGroupBox->isChecked());
    settings.setValue("Margins/Left", leftMarginSpinBox->value());
    settings.setValue("Margins/Right", rightMarginSpinBox->value());
    settings.setValue("Margins/Top", topMarginSpinBox->value());
    settings.setValue("Margins/Bottom", bottomMarginSpinBox->value());
    QMainWindow::close();
}


bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ToolTip && !showToolTips)
        return true;
    return QMainWindow::eventFilter(object, event);
}


void MainWindow::setFiles1(const QStringList &filenames)
{
    if (filenames.count() && !filenames.at(0).isEmpty()) {
        setFile1(filenames.at(0));
        if (filenames.count() > 1 && !filenames.at(1).isEmpty())
            setFile2(filenames.at(1));
    }
}


void MainWindow::setFiles2(const QStringList &filenames)
{
    if (filenames.count() && !filenames.at(0).isEmpty()) {
        setFile2(filenames.at(0));
        if (filenames.count() > 1 && !filenames.at(1).isEmpty())
            setFile1(filenames.at(1));
    }
}


void MainWindow::setFile1(QString filename)
{
    if (filename.isEmpty())
        filename = QFileDialog::getOpenFileName(this,
                tr("DiffPDF — Choose File #1"), currentPath,
                tr("PDF files (*.pdf)"));
    if (!filename.isEmpty()) {
        if (filename == filename2LineEdit->text()) {
            QMessageBox::warning(this, tr("DiffPDF — Error"),
                    tr("Cannot compare a file to itself."));
            return;
        }
        filename1LineEdit->setText(filename);
        if (!filename2LineEdit->text().isEmpty())
            page1Label->setText(tr("<p style='font-size: xx-large;"
                    "color: darkgreen'>DiffPDF: Click Compare<br>"
                    "or change File #2.</p>"));
        else
            page1Label->setText(tr("<p style='font-size: xx-large;"
                    "color: darkgreen'>DiffPDF: Choose File #2.</p>"));
        page2Label->clear();
        updateUi();
        int page_count = writeFileInfo(filename);
        pages1LineEdit->setText(tr("1-%1").arg(page_count));
        currentPath = QFileInfo(filename).canonicalPath();
        setFile2Button->setFocus();
        if (filename2LineEdit->text().isEmpty())
            statusLabel->setText(tr("Choose second file"));
        else
            statusLabel->setText(tr("Ready to compare"));
    }
}


void MainWindow::setFile2(QString filename)
{
    if (filename.isEmpty())
        filename = QFileDialog::getOpenFileName(this,
                tr("DiffPDF — Choose File #2"), currentPath,
                tr("PDF files (*.pdf)"));
    if (!filename.isEmpty()) {
        if (filename == filename1LineEdit->text()) {
            QMessageBox::warning(this, tr("DiffPDF — Error"),
                    tr("Cannot compare a file to itself."));
            return;
        }
        filename2LineEdit->setText(filename);
        if (!filename1LineEdit->text().isEmpty())
            page2Label->setText(tr("<p style='font-size: xx-large;"
                    "color: darkgreen'>DiffPDF: Click Compare<br>"
                    "or change File #1.</p>"));
        else
            page2Label->setText(tr("<p style='font-size: xx-large;"
                    "color: darkgreen'>DiffPDF: Choose File #1.</p>"));
        page1Label->clear();
        updateUi();
        int page_count = writeFileInfo(filename);
        pages2LineEdit->setText(tr("1-%1").arg(page_count));
        currentPath = QFileInfo(filename).canonicalPath();
        compareButton->setFocus();
        if (filename1LineEdit->text().isEmpty())
            statusLabel->setText(tr("Choose first file"));
        else
            statusLabel->setText(tr("Ready to compare"));
    }
}


PdfDocument MainWindow::getPdf(const QString &filename)
{
    PdfDocument pdf(Poppler::Document::load(filename));
    if (!pdf)
        QMessageBox::warning(this, tr("DiffPDF — Error"),
                tr("Cannot load '%1'.").arg(filename));
    else if (pdf->isLocked()) {
        QMessageBox::warning(this, tr("DiffPDF — Error"),
                tr("Cannot read a locked PDF ('%1').").arg(filename));
#if QT_VERSION >= 0x040600
        pdf.clear();
#else
        pdf.reset();
#endif
    }
    return pdf;
}


int MainWindow::writeFileInfo(const QString &filename)
{
    int page_count = 0;
    PdfDocument pdf = getPdf(filename);
    if (!pdf)
        return page_count;
    writeLine(tr("<b>%1</b>").arg(filename));
    foreach (const QString &key, pdf->infoKeys()) {
        if (key == "CreationDate" || key == "ModDate")
            continue;
        writeLine(tr("%1: %2.").arg(key).arg(pdf->info(key)));
    }
    QDateTime created = pdf->date("CreationDate");
    QDateTime modified = pdf->date("ModDate");
    if (created != modified)
        writeLine(tr("Created: %1, last modified %2.")
                  .arg(created.toString())
                  .arg(modified.toString()));
    else
        writeLine(tr("Created: %1.").arg(created.toString()));
    page_count = pdf->numPages();
    writeLine(tr("Page count: %1.").arg(page_count));
    if (page_count > 0) {
        const double PointToMM = 0.3527777777;
        PdfPage page1(pdf->page(0));
        QSize size = page1->pageSize();
        writeLine(tr("Page size: %1pt x %2pt (%3mm x %4mm).")
                  .arg(size.width()).arg(size.height())
                  .arg(qRound(size.width() * PointToMM))
                  .arg(qRound(size.height() * PointToMM)));
        topMarginSpinBox->setRange(0, (size.height() / 2) - 10);
        bottomMarginSpinBox->setRange(0, (size.height() / 2) - 10);
        leftMarginSpinBox->setRange(0, (size.width() / 2) - 10);
        rightMarginSpinBox->setRange(0, (size.width() / 2) - 10);
    }
    return page_count;
}


void MainWindow::writeLine(const QString &text)
{
    logEdit->appendHtml(text);
    logEdit->ensureCursorVisible();
}


void MainWindow::writeError(const QString &text)
{
    logEdit->appendHtml(tr("<font color=red>%1</font>").arg(text));
    logEdit->ensureCursorVisible();
}


QList<int> MainWindow::getPageList(int which, PdfDocument pdf)
{
    // Poppler has 0-based page numbers; the UI has 1-based page numbers
    QLineEdit *pagesEdit = (which == 1 ? pages1LineEdit : pages2LineEdit);
    bool error = false;
    QList<int> pages;
    QString page_string = pagesEdit->text();
    page_string = page_string.replace(QRegExp("\\s+"), "");
    QStringList page_list = page_string.split(",");
    bool ok;
    foreach (const QString &page, page_list) {
        int hyphen = page.indexOf("-");
        if (hyphen > -1) {
            int p1 = page.left(hyphen).toInt(&ok);
            if (!ok || p1 < 1) {
                error = true;
                break;
            }
            int p2 = page.mid(hyphen + 1).toInt(&ok);
            if (!ok || p2 < 1 || p2 < p1) {
                error = true;
                break;
            }
            if (p1 == p2)
                pages.append(p1 - 1);
            else {
                for (int p = p1; p <= p2; ++p) {
                    if (p > pdf->numPages())
                        break;
                    pages.append(p - 1);
                }
            }
        }
        else {
            int p = page.toInt(&ok);
            if (ok && p > 0 && p <= pdf->numPages())
                pages.append(p - 1);
            else {
                error = true;
                break;
            }
        }
    }
    if (error) {
        pages.clear();
        writeError(tr("Failed to understand page range '%1'.")
                   .arg(pagesEdit->text()));
        pagesEdit->setText(tr("1-%1").arg(pdf->numPages()));
        for (int page = 0; page < pdf->numPages(); ++page)
            pages.append(page);
    }
    return pages;
}


void MainWindow::compare()
{
    if (compareButton->text() == tr("&Cancel")) {
        cancel = true;
        compareButton->setText(tr("&Compare"));
        compareButton->setEnabled(true);
        return;
    }
    cancel = false;
    QString filename1 = filename1LineEdit->text();
    PdfDocument pdf1 = getPdf(filename1);
    if (!pdf1)
        return;
    QString filename2 = filename2LineEdit->text();
    PdfDocument pdf2 = getPdf(filename2);
    if (!pdf2) {
        return;
    }

    comparePrepareUi();
    QTime time;
    time.start();
    const QPair<int, int> pair = comparePages(filename1, pdf1, filename2,
                                              pdf2);
    compareUpdateUi(pair, time.elapsed());
}


void MainWindow::comparePrepareUi()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    compareButton->setText(tr("&Cancel"));
    compareButton->setEnabled(true);
    compareButton->setFocus();
    viewDiffComboBox->clear();
    viewDiffComboBox->addItem(tr("(Not viewing)"));
    saveButton->setEnabled(false);
    statusLabel->setText(tr("Ready"));
}


const QPair<int, int> MainWindow::comparePages(const QString &filename1,
        const PdfDocument &pdf1, const QString &filename2,
        const PdfDocument &pdf2)
{
    QList<int> pages1 = getPageList(1, pdf1);
    QList<int> pages2 = getPageList(2, pdf2);
    int total = qMin(pages1.count(), pages2.count());
    int number = 0;
    int index = 0;
    while (!pages1.isEmpty() && !pages2.isEmpty()) {
        int p1 = pages1.takeFirst();
        PdfPage page1(pdf1->page(p1));
        if (!page1) {
            writeError(tr("Failed to read page %1 from '%2'.")
                          .arg(p1 + 1).arg(filename1));
            continue;
        }
        int p2 = pages2.takeFirst();
        PdfPage page2(pdf2->page(p2));
        if (!page2) {
            writeError(tr("Failed to read page %1 from '%2'.")
                          .arg(p2 + 1).arg(filename2));
            continue;
        }
        writeLine(tr("Comparing: %1 vs. %2.").arg(p1 + 1).arg(p2 + 1));
        QApplication::processEvents();
        if (cancel) {
            writeError(tr("Cancelled."));
            break;
        }
        Difference difference = getTheDifference(page1, page2);
        if (difference != NoDifference) {
            QVariant v;
            v.setValue(PagePair(p1, p2, difference == VisualDifference));
            viewDiffComboBox->addItem(tr("%1 vs. %2 %3 %4")
                    .arg(p1 + 1).arg(p2 + 1).arg(QChar(0x2022))
                    .arg(++index), v);
        }
        statusLabel->setText(tr("Comparing %1/%2").arg(++number)
                                                  .arg(total));
    }
    return qMakePair(number, total);
}


void MainWindow::compareUpdateUi(const QPair<int, int> &pair,
        const int millisec)
{
    const int differ = viewDiffComboBox->count() - 1;
    if (!cancel) {
        if (millisec > 1000)
            writeLine(tr("Completed in %1 seconds.")
            .arg(millisec / 1000.0, 0, 'f', 2));
        if (viewDiffComboBox->count() > 1) {
            if (viewDiffComboBox->count() == 2)
                writeLine(tr("<font color=brown>Files differ on 1 page "
                            "(%1 page%2 compared).</font>")
                        .arg(pair.first)
                        .arg(pair.first == 1 ? tr(" was") : tr("s were")));
            else
                writeLine(tr("<font color=brown>Files differ on %1 pages "
                            "(%2 page%3 compared).</font>")
                            .arg(differ).arg(pair.first)
                            .arg(pair.first == 1 ? tr(" was")
                                                 : tr("s were")));
            viewDiffComboBox->setFocus();
            viewDiffComboBox->setCurrentIndex(1);
        }
        else {
            writeLine(tr("The PDFs appear to be the same."));
            const QString message(tr("<p style='font-size: x-large;"
                    "color: darkgreen'>"
                    "DiffPDF: The PDFs appear to be the same.</p>"));
            page1Label->setText(message);
            page2Label->setText(message);
        }
    }

    compareButton->setText(tr("&Compare"));
    if (differ == 1) // Separated the cases for ease of translation
        statusLabel->setText(tr("1 differs %1/%2 compared").arg(pair.first)
                .arg(pair.second));
    else
        statusLabel->setText(tr("%1 differ %2/%3 compared").arg(differ)
                .arg(pair.first).arg(pair.second));
    saveButton->setEnabled(true);
    updateUi();
    if (!cancel)
        viewDiffComboBox->setFocus();
    QApplication::restoreOverrideCursor();
}


MainWindow::Difference MainWindow::getTheDifference(PdfPage page1,
                                                    PdfPage page2)
{
    QRectF rect;
    if (marginsGroupBox->isChecked())
        rect = pointRectForMargins(page1->pageSize());
    const TextBoxList list1 = getTextBoxes(page1, rect);
    const TextBoxList list2 = getTextBoxes(page2, rect);
    if (list1.count() != list2.count())
        return TextualDifference;
    for (int i = 0; i < list1.count(); ++i)
        if (list1[i]->text() != list2[i]->text())
            return TextualDifference;

    if (compareComboBox->currentIndex() == CompareAppearance) {
        int x = -1;
        int y = -1;
        int width = -1;
        int height = -1;
        if (marginsGroupBox->isChecked())
            computeImageOffsets(page1->pageSize(), &x, &y, &width,
                    &height);
        QImage image1 = page1->renderToImage(POINTS_PER_INCH,
                POINTS_PER_INCH, x, y, width, height);
        QImage image2 = page2->renderToImage(POINTS_PER_INCH,
                POINTS_PER_INCH, x, y, width, height);
        if (image1 != image2)
            return VisualDifference;
    }
    return NoDifference;
}


QRectF MainWindow::pointRectForMargins(const QSize &size)
{
    return rectForMargins(size.width(), size.height(),
            topMarginSpinBox->value(), bottomMarginSpinBox->value(),
            leftMarginSpinBox->value(), rightMarginSpinBox->value());
}


void MainWindow::computeImageOffsets(const QSize &size, int *x, int *y,
        int *width, int *height)
{
    const int DPI = static_cast<int>(POINTS_PER_INCH *
                (zoomSpinBox->value() / 100.0));
    *y = pixelOffsetForPointValue(DPI, topMarginSpinBox->value());
    *x = pixelOffsetForPointValue(DPI, leftMarginSpinBox->value());
    *width = pixelOffsetForPointValue(DPI, size.width() -
            (leftMarginSpinBox->value() + rightMarginSpinBox->value()));
    *height = pixelOffsetForPointValue(DPI, size.height() -
            (topMarginSpinBox->value() + bottomMarginSpinBox->value()));
}


void MainWindow::options()
{
    QSettings settings;
    qreal ruleWidth = settings.value("RuleWidth", 1.5).toDouble();
    int cacheSize = QPixmapCache::cacheLimit() / 1000;
    int alpha = settings.value("Opacity", 13).toInt();
    int squareSize = settings.value("SquareSize", 10).toInt();
    OptionsForm form(&pen, &brush, &ruleWidth, &showToolTips,
            &combineTextHighlighting, &cacheSize, &alpha, &squareSize,
            this);
    if (form.exec()) {
        settings.setValue("RuleWidth", ruleWidth);
        settings.setValue("CombineTextHighlighting",
                          combineTextHighlighting);
        settings.setValue("CacheSizeMB", cacheSize);
        settings.setValue("Opacity", alpha);
        settings.setValue("SquareSize", squareSize);
        QPixmapCache::clear();
        QPixmapCache::setCacheLimit(1000 * cacheSize);
        updateViews();
    }
}


void MainWindow::save()
{
    SaveForm form(currentPath, &saveFilename, &saveAll, &savePages, this);
    if (form.exec()) {
        QString filename1 = filename1LineEdit->text();
        PdfDocument pdf1 = getPdf(filename1);
        if (!pdf1)
            return;
        QString filename2 = filename2LineEdit->text();
        PdfDocument pdf2 = getPdf(filename2);
        if (!pdf2)
            return;
        saveButton->setEnabled(false);
        QApplication::processEvents();
        const int originalIndex = viewDiffComboBox->currentIndex();
        int start = originalIndex;
        int end = originalIndex + 1;
        if (saveAll) {
            start = 0;
            end = viewDiffComboBox->count();
        }
        QString header;
        const QChar bullet(0x2022);
        if (savePages == SaveLeftPages)
            header = tr("DiffPDF %1 %2 %1 %3").arg(bullet)
                .arg(filename1)
                .arg(QDate::currentDate().toString(Qt::ISODate));
        else if (savePages == SaveRightPages)
            header = tr("DiffPDF %1 %2 %1 %3").arg(bullet)
                .arg(filename2)
                .arg(QDate::currentDate().toString(Qt::ISODate));
        else
            header = tr("DiffPDF %1 %2 vs. %3 %1 %4").arg(bullet)
                .arg(filename1).arg(filename2)
                .arg(QDate::currentDate().toString(Qt::ISODate));
        if (saveFilename.toLower().endsWith(".pdf"))
            saveAsPdf(start, end, pdf1, pdf2, header);
        else
            saveAsImages(start, end, pdf1, pdf2, header);
        updateViews(originalIndex);
        if (saveFilename.toLower().endsWith(".pdf"))
            writeLine(tr("Saved %1").arg(saveFilename));
        saveButton->setEnabled(true);
    }
}


void MainWindow::saveAsImages(const int start, const int end,
        const PdfDocument &pdf1, const PdfDocument &pdf2,
        const QString &header)
{
    PdfPage page1(pdf1->page(0));
    if (!page1)
        return;
    PdfPage page2(pdf2->page(0));
    if (!page2)
        return;
    int width = 2 * (savePages == SaveBothPages
            ? page1->pageSize().width() + page2->pageSize().width()
            : page1->pageSize().width());
    const int y = fontMetrics().lineSpacing();
    const int height = (2 * page1->pageSize().height()) - y;
    const int gap = 30;
    const QRect rect(0, 0, width, height);
    if (savePages == SaveBothPages)
        width = (width / 2) - gap;
    const QRect leftRect(0, y, width, height);
    const QRect rightRect(width + gap, y, width, height);
    int count = 0;
    QString imageFilename = saveFilename;
    int i = imageFilename.lastIndexOf(".");
    if (i > -1)
        imageFilename.insert(i, "-%1");
    else
        imageFilename += "-%1.png";
    for (int index = start; index < end; ++index) {
        QImage image(rect.size(), QImage::Format_ARGB32);
        QPainter painter(&image);
        painter.setFont(QFont("Helvetica", 11));
        painter.setPen(Qt::darkCyan);
        painter.fillRect(rect, Qt::white);
        if (!paintSaveAs(&painter, index, pdf1, pdf2, header, rect,
                    leftRect, rightRect))
            continue;
        QString filename = imageFilename;
        filename = filename.arg(++count);
        if (image.save(filename))
            writeLine(tr("Saved %1").arg(filename));
        else
            writeLine(tr("Failed to save %1").arg(filename));
    }
}


void MainWindow::saveAsPdf(const int start, const int end,
        const PdfDocument &pdf1, const PdfDocument &pdf2,
        const QString &header)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFileName(saveFilename);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setCreator(tr("DiffPDF"));
    printer.setOrientation(savePages == SaveBothPages
            ? QPrinter::Landscape : QPrinter::Portrait);
    QPainter painter(&printer);
    painter.setFont(QFont("Helvetica", 11));
    painter.setPen(Qt::darkCyan);
    const QRect rect(0, 0, painter.viewport().width(),
                        painter.fontMetrics().height());
    const int y = painter.fontMetrics().lineSpacing();
    const int height = painter.viewport().height() - y;
    const int gap = 30;
    int width = (painter.viewport().width() / 2) - gap;
    if (savePages != SaveBothPages)
        width = painter.viewport().width();
    const QRect leftRect(0, y, width, height);
    const QRect rightRect(width + gap, y, width, height);
    for (int index = start; index < end; ++index) {
        if (!paintSaveAs(&painter, index, pdf1, pdf2, header, rect,
                    leftRect, rightRect))
            continue;
        if (index + 1 < end)
            printer.newPage();
    }
}


bool MainWindow::paintSaveAs(QPainter *painter, const int index,
        const PdfDocument &pdf1, const PdfDocument &pdf2,
        const QString &header, const QRect &rect, const QRect &leftRect,
        const QRect &rightRect)
{
    PagePair pair = viewDiffComboBox->itemData(index)
        .value<PagePair>();
    if (pair.isNull())
        return false;
    PdfPage page1(pdf1->page(pair.left));
    if (!page1)
        return false;
    PdfPage page2(pdf2->page(pair.left));
    if (!page2)
        return false;
    const QPair<QString, QString> keys = cacheKeys(index, pair);
    const QPair<QPixmap, QPixmap> pixmaps = populatePixmaps(pdf1,
            page1, pdf2, page2, pair.hasVisualDifference,
            keys.first, keys.second);
    painter->drawText(rect, header, QTextOption(Qt::AlignHCenter|
                                                Qt::AlignTop));
    if (savePages == SaveBothPages) {
        QRect rect = resizeRect(leftRect, pixmaps.first.size());
        painter->drawPixmap(rect, pixmaps.first);
        rect = resizeRect(rightRect, pixmaps.second.size());
        painter->drawPixmap(rect, pixmaps.second);
        painter->drawRect(rightRect.adjusted(2.5, 2.5, 2.5, 2.5));
    } else if (savePages == SaveLeftPages) {
        QRect rect = resizeRect(leftRect, pixmaps.first.size());
        painter->drawPixmap(rect, pixmaps.first);
    } else { // (savePages == SaveRightPages)
        QRect rect = resizeRect(leftRect, pixmaps.second.size());
        painter->drawPixmap(rect, pixmaps.second);
    }
    painter->drawRect(leftRect.adjusted(2.5, 2.5, 2.5, 2.5));
    return true;
}


void MainWindow::help()
{
    if (!helpForm)
        helpForm = new HelpForm(language, this);
    helpForm->show();
    helpForm->raise();
    helpForm->activateWindow();
}


void MainWindow::about()
{
    if (!aboutForm)
        aboutForm = new AboutForm(this);
    aboutForm->show();
    aboutForm->raise();
    aboutForm->activateWindow();
}


void MainWindow::showZones()
{
    PagePair pair = viewDiffComboBox->itemData(
            viewDiffComboBox->currentIndex()).value<PagePair>();
    if (pair.isNull())
        return;
    QString filename1 = filename1LineEdit->text();
    PdfDocument pdf1 = getPdf(filename1);
    if (!pdf1)
        return;
    PdfPage page1(pdf1->page(pair.left));
    if (!page1)
        return;
    const TextBoxList list1 = getTextBoxes(page1);
    showZones(page1->pageSize().width(), list1, page1Label);

    QString filename2 = filename2LineEdit->text();
    PdfDocument pdf2 = getPdf(filename2);
    if (!pdf2)
        return;
    PdfPage page2(pdf2->page(pair.right));
    if (!page2)
        return;
    const TextBoxList list2 = getTextBoxes(page2);
    showZones(page2->pageSize().width(), list2, page2Label);
}


void MainWindow::showZones(const int Width, const TextBoxList &list,
                           QLabel *label)
{
    if (!label || !label->pixmap() || label->pixmap()->isNull())
        return;
    const bool ComparingWords = compareComboBox->currentIndex() ==
                                CompareWords;
    TextItems items = ComparingWords ? getWords(list)
                                     : getCharacters(list);
    items.columnYxOrder(Width, toleranceYSpinBox->value(),
                        columnsSpinBox->value());
    QList<QPainterPath> paths = items.generateZones(Width,
            toleranceRSpinBox->value(), toleranceYSpinBox->value(),
            columnsSpinBox->value());
    const int DPI = static_cast<int>(POINTS_PER_INCH *
            (zoomSpinBox->value() / 100.0));
    QPixmap pixmap = label->pixmap()->copy();
    QPainter painter(&pixmap);
    painter.setPen(Qt::green);
    for (int i = 0; i < paths.count(); ++i) {
        const QPainterPath &path = paths.at(i);
        QRectF rect = path.boundingRect();
        scaleRect(DPI, &rect);
        painter.drawRect(rect);
        painter.drawText(rect.x(), rect.y(), QString("#%1").arg(i + 1));
    }
    painter.end();
    label->setPixmap(pixmap);
}


void MainWindow::showMargins()
{
    if (leftMarginSpinBox->value() == 0 &&
        rightMarginSpinBox->value() == 0 &&
        topMarginSpinBox->value() == 0 &&
        bottomMarginSpinBox->value() == 0)
        return;
    showMargins(page1Label);
    showMargins(page2Label);
}


void MainWindow::showMargins(QLabel *label)
{
    if (!label || !label->pixmap() || label->pixmap()->isNull())
        return;
    const int DPI = static_cast<int>(POINTS_PER_INCH *
                (zoomSpinBox->value() / 100.0));
    QPixmap pixmap = label->pixmap()->copy();
    QPainter painter(&pixmap);
    painter.setPen(Qt::cyan);
    int left = leftMarginSpinBox->value();
    if (left) {
        const int x = pixelOffsetForPointValue(DPI, left);
        painter.drawLine(x, 0, x, pixmap.height());
    }
    int right = rightMarginSpinBox->value();
    if (right) {
        const int x = pixmap.width() -
            pixelOffsetForPointValue(DPI, right);
        painter.drawLine(x, 0, x, pixmap.height());
    }
    int top = topMarginSpinBox->value();
    if (top) {
        const int y = pixelOffsetForPointValue(DPI, top);
        painter.drawLine(0, y, pixmap.width(), y);
    }
    int bottom = bottomMarginSpinBox->value();
    if (bottom) {
        const int y = pixmap.height() -
            pixelOffsetForPointValue(DPI, bottom);
        painter.drawLine(0, y, pixmap.width(), y);
    }
    painter.end();
    label->setPixmap(pixmap);
}


void MainWindow::setAMargin(const QPoint &pos)
{
    if (!marginsGroupBox->isChecked() || !page1Label->pixmap() ||
        page1Label->pixmap()->isNull())
        return;
    const int DPI = static_cast<int>(POINTS_PER_INCH *
                (zoomSpinBox->value() / 100.0));
    const QSize &size = page1Label->pixmap()->size();
    int x = pos.x();
    int y = pos.y();
    const int HorizontalMiddle = size.width() / 2;
    const int TopOffset = size.height() / 3;
    const int BottomOffset = size.height() - TopOffset;
    const int VerticalMiddle = size.height() / 2;
    if (y > TopOffset && y < BottomOffset) { // Setting left or right
        if (x < HorizontalMiddle)
            leftMarginSpinBox->setValue(pointValueForPixelOffset(DPI, x));
        else
            rightMarginSpinBox->setValue(pointValueForPixelOffset(DPI,
                        (size.width() - x)));
    } else { // Setting top or bottom
        if (y < VerticalMiddle)
            topMarginSpinBox->setValue(pointValueForPixelOffset(DPI, y));
        else
            bottomMarginSpinBox->setValue(pointValueForPixelOffset(DPI,
                        (size.height() - y)));
    }
}
