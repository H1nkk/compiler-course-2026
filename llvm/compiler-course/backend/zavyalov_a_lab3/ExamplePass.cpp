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
public:

  static char ID;
  LoopUnrollPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;

  // ── getTripCount ────────────────────────────────────────────────────────────
  // CHANGED: filled in the empty body with real CMP32ri detection logic

  unsigned getTripCount(MachineLoop *Loop) {
    // Try latch first, then fall back to header
    MachineBasicBlock *Latch  = Loop->getLoopLatch();
    MachineBasicBlock *Header = Loop->getHeader();

    for (MachineBasicBlock *BB : {Latch, Header}) {
      if (!BB) continue;
      for (auto &MI : reverse(*BB)) {
        if (MI.getOpcode() == X86::CMP32ri ||
            MI.getOpcode() == X86::CMP32ri8) {
          int64_t Imm = MI.getOperand(1).getImm();
          
          return (unsigned)(Imm + 1);
        }
      }
    }
    return 0;
  }
/*
  unsigned getTripCount(MachineLoop *Loop) {
    MachineBasicBlock *Latch = Loop->getLoopLatch();
    if (!Latch) return 0;

    MachineInstr *CmpMI = nullptr;
    for (auto &MI : reverse(*Latch)) {
      // ADDED: actually look for the CMP instruction by opcode
      if (MI.getOpcode() == X86::CMP32ri || MI.getOpcode() == X86::CMP32ri8) {
        CmpMI = &MI;
        break;
      }
    }

    if (!CmpMI) return 0;

    // ADDED: the immediate is N-1 because "i < N" compiles to "CMP reg, N-1"
    int64_t Imm = CmpMI->getOperand(1).getImm();
    unsigned TripCount = (unsigned)(Imm + 1);
    return TripCount;
  }
*/

  // ── unrollLoop ──────────────────────────────────────────────────────────────
  bool unrollLoop(MachineLoop *Loop, unsigned Count, MachineFunction &MF) {
    llvm::outs() << "  unrollLoop: Count=" << Count << "\n";
    
    MachineBasicBlock *Header = Loop->getHeader();
    MachineBasicBlock *Latch  = Loop->getLoopLatch();
    MachineBasicBlock *Exit   = Loop->getExitBlock();

    llvm::outs() << "  Header=" << Header->getName() << "\n";
    llvm::outs() << "  Latch="  << Latch->getName()  << "\n";
    llvm::outs() << "  Exit="   << (Exit ? Exit->getName() : "null") << "\n";

    if (!Latch || !Exit) {
      llvm::outs() << "  Aborting: null Latch or Exit\n";
      return false;
    }

    SmallVector<MachineBasicBlock*, 4> BodyBlocks(Loop->block_begin(),
                                                  Loop->block_end());

    MachineBasicBlock *PrevLatch = Latch; // end of the previous iteration

    // Safe removeSuccessor — only remove if it's actually a successor
    auto safeRemoveSuccessor = [](MachineBasicBlock *From, MachineBasicBlock *To) {
      for (auto I = From->succ_begin(); I != From->succ_end(); ++I) {
        if (*I == To) {
          From->removeSuccessor(I);
          return;
        }
      }
    };

    for (unsigned i = 1; i < Count; ++i) {
      DenseMap<MachineBasicBlock*, MachineBasicBlock*> BlockMap;

      // Clone every block in the loop body
      for (MachineBasicBlock *MBB : BodyBlocks) {
        MachineBasicBlock *Clone = MF.CreateMachineBasicBlock();
        MF.insert(MF.end(), Clone);
        BlockMap[MBB] = Clone;
        for (const MachineInstr &MI : *MBB)
          Clone->push_back(MF.CloneMachineInstr(&MI));
      }

      // Fix up branch targets inside clones
      for (auto &[OldBB, NewBB] : BlockMap) {
        for (MachineInstr &MI : *NewBB) {
          for (MachineOperand &MO : MI.operands()) {
            if (MO.isMBB() && BlockMap.count(MO.getMBB()))
              MO.setMBB(BlockMap[MO.getMBB()]);
          }
        }
      }

      // Connect previous latch to the new header clone —
      // remove the old back-edge branch and add a fall-through
      MachineBasicBlock *NewHeader = BlockMap[Header];
      PrevLatch->erase(PrevLatch->getFirstTerminator(), PrevLatch->end());
      safeRemoveSuccessor(PrevLatch, Header);          // original latch → original header
      safeRemoveSuccessor(PrevLatch, BlockMap[Header]); // clone latch → clone header (2nd+ iters)
      PrevLatch->addSuccessor(NewHeader);
      BuildMI(*PrevLatch, PrevLatch->end(), DebugLoc(),
              TII->get(X86::JMP_1)).addMBB(NewHeader);

      PrevLatch = BlockMap[Latch];
    }

    // Remove the back-edge from the final latch — it should fall to Exit
    PrevLatch->erase(PrevLatch->getFirstTerminator(), PrevLatch->end());
    safeRemoveSuccessor(PrevLatch, Header);
    PrevLatch->addSuccessor(Exit);
    BuildMI(*PrevLatch, PrevLatch->end(), DebugLoc(),
            TII->get(X86::JMP_1)).addMBB(Exit);
    return true;
  }

  /*
  bool unrollLoop(MachineLoop *Loop, unsigned Count, MachineFunction &MF) {
    SmallVector<MachineBasicBlock*, 4> BodyBlocks(Loop->block_begin(),
                                                  Loop->block_end());
    for (unsigned i = 1; i < Count; ++i) {
      DenseMap<MachineBasicBlock*, MachineBasicBlock*> BlockMap;
      for (MachineBasicBlock *MBB : BodyBlocks) {
        MachineBasicBlock *Clone = MF.CreateMachineBasicBlock();
        MF.insert(MF.end(), Clone);
        BlockMap[MBB] = Clone;

        for (const MachineInstr &MI : *MBB)
          Clone->push_back(MF.CloneMachineInstr(&MI));
      }

      // ADDED: fix up branch targets in cloned blocks to point to other clones
      // instead of the originals
      for (auto &[OldBB, NewBB] : BlockMap) {
        for (MachineInstr &MI : *NewBB) {
          for (MachineOperand &MO : MI.operands()) {
            if (MO.isMBB() && BlockMap.count(MO.getMBB()))
              MO.setMBB(BlockMap[MO.getMBB()]);
          }
        }
      }
    }

    // ADDED: remove the backedge branch from the original latch so the loop
    // doesn't jump back to the header after the last iteration
    MachineBasicBlock *OrigLatch = Loop->getLoopLatch();
    if (OrigLatch) {
      OrigLatch->erase(OrigLatch->getFirstTerminator(), OrigLatch->end());
    }

    return true;
  }
  */

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
    Changed |= unrollLoop(Loop, TripCount, MF);
    return Changed;
  }

  // ── getAnalysisUsage ────────────────────────────────────────────────────────
  // unchanged — was already correct
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineLoopInfoWrapperPass>();
    AU.addRequired<MachineDominatorTreeWrapperPass>();
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

  // REMOVED: the llvm::outs() debug print — it fired on every function
  // even ones with no loops, which is just noise

  return Changed;
}
} // namespace

static RegisterPass<LoopUnrollPass> X("loop-unroll-x86", "loop unrolling pass", false,
                                      false);