/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include <QAbstractItemModel>

#include "mi/miparser.h"
#include "gdbglobal.h"

namespace GDBDebugger
{
class GDBController;
class TreeItem;
class TreeModel;

class StackManager : public QObject
{
    Q_OBJECT

public:

    StackManager(GDBController* controller);
    virtual ~StackManager();

    void setAutoUpdate(bool);

    GDBController* controller() const;

    TreeModel *model();

private:

    void updateThreads();

Q_SIGNALS:
    void selectThread(const QModelIndex& index);   

public:
    // FIXME: there should be some other way for model
    // to make a given thread selected.
    void selectThreadReally(const QModelIndex& index)
    { emit selectThread(index); }

public Q_SLOTS:
    void slotEvent(event_t e);

private:
    GDBController* controller_;
    bool autoUpdate_;
    TreeModel* model_;
    class DebugUniverse *universe_;
};

}

#endif // STACKMANAGER_H
