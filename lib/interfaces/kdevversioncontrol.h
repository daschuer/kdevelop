/***************************************************************************
 *   Copyright (C) 2001-2003 by KDevelop Authors                           *
 *   kdevelop-devel@kde.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVVERSIONCONTROL_H_
#define _KDEVVERSIONCONTROL_H_

#include <qobject.h>
#include <qstring.h>
#include <qwidget.h>
#include <qmap.h>
#include <qvaluelist.h>

#include "kdevplugin.h"
#include "kdevvcsfileinfoprovider.h"

/**
* This is the abstract base class which encapsulates everything
* necessary for communicating with version control systems.
*/
class KDevVersionControl : public KDevPlugin
{
    Q_OBJECT

public:
    enum State { invalid = -1, canBeCommited = 1, canBeAdded = 2 };

    /**
    * Builds the object.
    *   @p pluginName   this _is_ *** unique *** identintifier between all available version controls.
    *   @p icon     symbolic name for the icon
    *   @p parent   the parent QObject for this version control system
    *   @p name     a name for this object; it may be null since it is usefull for debugging
    */
    KDevVersionControl( const QString& pluginName, const QString& icon, QObject *parent, const char *name );
    /**
    * Destructor. Removes current VCS from the list of available ones.
    */
    virtual ~KDevVersionControl();

    /**
    * Return a pointer to the associated configuration widget for this vcs.
    */
    virtual QWidget* newProjectWidget( QWidget */*parent*/ );
    /**
    * Creates a new project in the passed path @p dir.
    */
    virtual void createNewProject( const QString&/* dir*/ );
    /**
    * Fetch a module from remote repository, so it can be used for importing
    */
    virtual bool fetchFromRepository();
    /**
    * @return the file info provider for this version control (0 if none is available)
    */
    virtual KDevVCSFileInfoProvider *fileInfoProvider() const;
    /**
    * Checks if the directory is valid for this version control (for example
    * CVS may check for the presence of "<dirPath>/CVS/" subdir and something else)
    * @param dirPath absolute path of the directory
    * @return true if the directory is valid for this version control
    *     <b>warning</b>: this returns false by default
    */
    virtual bool isValidDirectory( const QString &dirPath ) const;
    /**
    * @return the unique identifier for this plugin (so it can be used for retrieving
    * it from collections).
    */
    QString uid() const;

signals:
    /**
    * Emitted when the Version Control has finished importing a module from remote
    * repository
    * @param destinationDir is the directory where the module has been fetched
    */
    void finishedFetching( QString destinationDir );
    /**
    * Emitted when files registered into repository have been modified: registered
    * clients can update their state accordingly with the info provided
    * @param modifiedFiles files which have got modified
    * @deprecated
    */
//    void fileStateChanged( const VCSFileInfoMap &modifiedFiles );
};

#endif
