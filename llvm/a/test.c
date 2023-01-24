// references:
// https://llvm.org/docs/LangRef.html
// https://llvm.org/doxygen/group__LLVMC.html

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/LLJIT.h>
#include <llvm-c/TargetMachine.h>

LLVMOrcThreadSafeModuleRef
create_module(void)
{
        // or LLVMContextCreate
        LLVMContextRef context = LLVMGetGlobalContext();

        // create types
        LLVMTypeRef type_i1 = LLVMInt1TypeInContext(context);
        LLVMTypeRef type_i32 = LLVMInt32TypeInContext(context);
        LLVMTypeRef type_i64 = LLVMInt64TypeInContext(context);
        LLVMTypeRef type_i32_ptr = LLVMPointerType(type_i32, 0);

        LLVMTypeRef elems[2];
        elems[0] = type_i64;
        elems[1] = type_i32;
        LLVMTypeRef type_context = LLVMStructType(elems, 2, false);

        LLVMTypeRef rettype = LLVMVoidType();
        LLVMTypeRef paramtypes[2];
        paramtypes[0] = LLVMPointerType(type_context, 0);
        paramtypes[1] = type_i32;
        LLVMTypeRef type_func =
                LLVMFunctionType(type_i32, paramtypes, 2, false);

        // create values
        LLVMValueRef const_i32_1 = LLVMConstInt(type_i32, 1, true);

        // create a module
        LLVMModuleRef m = LLVMModuleCreateWithNameInContext("mytest", context);

        // create a function
        LLVMValueRef func = LLVMAddFunction(m, "func", type_func);
        LLVMValueRef ctx = LLVMGetParam(func, 0);
        LLVMValueRef n = LLVMGetParam(func, 1);

        // create blocks
        LLVMBasicBlockRef block1 =
                LLVMAppendBasicBlockInContext(context, func, "block1");
        LLVMBasicBlockRef block2 =
                LLVMAppendBasicBlockInContext(context, func, "block2");
        LLVMBasicBlockRef block3 =
                LLVMAppendBasicBlockInContext(context, func, "block3");
        LLVMMoveBasicBlockAfter(block2, block1);
        LLVMMoveBasicBlockAfter(block3, block2);

        // populate blocks with IRs
        LLVMBuilderRef b = LLVMCreateBuilderInContext(context);

        // block3
        LLVMPositionBuilderAtEnd(b, block3);
        LLVMValueRef phi = LLVMBuildPhi(b, type_i32, "phi");
        // LLVMBuildRetVoid(b);
        LLVMBuildRet(b, phi);

        // block1
        LLVMPositionBuilderAtEnd(b, block1);
        LLVMValueRef countp =
                LLVMBuildStructGEP2(b, type_context, ctx, 1, "countp");
        LLVMValueRef count = LLVMBuildLoad2(b, type_i32, countp, "count");
        LLVMValueRef count2 = LLVMBuildAdd(b, count, const_i32_1, "inc");
        // why does LLVMBuildStore return LLVMValueRef?
        LLVMBuildStore(b, count2, countp);
        LLVMValueRef cond = LLVMBuildICmp(b, LLVMIntULT, count2, n, "cmp");
        LLVMBuildCondBr(b, cond, block2, block3);

        LLVMAddIncoming(phi, &count2, &block1, 1);

        // block2
        LLVMPositionBuilderAtEnd(b, block2);
        LLVMValueRef args[2];
        args[0] = ctx;
        args[1] = n;
        LLVMValueRef retval =
                LLVMBuildCall2(b, type_func, func, args, 2, "call");
        LLVMSetTailCall(retval, true);

        LLVMBuildBr(b, block3);

        LLVMAddIncoming(phi, &retval, &block2, 1);

        LLVMDisposeBuilder(b);

        // dump the module
        LLVMDumpModule(m);

        // verify
        char *errormsg;
        LLVMBool ret;
        ret = LLVMVerifyModule(m, LLVMAbortProcessAction, &errormsg);
        if (ret) {
                printf("LLVMVerifyModule failed: %s\n", errormsg);
                LLVMDisposeMessage(errormsg);
                exit(1);
        }

        // emit machine code
        char *triple = LLVMGetDefaultTargetTriple();
        printf("triple: %s\n", triple);
        LLVMInitializeAllTargetInfos();
        LLVMInitializeAllTargets();
        LLVMInitializeAllTargetMCs();
        LLVMInitializeAllAsmPrinters();

        LLVMTargetRef target;
        ret = LLVMGetTargetFromTriple(triple, &target, &errormsg);
        if (ret) {
                printf("LLVMGetTargetFromTriple failed: %s\n", errormsg);
                LLVMDisposeMessage(errormsg);
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
                LLVMDisposeMessage(errormsg);
                exit(1);
        }

        LLVMOrcThreadSafeContextRef tsctx =
                LLVMOrcCreateNewThreadSafeContext();
        LLVMOrcThreadSafeModuleRef tsm =
                LLVMOrcCreateNewThreadSafeModule(m, tsctx);
        return tsm;
}

int
main(int argc, char **argv)
{
        LLVMOrcThreadSafeModuleRef tsm = create_module();

        // JIT
        LLVMOrcLLJITBuilderRef jit_builder = LLVMOrcCreateLLJITBuilder();
        LLVMOrcLLJITRef jit;
        LLVMOrcCreateLLJIT(&jit, jit_builder);
        LLVMOrcJITDylibRef jd = LLVMOrcLLJITGetMainJITDylib(jit);
        LLVMOrcLLJITAddLLVMIRModule(jit, jd, tsm);

        LLVMOrcExecutorAddress funcp;
        LLVMOrcLLJITLookup(jit, &funcp, "func");

        // test JIT'ed code
        struct uctx {
                int64_t dummy;
                int32_t count;
        } uctx;
        uctx.count = 0;
        int (*testfunc)(struct uctx *, int32_t) = (void *)funcp;
        int32_t result = testfunc(&uctx, 16);
        printf("return %" PRIu32 " (expected 16)\n", result);
        printf("uctx.count %" PRIu32 " (expected 16)\n", uctx.count);

        // done with the module
        // LLVMDisposeModule(m);
}
