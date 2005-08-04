/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <klineedit.h>
#include <q3listview.h>
#include <kcompletionbox.h>

#include "kcomboview.h"

KComboView::KComboView( bool rw, int defaultWidth, QWidget* parent, const char* name )
    :QComboView(rw, parent, name), m_defaultWidth(defaultWidth)
{
    if (rw)
    {
        KLineEdit *ed = new KLineEdit(this, "combo edit");
        ed->setCompletionMode(KGlobalSettings::CompletionPopup);
        ed->setCompletionObject(&m_comp);
        ed->completionBox()->setHScrollBarMode(Q3ListBox::Auto);
        setLineEdit(ed);
    }
    setMinimumWidth(defaultWidth);
}

void KComboView::addItem(Q3ListViewItem *it)
{
    m_comp.addItem(it->text(0));
}

void KComboView::removeItem(Q3ListViewItem *it)
{
    if (it == currentItem())
    {
        setCurrentItem(0);
        setCurrentText(m_defaultText);
    }
    m_comp.removeItem(it->text(0));
    delete it;
}

void KComboView::renameItem(Q3ListViewItem *it, const QString &newName)
{
    m_comp.removeItem(it->text(0));
    it->setText(0, newName);
    m_comp.addItem(newName);
}

void KComboView::clear( )
{
    m_comp.clear();
    QComboView::clear();

    setCurrentText(m_defaultText);
}

int KComboView::defaultWidth( )
{
    return m_defaultWidth;
}

void KComboView::setDefaultText( const QString & text )
{
    m_defaultText = text;
}

#include "kcomboview.moc"
