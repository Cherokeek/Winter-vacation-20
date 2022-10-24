
// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "clang_tu.hh"

#include "config.hh"
#include "platform.hh"

#include <clang/AST/Type.h>
#include <clang/Driver/Action.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Tool.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>

using namespace clang;

namespace ccls {
std::string pathFromFileEntry(const FileEntry &file) {
  std::string ret;
  if (file.getName().startswith("/../")) {
    // Resolve symlinks outside of working folders. This handles leading path
    // components, e.g. (/lib -> /usr/lib) in
    // /../lib/gcc/x86_64-linux-gnu/10/../../../../include/c++/10/utility
    ret = file.tryGetRealPathName();
  } else {
    // If getName() refers to a file within a workspace folder, we prefer it
    // (which may be a symlink).
    ret = normalizePath(file.getName());
  }
  if (normalizeFolder(ret))
    return ret;
  ret = realPath(ret);
  normalizeFolder(ret);
  return ret;
}

bool isInsideMainFile(const SourceManager &sm, SourceLocation sl) {
  if (!sl.isValid())
    return false;
  FileID fid = sm.getFileID(sm.getExpansionLoc(sl));
  return fid == sm.getMainFileID() || fid == sm.getPreambleFileID();
}

static Pos decomposed2LineAndCol(const SourceManager &sm,
                                 std::pair<FileID, unsigned> i) {
  int l = (int)sm.getLineNumber(i.first, i.second) - 1,
      c = (int)sm.getColumnNumber(i.first, i.second) - 1;
  bool invalid = false;
  StringRef buf = sm.getBufferData(i.first, &invalid);
  if (!invalid) {
    StringRef p = buf.substr(i.second - c, c);
    c = 0;
    for (size_t i = 0; i < p.size();)
      if (c++, (uint8_t)p[i++] >= 128)
        while (i < p.size() && (uint8_t)p[i] >= 128 && (uint8_t)p[i] < 192)
          i++;
  }
  return {(uint16_t)std::min<int>(l, UINT16_MAX),
          (int16_t)std::min<int>(c, INT16_MAX)};
}

Range fromCharSourceRange(const SourceManager &sm, const LangOptions &lang,
                          CharSourceRange csr, FileID *fid) {
  SourceLocation bloc = csr.getBegin(), eloc = csr.getEnd();
  std::pair<FileID, unsigned> binfo = sm.getDecomposedLoc(bloc),
                              einfo = sm.getDecomposedLoc(eloc);
  if (csr.isTokenRange())
    einfo.second += Lexer::MeasureTokenLength(eloc, sm, lang);
  if (fid)
    *fid = binfo.first;
  return {decomposed2LineAndCol(sm, binfo), decomposed2LineAndCol(sm, einfo)};
}

Range fromTokenRange(const SourceManager &sm, const LangOptions &lang,
                     SourceRange sr, FileID *fid) {
  return fromCharSourceRange(sm, lang, CharSourceRange::getTokenRange(sr), fid);
}

Range fromTokenRangeDefaulted(const SourceManager &sm, const LangOptions &lang,
                              SourceRange sr, FileID fid, Range range) {
  auto decomposed = sm.getDecomposedLoc(sm.getExpansionLoc(sr.getBegin()));
  if (decomposed.first == fid)
    range.start = decomposed2LineAndCol(sm, decomposed);
  SourceLocation sl = sm.getExpansionLoc(sr.getEnd());
  decomposed = sm.getDecomposedLoc(sl);
  if (decomposed.first == fid) {
    decomposed.second += Lexer::MeasureTokenLength(sl, sm, lang);
    range.end = decomposed2LineAndCol(sm, decomposed);
  }
  return range;
}

std::unique_ptr<CompilerInvocation>
buildCompilerInvocation(const std::string &main, std::vector<const char *> args,
                        IntrusiveRefCntPtr<llvm::vfs::FileSystem> vfs) {
  std::string save = "-resource-dir=" + g_config->clang.resourceDir;
  args.push_back(save.c_str());
  args.push_back("-fsyntax-only");

  // Similar to clang/tools/driver/driver.cpp:insertTargetAndModeArgs but don't
  // require llvm::InitializeAllTargetInfos().
  auto target_and_mode =
      driver::ToolChain::getTargetAndModeFromProgramName(args[0]);
  if (target_and_mode.DriverMode)
    args.insert(args.begin() + 1, target_and_mode.DriverMode);
  if (!target_and_mode.TargetPrefix.empty()) {
    const char *arr[] = {"-target", target_and_mode.TargetPrefix.c_str()};
    args.insert(args.begin() + 1, std::begin(arr), std::end(arr));
  }

  IntrusiveRefCntPtr<DiagnosticsEngine> diags(
      CompilerInstance::createDiagnostics(new DiagnosticOptions,
                                          new IgnoringDiagConsumer, true));
#if LLVM_VERSION_MAJOR < 12 // llvmorg-12-init-5498-g257b29715bb
  driver::Driver d(args[0], llvm::sys::getDefaultTargetTriple(), *diags, vfs);
#else
  driver::Driver d(args[0], llvm::sys::getDefaultTargetTriple(), *diags, "ccls", vfs);
#endif
  d.setCheckInputsExist(false);
  std::unique_ptr<driver::Compilation> comp(d.BuildCompilation(args));
  if (!comp)
    return nullptr;
  const driver::JobList &jobs = comp->getJobs();
  bool offload_compilation = false;
  if (jobs.size() > 1) {
    for (auto &a : comp->getActions()){
      // On MacOSX real actions may end up being wrapped in BindArchAction
      if (isa<driver::BindArchAction>(a))
        a = *a->input_begin();
      if (isa<driver::OffloadAction>(a)) {
        offload_compilation = true;
        break;
      }
    }
    if (!offload_compilation)
      return nullptr;
  }
  if (jobs.size() == 0 || !isa<driver::Command>(*jobs.begin()))
    return nullptr;

  const driver::Command &cmd = cast<driver::Command>(*jobs.begin());
  if (StringRef(cmd.getCreator().getName()) != "clang")
    return nullptr;
  const llvm::opt::ArgStringList &cc_args = cmd.getArguments();
  auto ci = std::make_unique<CompilerInvocation>();
#if LLVM_VERSION_MAJOR >= 10 // rC370122
  if (!CompilerInvocation::CreateFromArgs(*ci, cc_args, *diags))
#else
  if (!CompilerInvocation::CreateFromArgs(
          *ci, cc_args.data(), cc_args.data() + cc_args.size(), *diags))
#endif
    return nullptr;

  ci->getDiagnosticOpts().IgnoreWarnings = true;
  ci->getFrontendOpts().DisableFree = false;
  // Enable IndexFrontendAction::shouldSkipFunctionBody.
  ci->getFrontendOpts().SkipFunctionBodies = true;
  ci->getLangOpts()->SpellChecking = false;
#if LLVM_VERSION_MAJOR >= 11
  ci->getLangOpts()->RecoveryAST = true;
  ci->getLangOpts()->RecoveryASTType = true;
#endif
  auto &isec = ci->getFrontendOpts().Inputs;
  if (isec.size())
    isec[0] = FrontendInputFile(main, isec[0].getKind(), isec[0].isSystem());
#if LLVM_VERSION_MAJOR >= 10 // llvmorg-11-init-2414-g75f09b54429
  ci->getPreprocessorOpts().DisablePragmaDebugCrash = true;
#endif
  // clangSerialization has an unstable format. Disable PCH reading/writing
  // to work around PCH mismatch problems.
  ci->getPreprocessorOpts().ImplicitPCHInclude.clear();
  ci->getPreprocessorOpts().PrecompiledPreambleBytes = {0, false};
  ci->getPreprocessorOpts().PCHThroughHeader.clear();

  ci->getHeaderSearchOpts().ModuleFormat = "raw";
  return ci;
}

// clang::BuiltinType::getName without PrintingPolicy
const char *clangBuiltinTypeName(int kind) {
  switch (BuiltinType::Kind(kind)) {
  case BuiltinType::Void:
    return "void";
  case BuiltinType::Bool:
    return "bool";
  case BuiltinType::Char_S:
  case BuiltinType::Char_U:
    return "char";
  case BuiltinType::SChar:
    return "signed char";
  case BuiltinType::Short:
    return "short";
  case BuiltinType::Int:
    return "int";
  case BuiltinType::Long:
    return "long";