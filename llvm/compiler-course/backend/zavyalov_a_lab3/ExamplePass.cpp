#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineBasicBlock.h"

using namespace llvm;

namespace {
class LoopUnrollPass : public MachineFunctionPass {
  const X86InstrInfo *TII = nullptr;
  static constexpr int maxUnrollingIters = 5;
public:

  static char ID;
  LoopUnrollPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;

  // ── getTripCount ────────────────────────────────────────────────────────────
  unsigned getTripCount(MachineLoop *Loop) {
    // сначала пробуем latch, потом header
    MachineBasicBlock *Latch  = Loop->getLoopLatch();
    MachineBasicBlock *Header = Loop->getHeader();

    for (MachineBasicBlock *BB : {Latch, Header}) {
      if (!BB) continue;
      for (auto &MI : reverse(*BB)) {
        if (MI.getOpcode() == X86::CMP32ri || MI.getOpcode() == X86::CMP32ri8) {
          for (auto &MO : MI.operands()) {
            if (MO.isImm()) {
              return (unsigned)(MO.getImm() + 1);
            }
          }
        }
      }
    }
    return 0;
  }

  // ── unrollLoop ──────────────────────────────────────────────────────────────
  bool unrollLoop(MachineLoop *Loop, unsigned Count, MachineFunction &MF, MachineLoopInfo &MLI) {
    if (Count <= 1) return false;

    MachineBasicBlock *Latch = Loop->getLoopLatch();
    MachineBasicBlock *Exiting = Loop->getUniqueExitBlock(); // TODO чекнуть

    if (!Latch || !Exiting || Exiting != Latch) {
      return false;
    }

    // ищем число итераций в одном блоке
    int itersInBlock = 1;
    for (int div = maxUnrollingIters; div > 0; div--) {
      if (Count % div == 0) {
        itersInBlock = div;
        break;
      }
    }
    if (itersInBlock == 1) {
      return false;
    }

    SmallVector<MachineInstr *, 16> LoopBody;

    // ищем функции которые будем копировать
    for (auto &MBB : Loop->blocks()) {
      if (MLI.getLoopFor(MBB) != Loop) {
        continue;
      }

      for (auto &MI : *MBB) {
        if (MI.isBranch() || MI.isTerminator() || MI.isDebugInstr()) {
          continue;
        }

        LoopBody.push_back(&MI);
      }
    }

    if (LoopBody.empty()) {
      return false;
    }

    // TODO чекнуть, мб тут iterator надо
    const auto& WhereToInsert = Latch->getFirstTerminator(); // первая инструкция-терминатор

    
    int AmountOfCopies = itersInBlock - 1;
    // проверка на вложенные циклы
    if (Loop->begin() != Loop->end()) { // если есть вложенные циклы, то копируем на 1 раз больше
      ++AmountOfCopies;
    }

    for (int i = 0; i < AmountOfCopies; i++) {
      for (auto &MI : LoopBody) {
        const auto& Clone = MF.CloneMachineInstr(MI);
        Latch->insert(WhereToInsert, Clone);
      }
    }

    // изменяем блок обновления переменной индукции
    for (auto &MI : *Latch) {
      if (MI.getOpcode() != X86::ADD32ri8) {
        continue;
      }

      for (auto &Op : MI.operands()) {
        if (Op.isImm() && Op.getImm() == 1) {
          Op.setImm(itersInBlock);
        }
      }
    }

    return true;
  }


  // ── tryUnrollLoop ───────────────────────────────────────────────────────────
  // unchanged — was already correct
  bool tryUnrollLoop(MachineLoop *Loop, MachineLoopInfo &MLI, MachineFunction &MF) {
    bool Changed = false;

    for (MachineLoop *SubLoop : *Loop) {
      Changed |= tryUnrollLoop(SubLoop, MLI, MF);
    }

    unsigned TripCount = getTripCount(Loop);
    llvm::outs() << "Loop in " << MF.getName() 
             << " TripCount=" << TripCount << "\n";
    if (TripCount == 0 || TripCount > 5)
      return Changed;

    llvm::outs() << "  Unrolling!\n";
    Changed |= unrollLoop(Loop, TripCount, MF, MLI);
    return Changed;
  }

  // ── getAnalysisUsage ────────────────────────────────────────────────────────
  // unchanged — was already correct
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfoWrapperPass>();
    // AU.addRequired<MachineDominatorTreeWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

char LoopUnrollPass::ID = 0;

bool LoopUnrollPass::runOnMachineFunction(MachineFunction &MF) {
  TII = MF.getSubtarget<X86Subtarget>().getInstrInfo();
  auto &MLI = getAnalysis<MachineLoopInfoWrapperPass>().getLI();
  bool Changed = false;

  SmallVector<MachineLoop*, 4> Loops(MLI.begin(), MLI.end());

  for (MachineLoop *Loop : Loops) {
    Changed |= tryUnrollLoop(Loop, MLI, MF);
  }


  return Changed;
}
} // namespace

static RegisterPass<LoopUnrollPass> X("loop-unroll-x86", "loop unrolling pass", false,
                                      false);