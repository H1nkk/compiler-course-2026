#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct ExamplePass : llvm::PassInfoMixin<ExamplePass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    for (auto &bb : func) {
      for (auto &instr : llvm::make_early_inc_range(bb)) {
        if (llvm::ICmpInst *icmp = llvm::dyn_cast<llvm::ICmpInst>(&instr)) {
          switch (icmp->getPredicate()) {
          case llvm::ICmpInst::ICMP_SLT: {
            llvm::Value *lhs = icmp->getOperand(0);
            llvm::Value *rhs = icmp->getOperand(1);
            llvm::IRBuilder<> builder(icmp);

            llvm::Value *sge =
                builder.CreateICmpSGE(lhs, rhs, icmp->getName() + ".sge");

            llvm::Value *not_sge =
                builder.CreateNot(sge, sge->getName() + ".not");

            icmp->replaceAllUsesWith(not_sge);
            icmp->eraseFromParent();

            break;
          }
          case llvm::ICmpInst::ICMP_SGT: {
            llvm::Value *lhs = icmp->getOperand(0);
            llvm::Value *rhs = icmp->getOperand(1);
            llvm::IRBuilder<> builder(icmp);

            llvm::Value *sle =
                builder.CreateICmpSLE(lhs, rhs, icmp->getName() + ".sle");

            llvm::Value *not_sle =
                builder.CreateNot(sle, sle->getName() + ".not");

            icmp->replaceAllUsesWith(not_sle);
            icmp->eraseFromParent();

            break;
          }
          case llvm::ICmpInst::ICMP_SLE: {
            llvm::Value *lhs = icmp->getOperand(0);
            llvm::Value *rhs = icmp->getOperand(1);
            llvm::IRBuilder<> builder(icmp);

            llvm::Value *sgt =
                builder.CreateICmpSGT(lhs, rhs, icmp->getName() + ".sgt");

            llvm::Value *not_sgt =
                builder.CreateNot(sgt, sgt->getName() + ".not");

            icmp->replaceAllUsesWith(not_sgt);
            icmp->eraseFromParent();

            break;
          }
          case llvm::ICmpInst::ICMP_SGE: {
            llvm::Value *lhs = icmp->getOperand(0);
            llvm::Value *rhs = icmp->getOperand(1);
            llvm::IRBuilder<> builder(icmp);

            llvm::Value *slt =
                builder.CreateICmpSLT(lhs, rhs, icmp->getName() + ".slt");

            llvm::Value *not_slt =
                builder.CreateNot(slt, slt->getName() + ".not");

            icmp->replaceAllUsesWith(not_slt);
            icmp->eraseFromParent();

            break;
          }
          case llvm::ICmpInst::ICMP_ULT: {
            llvm::Value *lhs = icmp->getOperand(0);
            llvm::Value *rhs = icmp->getOperand(1);
            llvm::IRBuilder<> builder(icmp);

            llvm::Value *uge =
                builder.CreateICmpUGE(lhs, rhs, icmp->getName() + ".uge");

            llvm::Value *not_uge =
                builder.CreateNot(uge, uge->getName() + ".not");

            icmp->replaceAllUsesWith(not_uge);
            icmp->eraseFromParent();

            break;
          }
          case llvm::ICmpInst::ICMP_UGT: {
            llvm::Value *lhs = icmp->getOperand(0);
            llvm::Value *rhs = icmp->getOperand(1);
            llvm::IRBuilder<> builder(icmp);

            llvm::Value *ule =
                builder.CreateICmpULE(lhs, rhs, icmp->getName() + ".ule");

            llvm::Value *not_ule =
                builder.CreateNot(ule, ule->getName() + ".not");

            icmp->replaceAllUsesWith(not_ule);
            icmp->eraseFromParent();

            break;
          }
          case llvm::ICmpInst::ICMP_ULE: {
            llvm::Value *lhs = icmp->getOperand(0);
            llvm::Value *rhs = icmp->getOperand(1);
            llvm::IRBuilder<> builder(icmp);

            llvm::Value *ugt =
                builder.CreateICmpUGT(lhs, rhs, icmp->getName() + ".ugt");

            llvm::Value *not_ugt =
                builder.CreateNot(ugt, ugt->getName() + ".not");

            icmp->replaceAllUsesWith(not_ugt);
            icmp->eraseFromParent();

            break;
          }
          case llvm::ICmpInst::ICMP_UGE: {
            llvm::Value *lhs = icmp->getOperand(0);
            llvm::Value *rhs = icmp->getOperand(1);
            llvm::IRBuilder<> builder(icmp);

            llvm::Value *ult =
                builder.CreateICmpULT(lhs, rhs, icmp->getName() + ".ult");

            llvm::Value *not_ult =
                builder.CreateNot(ult, ult->getName() + ".not");

            icmp->replaceAllUsesWith(not_ult);
            icmp->eraseFromParent();

            break;
          }
          default:
            break;
          }
        }
      }
    }
    return llvm::PreservedAnalyses::none();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "IcmpReplacerPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "IcmpReplacer") {
                    FPM.addPass(ExamplePass{});
                    return true;
                  }
                  return false;
                });
          }};
}
