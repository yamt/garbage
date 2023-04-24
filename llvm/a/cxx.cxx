
#include <llvm-c/TargetMachine.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>

#include "cxx.h"

LLVMTargetMachineRef
create_target_machine(LLVMTargetRef ctarget, const char *triple,
                      const char *cpu, const char *features)
{
        llvm::Target *target = reinterpret_cast<llvm::Target *>(ctarget);
        llvm::TargetOptions opts;

        // -fstack-size-section equiv
        // emit it to ".stack_sizes" section in case of ELF
        // you can read it with "llvm-readobj --stack-sizes"
        opts.EmitStackSizeSection = true;

        // -fstack-usage equiv
        opts.StackUsageOutput = "out.su";

        llvm::TargetMachine *targetmachine = target->createTargetMachine(
                triple, cpu, features, opts, llvm::Reloc::PIC_, llvm::None,
                llvm::CodeGenOpt::Default, false);
        return reinterpret_cast<LLVMTargetMachineRef>(targetmachine);
}
