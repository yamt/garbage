#include <llvm-c/TargetMachine.h>

LLVM_C_EXTERN_C_BEGIN
LLVMTargetMachineRef LLVMCreateTargetMachineWithOpts(
        LLVMTargetRef ctarget, const char *triple, const char *cpu,
        const char *features, LLVMCodeGenOptLevel opt_level,
        LLVMRelocMode reloc_mode, LLVMCodeModel code_model,
        bool EmitStackSizeSection, const char *StackUsageOutput);
LLVM_C_EXTERN_C_END
