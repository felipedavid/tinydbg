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
    for (;;) {
        WaitForDebugEvent(&debug_event, INFINITE);
        printf("%s\n", debug_event_name[debug_event.dwDebugEventCode]);

        if (debug_event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) break;

        for (;;) {
            printf("> ");
            fgets(input_buf, sizeof(input_buf), stdin);
            input_buf[strlen(input_buf)-1] = 0;
            if (!strcmp(input_buf, "g")) {
                break;
            } else {
                printf("Unknown command: '%s'", input_buf);
            }
        }

        ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
    }
}