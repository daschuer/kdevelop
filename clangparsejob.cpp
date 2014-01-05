/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

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

#include "clangparsejob.h"

#include <language/backgroundparser/urlparselock.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include "duchain/parsesession.h"
#include "duchain/buildduchainvisitor.h"
#include "duchain/clangtypes.h"

#include "debug.h"
#include "clanglanguagesupport.h"

#include <QReadLocker>
#include <QProcess>
#include <memory>

using namespace KDevelop;

namespace {
// TODO: investigate why this is required to find e.g. stddef.h
KUrl::List defaultIncludes()
{
    static KUrl::List includePaths;

    if (!includePaths.isEmpty()) {
        return includePaths;
    }

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);

    // The following command will spit out a bnuch of information we don't care
    // about before spitting out the include paths.  The parts we care about
    // look like this:
    // #include "..." search starts here:
    // #include <...> search starts here:
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/i486-linux-gnu
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/backward
    //  /usr/local/include
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/include
    //  /usr/include
    // End of search list.
    proc.start("clang++", {"-std=c++11", "-xc++", "-E", "-v", "/dev/null"});
    if (!proc.waitForStarted(1000) || !proc.waitForFinished(1000)) {
        return {};
    }

    // We'll use the following constants to know what we're currently parsing.
    enum Status {
        Initial,
        FirstSearch,
        Includes,
        Finished
    };
    Status mode = Initial;

    foreach(const QString& line, QString::fromLocal8Bit(proc.readAllStandardOutput()).split('\n')) {
        switch (mode) {
            case Initial:
                if (line.indexOf("#include \"...\"") != -1) {
                    mode = FirstSearch;
                }
                break;
            case FirstSearch:
                if (line.indexOf("#include <...>") != -1) {
                    mode = Includes;
                    break;
                }
            case Includes:
                //if (!line.indexOf(QDir::separator()) == -1 && line != "." ) {
                //Detect the include-paths by the first space that is prepended. Reason: The list may contain relative paths like "."
                if (!line.startsWith(" ") ) {
                    // We've reached the end of the list.
                    mode = Finished;
                } else {
                    // This is an include path, add it to the list.
                    includePaths << QDir::cleanPath(line.trimmed());
                }
                break;
            default:
                break;
        }
        if (mode == Finished) {
            break;
        }
    }

    return includePaths;
}

ReferencedTopDUContext createTopContext(const IndexedString& path)
{
    ParsingEnvironmentFile *file = new ParsingEnvironmentFile(path);
    file->setLanguage(ParseSession::languageString());
    file->setModificationRevision(ModificationRevision());
    ReferencedTopDUContext context = new TopDUContext(path, RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
    DUChain::self()->addDocumentChain(context);
    return context;
}

using IncludeLocks = std::vector<std::unique_ptr<UrlParseLock>>;

struct InclusionClientData
{
    IncludeLocks* locks;
    IncludeFileContexts* contexts;
};

void visitInclusions(CXFile file, CXSourceLocation* stack, unsigned stackDepth, CXClientData d)
{
    auto data = static_cast<InclusionClientData*>(d);

    if (!data->contexts->contains(file)) {
        IndexedString indexedInclude(ClangString(clang_getFileName(file)));
        // TODO: only keep parse lock when we actually update this file
        data->locks->emplace_back(new UrlParseLock(indexedInclude));

        DUChainWriteLocker lock;
        ReferencedTopDUContext include = DUChain::self()->chainForDocument(indexedInclude);
        if (!include) {
            include = createTopContext(indexedInclude);
        }
        // TODO: for something like A -> B -> C and C changed, we have to update B and A...
        data->contexts->insert(file, {include, include->parsingEnvironmentFile()->needsUpdate()});
    }

    if (stackDepth) {
        auto included = data->contexts->value(file);
        Q_ASSERT(included.topContext);

        CXFile parentFile;
        uint line, column;
        clang_getFileLocation(stack[0], &parentFile, &line, &column, nullptr);
        auto importer = data->contexts->value(parentFile);
        Q_ASSERT(importer.topContext);
        if (importer.needsUpdate) {
            DUChainWriteLocker lock;
            importer.topContext->addImportedParentContext(included.topContext, CursorInRevision(line, column));
        }
    }
}

}

ClangParseJob::ClangParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: ParseJob(url, languageSupport)
{
    auto item = ICore::self()->projectController()->projectModel()->itemForUrl(url);
    if (item && item->project()->buildSystemManager()) {
        auto bsm = item->project()->buildSystemManager();
        m_includes = bsm->includeDirectories(item);
        m_defines = bsm->defines(item);
    }

    m_includes += defaultIncludes();
}

ClangLanguageSupport* ClangParseJob::clang() const
{
    return static_cast<ClangLanguageSupport*>(languageSupport());
}

void ClangParseJob::run()
{
    UrlParseLock urlLock(document());
    if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
        return;
    }

    ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return;
    }

    KSharedPtr<ParseSession> session;
    ReferencedTopDUContext context;

    {
        DUChainReadLocker lock;
        context = DUChainUtils::standardContextForUrl(document().toUrl());
        if (context) {
            session = KSharedPtr<ParseSession>::dynamicCast(context->ast());
        }
    }

    if (abortRequested()) {
        return;
    }

    if (!session || !session->reparse(contents().contents)) {
        session = new ParseSession(document(), contents().contents, clang()->index(), m_includes, m_defines);
    } else {
        Q_ASSERT(session->url() == document());
        Q_ASSERT(session->unit());
    }

    if (abortRequested() || !session->unit()) {
        return;
    }

    if (!context) {
        DUChainWriteLocker lock;
        context = createTopContext(document());
    } else {
        //TODO: update existing contexts
        DUChainWriteLocker lock;
        context->cleanIfNotEncountered({});
    }

    IncludeLocks includeLocks;
    IncludeFileContexts includedFiles;
    includedFiles.insert(session->file(), {context, true});

    {
        InclusionClientData data{&includeLocks, &includedFiles};
        // FIXME: the use of the UrlParseLock here can lead to deadlocks in cases
        // of include stacks such as this: A -> B -> C; D -> C -> B
        // There, A will lock B and D locks C, and then they cannot proceed...
        clang_getInclusions(session->unit(), &::visitInclusions, &data);
    }

    if (abortRequested() || !session->unit()) {
        return;
    }

    setDuChain(context);
    {
        QReadLocker parseLock(languageSupport()->language()->parseLock());

        if (abortRequested()) {
            return abortJob();
        }

        BuildDUChainVisitor visitor;
        visitor.visit(session.data(), context, includedFiles);
    }

    if (abortRequested()) {
        return abortJob();
    }

    {
        DUChainWriteLocker lock;
        context->setProblems(session->problems());
        context->setFeatures(minimumFeatures());
        if (hasTracker()) {
            // cache the parse session and the contained translation unit for this chain
            // this then allows us to quickly reparse the document if it is changed by
            // the user
            context->setAst(KSharedPtr<IAstContainer>::staticCast(session));
        } else {
            // otherwise no editor component is open for this document and we can dispose
            // the TU to save memory
            context->setAst({});
        }
        ParsingEnvironmentFilePointer file = context->parsingEnvironmentFile();
        Q_ASSERT(file);
        file->setModificationRevision(contents().modification);
        DUChain::self()->updateContextEnvironment( context->topContext(), file.data() );

        foreach (const Include& include, includedFiles) {
            if (!include.needsUpdate || include.topContext == context) {
                continue;
            }
            auto revision = ModificationRevision::revisionForFile(include.topContext->url());
            include.topContext->parsingEnvironmentFile()->setModificationRevision(revision);
        }
    }
    highlightDUChain();
}
