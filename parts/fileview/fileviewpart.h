/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILEVIEWPART_H_
#define _FILEVIEWPART_H_

#include <qguardedptr.h>
#include "kdevplugin.h"

class PartWidget;

class FileViewPart : public KDevPlugin
{
    Q_OBJECT

public:
    FileViewPart( QObject *parent, const char *name, const QStringList & );
    ~FileViewPart();

private:
    QGuardedPtr<PartWidget> m_widget;
};

#endif
