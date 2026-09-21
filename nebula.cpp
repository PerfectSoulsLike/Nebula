#include <windows.h>
#include <iostream>

DWORD WINAPI NebulaThread(LPVOID lpParam) {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    std::cout << "=========================================\n";
    std::cout << "   NEBULA VISUAL CLIENT LOADED SUCCESS   \n";
    std::cout << "=========================================\n";
    std::cout << "[!] Меню ImGui будет доступно в обновлении.\n";
    std::cout << "[!] Нажмите END для выгрузки мода.\n";

    while (!(GetAsyncKeyState(VK_END) & 0x8000)) {
        Sleep(100);
    }

    if (f) fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, NebulaThread, hModule, 0, NULL);
    }
    return TRUE;
}
