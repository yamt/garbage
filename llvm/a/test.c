#include <stdbool.h>
#include <stddef.h>

#include <llvm-c/Core.h>

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
        LLVMDisposeModule(m);
}
