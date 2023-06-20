## -fstack-check

on by default on xcode clang
call ___chkstk_darwin before moving the stack pointer

## -fstack-clash-protection

for some reasons, no effects on macOS.

on ubuntu, it seemingly produce code to move the stack pointer
in a way it can't jump over a guard page.

```
0000000000000000 <func>:
   0:   53                      push   %rbx
   1:   49 89 e3                mov    %rsp,%r11
   4:   49 81 eb 00 10 06 00    sub    $0x61000,%r11
   b:   48 81 ec 00 10 00 00    sub    $0x1000,%rsp
  12:   48 c7 04 24 00 00 00    movq   $0x0,(%rsp)
  19:   00 
  1a:   4c 39 dc                cmp    %r11,%rsp
  1d:   75 ec                   jne    b <func+0xb>
  1f:   48 81 ec 80 0a 00 00    sub    $0xa80,%rsp
```
cf. X86FrameLowering::emitStackProbeInlineGenericLoop

## -fstack-protector

on by default on macOS clang
canary-based corruption detection
__stack_chk_guard __stack_chk_fail

-fstack-protector-strong
-fstack-protector-all

## -fsplit-stack

https://llvm.org/docs/SegmentedStacks.html

## SafeStack

https://clang.llvm.org/docs/SafeStack.html
-fsanitize=safe-stack

## others

### windows _chkstk
http://msdn.microsoft.com/en-us/library/ms648426.aspx
cf. X86FrameLowering::emitStackProbeCall

