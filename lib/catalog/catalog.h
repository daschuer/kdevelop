/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef CATALOG_H
#define CATALOG_H

#include <q3valuelist.h>
#include <qpair.h>
#include <qvariant.h>
//Added by qt3to4:
#include <Q3CString>


#include "tag.h"
/**
@file catalog.h
Catalog database - the persistant symbol store database.
*/

/**
Catalog objects represent separate symbol databases.
Catalogs can be created/loaded/unloaded dynamically.
To find a symbol in the repository each catalog should be queried.

Persistant symbol store is useful to keep information about code that
never or rarely changes. System libraries are perfect examples of such code.
*/
class Catalog
{
public:
    typedef QPair<Q3CString, QVariant> QueryArgument;

public:
    Catalog();
    virtual ~Catalog();

    bool isValid() const;
    QString dbName() const;
    
    bool enabled() const;
    void setEnabled( bool en );

    virtual void open( const QString& dbName );
    virtual void close();
    virtual void sync();

    Q3ValueList<Q3CString> indexList() const;
    void addIndex( const Q3CString& name );

    void addItem( Tag& tag );

    Tag getItemById( const Q3CString& id );
    Q3ValueList<Tag> query( const Q3ValueList<QueryArgument>& args );

    Q3CString generateId();

private:
   class _Catalog_Private* d;

private:
    Catalog( const Catalog& source );
    void operator = ( const Catalog& source );
};


#endif
