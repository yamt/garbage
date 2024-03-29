
#include <llvm-c/TargetMachine.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>

#include "cxx.h"

static llvm::Optional<llvm::Reloc::Model>
convert(LLVMRelocMode reloc_mode)
{
        switch (reloc_mode) {
        case LLVMRelocDefault:
                return llvm::None;
        case LLVMRelocStatic:
                return llvm::Reloc::Static;
        case LLVMRelocPIC:
                return llvm::Reloc::PIC_;
        case LLVMRelocDynamicNoPic:
                return llvm::Reloc::DynamicNoPIC;
        case LLVMRelocROPI:
                return llvm::Reloc::ROPI;
        case LLVMRelocRWPI:
                return llvm::Reloc::RWPI;
        case LLVMRelocROPI_RWPI:
                return llvm::Reloc::ROPI_RWPI;
        }
}

static llvm::CodeGenOpt::Level
convert(LLVMCodeGenOptLevel opt_level)
{
        switch (opt_level) {
        case LLVMCodeGenLevelNone:
                return llvm::CodeGenOpt::None;
        case LLVMCodeGenLevelLess:
                return llvm::CodeGenOpt::Less;
        case LLVMCodeGenLevelDefault:
                return llvm::CodeGenOpt::Default;
        case LLVMCodeGenLevelAggressive:
                return llvm::CodeGenOpt::Aggressive;
        }
}

static llvm::Optional<llvm::CodeModel::Model>
convert(LLVMCodeModel code_model, bool *jit)
{
        *jit = false;
        switch (code_model) {
        case LLVMCodeModelDefault:
                return llvm::None;
        case LLVMCodeModelJITDefault:
                *jit = true;
                return llvm::None;
        case LLVMCodeModelTiny:
                return llvm::CodeModel::Tiny;
        case LLVMCodeModelSmall:
                return llvm::CodeModel::Small;
        case LLVMCodeModelKernel:
                return llvm::CodeModel::Kernel;
        case LLVMCodeModelMedium:
                return llvm::CodeModel::Medium;
        case LLVMCodeModelLarge:
                return llvm::CodeModel::Large;
        }
}

LLVMTargetMachineRef
LLVMCreateTargetMachineWithOpts(LLVMTargetRef ctarget, const char *triple,
                                const char *cpu, const char *features,
                                LLVMCodeGenOptLevel opt_level,
                                LLVMRelocMode reloc_mode,
                                LLVMCodeModel code_model,
                                bool EmitStackSizeSection,
                                const char *StackUsageOutput)
{
        llvm::TargetOptions opts;

        // -fstack-size-section equiv
        // emit it to ".stack_sizes" section in case of ELF
        // you can read it with "llvm-readobj --stack-sizes"
        opts.EmitStackSizeSection = EmitStackSizeSection;

        // -fstack-usage equiv
        if (StackUsageOutput != NULL) {
                opts.StackUsageOutput = StackUsageOutput;
        }

        auto target = reinterpret_cast<llvm::Target *>(ctarget);
        auto rm = convert(reloc_mode);
        auto ol = convert(opt_level);
        bool jit;
        auto cm = convert(code_model, &jit);
        auto targetmachine = target->createTargetMachine(
                triple, cpu, features, opts, rm, cm, ol, jit);
        return reinterpret_cast<LLVMTargetMachineRef>(targetmachine);
}
