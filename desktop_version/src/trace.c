#include <stdio.h>
#include <time.h>
#include <inttypes.h>

static FILE *fp_trace;
extern char __code_start;

void __attribute__ ((constructor)) trace_begin (void)
{
    fp_trace = fopen("trace.out", "w");
}

void __attribute__ ((destructor)) trace_end (void)
{
    if(fp_trace != NULL) {
        fclose(fp_trace);
    }
}

void __cyg_profile_func_enter (void *func,  void *caller) {
    if(fp_trace != NULL) {
        fprintf(fp_trace, "e 0x%"PRIxPTR" 0x%"PRIxPTR"\n", ((uintptr_t) func) - ((uintptr_t) &__code_start), ((uintptr_t) caller) - ((uintptr_t) &__code_start));
    }
}

void __cyg_profile_func_exit (void *func, void *caller) {
    if(fp_trace != NULL) {
        fprintf(fp_trace, "x 0x%"PRIxPTR" 0x%"PRIxPTR"\n", ((uintptr_t) func) - ((uintptr_t) &__code_start), ((uintptr_t) caller) - ((uintptr_t) &__code_start));
    }
}
