#include <llvm-c/TargetMachine.h>

LLVM_C_EXTERN_C_BEGIN
LLVMTargetMachineRef
create_target_machine(LLVMTargetRef ctarget, const char *triple, const char *cpu, const char *features);
LLVM_C_EXTERN_C_END
