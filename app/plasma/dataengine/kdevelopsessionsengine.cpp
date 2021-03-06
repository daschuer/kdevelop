/*****************************************************************************
*   Copyright (C) 2012 by Eike Hein <hein@kde.org>                           *
*   Copyright (C) 2011 by Shaun Reich <shaun.reich@kdemail.net>              *
*   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                    *
*                                                                            *
*   This program is free software; you can redistribute it and/or            *
*   modify it under the terms of the GNU General Public License as           *
*   published by the Free Software Foundation; either version 2 of           *
*   the License, or (at your option) any later version.                      *
*                                                                            *
*   This program is distributed in the hope that it will be useful,          *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*   GNU General Public License for more details.                             *
*                                                                            *
*   You should have received a copy of the GNU General Public License        *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
*****************************************************************************/

#include "kdevelopsessionsengine.h"
#include "kdevelopsessionsservice.h"

#include <KDirWatch>
#include <KConfig>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

KDevelopSessionsEngine::KDevelopSessionsEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args),
      m_dirWatch(nullptr)
{
    init();
}

KDevelopSessionsEngine::~KDevelopSessionsEngine()
{
}

void KDevelopSessionsEngine::init()
{
    m_dirWatch = new KDirWatch( this );

    m_sessionDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kdevelop/sessions/");
    m_dirWatch->addDir(m_sessionDir, KDirWatch::WatchSubDirs);

    connect(m_dirWatch, &KDirWatch::dirty, this, &KDevelopSessionsEngine::sessionSourceChanged);
    updateSessions();
}

Plasma::Service *KDevelopSessionsEngine::serviceForSource(const QString &source)
{
    return new KDevelopSessionsService( this, source );
}

QStringList KDevelopSessionsEngine::findSessions()
{
    QStringList sessionrcs;
    QDir d(m_sessionDir);
    const auto dirEntries = d.entryList(QDir::Dirs);
    for (const QString& sessionDir : dirEntries) {
        QDir sd(d.absoluteFilePath(sessionDir));
        QString path(sd.filePath(QStringLiteral("sessionrc")));
        if(QFile::exists(path)) {
            sessionrcs += path;
        }
    }
    return sessionrcs;
}

void KDevelopSessionsEngine::sessionSourceChanged(const QString& path)
{
    // This is the case if a session gets added/deleted
    if (m_sessionDir == path) {
        updateSessions();
    } else {
        // If a sessionrc file got changed we reload the config too
        QFileInfo info(path);
        if (info.isFile() && info.fileName() == QLatin1String("sessionrc")) {
            updateSessions();
        }
    }
}

void KDevelopSessionsEngine::updateSessions()
{
    const QStringList sessionrcs = findSessions();

    QHash<QString, Session> sessions;

    for (auto& sessionrc : sessionrcs) {
        KConfig cfg(sessionrc, KConfig::SimpleConfig);
        Session session;
        session.hash = QFileInfo(sessionrc).dir().dirName();
        session.name = cfg.group( "" ).readEntry( "SessionName", "" );
        session.description = cfg.group( "" ).readEntry( "SessionPrettyContents", "" );

        sessions.insert(session.hash, session);
    }

    for (const Session& session : qAsConst(sessions)) {
        auto sessionIt = m_currentSessions.constFind(session.hash);
        if (sessionIt == m_currentSessions.constEnd()) {
            // Publish new session.

            m_currentSessions.insert( session.hash, session );
            setData( session.hash, QStringLiteral("sessionName"), session.name );
            setData( session.hash, QStringLiteral("sessionString"), session.description );
        }
        else
        {
            // Publish data changes for older sessions.

            Session oldSession(*sessionIt);

            bool modified = false;

            if ( session.name != oldSession.name )
            {
                oldSession.name = session.name;
                modified = true;
                setData( session.hash, QStringLiteral("sessionName"), session.name );
            }

            if ( session.description != oldSession.description )
            {
                oldSession.description = session.description;
                modified = true;
                setData( session.hash, QStringLiteral("sessionString"), session.description );
            }

            if ( modified )
                m_currentSessions.insert( oldSession.hash, oldSession );
        }
    }

    QHash<QString, Session>::iterator it3 = m_currentSessions.begin();

    while ( it3 != m_currentSessions.end() )
    {
        const Session& session = it3.value();

        if ( !sessions.contains( session.hash ) )
        {
            removeSource( session.hash );
            it3 = m_currentSessions.erase( it3 );
        }
        else
            ++it3;
    }
}

K_EXPORT_PLASMA_DATAENGINE_WITH_JSON(kdevelopsessionsengine, KDevelopSessionsEngine, "plasma-dataengine-kdevelopsessions.json")

#include "kdevelopsessionsengine.moc"
