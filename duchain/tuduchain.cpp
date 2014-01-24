/*
 *    This file is part of KDevelop
 *
 *    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#include "tuduchain.h"

//BEGIN DeclType

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevDeclaration(CK, isClassMember)>::type>
{
    typedef Declaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevForwardDeclaration(CK, isDefinition)>::type>
{
    typedef ForwardDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevClassDeclaration(CK, isDefinition)>::type>
{
    typedef ClassDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevClassFunctionDeclaration(CK, isDefinition)>::type>
{
    typedef ClassFunctionDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevFunctionDeclaration(CK, isDefinition)>::type>
{
    typedef FunctionDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevFunctionDefinition(CK, isDefinition)>::type>
{
    typedef FunctionDefinition Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevNamespaceAliasDeclaration(CK, isDefinition)>::type>
{
    typedef NamespaceAliasDeclaration Type;
};

template<CXCursorKind CK, bool isDefinition, bool isClassMember>
struct DeclType<CK, isDefinition, isClassMember,
    typename std::enable_if<CursorKindTraits::isKDevClassMemberDeclaration(CK, isClassMember)>::type>
{
    typedef ClassMemberDeclaration Type;
};
//END DeclType

TUDUChain::TUDUChain(CXTranslationUnit tu, CXFile file, const IncludeFileContexts& includes, const bool update)
: m_file(file)
, m_includes(includes)
, m_parentContext(nullptr)
, m_update(update)
{
    CXCursor tuCursor = clang_getTranslationUnitCursor(tu);
    CurrentContext parent(includes[file]);
    m_parentContext = &parent;
    clang_visitChildren(tuCursor, &visitCursor, this);

    TopDUContext *top = m_parentContext->context->topContext();
    if (m_update) {
        DUChainWriteLocker lock;
        top->deleteUsesRecursively();
    }
    for (const auto &contextUses : m_uses) {
        for (const auto &cursor : contextUses.second) {
            auto referenced = clang_getCursorReferenced(cursor);
            auto used = findDeclaration(referenced, includes);
            if (!used) {
                continue;
            }
            auto useRange = ClangRange(clang_getCursorReferenceNameRange(cursor, CXNameRange_WantSinglePiece, 0)).toRangeInRevision();
            DUChainWriteLocker lock;
            auto usedIndex = top->indexForUsedDeclaration(used.data());
            contextUses.first->createUse(usedIndex, useRange);
        }
    }
}

CXChildVisitResult TUDUChain::visitCursor(CXCursor cursor, CXCursor parent, CXClientData data)
{
    TUDUChain *thisPtr = static_cast<TUDUChain*>(data);

    auto location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getFileLocation(location, &file, nullptr, nullptr, nullptr);
    if (file != thisPtr->m_file) {
        return CXChildVisit_Continue;
    }

#define UseCursorKind(CursorKind, ...) case CursorKind: return thisPtr->dispatchCursor<CursorKind>(__VA_ARGS__);
    switch (clang_getCursorKind(cursor))
    {
    UseCursorKind(CXCursor_UnexposedDecl, cursor, parent);
    UseCursorKind(CXCursor_StructDecl, cursor, parent);
    UseCursorKind(CXCursor_UnionDecl, cursor, parent);
    UseCursorKind(CXCursor_ClassDecl, cursor, parent);
    UseCursorKind(CXCursor_EnumDecl, cursor, parent);
    UseCursorKind(CXCursor_FieldDecl, cursor, parent);
    UseCursorKind(CXCursor_EnumConstantDecl, cursor, parent);
    UseCursorKind(CXCursor_FunctionDecl, cursor, parent);
    UseCursorKind(CXCursor_VarDecl, cursor, parent);
    UseCursorKind(CXCursor_ParmDecl, cursor, parent);
    UseCursorKind(CXCursor_TypedefDecl, cursor, parent);
    UseCursorKind(CXCursor_CXXMethod, cursor, parent);
    UseCursorKind(CXCursor_Namespace, cursor, parent);
    UseCursorKind(CXCursor_Constructor, cursor, parent);
    UseCursorKind(CXCursor_Destructor, cursor, parent);
    UseCursorKind(CXCursor_ConversionFunction, cursor, parent);
    UseCursorKind(CXCursor_TemplateTypeParameter, cursor, parent);
    UseCursorKind(CXCursor_NonTypeTemplateParameter, cursor, parent);
    UseCursorKind(CXCursor_TemplateTemplateParameter, cursor, parent);
    UseCursorKind(CXCursor_FunctionTemplate, cursor, parent);
    UseCursorKind(CXCursor_ClassTemplate, cursor, parent);
    UseCursorKind(CXCursor_ClassTemplatePartialSpecialization, cursor, parent);
    UseCursorKind(CXCursor_TypeRef, cursor);
    UseCursorKind(CXCursor_CXXBaseSpecifier, cursor);
    UseCursorKind(CXCursor_TemplateRef, cursor);
    UseCursorKind(CXCursor_NamespaceRef, cursor);
    UseCursorKind(CXCursor_MemberRef, cursor);
    UseCursorKind(CXCursor_LabelRef, cursor);
    UseCursorKind(CXCursor_OverloadedDeclRef, cursor);
    UseCursorKind(CXCursor_VariableRef, cursor);
    UseCursorKind(CXCursor_DeclRefExpr, cursor);
    UseCursorKind(CXCursor_MemberRefExpr, cursor);
    default: return CXChildVisit_Recurse;
    }
}

KDevelop::RangeInRevision TUDUChain::makeContextRange(CXCursor cursor) const
{
    auto start = clang_getRangeEnd(clang_Cursor_getSpellingNameRange(cursor, 0, 0));
    auto end = clang_getRangeEnd(clang_getCursorExtent(cursor));
    return {ClangLocation(start), ClangLocation(end)};
}

KDevelop::Identifier TUDUChain::makeId(CXCursor cursor) const
{
    return Identifier(IndexedString(ClangString(clang_getCursorSpelling(cursor))));
}

QByteArray TUDUChain::makeComment(CXComment comment) const
{
    auto kind = clang_Comment_getKind(comment);
    if (kind == CXComment_Text)
        return {ClangString(clang_TextComment_getText(comment))};

    QByteArray text;
    int numChildren = clang_Comment_getNumChildren(comment);
    for (int i = 0; i < numChildren; ++i)
        text += makeComment(clang_Comment_getChild(comment, i));
    return text;
}
