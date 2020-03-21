/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup SEGSAdmin Projects/CoX/Utilities/SEGSAdmin
 * @{
 */

#include "TextEdit.h"
#include "SelectScriptDialog.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QDesktopWidget>
#include <QInputDialog>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMimeData>


/**
#ifdef Q_OS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif
**/
TextEdit::TextEdit(QWidget *parent)
    : QMainWindow(parent)
{
/**
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
**/
    setWindowTitle("SEGSAdmin Script Editor");
    resize(QDesktopWidget().availableGeometry(this).size() * 0.5);

    textEdit = new QTextEdit(this);
    setCentralWidget(textEdit);

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupTextActions();

    QFont textFont("Helvetica");
    textFont.setStyleHint(QFont::SansSerif);
    textEdit->setFont(textFont);
    textEdit->setFontPointSize(12);
    colorChanged(textEdit->textColor());

    connect(textEdit->document(), &QTextDocument::modificationChanged,
            actionSave, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);
    connect(textEdit->document(), &QTextDocument::undoAvailable,
            actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable,
            actionRedo, &QAction::setEnabled);

    setWindowModified(textEdit->document()->isModified());
    actionSave->setEnabled(textEdit->document()->isModified());
    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

    textEdit->setFocus();
    setCurrentFileName(QString());
}

void TextEdit::show_text_editor(const QString &script)
{
    highlighter = new SyntaxHighlighter(textEdit->document());
    show();
    qDebug()<<"Show text editor called";
    qDebug()<<"Script: "<<script;
    load(script);
}
void TextEdit::closeEvent(QCloseEvent *e)
{
    if(maybeSave())
        e->accept();
    else
        e->ignore();
}

void TextEdit::setupFileActions()
{
    QToolBar *tb = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":icons/Resources/save.svg"));
    actionSave = menu->addAction(saveIcon, tr("&Save"), this, &TextEdit::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    tb->addAction(actionSave);
}

void TextEdit::setupEditActions()
{
    QToolBar *tb = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Edit"));

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":icons/Resources/corner-down-left.svg"));
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(":icons/Resources/corner-down-right.svg"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb->addAction(actionRedo);
    menu->addSeparator();
}

void TextEdit::setupTextActions()
{
    QToolBar *tb = addToolBar(tr("Format Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Format"));

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(":icons/Resources/bold.svg"));
    actionTextBold = menu->addAction(boldIcon, tr("&Bold"), this, &TextEdit::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    tb->addAction(actionTextBold);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(":icons/Resources/italic.svg"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Italic"), this, &TextEdit::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    tb->addAction(actionTextItalic);

    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = menu->addAction(pix, tr("&Color..."), this, &TextEdit::textColor);
    tb->addAction(actionTextColor);

    const QIcon newLineIcon = QIcon::fromTheme("format-text-newline", QIcon(":icons/Resources/terminal.svg"));
    newLineButton = menu->addAction(newLineIcon, tr("&New Line"), this, &TextEdit::insert_new_line);
    newLineButton->setShortcut(Qt::CTRL + Qt::EnterKeyReturn);
    tb->addAction(newLineButton);

    // Keeping in for later on, not used currently.
    /**
    const QIcon newLinkIcon = QIcon::fromTheme("format-text-addlink", QIcon(":icons/Resources/link.svg"));
    newLinkButton = menu->addAction(newLinkIcon, tr("&New Link"), this, &TextEdit::insert_new_link);
    tb->addAction(newLinkButton);
    **/


    tb = addToolBar(tr("Format Actions"));
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    QLabel *font_label = new QLabel(this);
    font_label->setText("Fonts: ");
    tb->addWidget(font_label);

    tb->addSeparator();

    comboFont = new QComboBox(tb);
    QStringList faces = {
        "Redcircle",
        "Verdana-bold",
        "Verdana",
        "Courier",
        "Small",
        "Computer",
    };
    comboFont->addItems(faces);
    tb->addWidget(comboFont);
    connect(comboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textFamily);

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    QStringList sizes = {
        "1",
        "2",
        "3",
        "4",
    };
    comboSize->addItems(sizes);
    connect(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textSize);

    tb->addSeparator();

    QLabel *shadow_label = new QLabel(this);
    shadow_label->setText("Shadow: ");
    tb->addWidget(shadow_label);

    comboShadow = new QComboBox(tb);
    comboShadow->setObjectName("comboShadow");
    tb->addWidget(comboShadow);
    comboShadow->setEditable(true);

    QStringList shadow_values = {
        "1",
        "2",
        "3",
        "4",
        "5",
        "6"
    };
    comboShadow->addItems(shadow_values);
    connect(comboShadow, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textShadow);

    tb->addSeparator();

    QLabel *outline_label = new QLabel(this);
    outline_label->setText("Outline: ");
    tb->addWidget(outline_label);

    comboOutline = new QComboBox(tb);
    comboOutline->setObjectName("comboOutline");
    tb->addWidget(comboOutline);
    comboOutline->setEditable(true);

    QStringList outline_values = {
        "1",
        "2",
        "3",
        "4",
        "5",
        "6"
    };
    comboOutline->addItems(outline_values);
    connect(comboOutline, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textOutline);

}

bool TextEdit::load(const QString &f)
{
    if(!QFile::exists(f))
        return false;
    QFile file(f);
    if(!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if(Qt::mightBeRichText(str)) {
        textEdit->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        textEdit->setPlainText(str);
    }

    setCurrentFileName(f);
    return true;
}

bool TextEdit::maybeSave()
{
    if(!textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, QCoreApplication::applicationName(),
                             tr("The document has been modified.\n"
                                "Do you want to save your changes?"),
                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if(ret == QMessageBox::Save)
        return fileSave();
    else if(ret == QMessageBox::Cancel)
        return false;
    return true;
}

void TextEdit::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    textEdit->document()->setModified(false);

    QString shownName;
    if(fileName.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1[*] - %2").arg(shownName, QCoreApplication::applicationName()));
    setWindowModified(false);
}

void TextEdit::fileNew()
{
    if(maybeSave()) {
        textEdit->clear();
        setCurrentFileName(QString());
    }
}

void TextEdit::fileOpen()
{
    QFileDialog fileDialog(this, tr("Open File..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setMimeTypeFilters(QStringList() << "text/html" << "text/plain");
    if(fileDialog.exec() != QDialog::Accepted)
        return;
    const QString fn = fileDialog.selectedFiles().first();
    if(load(fn))
        statusBar()->showMessage(tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fn)));
    else
        statusBar()->showMessage(tr("Could not open \"%1\"").arg(QDir::toNativeSeparators(fn)));
}

bool TextEdit::fileSave()
{
    if(fileName.isEmpty())
        return fileSaveAs();
    if(fileName.startsWith(QStringLiteral(":/")))
        return fileSaveAs();

    QTextDocumentWriter writer(fileName);
    writer.setFormat("plaintext");
    bool success = writer.write(textEdit->document());
    if(success) {
        textEdit->document()->setModified(false);
        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    } else {
        statusBar()->showMessage(tr("Could not write to file \"%1\"")
                                 .arg(QDir::toNativeSeparators(fileName)));
        qDebug()<<writer.device()->errorString();
    }
    return success;
}

bool TextEdit::fileSaveAs()
{
    QFileDialog fileDialog(this, tr("Save as..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList mimeTypes;
    mimeTypes << "application/vnd.oasis.opendocument.text" << "text/html" << "text/plain";
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.setDefaultSuffix("odt");
    if(fileDialog.exec() != QDialog::Accepted)
        return false;
    const QString fn = fileDialog.selectedFiles().first();
    setCurrentFileName(fn);
    return fileSave();
}

void TextEdit::insert_new_line()
{
   QTextCursor cursor = textEdit->textCursor();
   cursor.insertText("<br>\n");
}


// Keeping in for later on but not used currently.
/**
void TextEdit::insert_new_link()
{
    QString link = QInputDialog::getText(this, "Insert Link", "Enter a link/URL");
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.insertText("<a href " + link + ">");
        int oldPosition = cursor.position();
        cursor.insertText("</a>");
        cursor.setPosition(oldPosition, QTextCursor::MoveAnchor);
        textEdit->setTextCursor(cursor);
    }
    else
    {
        QString temp = cursor.selectedText();
        temp.prepend("<a href " + link + ">");
        temp.append("</a>");
        int oldAnchor = cursor.anchor();
        int oldPosition = cursor.position();

        cursor.insertText(temp);
        int newPosition, newAnchor;
        if(oldAnchor < oldPosition)
        {
            newAnchor = oldAnchor;
            newPosition = cursor.position();
        }
        else
        {
            newAnchor = cursor.position();
            newPosition = oldPosition;
        }
        cursor.setPosition(newAnchor, QTextCursor::MoveAnchor);
        cursor.setPosition(newPosition, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}
**/

void TextEdit::textBold()
{
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.insertText("<b>");
        int oldPosition = cursor.position();
        cursor.insertText("</b>");
        cursor.setPosition(oldPosition, QTextCursor::MoveAnchor);
        textEdit->setTextCursor(cursor);
    }
    else
    {
        QString temp = cursor.selectedText();
        temp.prepend("<b>");
        temp.append("</b>");
        int oldAnchor = cursor.anchor();
        int oldPosition = cursor.position();

        cursor.insertText(temp);
        int newPosition, newAnchor;
        if(oldAnchor < oldPosition)
        {
            newAnchor = oldAnchor;
            newPosition = cursor.position();
        }
        else
        {
            newAnchor = cursor.position();
            newPosition = oldPosition;
        }
        cursor.setPosition(newAnchor, QTextCursor::MoveAnchor);
        cursor.setPosition(newPosition, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void TextEdit::textItalic()
{
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.insertText("<i>");
        int oldPosition = cursor.position();
        cursor.insertText("</i>");
        cursor.setPosition(oldPosition, QTextCursor::MoveAnchor);
        textEdit->setTextCursor(cursor);
    }
    else
    {
        QString temp = cursor.selectedText();
        temp.prepend("<i>");
        temp.append("</i>");
        int oldAnchor = cursor.anchor();
        int oldPosition = cursor.position();

        cursor.insertText(temp);
        int newPosition, newAnchor;
        if(oldAnchor < oldPosition)
        {
            newAnchor = oldAnchor;
            newPosition = cursor.position();
        }
        else
        {
            newAnchor = cursor.position();
            newPosition = oldPosition;
        }
        cursor.setPosition(newAnchor, QTextCursor::MoveAnchor);
        cursor.setPosition(newPosition, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void TextEdit::textFamily(const QString &f)
{
    QString face = f;
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.insertText("<face " + face + ">");
        int oldPosition = cursor.position();
        cursor.insertText("</face>");
        cursor.setPosition(oldPosition, QTextCursor::MoveAnchor);
        textEdit->setTextCursor(cursor);
    }
    else
    {
        QString temp = cursor.selectedText();
        temp.prepend("<face " + face + ">");
        temp.append("</face>");
        int oldAnchor = cursor.anchor();
        int oldPosition = cursor.position();

        cursor.insertText(temp);
        int newPosition, newAnchor;
        if(oldAnchor < oldPosition)
        {
            newAnchor = oldAnchor;
            newPosition = cursor.position();
        }
        else
        {
            newAnchor = cursor.position();
            newPosition = oldPosition;
        }
        cursor.setPosition(newAnchor, QTextCursor::MoveAnchor);
        cursor.setPosition(newPosition, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void TextEdit::textSize(const QString &p)
{
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.insertText("<scale " + p + ">");
        int oldPosition = cursor.position();
        cursor.insertText("</scale>");
        cursor.setPosition(oldPosition, QTextCursor::MoveAnchor);
        textEdit->setTextCursor(cursor);
    }
    else
    {
        QString temp = cursor.selectedText();
        temp.prepend("<scale " + p + ">");
        temp.append("</scale>");
        int oldAnchor = cursor.anchor();
        int oldPosition = cursor.position();

        cursor.insertText(temp);
        int newPosition, newAnchor;
        if(oldAnchor < oldPosition)
        {
            newAnchor = oldAnchor;
            newPosition = cursor.position();
        }
        else
        {
            newAnchor = cursor.position();
            newPosition = oldPosition;
        }
        cursor.setPosition(newAnchor, QTextCursor::MoveAnchor);
        cursor.setPosition(newPosition, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void TextEdit::textStyle(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();

    if(styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;
            case 2:
                style = QTextListFormat::ListCircle;
                break;
            case 3:
                style = QTextListFormat::ListSquare;
                break;
            case 4:
                style = QTextListFormat::ListDecimal;
                break;
            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
            case 7:
                style = QTextListFormat::ListLowerRoman;
                break;
            case 8:
                style = QTextListFormat::ListUpperRoman;
                break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if(cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

void TextEdit::textColor()
{
    QColor col = QColorDialog::getColor();
    if(!col.isValid())
        return;
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.insertText("<color " + col.name() + ">");
        int oldPosition = cursor.position();
        cursor.insertText("</color>");
        cursor.setPosition(oldPosition, QTextCursor::MoveAnchor);
        textEdit->setTextCursor(cursor);
    }
    else
    {
        QString temp = cursor.selectedText();
        temp.prepend("<color " + col.name() + ">");
        temp.append("</color>");
        int oldAnchor = cursor.anchor();
        int oldPosition = cursor.position();

        cursor.insertText(temp);
        int newPosition, newAnchor;
        if(oldAnchor < oldPosition)
        {
            newAnchor = oldAnchor;
            newPosition = cursor.position();
        }
        else
        {
            newAnchor = cursor.position();
            newPosition = oldPosition;
        }
        cursor.setPosition(newAnchor, QTextCursor::MoveAnchor);
        cursor.setPosition(newPosition, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void TextEdit::textShadow(const QString &p)
{
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.insertText("<shadow " + p + ">");
        int oldPosition = cursor.position();
        cursor.insertText("</shadow>");
        cursor.setPosition(oldPosition, QTextCursor::MoveAnchor);
        textEdit->setTextCursor(cursor);
    }
    else
    {
        QString temp = cursor.selectedText();
        temp.prepend("<shadow " + p + ">");
        temp.append("</shadow>");
        int oldAnchor = cursor.anchor();
        int oldPosition = cursor.position();

        cursor.insertText(temp);
        int newPosition, newAnchor;
        if(oldAnchor < oldPosition)
        {
            newAnchor = oldAnchor;
            newPosition = cursor.position();
        }
        else
        {
            newAnchor = cursor.position();
            newPosition = oldPosition;
        }
        cursor.setPosition(newAnchor, QTextCursor::MoveAnchor);
        cursor.setPosition(newPosition, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void TextEdit::textOutline(const QString &p)
{
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection())
    {
        cursor.insertText("<outline " + p + ">");
        int oldPosition = cursor.position();
        cursor.insertText("</outline>");
        cursor.setPosition(oldPosition, QTextCursor::MoveAnchor);
        textEdit->setTextCursor(cursor);
    }
    else
    {
        QString temp = cursor.selectedText();
        temp.prepend("<outline " + p + ">");
        temp.append("</outline>");
        int oldAnchor = cursor.anchor();
        int oldPosition = cursor.position();

        cursor.insertText(temp);
        int newPosition, newAnchor;
        if(oldAnchor < oldPosition)
        {
            newAnchor = oldAnchor;
            newPosition = cursor.position();
        }
        else
        {
            newAnchor = cursor.position();
            newPosition = oldPosition;
        }
        cursor.setPosition(newAnchor, QTextCursor::MoveAnchor);
        cursor.setPosition(newPosition, QTextCursor::KeepAnchor);
        textEdit->setTextCursor(cursor);
    }
}

void TextEdit::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
}

void TextEdit::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

//!@}
