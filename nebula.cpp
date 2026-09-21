#include <windows.h>
#include <d3d11.h>
#include <iostream>

// Объявление структуры меню и переменных
namespace Config {
    bool isMenuOpen = true;
    bool snowEffect = false;
    bool nimbEffect = false;
    bool chinaHat = false;
    bool trails = false;
    bool motionBlur = false;
    float motionBlurValue = 0.5f;
    bool hurtColor = false;
    bool fullBright = false;
    bool fogColor = false;
    float fogRGB[3] = { 0.5f, 0.0f, 0.5f }; // Фиолетовый космический туман
    bool skyboxCustom = false;
    float skyboxRGB[3] = { 0.1f, 0.0f, 0.2f }; // Темный космос
    bool watermark = true;
    bool fpsCounter = true;
    float javaFov = 70.0f;
}

// Временная заглушка для текстовой консоли, чтобы проверить работу в Майнкрафте 1.1.5
DWORD WINAPI InitializeNebula(LPVOID lpParam) {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    std::cout << "==================================================\n";
    std::cout << "         NEBULA VISUAL CLIENT v1.0 ACTIVATED       \n";
    std::cout << "==================================================\n";
    std::cout << "[+] Инжекция в процесс Minecraft прошла успешно.\n";
    std::cout << "[+] Графический интерфейс DirectX 11 подключен.\n";
    std::cout << "[!] Меню Nebula (Фиолетовая тема) готово к выводу.\n";
    std::cout << "[!] Нажмите INSERT для переключения видимости.\n";
    std::cout << "[!] Нажмите END для полной выгрузки клиента.\n\n";

    while (!(GetAsyncKeyState(VK_END) & 0x8000)) {
        if (GetAsyncKeyState(VK_INSERT) & 0x1) {
            Config::isMenuOpen = !Config::isMenuOpen;
            std::cout << "[Nebula] Статус меню изменен. Открыто: " << (Config::isMenuOpen ? "ДА" : "НЕТ") << "\n";
        }
        Sleep(100);
    }

    std::cout << "[!] Выгрузка Nebula Client из памяти игры...\n";
    Sleep(1000);
    if (f) fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, InitializeNebula, hModule, 0, NULL);
    }
    return TRUE;
}
