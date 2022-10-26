
// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "indexer.hh"

#include "clang_tu.hh"
#include "log.hh"
#include "pipeline.hh"
#include "platform.hh"
#include "sema_manager.hh"

#include <clang/AST/AST.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/MultiplexConsumer.h>
#include <clang/Index/IndexDataConsumer.h>
#include <clang/Index/IndexingAction.h>
#include <clang/Index/USRGeneration.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/Support/CrashRecoveryContext.h>
#include <llvm/Support/Path.h>

#include <algorithm>
#include <inttypes.h>
#include <map>
#include <unordered_set>

using namespace clang;

namespace ccls {
namespace {

GroupMatch *multiVersionMatcher;

struct File {
  std::string path;
  int64_t mtime;
  std::string content;
  std::unique_ptr<IndexFile> db;
};

struct IndexParam {
  std::unordered_map<FileID, File> uid2file;
  std::unordered_map<FileID, bool> uid2multi;
  struct DeclInfo {
    Usr usr;
    std::string short_name;
    std::string qualified;
  };
  std::unordered_map<const Decl *, DeclInfo> decl2Info;

  VFS &vfs;
  ASTContext *ctx;
  bool no_linkage;
  IndexParam(VFS &vfs, bool no_linkage) : vfs(vfs), no_linkage(no_linkage) {}

  void seenFile(FileID fid) {
    // If this is the first time we have seen the file (ignoring if we are
    // generating an index for it):
    auto [it, inserted] = uid2file.try_emplace(fid);
    if (inserted) {
      const FileEntry *fe = ctx->getSourceManager().getFileEntryForID(fid);
      if (!fe)
        return;
      std::string path = pathFromFileEntry(*fe);
      it->second.path = path;
      it->second.mtime = fe->getModificationTime();
      if (!it->second.mtime)
        if (auto tim = lastWriteTime(path))
          it->second.mtime = *tim;
      if (std::optional<std::string> content = readContent(path))
        it->second.content = *content;

      if (!vfs.stamp(path, it->second.mtime, no_linkage ? 3 : 1))
        return;
      it->second.db =
          std::make_unique<IndexFile>(path, it->second.content, no_linkage);
    }
  }

  IndexFile *consumeFile(FileID fid) {
    seenFile(fid);
    return uid2file[fid].db.get();
  }

  bool useMultiVersion(FileID fid) {
    auto it = uid2multi.try_emplace(fid);
    if (it.second)
      if (const FileEntry *fe = ctx->getSourceManager().getFileEntryForID(fid))
        it.first->second = multiVersionMatcher->matches(pathFromFileEntry(*fe));
    return it.first->second;
  }
};

StringRef getSourceInRange(const SourceManager &sm, const LangOptions &langOpts,
                           SourceRange sr) {
  SourceLocation bloc = sr.getBegin(), eLoc = sr.getEnd();
  std::pair<FileID, unsigned> bInfo = sm.getDecomposedLoc(bloc),
                              eInfo = sm.getDecomposedLoc(eLoc);
  bool invalid = false;
  StringRef buf = sm.getBufferData(bInfo.first, &invalid);
  if (invalid)
    return "";
  return buf.substr(bInfo.second,
                    eInfo.second +
                        Lexer::MeasureTokenLength(eLoc, sm, langOpts) -
                        bInfo.second);
}

Kind getKind(const Decl *d, SymbolKind &kind) {
  switch (d->getKind()) {
  case Decl::LinkageSpec:
    return Kind::Invalid;
  case Decl::Namespace:
  case Decl::NamespaceAlias:
    kind = SymbolKind::Namespace;
    return Kind::Type;
  case Decl::ObjCCategory:
  case Decl::ObjCCategoryImpl:
  case Decl::ObjCImplementation:
  case Decl::ObjCInterface:
  case Decl::ObjCProtocol:
    kind = SymbolKind::Interface;
    return Kind::Type;
  case Decl::ObjCMethod:
    kind = SymbolKind::Method;
    return Kind::Func;
  case Decl::ObjCProperty:
    kind = SymbolKind::Property;
    return Kind::Type;
  case Decl::ClassTemplate:
    kind = SymbolKind::Class;
    return Kind::Type;
  case Decl::FunctionTemplate:
    kind = SymbolKind::Function;
    return Kind::Func;
  case Decl::TypeAliasTemplate:
    kind = SymbolKind::TypeAlias;
    return Kind::Type;
  case Decl::VarTemplate:
    kind = SymbolKind::Variable;
    return Kind::Var;
  case Decl::TemplateTemplateParm:
    kind = SymbolKind::TypeParameter;
    return Kind::Type;
  case Decl::Enum:
    kind = SymbolKind::Enum;
    return Kind::Type;
  case Decl::CXXRecord:
  case Decl::Record:
    kind = SymbolKind::Class;
    // spec has no Union, use Class
    if (auto *rd = dyn_cast<RecordDecl>(d))
      if (rd->getTagKind() == TTK_Struct)
        kind = SymbolKind::Struct;
    return Kind::Type;
  case Decl::ClassTemplateSpecialization:
  case Decl::ClassTemplatePartialSpecialization:
    kind = SymbolKind::Class;
    return Kind::Type;
  case Decl::TemplateTypeParm:
    kind = SymbolKind::TypeParameter;
    return Kind::Type;
  case Decl::TypeAlias:
  case Decl::Typedef:
  case Decl::UnresolvedUsingTypename:
    kind = SymbolKind::TypeAlias;
    return Kind::Type;
  case Decl::Using:
    kind = SymbolKind::Null; // ignored
    return Kind::Invalid;
  case Decl::Binding:
    kind = SymbolKind::Variable;
    return Kind::Var;
  case Decl::Field:
  case Decl::ObjCIvar:
    kind = SymbolKind::Field;
    return Kind::Var;
  case Decl::Function:
    kind = SymbolKind::Function;
    return Kind::Func;
  case Decl::CXXMethod: {
    const auto *md = cast<CXXMethodDecl>(d);
    kind = md->isStatic() ? SymbolKind::StaticMethod : SymbolKind::Method;
    return Kind::Func;
  }
  case Decl::CXXConstructor:
    kind = SymbolKind::Constructor;
    return Kind::Func;
  case Decl::CXXConversion:
  case Decl::CXXDestructor:
    kind = SymbolKind::Method;
    return Kind::Func;
  case Decl::NonTypeTemplateParm:
    // ccls extension
    kind = SymbolKind::Parameter;
    return Kind::Var;
  case Decl::Var: {
    auto vd = cast<VarDecl>(d);
    if (vd->isStaticDataMember()) {
      kind = SymbolKind::Field;
      return Kind::Var;
    }
    [[fallthrough]];
  }
  case Decl::Decomposition:
    kind = SymbolKind::Variable;
    return Kind::Var;
  case Decl::ImplicitParam:
  case Decl::ParmVar:
    // ccls extension
    kind = SymbolKind::Parameter;
    return Kind::Var;
  case Decl::VarTemplateSpecialization:
  case Decl::VarTemplatePartialSpecialization:
    kind = SymbolKind::Variable;
    return Kind::Var;
  case Decl::EnumConstant:
    kind = SymbolKind::EnumMember;
    return Kind::Var;
  case Decl::UnresolvedUsingValue:
    kind = SymbolKind::Variable;
    return Kind::Var;
  case Decl::TranslationUnit:
    return Kind::Invalid;

  default:
    return Kind::Invalid;
  }
}

LanguageId getDeclLanguage(const Decl *d) {
  switch (d->getKind()) {
  default:
    return LanguageId::C;
  case Decl::ImplicitParam:
  case Decl::ObjCAtDefsField:
  case Decl::ObjCCategory:
  case Decl::ObjCCategoryImpl:
  case Decl::ObjCCompatibleAlias:
  case Decl::ObjCImplementation:
  case Decl::ObjCInterface:
  case Decl::ObjCIvar:
  case Decl::ObjCMethod:
  case Decl::ObjCProperty:
  case Decl::ObjCPropertyImpl:
  case Decl::ObjCProtocol:
  case Decl::ObjCTypeParam:
    return LanguageId::ObjC;
  case Decl::CXXConstructor:
  case Decl::CXXConversion:
  case Decl::CXXDestructor:
  case Decl::CXXMethod:
  case Decl::CXXRecord:
  case Decl::ClassTemplate:
  case Decl::ClassTemplatePartialSpecialization:
  case Decl::ClassTemplateSpecialization:
  case Decl::Friend:
  case Decl::FriendTemplate:
  case Decl::FunctionTemplate:
  case Decl::LinkageSpec:
  case Decl::Namespace:
  case Decl::NamespaceAlias:
  case Decl::NonTypeTemplateParm:
  case Decl::StaticAssert:
  case Decl::TemplateTemplateParm:
  case Decl::TemplateTypeParm:
  case Decl::UnresolvedUsingTypename:
  case Decl::UnresolvedUsingValue:
  case Decl::Using:
  case Decl::UsingDirective:
  case Decl::UsingShadow:
    return LanguageId::Cpp;
  }
}

// clang/lib/AST/DeclPrinter.cpp
QualType getBaseType(QualType t, bool deduce_auto) {
  QualType baseType = t;
  while (!baseType.isNull() && !baseType->isSpecifierType()) {
    if (const PointerType *pTy = baseType->getAs<PointerType>())
      baseType = pTy->getPointeeType();
    else if (const BlockPointerType *bPy = baseType->getAs<BlockPointerType>())
      baseType = bPy->getPointeeType();
    else if (const ArrayType *aTy = dyn_cast<ArrayType>(baseType))
      baseType = aTy->getElementType();
    else if (const VectorType *vTy = baseType->getAs<VectorType>())
      baseType = vTy->getElementType();
    else if (const ReferenceType *rTy = baseType->getAs<ReferenceType>())
      baseType = rTy->getPointeeType();
    else if (const ParenType *pTy = baseType->getAs<ParenType>())
      baseType = pTy->desugar();
    else if (deduce_auto) {
      if (const AutoType *aTy = baseType->getAs<AutoType>())
        baseType = aTy->getDeducedType();
      else
        break;
    } else
      break;
  }
  return baseType;
}

const Decl *getTypeDecl(QualType t, bool *specialization = nullptr) {
  Decl *d = nullptr;
  t = getBaseType(t.getUnqualifiedType(), true);
  const Type *tp = t.getTypePtrOrNull();
  if (!tp)
    return nullptr;

try_again:
  switch (tp->getTypeClass()) {
  case Type::Typedef:
    d = cast<TypedefType>(tp)->getDecl();
    break;
  case Type::ObjCObject:
    d = cast<ObjCObjectType>(tp)->getInterface();
    break;
  case Type::ObjCInterface:
    d = cast<ObjCInterfaceType>(tp)->getDecl();
    break;
  case Type::Record:
  case Type::Enum:
    d = cast<TagType>(tp)->getDecl();
    break;
  case Type::TemplateTypeParm:
    d = cast<TemplateTypeParmType>(tp)->getDecl();
    break;
  case Type::TemplateSpecialization:
    if (specialization)
      *specialization = true;
    if (const RecordType *record = tp->getAs<RecordType>())
      d = record->getDecl();
    else
      d = cast<TemplateSpecializationType>(tp)
              ->getTemplateName()
              .getAsTemplateDecl();
    break;

  case Type::Auto:
  case Type::DeducedTemplateSpecialization:
    tp = cast<DeducedType>(tp)->getDeducedType().getTypePtrOrNull();
    if (tp)
      goto try_again;
    break;

  case Type::InjectedClassName:
    d = cast<InjectedClassNameType>(tp)->getDecl();
    break;

    // FIXME: Template type parameters!

  case Type::Elaborated:
    tp = cast<ElaboratedType>(tp)->getNamedType().getTypePtrOrNull();
    goto try_again;

  default:
    break;
  }
  return d;
}

const Decl *getAdjustedDecl(const Decl *d) {
  while (d) {
    if (auto *r = dyn_cast<CXXRecordDecl>(d)) {
      if (auto *s = dyn_cast<ClassTemplateSpecializationDecl>(r)) {
        if (!s->isExplicitSpecialization()) {
          llvm::PointerUnion<ClassTemplateDecl *,
                             ClassTemplatePartialSpecializationDecl *>
              result = s->getSpecializedTemplateOrPartial();
          if (result.is<ClassTemplateDecl *>())
            d = result.get<ClassTemplateDecl *>();
          else
            d = result.get<ClassTemplatePartialSpecializationDecl *>();
          continue;
        }
      } else if (auto *d1 = r->getInstantiatedFromMemberClass()) {
        d = d1;
        continue;
      }
    } else if (auto *ed = dyn_cast<EnumDecl>(d)) {
      if (auto *d1 = ed->getInstantiatedFromMemberEnum()) {
        d = d1;
        continue;
      }
    }
    break;
  }
  return d;
}

bool validateRecord(const RecordDecl *rd) {
  for (const auto *i : rd->fields()) {
    QualType fqt = i->getType();
    if (fqt->isIncompleteType() || fqt->isDependentType())
      return false;
    if (const RecordType *childType = i->getType()->getAs<RecordType>())
      if (const RecordDecl *child = childType->getDecl())
        if (!validateRecord(child))
          return false;
  }
  return true;
}

class IndexDataConsumer : public index::IndexDataConsumer {
public:
  ASTContext *ctx;
  IndexParam &param;

  std::string getComment(const Decl *d) {
    SourceManager &sm = ctx->getSourceManager();
    const RawComment *rc = ctx->getRawCommentForAnyRedecl(d);
    if (!rc)
      return "";
    StringRef raw = rc->getRawText(ctx->getSourceManager());
    SourceRange sr = rc->getSourceRange();
    std::pair<FileID, unsigned> bInfo = sm.getDecomposedLoc(sr.getBegin());
    unsigned start_column = sm.getLineNumber(bInfo.first, bInfo.second);
    std::string ret;
    int pad = -1;
    for (const char *p = raw.data(), *e = raw.end(); p < e;) {
      // The first line starts with a comment marker, but the rest needs
      // un-indenting.
      unsigned skip = start_column - 1;
      for (; skip > 0 && p < e && (*p == ' ' || *p == '\t'); p++)
        skip--;
      const char *q = p;
      while (q < e && *q != '\n')
        q++;
      if (q < e)
        q++;
      // A minimalist approach to skip Doxygen comment markers.
      // See https://www.stack.nl/~dimitri/doxygen/manual/docblocks.html
      if (pad < 0) {
        // First line, detect the length of comment marker and put into |pad|
        const char *begin = p;
        while (p < e && (*p == '/' || *p == '*' || *p == '-' || *p == '='))
          p++;
        if (p < e && (*p == '<' || *p == '!'))
          p++;
        if (p < e && *p == ' ')
          p++;
        if (p + 1 == q)
          p++;
        else
          pad = int(p - begin);
      } else {
        // Other lines, skip |pad| bytes
        int prefix = pad;
        while (prefix > 0 && p < e &&
               (*p == ' ' || *p == '/' || *p == '*' || *p == '<' || *p == '!'))
          prefix--, p++;
      }
      ret.insert(ret.end(), p, q);
      p = q;
    }
    while (ret.size() && isspace(ret.back()))
      ret.pop_back();
    if (StringRef(ret).endswith("*/") || StringRef(ret).endswith("\n/"))
      ret.resize(ret.size() - 2);
    while (ret.size() && isspace(ret.back()))
      ret.pop_back();
    return ret;
  }

  Usr getUsr(const Decl *d, IndexParam::DeclInfo **info = nullptr) const {
    d = d->getCanonicalDecl();
    auto [it, inserted] = param.decl2Info.try_emplace(d);
    if (inserted) {
      SmallString<256> usr;
      index::generateUSRForDecl(d, usr);
      auto &info = it->second;
      info.usr = hashUsr(usr);
      if (auto *nd = dyn_cast<NamedDecl>(d)) {
        info.short_name = nd->getNameAsString();
        llvm::raw_string_ostream os(info.qualified);
        nd->printQualifiedName(os, getDefaultPolicy());
        simplifyAnonymous(info.qualified);
      }
    }
    if (info)
      *info = &it->second;
    return it->second.usr;
  }

  PrintingPolicy getDefaultPolicy() const {
    PrintingPolicy pp(ctx->getLangOpts());
    pp.AnonymousTagLocations = false;
    pp.TerseOutput = true;
    pp.PolishForDeclaration = true;
    pp.ConstantsAsWritten = true;
    pp.SuppressTagKeyword = true;
    pp.SuppressUnwrittenScope = g_config->index.name.suppressUnwrittenScope;
    pp.SuppressInitializers = true;
    pp.FullyQualifiedName = false;
    return pp;
  }

  static void simplifyAnonymous(std::string &name) {
    for (std::string::size_type i = 0;;) {
      if ((i = name.find("(anonymous ", i)) == std::string::npos)
        break;
      i++;
      if (name.size() - i > 19 && name.compare(i + 10, 9, "namespace") == 0)
        name.replace(i, 19, "anon ns");
      else
        name.replace(i, 9, "anon");
    }
  }

  template <typename Def>
  void setName(const Decl *d, std::string_view short_name,
               std::string_view qualified, Def &def) {
    SmallString<256> str;
    llvm::raw_svector_ostream os(str);
    d->print(os, getDefaultPolicy());

    std::string name(str.data(), str.size());
    simplifyAnonymous(name);
    // Remove \n in DeclPrinter.cpp "{\n" + if(!TerseOutput)something + "}"
    for (std::string::size_type i = 0;;) {
      if ((i = name.find("{\n}", i)) == std::string::npos)
        break;
      name.replace(i, 3, "{}");
    }
    auto i = name.find(short_name);
    if (short_name.size())
      while (i != std::string::npos &&
             ((i && isAsciiIdentifierContinue(name[i - 1])) ||
              isAsciiIdentifierContinue(name[i + short_name.size()])))
        i = name.find(short_name, i + short_name.size());
    if (i == std::string::npos) {
      // e.g. operator type-parameter-1
      i = 0;
      def.short_name_offset = 0;
      def.short_name_size = name.size();
    } else {
      if (short_name.empty() || (i >= 2 && name[i - 2] == ':')) {
        // Don't replace name with qualified name in ns::name Cls::*name
        def.short_name_offset = i;
      } else {
        name.replace(i, short_name.size(), qualified);
        def.short_name_offset = i + qualified.size() - short_name.size();
      }
      // name may be empty while short_name is not.
      def.short_name_size = name.empty() ? 0 : short_name.size();
    }
    for (int paren = 0; i; i--) {
      // Skip parentheses in "(anon struct)::name"
      if (name[i - 1] == ')')
        paren++;
      else if (name[i - 1] == '(')
        paren--;
      else if (!(paren > 0 || isAsciiIdentifierContinue(name[i - 1]) ||
                 name[i - 1] == ':'))
        break;
    }
    def.qual_name_offset = i;
    def.detailed_name = intern(name);
  }

  void setVarName(const Decl *d, std::string_view short_name,
                  std::string_view qualified, IndexVar::Def &def) {
    QualType t;
    const Expr *init = nullptr;
    bool deduced = false;
    if (auto *vd = dyn_cast<VarDecl>(d)) {
      t = vd->getType();
      init = vd->getAnyInitializer();
      def.storage = vd->getStorageClass();
    } else if (auto *fd = dyn_cast<FieldDecl>(d)) {
      t = fd->getType();
      init = fd->getInClassInitializer();
    } else if (auto *bd = dyn_cast<BindingDecl>(d)) {
      t = bd->getType();
      deduced = true;
    }
    if (!t.isNull()) {
      if (t->getContainedDeducedType()) {
        deduced = true;
      } else if (auto *dt = dyn_cast<DecltypeType>(t)) {
        // decltype(y) x;
        while (dt && !dt->getUnderlyingType().isNull()) {
          t = dt->getUnderlyingType();
          dt = dyn_cast<DecltypeType>(t);
        }
        deduced = true;
      }
    }
    if (!t.isNull() && deduced) {
      SmallString<256> str;
      llvm::raw_svector_ostream os(str);
      PrintingPolicy pp = getDefaultPolicy();
      t.print(os, pp);
      if (str.size() &&
          (str.back() != ' ' && str.back() != '*' && str.back() != '&'))
        str += ' ';
      def.qual_name_offset = str.size();
      def.short_name_offset = str.size() + qualified.size() - short_name.size();
      def.short_name_size = short_name.size();
      str += StringRef(qualified.data(), qualified.size());
      def.detailed_name = intern(str);
    } else {
      setName(d, short_name, qualified, def);
    }
    if (init) {
      SourceManager &sm = ctx->getSourceManager();
      const LangOptions &lang = ctx->getLangOpts();
      SourceRange sr =
          sm.getExpansionRange(init->getSourceRange()).getAsRange();
      SourceLocation l = d->getLocation();
      if (l.isMacroID() || !sm.isBeforeInTranslationUnit(l, sr.getBegin()))
        return;
      StringRef buf = getSourceInRange(sm, lang, sr);
      Twine init = buf.count('\n') <= g_config->index.maxInitializerLines - 1