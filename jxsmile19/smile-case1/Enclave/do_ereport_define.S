/*******************************************************************/

/*
 * -------------------------------------------------------------------------
 * extern "C" int do_ereport(sgx_target_info_t *target_info, sgx_report_data_t *report_data, sgx_report_t *report);
 * EREPORT: rbx - the address of TARGETINFO;
 *          rcx - the address of REPORTDATA;
 *          rdx - the address where REPORT is outputted
 * return value:
 *          zero: success
 *          non-zero: failure
 * -------------------------------------------------------------------------
 */
.text
.global do_ereport_define
do_ereport_define:
//	push   %rbx
//	push   %rcx
//	push   %rdx
mov    %rdi, %rbx
mov    %rsi, %rcx
mov    %rdx, %rdx
mov    $0, %rax
enclu
// 	setb   %al
//	pop    %rdx
//	pop    %rcx
//	pop    %rbx
retq
