/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "noprojectcustomincludepaths.h"

#include "ui_noprojectcustomincludepaths.h"

#include <KLocalizedString>

#include <QFileDialog>
#include <QUrl>


NoProjectCustomIncludePaths::NoProjectCustomIncludePaths(QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui::CustomIncludePaths)
{
    m_ui->setupUi(this);
    m_ui->storageDirectory->setMode(KFile::Directory);

    setWindowTitle(i18n("Setup Custom Include Paths"));

    connect(m_ui->directorySelector, &QPushButton::clicked, this, &NoProjectCustomIncludePaths::openAddIncludeDirectoryDialog);
}

void NoProjectCustomIncludePaths::setStorageDirectory(const QString& path)
{
    m_ui->storageDirectory->setUrl(QUrl::fromLocalFile(path));
}

QString NoProjectCustomIncludePaths::storageDirectory() const
{
    return m_ui->storageDirectory->url().toLocalFile();
}

void NoProjectCustomIncludePaths::appendCustomIncludePath(const QString& path)
{
    m_ui->customIncludePaths->appendPlainText(path);
}

QStringList NoProjectCustomIncludePaths::customIncludePaths() const
{
    const QString pathsText = m_ui->customIncludePaths->document()->toPlainText();
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const QStringList paths = pathsText.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
#else
    const QStringList paths = pathsText.split(QLatin1Char('\n'), QString::SkipEmptyParts);
#endif
    return paths;
}

void NoProjectCustomIncludePaths::setCustomIncludePaths(const QStringList& paths)
{
    m_ui->customIncludePaths->setPlainText(paths.join(QLatin1Char('\n')));
}

void NoProjectCustomIncludePaths::openAddIncludeDirectoryDialog()
{
    const QString dirName = QFileDialog::getExistingDirectory(this, i18n("Select directory to include"));
    if (dirName.isEmpty())
        return;

    appendCustomIncludePath(dirName);
}
