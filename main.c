#include <windows.h>
#include <stdio.h>

const char *debug_event_name[10] = {
    NULL,
    [EXCEPTION_DEBUG_EVENT]      = "Exception",
    [CREATE_THREAD_DEBUG_EVENT]  = "CreateThread",
    [CREATE_PROCESS_DEBUG_EVENT] = "CreateProcess",
    [EXIT_THREAD_DEBUG_EVENT]    = "ExitThread",
    [EXIT_PROCESS_DEBUG_EVENT]   = "ExitProcess",
    [LOAD_DLL_DEBUG_EVENT]       = "LoadDLL",
    [UNLOAD_DLL_DEBUG_EVENT]     = "UnloadDLL",
    [OUTPUT_DEBUG_STRING_EVENT]  = "OutputDebugString",
    [RIP_EVENT]                  = "RIPEvent",
};

int main(int argc, char **argv) {
    STARTUPINFO startup_info = { 0 };
    PROCESS_INFORMATION process_information = { 0 };

    int status = CreateProcessA(
        NULL, 
        argv[1], 
        NULL, 
        NULL, 
        FALSE, 
        DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &startup_info,
        &process_information
    );
    if (status == 0) {
        int error_code = GetLastError();
        fprintf(stderr, "Unable to create process (error code: %d)\n", error_code);
        return -1;
    }

    DEBUG_EVENT debug_event;
    char input_buf[256];
    BOOL expect_step_exception = FALSE;
    for (;;) {
        DWORD debug_event_flags = INFINITE;
        if (expect_step_exception) debug_event_flags |= DBG_CONTINUE;

        WaitForDebugEvent(&debug_event, debug_event_flags);
        expect_step_exception = FALSE;
        printf("%s\n", debug_event_name[debug_event.dwDebugEventCode]);

        if (debug_event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) break;

        HANDLE thread = OpenThread(THREAD_GET_CONTEXT|THREAD_SET_CONTEXT, FALSE, debug_event.dwThreadId);
        CONTEXT context = {0};
        context.ContextFlags = CONTEXT_ALL;

        BOOL did_it_work = GetThreadContext(thread, &context);
        if (!did_it_work) {
            int error_code = GetLastError();
            fprintf(stderr, "Unable to get thread context (error code: %d)\n", error_code);
            return -1;
        }

        for (;;) {
            printf("> ");

            fgets(input_buf, sizeof(input_buf), stdin);
            input_buf[strlen(input_buf)-1] = 0;
            if (input_buf[0] == 'g') {
                break;
            } else if (input_buf[0] == 't') {
                context.EFlags |= (1 << 8);
                did_it_work = SetThreadContext(thread, &context);
                if (!did_it_work) {
                    printf("unable to step.\n");
                } else {
                    expect_step_exception = TRUE;
                }
            } else if (input_buf[0] == 'r') {
                printf("rax={0x%018llx} rbx={0x%018llx} rcx={0x%018llx}\n", context.Rax, context.Rbx, context.Rcx);
                printf("rdx={0x%018llx} rsi={0x%018llx} rdi={0x%018llx}\n", context.Rdx, context.Rsi, context.Rdi);
                printf("rip={0x%018llx} rsp={0x%018llx} rbp={0x%018llx}\n", context.Rip, context.Rsp, context.Rbp);
                printf(" r8={0x%018llx}  r9={0x%018llx} r10={0x%018llx}\n", context.R8, context.R9, context.R10);
                printf("r11={0x%018llx} r12={0x%018llx} r13={0x%018llx}\n", context.R11, context.R12, context.R13);
                printf("r14={0x%018llx} r15={0x%018llx} eflags={0x%010lx}\n", context.R14, context.R15, context.EFlags);
            } else {
                printf("Unknown command: '%s'", input_buf);
            }
        }

        ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
    }
}
