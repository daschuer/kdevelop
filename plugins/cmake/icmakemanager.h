/* This file is part of KDevelop
    Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef ICMAKEMANAGER_H
#define ICMAKEMANAGER_H

#include <QObject>
#include <QPair>

namespace KDevelop
{
class IProject;
}

class ICMakeManager
{
public:
    virtual ~ICMakeManager() {}

    virtual QPair<QString, QString> cacheValue(KDevelop::IProject* project, const QString& id) const=0;

};

Q_DECLARE_INTERFACE( ICMakeManager, "org.kdevelop.ICMakeManager" )

#endif
