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
  static constexpr int maxUnrollingIters = 5;
  const X86InstrInfo *TII = nullptr;
public:

  void FillLoops(MachineLoop *Loop, SmallVectorImpl<MachineLoop *> &Loops) {
    for (MachineLoop *SubLoop : *Loop)
      FillLoops(SubLoop, Loops);
    Loops.push_back(Loop);
  }

  bool hasUniquePreheader(MachineLoop *L) {
    MachineBasicBlock *Header = L->getHeader();
    if (!Header)
      return false;

    MachineBasicBlock *Preheader = nullptr;

    for (MachineBasicBlock *Pred : Header->predecessors()) {
      if (L->contains(Pred))
        continue;

      if (Preheader)
        return false;

      Preheader = Pred;
    }

    return Preheader != nullptr;
  }

  MachineBasicBlock *getUniqueExitingBlock(MachineLoop *L) {
    MachineBasicBlock *Exiting = nullptr;

    for (MachineBasicBlock *MBB : L->blocks()) {
      bool HasOutsideSucc = false;

      for (MachineBasicBlock *Succ : MBB->successors()) {
        if (!L->contains(Succ)) {
          HasOutsideSucc = true;
          break;
        }
      }

      if (!HasOutsideSucc)
        continue;

      if (Exiting)
        return nullptr;

      Exiting = MBB;
    }

    return Exiting;
  }

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
    llvm::outs() << "Loop in " << MF.getName() 
          << " TripCount=" << Count << "\n";
    MachineBasicBlock *Latch = Loop->getLoopLatch();
    MachineBasicBlock *Exiting = getUniqueExitingBlock(Loop); // TODO чекнуть

    if (!hasUniquePreheader(Loop) || !Latch || !Exiting ) {
      return false;
    }

    llvm::outs() << "дожиди\n";
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


      for (auto &MI : *MBB) {
        if (MI.isBranch() || MI.isTerminator() || MI.isDebugInstr()) {
          continue;
        }
        if (MI.getOpcode() == X86::CMP32ri || MI.getOpcode() == X86::CMP32ri8 || MI.getOpcode() == X86::INC32r) {
          continue;
        }

        LoopBody.push_back(&MI);
      }
    }

    if (LoopBody.empty()) {
      return false;
    }

    // TODO чекнуть, мб тут iterator надо
    const auto& WhereToInsert = Exiting->getFirstTerminator(); // первая инструкция-терминатор

    
    int AmountOfCopies = itersInBlock - 1;
    // проверка на вложенные циклы
    if (Loop->begin() != Loop->end()) { // если есть вложенные циклы, то копируем на 1 раз больше
      ++AmountOfCopies;
    }


    Register CounterReg;
    for (auto &MI : *Latch) {
      if (MI.getOpcode() == X86::INC32r) {
        CounterReg = MI.getOperand(0).getReg();
        break;
      }
    }

    // копируем
    for (int i = 0; i < AmountOfCopies; i++) {
      // сначала инкремент счётчика
      BuildMI(*Exiting, WhereToInsert, DebugLoc(), TII->get(X86::INC32r), CounterReg)
          .addReg(CounterReg);

      // потом копия тела
      for (auto &MI : LoopBody) {
        MachineInstr *Clone = MF.CloneMachineInstr(MI);
        Exiting->insert(WhereToInsert, Clone);
      }
    }

    return true;
  }

  // ── getAnalysisUsage ────────────────────────────────────────────────────────
  // unchanged — was already correct
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfoWrapperPass>();
     AU.setPreservesCFG();
    // AU.addRequired<MachineDominatorTreeWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

char LoopUnrollPass::ID = 0;

bool LoopUnrollPass::runOnMachineFunction(MachineFunction &MF)  {
  TII = MF.getSubtarget<X86Subtarget>().getInstrInfo();
    auto &MLI = getAnalysis<MachineLoopInfoWrapperPass>().getLI();

    bool Changed = false;
    SmallVector<MachineLoop*, 8> Loops;

    for (MachineLoop *Loop : MLI) {
      FillLoops(Loop, Loops);
    }
    
    for (MachineLoop *Loop : Loops) {
      
      unsigned TripCount = getTripCount(Loop);
      Changed |= unrollLoop(Loop, TripCount, MF, MLI);
    }

    return Changed;
  }

} // namespace

static RegisterPass<LoopUnrollPass> X("loop-unroll-x86", "loop unrolling pass", false,
                                      false);
