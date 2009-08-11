/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "modificationrevision.h"

#include <QString>
#include <QFileInfo>

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#if defined(Q_CC_MSVC)
#include <hash_map>
#else
// DEPRECATED (should use unordered_map), but needed to support gcc 3.4 and early 4.x
#include <ext/hash_map>
#endif

namespace std {
#if defined(Q_CC_MSVC)
  using namespace stdext;
#else
  using namespace __gnu_cxx;
#endif
}


#include "editorintegrator.h"
#include "hashedstring.h"
#include "../duchain/indexedstring.h"
#include "modificationrevisionset.h"
#include <sys/time.h>

const int cacheModTimeForSeconds = 2;

namespace KDevelop {

struct IndexedStringHash {
  uint operator() (const KDevelop::IndexedString& str) const {
    return str.hash();
  }
  
  #ifdef Q_CC_MSVC
  bool operator() (const KDevelop::IndexedString& lhs, const KDevelop::IndexedString& rhs) const {
    return lhs < rhs;
  }
    enum
        {   // parameters for hash table
        bucket_size = 4,    // 0 < bucket_size
        min_buckets = 8};   // min_buckets = 2 ^^ N, 0 < N
  #endif
};

QMutex fileModificationTimeCacheMutex(QMutex::Recursive);

struct FileModificationCache {
  timeval m_readTime;
  QDateTime m_modificationTime;
};
#ifdef Q_CC_MSVC
    typedef stdext::hash_map<KDevelop::IndexedString, FileModificationCache, IndexedStringHash> FileModificationMap;
#else    
    typedef __gnu_cxx::hash_map<KDevelop::IndexedString, FileModificationCache, IndexedStringHash> FileModificationMap;
#endif

FileModificationMap& fileModificationCache() {
  static FileModificationMap cache;
  return cache;
}

QDateTime fileModificationTimeCached( const IndexedString& fileName ) {
  QMutexLocker lock(&fileModificationTimeCacheMutex);
  
  timeval currentTime;
  gettimeofday(&currentTime, 0);
  
  FileModificationMap::const_iterator it = fileModificationCache().find( fileName );
  if( it != fileModificationCache().end() ) {
    ///Use the cache for X seconds
    timeval  age;
    timersub(&currentTime, &(*it).second.m_readTime, &age);
    if( age.tv_sec < cacheModTimeForSeconds )
      return (*it).second.m_modificationTime;
  }

  QFileInfo fileInfo( fileName.str() );
  fileModificationCache()[fileName].m_readTime = currentTime;
  fileModificationCache()[fileName].m_modificationTime = fileInfo.lastModified();
  return fileInfo.lastModified();
}

void ModificationRevision::clearModificationCache(const IndexedString& fileName) {
  ModificationRevisionSet::clearCache();
  
  QMutexLocker lock(&fileModificationTimeCacheMutex);

  FileModificationMap::iterator it = fileModificationCache().find(fileName);
  if(it != fileModificationCache().end())
    fileModificationCache().erase(it);
}

ModificationRevision ModificationRevision::revisionForFile(const IndexedString& url) {

  ModificationRevision ret(fileModificationTimeCached(url));

  KTextEditor::Document* doc = EditorIntegrator::documentForUrl(url);
  if( doc ) {
    KTextEditor::SmartInterface* smart =   dynamic_cast<KTextEditor::SmartInterface*>(doc);
    if( smart )
      ret.revision = smart->currentRevision();
  }
  
  return ret;
}

ModificationRevision::ModificationRevision( const QDateTime& modTime , int revision_ ) : modificationTime(modTime.toTime_t()), revision(revision_) {
}

bool ModificationRevision::operator <( const ModificationRevision& rhs ) const {
  return modificationTime < rhs.modificationTime || (modificationTime == rhs.modificationTime && revision < rhs.revision);
}

bool ModificationRevision::operator ==( const ModificationRevision& rhs ) const {
  return modificationTime == rhs.modificationTime && revision == rhs.revision;
}

bool ModificationRevision::operator !=( const ModificationRevision& rhs ) const {
  return modificationTime != rhs.modificationTime || revision != rhs.revision;
}

QString ModificationRevision::toString() const {
  return QString("%1 (rev %2)").arg(QDateTime::fromTime_t(modificationTime).time().toString()).arg(revision);
}

}

kdbgstream& operator<< (kdbgstream& s, const KDevelop::ModificationRevision& rev) {
  s << rev.toString();
  return s;
}
