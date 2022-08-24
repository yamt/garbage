#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>

int
main(int argc, char **argv)
{
        LLVMModuleRef m = LLVMModuleCreateWithName("mytest");

        LLVMBuilderRef b = LLVMCreateBuilder();

        LLVMTypeRef MainType =
                LLVMFunctionType(LLVMVoidType(), NULL, 0, false);
        LLVMValueRef Main = LLVMAddFunction(m, "main", MainType);

        LLVMBasicBlockRef block = LLVMAppendBasicBlock(Main, "entry");
        LLVMBasicBlockRef block2 = LLVMAppendBasicBlock(Main, "end_main");
        LLVMMoveBasicBlockAfter(block2, block);

        LLVMPositionBuilderAtEnd(b, block);
        LLVMBuildBr(b, block2);

        LLVMPositionBuilderAtEnd(b, block2);
        LLVMBuildRetVoid(b);

        LLVMDisposeBuilder(b);

        LLVMDumpModule(m);

        char *triple = LLVMGetDefaultTargetTriple();
        printf("triple: %s\n", triple);
        LLVMInitializeAllTargetInfos();
        LLVMInitializeAllTargets();
        LLVMInitializeAllTargetMCs();
        LLVMInitializeAllAsmPrinters();

        LLVMTargetRef target;
        char *errormsg;
        LLVMBool ret;
        ret = LLVMGetTargetFromTriple(triple, &target, &errormsg);
        if (ret) {
                printf("LLVMGetTargetFromTriple failed: %s\n", errormsg);
                exit(1);
        }

        LLVMTargetMachineRef target_machine;
        target_machine = LLVMCreateTargetMachine(
                target, triple, "", "", LLVMCodeGenLevelDefault, LLVMRelocPIC,
                LLVMCodeModelDefault);

        ret = LLVMTargetMachineEmitToFile(target_machine, m, "test.S",
                                          LLVMAssemblyFile, &errormsg);
        if (ret) {
                printf("LLVMTargetMachineEmitToFile failed: %s\n", errormsg);
                exit(1);
        }

        LLVMDisposeModule(m);
}
