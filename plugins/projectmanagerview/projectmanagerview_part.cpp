/* This file is part of KDevelop
   Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
   Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#include "projectmanagerview_part.h"
#include "projectmodel.h"
#include "projectmanagerview.h"
#include "icore.h"

#include "iproject.h"
#include "iprojectfilemanager.h"
#include "ibuildsystemmanager.h"
#include "iuicontroller.h"
#include "idocumentcontroller.h"
#include "iprojectbuilder.h"
#include "iprojectcontroller.h"
#include "importprojectjob.h"
#include "context.h"

#include "ui_builddialog.h"

#include <kservicetypetrader.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdialog.h>
#include <kaboutdata.h>
#include <kplugininfo.h>

#include <kparts/componentfactory.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtCore/QSignalMapper>

using namespace KDevelop;

K_PLUGIN_FACTORY(ProjectManagerFactory, registerPlugin<ProjectManagerViewPart>(); )
K_EXPORT_PLUGIN(ProjectManagerFactory("kdevprojectmanagerview"))

class KDevProjectManagerViewFactory: public KDevelop::IToolViewFactory
{
    public:
        KDevProjectManagerViewFactory( ProjectManagerViewPart *part ): m_part( part )
        {}
        virtual QWidget* create( QWidget *parent = 0 )
        {
            return new ProjectManagerView( m_part, parent );
        }
        virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
        {
            return Qt::RightDockWidgetArea;
        }
    private:
        ProjectManagerViewPart *m_part;
};

class ProjectManagerViewPartPrivate
{
public:
    ProjectManagerViewPartPrivate()
    {}
    KDevProjectManagerViewFactory *factory;
    QList<KDevelop::ProjectBaseItem*> ctxProjectItemList;
    KAction* m_buildAll;
    KAction* m_build;
    KAction* m_install;
    KAction* m_clean;
};

ProjectManagerViewPart::ProjectManagerViewPart( QObject *parent, const QVariantList& )
        : IPlugin( ProjectManagerFactory::componentData(), parent ), d(new ProjectManagerViewPartPrivate)
{
    d->factory = new KDevProjectManagerViewFactory( this );
    core()->uiController()->addToolView( "Project Manager", d->factory );
    d->m_buildAll = new KAction( i18n("Build all Projects"), this );
    connect( d->m_buildAll, SIGNAL(triggered()), this, SLOT(buildAllProjects()) );
    actionCollection()->addAction( "project_buildall", d->m_buildAll );
    d->m_build = new KAction( i18n("Build"), this );
    d->m_build->setShortcut( Qt::Key_F8 );
    connect( d->m_build, SIGNAL(triggered()), this, SLOT(buildProjectItems()) );
    actionCollection()->addAction( "project_build", d->m_build );
    d->m_install = new KAction( i18n("Install"), this );
    connect( d->m_install, SIGNAL(triggered()), this, SLOT(installProjectItems()) );
    actionCollection()->addAction( "project_install", d->m_install );
    d->m_clean = new KAction( i18n("Clean"), this );
    connect( d->m_clean, SIGNAL(triggered()), this, SLOT(cleanProjectItems()) );
    actionCollection()->addAction( "project_clean", d->m_clean );
    setXMLFile( "kdevprojectmanagerview.rc" );
}

ProjectManagerViewPart::~ProjectManagerViewPart()
{
    delete d;
}

void ProjectManagerViewPart::unload()
{
    core()->uiController()->removeToolView(d->factory);
}

QPair<QString, QList<QAction*> > ProjectManagerViewPart::requestContextMenuActions( KDevelop::Context* context )
{
    if( context->type() != KDevelop::Context::ProjectItemContext )
        return IPlugin::requestContextMenuActions( context );

    QList<QAction*> actions;
    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
    QList<KDevelop::ProjectBaseItem*> items = ctx->items();

    if( items.isEmpty() )
        return IPlugin::requestContextMenuActions( context );

    bool closeProjectsAdded = false;
    bool buildTargetsAdded = false;
    foreach( ProjectBaseItem* item, items )
    {
        d->ctxProjectItemList << item;
        KDevelop::ProjectFolderItem *prjitem = dynamic_cast<KDevelop::ProjectFolderItem*>(item);
        if ( !closeProjectsAdded && prjitem && prjitem->isProjectRoot() )
        {
            KAction* close = new KAction( i18n( "Close projects" ), this );
            connect( close, SIGNAL(triggered()), this, SLOT(closeProjects()) );
            actions << close;
            closeProjectsAdded = true;
        }
        else if ( KDevelop::ProjectFolderItem *folder = item->folder() )
        {
        }
        else if ( KDevelop::ProjectFileItem *file = item->file() )
        {
        }
        else if ( !buildTargetsAdded && item->target() )
        {
            KDevelop::ProjectTargetItem* target = item->target();
            KAction* action = new KAction( i18n( "Build target(s)" ), this );
            connect( action, SIGNAL( triggered() ), this, SLOT(buildProjectsFromContextMenu()) );
            actions << action;
            action = new KAction( i18n( "Install target(s)" ), this );
            connect( action, SIGNAL( triggered() ), this, SLOT(installProjectsFromContextMenu()) );
            actions << action;
            action = new KAction( i18n( "Clean target(s)" ), this );
            connect( action, SIGNAL( triggered() ), this, SLOT(cleanProjectsFromContextMenu()) );
            actions << action;
            buildTargetsAdded = true;
        }
    }
    return qMakePair(QString("Project Management"), actions);
}


KDevelop::IProjectBuilder* ProjectManagerViewPart::getProjectBuilder( KDevelop::ProjectBaseItem* item )
{
    if( !item )
        return 0;
    IProject* project = item->project();
    ProjectFolderItem* prjitem = project->projectItem();
    IPlugin* fmgr = project->managerPlugin();
    IBuildSystemManager* mgr = fmgr->extension<IBuildSystemManager>();
    if( mgr )
    {
        return mgr->builder( prjitem );
    }
    return 0;
}

void ProjectManagerViewPart::executeBuild( KDevelop::ProjectBaseItem* item )
{
    IProjectBuilder* builder = getProjectBuilder( item );
    kDebug(9511) << "Building item:" << item->text();
    if( builder )
        builder->build( item );
}

void ProjectManagerViewPart::executeClean( KDevelop::ProjectBaseItem* item )
{
    IProjectBuilder* builder = getProjectBuilder( item );
    kDebug(9511) << "Cleaning item:" << item->text();
    if( builder )
        builder->clean( item );
}

void ProjectManagerViewPart::executeInstall( KDevelop::ProjectBaseItem* item )
{
    IProjectBuilder* builder = getProjectBuilder( item );
    kDebug(9511) << "Installing item:" << item->text();
    if( builder )
        builder->install( item );
}

void ProjectManagerViewPart::closeProjects()
{
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        KDevelop::ProjectFolderItem *prjitem = dynamic_cast<KDevelop::ProjectFolderItem*>(item);
        if ( prjitem && prjitem->isProjectRoot() )
        {
            core()->projectController()->closeProject( prjitem->project() );
        }
    }
    d->ctxProjectItemList.clear();
}


void ProjectManagerViewPart::installProjectsFromContextMenu()
{
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        executeInstall( item );
    }
    d->ctxProjectItemList.clear();
}

void ProjectManagerViewPart::cleanProjectsFromContextMenu()
{
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        executeClean( item );
    }
    d->ctxProjectItemList.clear();
}

void ProjectManagerViewPart::buildProjectsFromContextMenu()
{
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        executeBuild( item );
    }
    d->ctxProjectItemList.clear();
}

void ProjectManagerViewPart::buildAllProjects()
{
    foreach( KDevelop::IProject* project, core()->projectController()->projects() )
    {
        executeBuild( project->projectItem() );
    }
}

QList<KDevelop::ProjectBaseItem*> ProjectManagerViewPart::getCheckedItems()
{
    KDevelop::ProjectModel* model = core()->projectController()->projectModel();
    QList<KDevelop::ProjectBaseItem*> items;
    for( int row = 0; row < model->rowCount(); row++ )
    {
        if( model->item( row )->checkState() == Qt::Checked )
        {
            items << dynamic_cast<KDevelop::ProjectBaseItem*>(model->item( row ) );
        }else if( model->item( row )->checkState() == Qt::PartiallyChecked )
        {
            items += recurseAndFetchCheckedItems(
                    dynamic_cast<KDevelop::ProjectBaseItem*>(model->item( row ) ) );
        }
    }
    return items;
}

void ProjectManagerViewPart::installProjectItems()
{
    foreach( KDevelop::ProjectBaseItem* item, getCheckedItems() )
    {
        executeInstall( item );
    }
}

void ProjectManagerViewPart::cleanProjectItems()
{
    foreach( KDevelop::ProjectBaseItem* item, getCheckedItems() )
    {
        executeInstall( item );
    }
}

void ProjectManagerViewPart::buildProjectItems()
{
    foreach( KDevelop::ProjectBaseItem* item, getCheckedItems() )
    {
        executeBuild( item );
    }
}

QList<KDevelop::ProjectBaseItem*>
ProjectManagerViewPart::recurseAndFetchCheckedItems( KDevelop::ProjectBaseItem* item )
{
    QList<KDevelop::ProjectBaseItem*> items;
    if( item->folder() )
    {
        if( item->checkState() == Qt::PartiallyChecked )
        {
            foreach( KDevelop::ProjectFolderItem* folderItem, item->folderList() )
            {
                items += recurseAndFetchCheckedItems( folderItem );
            }
            foreach( KDevelop::ProjectTargetItem* targetItem, item->targetList() )
            {
                if( targetItem->checkState() == Qt::Checked )
                {
                    items << targetItem;
                }
            }
        }else if( item->checkState() == Qt::Checked )
        {
            items << item;
        }
    }else if( item->target() && item->checkState() == Qt::Checked )
    {
        items << item;
    }
    return items;
}

#include "projectmanagerview_part.moc"

