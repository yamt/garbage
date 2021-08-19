	.text
	.file	"mod.c"
	.section	.text.do_some_file_io,"",@
	.hidden	do_some_file_io                 # -- Begin function do_some_file_io
	.globl	do_some_file_io
	.type	do_some_file_io,@function
do_some_file_io:                        # @do_some_file_io
	.functype	do_some_file_io () -> ()
	.local  	i32, i32
# %bb.0:
	global.get	__stack_pointer
	i32.const	80
	i32.sub 
	local.tee	0
	global.set	__stack_pointer
	local.get	0
	i32.const	.L.str
	i32.store	16
	i32.const	.L.str.1
	local.get	0
	i32.const	16
	i32.add 
	call	printf
	drop
	block   	
	block   	
	block   	
	i32.const	.L.str
	i32.const	.L.str.2
	call	fopen
	local.tee	1
	i32.eqz
	br_if   	0                               # 0: down to label2
# %bb.1:
	local.get	0
	i32.const	32
	i32.add 
	i32.const	.L__const.do_some_file_io.buf
	i32.const	44
	call	memcpy
	drop
	local.get	0
	i32.const	32
	i32.add 
	i32.const	43
	i32.const	1
	local.get	1
	call	fwrite
	i32.const	1
	i32.ne  
	br_if   	1                               # 1: down to label1
# %bb.2:
	local.get	1
	call	fclose
	br_if   	2                               # 2: down to label0
# %bb.3:
	local.get	0
	i32.const	80
	i32.add 
	global.set	__stack_pointer
	return
.LBB0_4:
	end_block                               # label2:
	local.get	0
	i32.const	0
	i32.load	errno
	call	strerror
	i32.store	0
	i32.const	.L.str.3
	local.get	0
	call	printf
	drop
	i32.const	.L.str.4
	i32.const	.L.str.5
	i32.const	21
	i32.const	.L__func__.do_some_file_io
	call	__assert_fail
	unreachable
.LBB0_5:
	end_block                               # label1:
	i32.const	.L.str.6
	i32.const	.L.str.5
	i32.const	26
	i32.const	.L__func__.do_some_file_io
	call	__assert_fail
	unreachable
.LBB0_6:
	end_block                               # label0:
	i32.const	.L.str.7
	i32.const	.L.str.5
	i32.const	30
	i32.const	.L__func__.do_some_file_io
	call	__assert_fail
	unreachable
	end_function
.Lfunc_end0:
	.size	do_some_file_io, .Lfunc_end0-do_some_file_io
                                        # -- End function
	.section	.text.cb1,"",@
	.hidden	cb1                             # -- Begin function cb1
	.globl	cb1
	.type	cb1,@function
cb1:                                    # @cb1
	.functype	cb1 (i32) -> (i32)
	.local  	i32
# %bb.0:
	global.get	__stack_pointer
	i32.const	16
	i32.sub 
	local.tee	1
	global.set	__stack_pointer
	local.get	1
	local.get	0
	i32.store	0
	i32.const	.L.str.8
	local.get	1
	call	printf
	drop
	local.get	1
	i32.const	16
	i32.add 
	global.set	__stack_pointer
	local.get	0
                                        # fallthrough-return
	end_function
.Lfunc_end1:
	.size	cb1, .Lfunc_end1-cb1
                                        # -- End function
	.section	.text.cb2,"",@
	.hidden	cb2                             # -- Begin function cb2
	.globl	cb2
	.type	cb2,@function
cb2:                                    # @cb2
	.functype	cb2 (i32) -> (i32)
	.local  	i32
# %bb.0:
	global.get	__stack_pointer
	i32.const	16
	i32.sub 
	local.tee	1
	global.set	__stack_pointer
	local.get	1
	local.get	0
	i32.store	0
	i32.const	.L.str.9
	local.get	1
	call	printf
	drop
	local.get	0
	call	free
	i32.const	.L.str.10
	call	strdup
	local.set	0
	local.get	1
	i32.const	16
	i32.add 
	global.set	__stack_pointer
	local.get	0
                                        # fallthrough-return
	end_function
.Lfunc_end2:
	.size	cb2, .Lfunc_end2-cb2
                                        # -- End function
	.section	.text.f,"",@
	.hidden	f                               # -- Begin function f
	.globl	f
	.type	f,@function
f:                                      # @f
	.functype	f (i32) -> (i32)
# %bb.0:
	local.get	0
                                        # fallthrough-return
	end_function
.Lfunc_end3:
	.size	f, .Lfunc_end3-f
                                        # -- End function
	.section	.text.__original_main,"",@
	.hidden	__original_main                 # -- Begin function __original_main
	.globl	__original_main
	.type	__original_main,@function
__original_main:                        # @__original_main
	.functype	__original_main () -> (i32)
	.local  	i32, i32
# %bb.0:
	global.get	__stack_pointer
	i32.const	32
	i32.sub 
	local.tee	0
	global.set	__stack_pointer
	local.get	0
	i32.const	100
	call	malloc
	local.tee	1
	i32.store	16
	i32.const	.L.str.11
	local.get	0
	i32.const	16
	i32.add 
	call	printf
	drop
	local.get	0
	i32.const	10
	call	add3
	i32.store	0
	i32.const	.L.str.12
	local.get	0
	call	printf
	drop
	call	do_some_file_io
	block   	
	block   	
	block   	
	block   	
	local.get	1
	i32.const	cb1
	local.get	1
	call	call
	i32.ne  
	br_if   	0                               # 0: down to label6
# %bb.1:
	i32.const	cb2
	local.get	1
	call	call
	call	puts
	drop
	i32.const	.Lstr
	call	puts
	drop
	local.get	0
	i32.const	28
	i32.add 
	i32.const	0
	i32.const	f
	local.get	1
	call	pthread_create
	br_if   	1                               # 1: down to label5
# %bb.2:
	i32.const	.Lstr.18
	call	puts
	drop
	local.get	0
	i32.load	28
	local.get	0
	i32.const	24
	i32.add 
	call	pthread_join
	br_if   	2                               # 2: down to label4
# %bb.3:
	local.get	0
	i32.load	24
	local.get	1
	i32.ne  
	br_if   	3                               # 3: down to label3
# %bb.4:
	local.get	0
	i32.const	32
	i32.add 
	global.set	__stack_pointer
	i32.const	0
	return
.LBB4_5:
	end_block                               # label6:
	i32.const	.L.str.13
	i32.const	.L.str.5
	i32.const	78
	i32.const	.L__func__.main
	call	__assert_fail
	unreachable
.LBB4_6:
	end_block                               # label5:
	i32.const	.L.str.7
	i32.const	.L.str.5
	i32.const	86
	i32.const	.L__func__.main
	call	__assert_fail
	unreachable
.LBB4_7:
	end_block                               # label4:
	i32.const	.L.str.7
	i32.const	.L.str.5
	i32.const	90
	i32.const	.L__func__.main
	call	__assert_fail
	unreachable
.LBB4_8:
	end_block                               # label3:
	i32.const	.L.str.17
	i32.const	.L.str.5
	i32.const	91
	i32.const	.L__func__.main
	call	__assert_fail
	unreachable
	end_function
.Lfunc_end4:
	.size	__original_main, .Lfunc_end4-__original_main
                                        # -- End function
	.section	.text.main,"",@
	.hidden	main                            # -- Begin function main
	.globl	main
	.type	main,@function
main:                                   # @main
	.functype	main (i32, i32) -> (i32)
# %bb.0:
	call	__original_main
                                        # fallthrough-return
	end_function
.Lfunc_end5:
	.size	main, .Lfunc_end5-main
                                        # -- End function
	.type	.L.str,@object                  # @.str
	.section	.rodata..L.str,"",@
.L.str:
	.asciz	"hoge"
	.size	.L.str, 5

	.type	.L.str.1,@object                # @.str.1
	.section	.rodata..L.str.1,"",@
.L.str.1:
	.asciz	"opening file %s\n"
	.size	.L.str.1, 17

	.type	.L.str.2,@object                # @.str.2
	.section	.rodata..L.str.2,"",@
.L.str.2:
	.asciz	"w"
	.size	.L.str.2, 2

	.type	.L.str.3,@object                # @.str.3
	.section	.rodata..L.str.3,"",@
.L.str.3:
	.asciz	"fopen failed: %s\n"
	.size	.L.str.3, 18

	.type	.L.str.4,@object                # @.str.4
	.section	.rodata..L.str.4,"",@
.L.str.4:
	.asciz	"fp != NULL"
	.size	.L.str.4, 11

	.type	.L.str.5,@object                # @.str.5
	.section	.rodata..L.str.5,"",@
.L.str.5:
	.asciz	"mod.c"
	.size	.L.str.5, 6

	.type	.L__func__.do_some_file_io,@object # @__func__.do_some_file_io
	.section	.rodata..L__func__.do_some_file_io,"",@
.L__func__.do_some_file_io:
	.asciz	"do_some_file_io"
	.size	.L__func__.do_some_file_io, 16

	.type	.L__const.do_some_file_io.buf,@object # @__const.do_some_file_io.buf
	.section	.rodata..L__const.do_some_file_io.buf,"",@
	.p2align	4
.L__const.do_some_file_io.buf:
	.asciz	"hello this is data came from a wasm module\n"
	.size	.L__const.do_some_file_io.buf, 44

	.type	.L.str.6,@object                # @.str.6
	.section	.rodata..L.str.6,"",@
.L.str.6:
	.asciz	"written == 1"
	.size	.L.str.6, 13

	.type	.L.str.7,@object                # @.str.7
	.section	.rodata..L.str.7,"",@
.L.str.7:
	.asciz	"ret == 0"
	.size	.L.str.7, 9

	.type	.L.str.8,@object                # @.str.8
	.section	.rodata..L.str.8,"",@
.L.str.8:
	.asciz	"this is cb1 %p\n"
	.size	.L.str.8, 16

	.type	.L.str.9,@object                # @.str.9
	.section	.rodata..L.str.9,"",@
.L.str.9:
	.asciz	"this is cb2 %p\n"
	.size	.L.str.9, 16

	.type	.L.str.10,@object               # @.str.10
	.section	.rodata..L.str.10,"",@
.L.str.10:
	.asciz	"hello from cb2"
	.size	.L.str.10, 15

	.type	.L.str.11,@object               # @.str.11
	.section	.rodata..L.str.11,"",@
.L.str.11:
	.asciz	"this is a wasm module %p\n"
	.size	.L.str.11, 26

	.type	.L.str.12,@object               # @.str.12
	.section	.rodata..L.str.12,"",@
.L.str.12:
	.asciz	"i = %d\n"
	.size	.L.str.12, 8

	.type	.L.str.13,@object               # @.str.13
	.section	.rodata..L.str.13,"",@
.L.str.13:
	.asciz	"p == p2"
	.size	.L.str.13, 8

	.type	.L__func__.main,@object         # @__func__.main
	.section	.rodata..L__func__.main,"",@
.L__func__.main:
	.asciz	"main"
	.size	.L__func__.main, 5

	.type	.L.str.17,@object               # @.str.17
	.section	.rodata..L.str.17,"",@
.L.str.17:
	.asciz	"v == p"
	.size	.L.str.17, 7

	.type	.Lstr,@object                   # @str
	.section	.rodata..Lstr,"",@
.Lstr:
	.asciz	"pthread_create"
	.size	.Lstr, 15

	.type	.Lstr.18,@object                # @str.18
	.section	.rodata..Lstr.18,"",@
.Lstr.18:
	.asciz	"pthread_join"
	.size	.Lstr.18, 13

	.no_dead_strip	__main_void
	.globl	__main_void
	.type	__main_void,@function
.set __main_void, __original_main
	.ident	"clang version 11.0.0 (https://github.com/llvm/llvm-project 176249bd6732a8044d457092ed932768724a6f06)"
	.globaltype	__stack_pointer, i32
	.functype	printf (i32, i32) -> (i32)
	.functype	fopen (i32, i32) -> (i32)
	.functype	strerror (i32) -> (i32)
	.functype	__assert_fail (i32, i32, i32, i32) -> ()
	.functype	fwrite (i32, i32, i32, i32) -> (i32)
	.functype	fclose (i32) -> (i32)
	.functype	free (i32) -> ()
	.functype	strdup (i32) -> (i32)
	.functype	malloc (i32) -> (i32)
	.functype	add3 (i32) -> (i32)
	.functype	call (i32, i32) -> (i32)
	.functype	pthread_create (i32, i32, i32, i32) -> (i32)
	.functype	pthread_join (i32, i32) -> (i32)
	.functype	puts (i32) -> (i32)
	.size	errno, 4
	.section	.custom_section.producers,"",@
	.int8	1
	.int8	12
	.ascii	"processed-by"
	.int8	1
	.int8	5
	.ascii	"clang"
	.int8	86
	.ascii	"11.0.0 (https://github.com/llvm/llvm-project 176249bd6732a8044d457092ed932768724a6f06)"
	.section	.rodata..Lstr.18,"",@
	.section	.custom_section.target_features,"",@
	.int8	1
	.int8	45
	.int8	10
	.ascii	"shared-mem"
	.section	.rodata..Lstr.18,"",@
