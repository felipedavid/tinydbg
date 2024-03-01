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
    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_information;

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
    printf("status %d\n", status);
    if (status == 0) {
        fprintf(stderr, "Unable to create process");
        return -1;
    }

    DEBUG_EVENT debug_event;
    char input_buf[256];
    for (;;) {
        WaitForDebugEvent(&debug_event, INFINITE);
        printf("%s\n", debug_event_name[debug_event.dwDebugEventCode]);

        if (debug_event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) break;

        for (;;) {
            printf("> ");
            fgets(input_buf, sizeof(input_buf), stdin);
            if (!strcmp(input_buf, "g")) {
            } else {
                printf("Unknown command: '%s'", input_buf);
            }
        }

        ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
    }
}