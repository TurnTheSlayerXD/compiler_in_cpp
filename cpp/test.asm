	.def	@feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
.set @feat.00, 0
	.file	"test.c"
	.def	sprintf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,sprintf
	.globl	sprintf                         # -- Begin function sprintf
	.p2align	4
sprintf:                                # @sprintf
.seh_proc sprintf
# %bb.0:
	subq	$72, %rsp
	.seh_stackalloc 72
	.seh_endprologue
	movq	%r9, 104(%rsp)
	movq	%r8, 96(%rsp)
	movq	%rdx, 64(%rsp)
	movq	%rcx, 56(%rsp)
	leaq	96(%rsp), %rax
	movq	%rax, 40(%rsp)
	movq	40(%rsp), %r9
	movq	64(%rsp), %rdx
	movq	56(%rsp), %rcx
	xorl	%eax, %eax
	movl	%eax, %r8d
	callq	_vsprintf_l
	movl	%eax, 52(%rsp)
	movl	52(%rsp), %eax
	addq	$72, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	vsprintf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,vsprintf
	.globl	vsprintf                        # -- Begin function vsprintf
	.p2align	4
vsprintf:                               # @vsprintf
.seh_proc vsprintf
# %bb.0:
	subq	$72, %rsp
	.seh_stackalloc 72
	.seh_endprologue
	movq	%r8, 64(%rsp)
	movq	%rdx, 56(%rsp)
	movq	%rcx, 48(%rsp)
	movq	64(%rsp), %rax
	movq	56(%rsp), %r8
	movq	48(%rsp), %rcx
	movq	$-1, %rdx
	xorl	%r9d, %r9d
                                        # kill: def $r9 killed $r9d
	movq	%rax, 32(%rsp)
	callq	_vsnprintf_l
	nop
	addq	$72, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	_snprintf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_snprintf
	.globl	_snprintf                       # -- Begin function _snprintf
	.p2align	4
_snprintf:                              # @_snprintf
.seh_proc _snprintf
# %bb.0:
	subq	$72, %rsp
	.seh_stackalloc 72
	.seh_endprologue
	movq	%r9, 104(%rsp)
	movq	%r8, 64(%rsp)
	movq	%rdx, 56(%rsp)
	movq	%rcx, 48(%rsp)
	leaq	104(%rsp), %rax
	movq	%rax, 32(%rsp)
	movq	32(%rsp), %r9
	movq	64(%rsp), %r8
	movq	56(%rsp), %rdx
	movq	48(%rsp), %rcx
	callq	_vsnprintf
	movl	%eax, 44(%rsp)
	movl	44(%rsp), %eax
	addq	$72, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	_vsnprintf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_vsnprintf
	.globl	_vsnprintf                      # -- Begin function _vsnprintf
	.p2align	4
_vsnprintf:                             # @_vsnprintf
.seh_proc _vsnprintf
# %bb.0:
	subq	$72, %rsp
	.seh_stackalloc 72
	.seh_endprologue
	movq	%r9, 64(%rsp)
	movq	%r8, 56(%rsp)
	movq	%rdx, 48(%rsp)
	movq	%rcx, 40(%rsp)
	movq	64(%rsp), %rax
	movq	56(%rsp), %r8
	movq	48(%rsp), %rdx
	movq	40(%rsp), %rcx
	xorl	%r9d, %r9d
                                        # kill: def $r9 killed $r9d
	movq	%rax, 32(%rsp)
	callq	_vsnprintf_l
	nop
	addq	$72, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	fuu;
	.scl	2;
	.type	32;
	.endef
	.text
	.globl	fuu                             # -- Begin function fuu
	.p2align	4
fuu:                                    # @fuu
.seh_proc fuu
# %bb.0:
	subq	$120, %rsp
	.seh_stackalloc 120
	.seh_endprologue
	movq	%rdx, 48(%rsp)                  # 8-byte Spill
	movq	%rcx, 56(%rsp)                  # 8-byte Spill
	movq	%rcx, %rax
	movq	%rax, 64(%rsp)                  # 8-byte Spill
	movq	168(%rsp), %rax
	movq	%rax, 72(%rsp)                  # 8-byte Spill
	movb	160(%rsp), %al
	movq	%rcx, 112(%rsp)
	movl	%r9d, 108(%rsp)
	movq	%r8, 96(%rsp)
	movq	%rdx, 88(%rsp)
	movl	$0, 84(%rsp)
.LBB4_1:                                # =>This Inner Loop Header: Depth=1
	movslq	84(%rsp), %rax
	cmpq	$69, %rax
	jae	.LBB4_4
# %bb.2:                                #   in Loop: Header=BB4_1 Depth=1
	movl	84(%rsp), %eax
	movl	$8, %ecx
	cltd
	idivl	%ecx
	movq	48(%rsp), %rax                  # 8-byte Reload
                                        # kill: def $dl killed $dl killed $edx
	movslq	84(%rsp), %rcx
	movb	%dl, (%rax,%rcx)
	movslq	84(%rsp), %rcx
	movsbl	(%rax,%rcx), %edx
	leaq	"??_C@_02HAOIJKIC@?$CFc?$AA@"(%rip), %rcx
	callq	printf
# %bb.3:                                #   in Loop: Header=BB4_1 Depth=1
	movl	84(%rsp), %eax
	addl	$1, %eax
	movl	%eax, 84(%rsp)
	jmp	.LBB4_1
.LBB4_4:
	movl	$0, 80(%rsp)
.LBB4_5:                                # =>This Inner Loop Header: Depth=1
	movslq	80(%rsp), %rax
	cmpq	$69, %rax
	jae	.LBB4_8
# %bb.6:                                #   in Loop: Header=BB4_5 Depth=1
	movl	80(%rsp), %eax
	movl	$8, %ecx
	cltd
	idivl	%ecx
	movq	72(%rsp), %rax                  # 8-byte Reload
                                        # kill: def $dl killed $dl killed $edx
	movslq	80(%rsp), %rcx
	movb	%dl, (%rax,%rcx)
	movslq	80(%rsp), %rcx
	movsbl	(%rax,%rcx), %edx
	leaq	"??_C@_02HAOIJKIC@?$CFc?$AA@"(%rip), %rcx
	callq	printf
# %bb.7:                                #   in Loop: Header=BB4_5 Depth=1
	movl	80(%rsp), %eax
	addl	$1, %eax
	movl	%eax, 80(%rsp)
	jmp	.LBB4_5
.LBB4_8:
	movq	48(%rsp), %rdx                  # 8-byte Reload
	movq	72(%rsp), %rcx                  # 8-byte Reload
	movq	%rsp, %rax
	movq	%rcx, 40(%rax)
	leaq	160(%rsp), %rcx
	movq	%rcx, 32(%rax)
	leaq	"??_C@_0BD@IHOBFMNG@?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?$AA@"(%rip), %rcx
	leaq	96(%rsp), %r8
	leaq	108(%rsp), %r9
	callq	printf
	movsbl	160(%rsp), %eax
	cmpl	$0, %eax
	jle	.LBB4_10
# %bb.9:
	movq	48(%rsp), %rdx                  # 8-byte Reload
	movq	56(%rsp), %rcx                  # 8-byte Reload
	movl	$69, %r8d
	callq	memcpy
	jmp	.LBB4_11
.LBB4_10:
	movq	72(%rsp), %rdx                  # 8-byte Reload
	movq	56(%rsp), %rcx                  # 8-byte Reload
	movl	$69, %r8d
	callq	memcpy
.LBB4_11:
	movq	64(%rsp), %rax                  # 8-byte Reload
	addq	$120, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	printf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,printf
	.globl	printf                          # -- Begin function printf
	.p2align	4
printf:                                 # @printf
.seh_proc printf
# %bb.0:
	subq	$72, %rsp
	.seh_stackalloc 72
	.seh_endprologue
	movq	%r9, 104(%rsp)
	movq	%r8, 96(%rsp)
	movq	%rdx, 88(%rsp)
	movq	%rcx, 64(%rsp)
	leaq	88(%rsp), %rax
	movq	%rax, 48(%rsp)
	movq	48(%rsp), %rax
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	64(%rsp), %rax
	movq	%rax, 32(%rsp)                  # 8-byte Spill
	movl	$1, %ecx
	callq	__acrt_iob_func
	movq	32(%rsp), %rdx                  # 8-byte Reload
	movq	40(%rsp), %r9                   # 8-byte Reload
	movq	%rax, %rcx
	xorl	%eax, %eax
	movl	%eax, %r8d
	callq	_vfprintf_l
	movl	%eax, 60(%rsp)
	movl	60(%rsp), %eax
	addq	$72, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	bar;
	.scl	2;
	.type	32;
	.endef
	.text
	.globl	bar                             # -- Begin function bar
	.p2align	4
bar:                                    # @bar
.seh_proc bar
# %bb.0:
	subq	$104, %rsp
	.seh_stackalloc 104
	.seh_endprologue
	movq	%rdx, 40(%rsp)                  # 8-byte Spill
	movq	%rcx, 48(%rsp)                  # 8-byte Spill
	movq	%rcx, %rax
	movq	%rax, 56(%rsp)                  # 8-byte Spill
	movq	%rcx, 96(%rsp)
	movq	%r8, 88(%rsp)
	movl	%r9d, 84(%rsp)
	movq	%rdx, 72(%rsp)
	movl	$0, 68(%rsp)
.LBB6_1:                                # =>This Inner Loop Header: Depth=1
	movslq	68(%rsp), %rax
	cmpq	$69, %rax
	jae	.LBB6_4
# %bb.2:                                #   in Loop: Header=BB6_1 Depth=1
	movq	40(%rsp), %rax                  # 8-byte Reload
	movslq	68(%rsp), %rcx
	movsbl	(%rax,%rcx), %ecx
	movl	68(%rsp), %eax
	movl	$8, %r8d
	cltd
	idivl	%r8d
	movq	40(%rsp), %rax                  # 8-byte Reload
	addl	%edx, %ecx
	movb	%cl, %dl
	movslq	68(%rsp), %rcx
	movb	%dl, (%rax,%rcx)
	movslq	68(%rsp), %rcx
	movsbl	(%rax,%rcx), %edx
	leaq	"??_C@_02HAOIJKIC@?$CFc?$AA@"(%rip), %rcx
	callq	printf
# %bb.3:                                #   in Loop: Header=BB6_1 Depth=1
	movl	68(%rsp), %eax
	addl	$1, %eax
	movl	%eax, 68(%rsp)
	jmp	.LBB6_1
.LBB6_4:
	movl	$0, 64(%rsp)
.LBB6_5:                                # =>This Inner Loop Header: Depth=1
	movslq	64(%rsp), %rax
	cmpq	$8, %rax
	jae	.LBB6_8
# %bb.6:                                #   in Loop: Header=BB6_5 Depth=1
	movslq	64(%rsp), %rax
	movsbl	88(%rsp,%rax), %eax
	movl	%eax, 36(%rsp)                  # 4-byte Spill
	movl	64(%rsp), %eax
	movl	$8, %ecx
	cltd
	idivl	%ecx
	movl	36(%rsp), %eax                  # 4-byte Reload
	addl	%edx, %eax
	movb	%al, %cl
	movslq	64(%rsp), %rax
	movb	%cl, 88(%rsp,%rax)
	movslq	64(%rsp), %rax
	movsbl	88(%rsp,%rax), %edx
	leaq	"??_C@_02HAOIJKIC@?$CFc?$AA@"(%rip), %rcx
	callq	printf
# %bb.7:                                #   in Loop: Header=BB6_5 Depth=1
	movl	64(%rsp), %eax
	addl	$1, %eax
	movl	%eax, 64(%rsp)
	jmp	.LBB6_5
.LBB6_8:
	movq	40(%rsp), %rdx                  # 8-byte Reload
	leaq	"??_C@_06EBIOKLLN@?$CFp?0?5?$CFp?$AA@"(%rip), %rcx
	leaq	88(%rsp), %r8
	callq	printf
	cmpl	$0, 84(%rsp)
	jle	.LBB6_10
# %bb.9:
	movq	40(%rsp), %rdx                  # 8-byte Reload
	movq	48(%rsp), %rcx                  # 8-byte Reload
	movl	$69, %r8d
	callq	memcpy
	jmp	.LBB6_11
.LBB6_10:
	movq	40(%rsp), %rdx                  # 8-byte Reload
	movq	48(%rsp), %rcx                  # 8-byte Reload
	movl	$69, %r8d
	callq	memcpy
.LBB6_11:
	movq	56(%rsp), %rax                  # 8-byte Reload
	addq	$104, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	big;
	.scl	2;
	.type	32;
	.endef
	.globl	big                             # -- Begin function big
	.p2align	4
big:                                    # @big
.seh_proc big
# %bb.0:
	subq	$104, %rsp
	.seh_stackalloc 104
	.seh_endprologue
	movq	%rcx, 32(%rsp)                  # 8-byte Spill
	movq	%rcx, %rax
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	movq	152(%rsp), %rax
	movq	%rax, 48(%rsp)                  # 8-byte Spill
	movq	144(%rsp), %rax
	movq	%rcx, 96(%rsp)
	movq	%r9, 88(%rsp)
	movq	%r8, 80(%rsp)
	movq	%rdx, 72(%rsp)
	movl	$0, 68(%rsp)
	movl	$0, 64(%rsp)
.LBB7_1:                                # =>This Inner Loop Header: Depth=1
	cmpl	$69, 64(%rsp)
	jge	.LBB7_4
# %bb.2:                                #   in Loop: Header=BB7_1 Depth=1
	movq	48(%rsp), %rax                  # 8-byte Reload
	movslq	64(%rsp), %rcx
	movsbl	(%rax,%rcx), %eax
	addl	68(%rsp), %eax
	movl	%eax, 68(%rsp)
# %bb.3:                                #   in Loop: Header=BB7_1 Depth=1
	movl	64(%rsp), %eax
	addl	$1, %eax
	movl	%eax, 64(%rsp)
	jmp	.LBB7_1
.LBB7_4:
	movl	$0, 60(%rsp)
.LBB7_5:                                # =>This Inner Loop Header: Depth=1
	cmpl	$69, 60(%rsp)
	jge	.LBB7_8
# %bb.6:                                #   in Loop: Header=BB7_5 Depth=1
	movq	48(%rsp), %rax                  # 8-byte Reload
	movl	68(%rsp), %ecx
	movb	%cl, %dl
	movslq	60(%rsp), %rcx
	movb	%dl, (%rax,%rcx)
# %bb.7:                                #   in Loop: Header=BB7_5 Depth=1
	movl	60(%rsp), %eax
	addl	$1, %eax
	movl	%eax, 60(%rsp)
	jmp	.LBB7_5
.LBB7_8:
	movq	48(%rsp), %rdx                  # 8-byte Reload
	movq	32(%rsp), %rcx                  # 8-byte Reload
	movl	$69, %r8d
	callq	memcpy
	movq	40(%rsp), %rax                  # 8-byte Reload
	addq	$104, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	main;
	.scl	2;
	.type	32;
	.endef
	.globl	main                            # -- Begin function main
	.p2align	4
main:                                   # @main
.seh_proc main
# %bb.0:
	subq	$824, %rsp                      # imm = 0x338
	.seh_stackalloc 824
	.seh_endprologue
	movq	%rdx, 816(%rsp)
	movl	%ecx, 812(%rsp)
	leaq	743(%rsp), %rcx
	xorl	%edx, %edx
	movl	$69, %r8d
	callq	memset
	leaq	674(%rsp), %rcx
	xorl	%edx, %edx
	movl	$69, %r8d
	callq	memset
	leaq	605(%rsp), %rcx
	xorl	%edx, %edx
	movl	$69, %r8d
	callq	memset
	leaq	536(%rsp), %rcx
	xorl	%edx, %edx
	movl	$69, %r8d
	callq	memset
	leaq	467(%rsp), %rcx
	xorl	%edx, %edx
	movl	$69, %r8d
	callq	memset
	leaq	329(%rsp), %rcx
	leaq	467(%rsp), %rdx
	movl	$69, %r8d
	callq	memcpy
	leaq	260(%rsp), %rcx
	leaq	536(%rsp), %rdx
	movl	$69, %r8d
	callq	memcpy
	leaq	191(%rsp), %rcx
	leaq	605(%rsp), %rdx
	movl	$69, %r8d
	callq	memcpy
	leaq	122(%rsp), %rcx
	leaq	674(%rsp), %rdx
	movl	$69, %r8d
	callq	memcpy
	leaq	53(%rsp), %rcx
	leaq	743(%rsp), %rdx
	movl	$69, %r8d
	callq	memcpy
	leaq	398(%rsp), %rcx
	leaq	329(%rsp), %rdx
	leaq	260(%rsp), %r8
	leaq	191(%rsp), %r9
	leaq	122(%rsp), %r10
	leaq	53(%rsp), %rax
	movq	%r10, 32(%rsp)
	movq	%rax, 40(%rsp)
	callq	big
	xorl	%eax, %eax
	addq	$824, %rsp                      # imm = 0x338
	retq
	.seh_endproc
                                        # -- End function
	.def	_vsprintf_l;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_vsprintf_l
	.globl	_vsprintf_l                     # -- Begin function _vsprintf_l
	.p2align	4
_vsprintf_l:                            # @_vsprintf_l
.seh_proc _vsprintf_l
# %bb.0:
	subq	$72, %rsp
	.seh_stackalloc 72
	.seh_endprologue
	movq	%r9, 64(%rsp)
	movq	%r8, 56(%rsp)
	movq	%rdx, 48(%rsp)
	movq	%rcx, 40(%rsp)
	movq	64(%rsp), %rax
	movq	56(%rsp), %r9
	movq	48(%rsp), %r8
	movq	40(%rsp), %rcx
	movq	$-1, %rdx
	movq	%rax, 32(%rsp)
	callq	_vsnprintf_l
	nop
	addq	$72, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	_vsnprintf_l;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_vsnprintf_l
	.globl	_vsnprintf_l                    # -- Begin function _vsnprintf_l
	.p2align	4
_vsnprintf_l:                           # @_vsnprintf_l
.seh_proc _vsnprintf_l
# %bb.0:
	subq	$136, %rsp
	.seh_stackalloc 136
	.seh_endprologue
	movq	176(%rsp), %rax
	movq	%r9, 128(%rsp)
	movq	%r8, 120(%rsp)
	movq	%rdx, 112(%rsp)
	movq	%rcx, 104(%rsp)
	movq	176(%rsp), %rax
	movq	%rax, 88(%rsp)                  # 8-byte Spill
	movq	128(%rsp), %rax
	movq	%rax, 80(%rsp)                  # 8-byte Spill
	movq	120(%rsp), %rax
	movq	%rax, 72(%rsp)                  # 8-byte Spill
	movq	112(%rsp), %rax
	movq	%rax, 64(%rsp)                  # 8-byte Spill
	movq	104(%rsp), %rax
	movq	%rax, 56(%rsp)                  # 8-byte Spill
	callq	__local_stdio_printf_options
	movq	56(%rsp), %rdx                  # 8-byte Reload
	movq	64(%rsp), %r8                   # 8-byte Reload
	movq	72(%rsp), %r9                   # 8-byte Reload
	movq	80(%rsp), %r10                  # 8-byte Reload
	movq	%rax, %rcx
	movq	88(%rsp), %rax                  # 8-byte Reload
	movq	(%rcx), %rcx
	orq	$1, %rcx
	movq	%r10, 32(%rsp)
	movq	%rax, 40(%rsp)
	callq	__stdio_common_vsprintf
	movl	%eax, 100(%rsp)
	cmpl	$0, 100(%rsp)
	jge	.LBB10_2
# %bb.1:
	movl	$4294967295, %eax               # imm = 0xFFFFFFFF
	movl	%eax, 52(%rsp)                  # 4-byte Spill
	jmp	.LBB10_3
.LBB10_2:
	movl	100(%rsp), %eax
	movl	%eax, 52(%rsp)                  # 4-byte Spill
.LBB10_3:
	movl	52(%rsp), %eax                  # 4-byte Reload
	addq	$136, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.def	__local_stdio_printf_options;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,__local_stdio_printf_options
	.globl	__local_stdio_printf_options    # -- Begin function __local_stdio_printf_options
	.p2align	4
__local_stdio_printf_options:           # @__local_stdio_printf_options
# %bb.0:
	leaq	__local_stdio_printf_options._OptionsStorage(%rip), %rax
	retq
                                        # -- End function
	.def	_vfprintf_l;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_vfprintf_l
	.globl	_vfprintf_l                     # -- Begin function _vfprintf_l
	.p2align	4
_vfprintf_l:                            # @_vfprintf_l
.seh_proc _vfprintf_l
# %bb.0:
	subq	$104, %rsp
	.seh_stackalloc 104
	.seh_endprologue
	movq	%r9, 96(%rsp)
	movq	%r8, 88(%rsp)
	movq	%rdx, 80(%rsp)
	movq	%rcx, 72(%rsp)
	movq	96(%rsp), %rax
	movq	%rax, 64(%rsp)                  # 8-byte Spill
	movq	88(%rsp), %rax
	movq	%rax, 56(%rsp)                  # 8-byte Spill
	movq	80(%rsp), %rax
	movq	%rax, 48(%rsp)                  # 8-byte Spill
	movq	72(%rsp), %rax
	movq	%rax, 40(%rsp)                  # 8-byte Spill
	callq	__local_stdio_printf_options
	movq	40(%rsp), %rdx                  # 8-byte Reload
	movq	48(%rsp), %r8                   # 8-byte Reload
	movq	56(%rsp), %r9                   # 8-byte Reload
	movq	%rax, %rcx
	movq	64(%rsp), %rax                  # 8-byte Reload
	movq	(%rcx), %rcx
	movq	%rax, 32(%rsp)
	callq	__stdio_common_vfprintf
	nop
	addq	$104, %rsp
	retq
	.seh_endproc
                                        # -- End function
	.section	.rdata,"dr",discard,"??_C@_02HAOIJKIC@?$CFc?$AA@"
	.globl	"??_C@_02HAOIJKIC@?$CFc?$AA@"   # @"??_C@_02HAOIJKIC@?$CFc?$AA@"
"??_C@_02HAOIJKIC@?$CFc?$AA@":
	.asciz	"%c"

	.section	.rdata,"dr",discard,"??_C@_0BD@IHOBFMNG@?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?$AA@"
	.globl	"??_C@_0BD@IHOBFMNG@?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?$AA@" # @"??_C@_0BD@IHOBFMNG@?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?$AA@"
"??_C@_0BD@IHOBFMNG@?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?0?5?$CFp?$AA@":
	.asciz	"%p, %p, %p, %p, %p"

	.section	.rdata,"dr",discard,"??_C@_06EBIOKLLN@?$CFp?0?5?$CFp?$AA@"
	.globl	"??_C@_06EBIOKLLN@?$CFp?0?5?$CFp?$AA@" # @"??_C@_06EBIOKLLN@?$CFp?0?5?$CFp?$AA@"
"??_C@_06EBIOKLLN@?$CFp?0?5?$CFp?$AA@":
	.asciz	"%p, %p"

	.lcomm	__local_stdio_printf_options._OptionsStorage,8,8 # @__local_stdio_printf_options._OptionsStorage
	.addrsig
	.addrsig_sym _vsnprintf
	.addrsig_sym printf
	.addrsig_sym big
	.addrsig_sym _vsprintf_l
	.addrsig_sym _vsnprintf_l
	.addrsig_sym __stdio_common_vsprintf
	.addrsig_sym __local_stdio_printf_options
	.addrsig_sym _vfprintf_l
	.addrsig_sym __acrt_iob_func
	.addrsig_sym __stdio_common_vfprintf
	.addrsig_sym __local_stdio_printf_options._OptionsStorage
