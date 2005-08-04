/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef SEARCHVIEW_H
#define SEARCHVIEW_H

#include <qwidget.h>
//Added by qt3to4:
#include <QFocusEvent>

class DocumentationPart;
class KLineEdit;
class KComboBox;
class KListView;
class KPushButton;
class KProcess;
class Q3ListViewItem;

class SearchView: public QWidget
{
    Q_OBJECT
public:
    SearchView(DocumentationPart *part, QWidget *parent = 0, const char *name = 0);
    ~SearchView();
    
public slots:
    void search();
    void setSearchTerm(const QString &term);
    void askSearchTerm();
    
protected slots:
    void updateConfig();
    void updateIndex();

    void htsearchStdout(KProcess *, char *buffer, int len);
    void htsearchExited(KProcess *);
    void executed(Q3ListViewItem *item);
    
    void itemMouseButtonPressed(int button, Q3ListViewItem *item, const QPoint &pos, int c);

protected:
    virtual void focusInEvent(QFocusEvent *e);
    
    void runHtdig(const QString &arg);
    void analyseSearchResults();

private:
    DocumentationPart *m_part;
    
    KLineEdit *m_edit;
    KComboBox *m_searchMethodBox;
    KComboBox *m_sortMethodBox;
    KListView *m_view;
    KPushButton *m_configButton;
    KPushButton *m_indexButton;
    KPushButton *m_goSearchButton;
    
    QString searchResult;
};

#endif
