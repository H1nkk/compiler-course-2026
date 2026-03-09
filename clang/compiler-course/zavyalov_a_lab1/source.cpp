#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include <set>
#include <vector>
#include <map>

namespace {
class ZavyalovVisitor final : public clang::RecursiveASTVisitor<ZavyalovVisitor> {
public:
  explicit ZavyalovVisitor(clang::ASTContext *context, clang::Rewriter &rewriter) : m_context(context), m_rewriter(rewriter) {}
  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    func->dump();
    llvm::outs() << "im a function\n";
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *varDecl) {
    clang::QualType varType = varDecl->getType();
    if (varType->isPointerType()) {
      const_ptrs.insert(varDecl);
      llvm::outs() << "esketit\n";

      clang::SourceLocation typeStart = varDecl->getTypeSpecStartLoc();
      clang::SourceLocation typeEnd = varDecl->getTypeSpecEndLoc();

      if (typeStart.isValid() && typeEnd.isValid()) {
        llvm::outs() << "432234234e\n";
        //m_rewriter.ReplaceText(clang::SourceRange(typeStart, typeEnd), "long*");
      }
    }

    if (varType->isReferenceType()) {
      const_refs.insert(varDecl);
      llvm::outs() << "reference found\n";
    }
    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator *op) {
    if (op->isAssignmentOp()) {

      clang::Expr *lhsExpr = op->getLHS()->IgnoreParenImpCasts(); // TODO разбобраться ???

      // является ли lhs разыменованием ptr
      if (auto* unaryOp = llvm::dyn_cast<clang::UnaryOperator>(lhsExpr)) {
        if (unaryOp->getOpcode() == clang::UO_Deref) {
          clang::Expr *subExpr = unaryOp->getSubExpr()->IgnoreParenCasts(); 
          if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(subExpr)) {
            if (auto* varDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
              llvm::outs() << "ptr is being derefernsed and reassigned: " << varDecl->getNameAsString() << "\n";
              if (const_ptrs.find(varDecl) != const_ptrs.end()) {
                const_ptrs.erase(const_ptrs.find(varDecl));
              }
              if (varDecl->getType()->isPointerType()) {
                
              }
            }
          }
        }
      }

      // является ли lhs ссылкой
      if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(lhsExpr)) {
          if (auto* varDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
              
            if (varDecl->getType()->isReferenceType()) {
              llvm::outs() << "Найден AssignmentOp\n";
              llvm::outs() << "Слева ссылка: " << varDecl->getNameAsString() << "\n";
              
              clang::QualType referencedType = varDecl->getType().getNonReferenceType();
              llvm::outs() << "Ссылается на: " << referencedType.getAsString() << "\n";

              if (const_refs.find(varDecl) != const_refs.end()) {
              const_refs.erase(const_refs.find(varDecl));
            }
              }
          }
        }


    }
    if (op->isCompoundAssignmentOp()) {
      clang::Expr *lhsExpr = op->getLHS()->IgnoreParenImpCasts();

      if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(lhsExpr)) {
        if (auto* varDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
          if (varDecl->getType()->isReferenceType()) {
            llvm::outs() << "Найден CompoundAssignmentOp\n";
            llvm::outs() << "Слева ссылка: " << varDecl->getNameAsString() << "\n";
              
            clang::QualType referencedType = varDecl->getType().getNonReferenceType();
            llvm::outs() << "Ссылается на: " << referencedType.getAsString() << "\n";

            if (const_refs.find(varDecl) != const_refs.end()) {
              //const_refs.erase(const_refs.find(varDecl));
            }
          }
        }
      }
    }
    return true;
  }
  
  bool VisitUnaryOperator(clang::UnaryOperator *op) {
    if (op->getOpcode() == clang::UO_AddrOf) {  // оператор &
        clang::Expr *subExpr = op->getSubExpr()->IgnoreParenImpCasts();
        
        if (auto *declRef = llvm::dyn_cast<clang::DeclRefExpr>(subExpr)) {
            if (auto *pointerVar = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
                llvm::outs() << "address is being taken: " << pointerVar->getNameAsString() << "\n";
                
                if (const_ptrs.find(pointerVar) != const_ptrs.end()) {
                const_ptrs.erase(const_ptrs.find(pointerVar));
              }
            }
        }
    }
    return true;
  }

  void endOfFile() {
    for (clang::VarDecl* varDecl : const_ptrs) {
      clang::SourceLocation typeStart = varDecl->getTypeSpecStartLoc();
      clang::SourceLocation typeEnd = varDecl->getTypeSpecEndLoc();

        if (typeStart.isValid() && typeEnd.isValid()) {
          llvm::outs() << "я ща заменю " << varDecl->getNameAsString() << '\n';
          m_rewriter.ReplaceText(clang::SourceRange(typeStart, typeEnd), "long*");
      }
    }
    for (clang::VarDecl* varDecl : const_refs) {
      clang::SourceLocation typeStart = varDecl->getTypeSpecStartLoc();
      clang::SourceLocation typeEnd = varDecl->getTypeSpecEndLoc();

        if (typeStart.isValid() && typeEnd.isValid()) {
          llvm::outs() << "я ща заменю " << varDecl->getNameAsString() << '\n';
          m_rewriter.ReplaceText(clang::SourceRange(typeStart, typeEnd), "const " + varDecl->getType().getAsString());
      }
    }
    llvm::outs() << "IT SERASFD\n";
  }

private:
  clang::ASTContext *m_context;
  std::set<clang::VarDecl*> const_ptrs;
  std::set<clang::VarDecl*> const_refs;
  std::map<clang::VarDecl*, std::vector<clang::VarDecl*>> mp; // мапа из varDecl, где varDecl - это поинтер, в вектор поинтеров, указывающих на этот varDecl
  clang::Rewriter &m_rewriter;


};

class ZavyalovConsumer final : public clang::ASTConsumer {
public:
  explicit ZavyalovConsumer(clang::ASTContext *context, clang::Rewriter &rewriter) : m_visitor(context, rewriter), m_rewriter(rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
    m_visitor.endOfFile();
  }

private:
  ZavyalovVisitor m_visitor;
  clang::Rewriter &m_rewriter;
};

class ZavyalovAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<ZavyalovConsumer>(&ci.getASTContext(), m_rewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }

  void EndSourceFileAction() override {
    m_rewriter.getEditBuffer(m_rewriter.getSourceMgr().getMainFileID())
              .write(llvm::outs());
  }

private:
  clang::Rewriter m_rewriter;
};
} // namespace

static clang::FrontendPluginRegistry::Add<ZavyalovAction>
    X("zavyalov_a_lab1_plugin", "Description plugin");
