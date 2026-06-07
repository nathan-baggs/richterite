#include <cstring>
#include <ranges>
#include <string_view>
#include <unordered_map>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <mmsystem.h>

using namespace std::literals;

#include "utils.h"

using JoyGetNumDevsFn = ::UINT(WINAPI *)();
using JoyGetPosExFn = ::MMRESULT(WINAPI *)(::UINT, ::LPJOYINFOEX);
using JoyGetDevCapsAFn = ::MMRESULT(WINAPI *)(::UINT_PTR, ::LPJOYCAPSA, ::UINT);
using MidiInStartFn = ::MMRESULT(WINAPI *)(::HMIDIIN);
using MidiInOpenFn = ::MMRESULT(WINAPI *)(::LPHMIDIIN, ::UINT, ::DWORD_PTR, ::DWORD_PTR, ::DWORD);
using MidiInGetNumDevsFn = ::UINT(WINAPI *)();
using MidiInCloseFn = ::MMRESULT(WINAPI *)(::HMIDIIN);
using MidiInGetDevCapsAFn = ::MMRESULT(WINAPI *)(::UINT_PTR, ::LPMIDIINCAPSA, ::UINT);
using TimeGetTimeFn = ::DWORD(WINAPI *)();
using TimeBeginPeriodFn = ::MMRESULT(WINAPI *)(::UINT);
using TimeEndPeriodFn = ::MMRESULT(WINAPI *)(::UINT);

using glGetString_t = const unsigned char *(WINAPI *)(unsigned int);
using GetProcAddress_t = FARPROC(WINAPI *)(HMODULE, LPCSTR);
using LoadLibrary_t = HMODULE(WINAPI *)(LPCSTR);
using FreeLibrary_t = BOOL(WINAPI *)(HMODULE);

namespace
{

glGetString_t Original_glGetString = nullptr;
GetProcAddress_t Original_GetProcAddress = nullptr;
LoadLibrary_t Original_LoadLibrary = nullptr;
FreeLibrary_t Original_FreeLibrary = nullptr;
std::unordered_map<std::string, void *> loaded_libraries;

[[maybe_unused]] auto patch1() -> void
{
    static auto *patch_addr = reinterpret_cast<void *>(0x0044793a);
    const auto auto_prot = richterite::AutoProtect{reinterpret_cast<void *>(patch_addr), 6, PAGE_EXECUTE_READWRITE};

    std::memset(patch_addr, 0x90, 6);

    richterite::log("patch applied at {}", patch_addr);
}

[[maybe_unused]] auto patch2() -> void
{
    static auto *patch_addr = reinterpret_cast<void *>(0x00435fb0);

    static const std::uint8_t patch_bytes[] = {
        0xE8, 0xFB, 0xD5, 0xFD, 0xFF,                                     // CALL FUN_004135b0
        0xA3, 0xE0, 0x42, 0xC0, 0x00,                                     // MOV [DAT_00c042e0], EAX
        0x8B, 0x44, 0x24, 0x04,                                           // MOV EAX, dword ptr [ESP + 0x4]
        0x50,                                                             // PUSH EAX
        0xE8, 0x7C, 0x71, 0xFE, 0xFF,                                     // CALL FUN_0041d140
        0x50,                                                             // PUSH EAX (Acts as Save & Arg 4)
        0xA1, 0x44, 0x9E, 0xB9, 0x00,                                     // MOV EAX, [sv_maxclients]
        0x33, 0xC9,                                                       // XOR ECX, ECX
        0x8B, 0x50, 0x20,                                                 // MOV EDX, dword ptr [EAX + 0x20]
        0x85, 0xD2,                                                       // TEST EDX, EDX
        0x7E, 0x24,                                                       // JLE LAB_END_LOOP (+0x24)
        0x33, 0xC0,                                                       // XOR EAX, EAX
        0x8B, 0x15, 0x6C, 0x9E, 0xB9, 0x00,                               // MOV EDX, dword ptr [svs_clients]
        0x41,                                                             // INC ECX
        0xC7, 0x84, 0x10, 0x54, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // MOV [EAX + EDX*1 + 0x854], 0x0
        0x8B, 0x15, 0x44, 0x9E, 0xB9, 0x00,                               // MOV EDX, dword ptr [sv_maxclients]
        0x05, 0x80, 0xC8, 0x01, 0x00,                                     // ADD EAX, 0x1C880
        0x3B, 0x4A, 0x20,                                                 // CMP ECX, dword ptr [EDX + 0x20]
        0x7C, 0xDE,                                                       // JL LAB_LOOP_START (-0x22)
        0x8B, 0x0D, 0x64, 0x9E, 0xB9, 0x00,                               // MOV ECX, dword ptr [svs.time]
        0x8B, 0x15, 0x0C, 0x22, 0xA0, 0x00,                               // MOV EDX, dword ptr [DAT_00a0220c]
        0x51,                                                             // PUSH ECX (Arg 3)
        0x6A, 0x00,                                                       // PUSH 0x0 (Arg 2)
        0x52,                                                             // PUSH EDX (Arg 1)
        0xE8, 0x24, 0x85, 0x00, 0x00,                                     // CALL VM_Call (Recalculated offset)
        0x83, 0xC4, 0x14,                                                 // ADD ESP, 0x14
        0xC3                                                              // RET
    };

    const auto auto_prot =
        richterite::AutoProtect{reinterpret_cast<void *>(patch_addr), sizeof(patch_bytes), PAGE_EXECUTE_READWRITE};

    std::memcpy(patch_addr, patch_bytes, sizeof(patch_bytes));

    richterite::log("patch applied at {}", patch_addr);
}
}

extern "C"
{

__declspec(dllexport) ::UINT WINAPI joyGetNumDevs(void)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<JoyGetNumDevsFn>(::GetProcAddress(lib, "joyGetNumDevs"));
    }();

    return orig_func();
}

__declspec(dllexport) ::MMRESULT WINAPI joyGetPosEx(::UINT uJoyID, ::LPJOYINFOEX pji)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<JoyGetPosExFn>(::GetProcAddress(lib, "joyGetPosEx"));
    }();

    return orig_func(uJoyID, pji);
}

__declspec(dllexport) ::MMRESULT WINAPI joyGetDevCapsA(::UINT_PTR uJoyID, ::LPJOYCAPSA pjc, ::UINT cbjc)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<JoyGetDevCapsAFn>(::GetProcAddress(lib, "joyGetDevCapsA"));
    }();

    return orig_func(uJoyID, pjc, cbjc);
}

__declspec(dllexport) ::MMRESULT WINAPI midiInStart(::HMIDIIN hMidiIn)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<MidiInStartFn>(::GetProcAddress(lib, "midiInStart"));
    }();

    return orig_func(hMidiIn);
}

__declspec(dllexport) ::MMRESULT WINAPI
midiInOpen(::LPHMIDIIN phMidiIn, ::UINT uDeviceID, ::DWORD_PTR dwCallback, ::DWORD_PTR dwInstance, ::DWORD dwFlags)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<MidiInOpenFn>(::GetProcAddress(lib, "midiInOpen"));
    }();

    return orig_func(phMidiIn, uDeviceID, dwCallback, dwInstance, dwFlags);
}

__declspec(dllexport) ::UINT WINAPI midiInGetNumDevs(void)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<MidiInGetNumDevsFn>(::GetProcAddress(lib, "midiInGetNumDevs"));
    }();

    return orig_func();
}

__declspec(dllexport) ::MMRESULT WINAPI midiInClose(::HMIDIIN hMidiIn)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<MidiInCloseFn>(::GetProcAddress(lib, "midiInClose"));
    }();

    return orig_func(hMidiIn);
}

__declspec(dllexport) ::MMRESULT WINAPI midiInGetDevCapsA(::UINT_PTR uDeviceID, ::LPMIDIINCAPSA pmic, ::UINT cbmic)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<MidiInGetDevCapsAFn>(::GetProcAddress(lib, "midiInGetDevCapsA"));
    }();

    return orig_func(uDeviceID, pmic, cbmic);
}

__declspec(dllexport) ::DWORD WINAPI timeGetTime(void)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<TimeGetTimeFn>(::GetProcAddress(lib, "timeGetTime"));
    }();

    return orig_func();
}

__declspec(dllexport) ::MMRESULT WINAPI timeBeginPeriod(::UINT uPeriod)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<TimeBeginPeriodFn>(::GetProcAddress(lib, "timeBeginPeriod"));
    }();

    return orig_func(uPeriod);
}

__declspec(dllexport) ::MMRESULT WINAPI timeEndPeriod(::UINT uPeriod)
{
    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<TimeEndPeriodFn>(::GetProcAddress(lib, "timeEndPeriod"));
    }();

    return orig_func(uPeriod);
}

__declspec(dllexport) const unsigned char *WINAPI Hooked_glGetString(unsigned int name)
{
    richterite::log("hooked glGetString called");
    if (name != 0x1F03)
    {
        return Original_glGetString(name);
    }

    static char custom_ext_buffer[0x1000] = {0};
    std::memset(custom_ext_buffer, 0x90, sizeof(custom_ext_buffer) - 1);

    auto ret_addr = reinterpret_cast<std::uintptr_t>(custom_ext_buffer + 0x100);
    std::memcpy(custom_ext_buffer + 4081, &ret_addr, sizeof(ret_addr));

    // rando shellcode from the internet
    // https://idafchev.github.io/exploit/2017/09/26/writing_windows_shellcode.html
    unsigned char shellcode[] = "\x50\x53\x51\x52\x56\x57\x55\x89"
                                "\xe5\x83\xec\x18\x31\xf6\x56\x6a"
                                "\x63\x66\x68\x78\x65\x68\x57\x69"
                                "\x6e\x45\x89\x65\xfc\x31\xf6\x64"
                                "\x8b\x5e\x30\x8b\x5b\x0c\x8b\x5b"
                                "\x14\x8b\x1b\x8b\x1b\x8b\x5b\x10"
                                "\x89\x5d\xf8\x31\xc0\x8b\x43\x3c"
                                "\x01\xd8\x8b\x40\x78\x01\xd8\x8b"
                                "\x48\x24\x01\xd9\x89\x4d\xf4\x8b"
                                "\x78\x20\x01\xdf\x89\x7d\xf0\x8b"
                                "\x50\x1c\x01\xda\x89\x55\xec\x8b"
                                "\x58\x14\x31\xc0\x8b\x55\xf8\x8b"
                                "\x7d\xf0\x8b\x75\xfc\x31\xc9\xfc"
                                "\x8b\x3c\x87\x01\xd7\x66\x83\xc1"
                                "\x08\xf3\xa6\x74\x0a\x40\x39\xd8"
                                "\x72\xe5\x83\xc4\x26\xeb\x41\x8b"
                                "\x4d\xf4\x89\xd3\x8b\x55\xec\x66"
                                "\x8b\x04\x41\x8b\x04\x82\x01\xd8"
                                "\x31\xd2\x52\x68\x2e\x65\x78\x65"
                                "\x68\x63\x61\x6c\x63\x68\x6d\x33"
                                "\x32\x5c\x68\x79\x73\x74\x65\x68"
                                "\x77\x73\x5c\x53\x68\x69\x6e\x64"
                                "\x6f\x68\x43\x3a\x5c\x57\x89\xe6"
                                "\x6a\x0a\x56\xff\xd0\x83\xc4\x46"
                                "\x5d\x5f\x5e\x5a\x59\x5b\x58\xc3";
    std::memcpy(custom_ext_buffer + 0x108, shellcode, sizeof(shellcode) - 1);

    richterite::log("returning fake glGetString extensions");

    return (const unsigned char *)custom_ext_buffer;
}

FARPROC WINAPI Hooked_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    // if (HIWORD(lpProcName) != 0)
    // {
    //     if (::lstrcmpA(lpProcName, "glGetString") == 0)
    //     {
    //         richterite::log("hooked glGetString");
    //         Original_glGetString = reinterpret_cast<glGetString_t>(Original_GetProcAddress(hModule, lpProcName));
    //         return (FARPROC)&Hooked_glGetString;
    //     }
    // }
    //
    return Original_GetProcAddress(hModule, lpProcName);
}

::HMODULE WINAPI Hooked_LoadLibraryA(LPCSTR lpLibFileName)
{
    if (std::string_view{lpLibFileName} == "qagamex86.dll"sv)
    {
        const auto old_addr = loaded_libraries.find("qagamex86.dll");
        if (old_addr != std::ranges::cend(loaded_libraries))
        {
            const auto fudge =
                ::VirtualAlloc(old_addr->second, 0x6AB000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE | PAGE_GUARD);
            richterite::log("fudged in some memory: {} {} {}", fudge, old_addr->second, ::GetLastError());
        }
    }

    const auto module = Original_LoadLibrary(lpLibFileName);
    loaded_libraries[lpLibFileName] = static_cast<void *>(module);

    richterite::log("Loaded library: {} -> {}", lpLibFileName, static_cast<void *>(module));

    return module;
}

::BOOL WINAPI Hooked_FreeLibrary(HMODULE hModule)
{
    const auto result = Original_FreeLibrary(hModule);
    richterite::log("FreeLibrary called for module: {}", static_cast<void *>(hModule));

    return result;
}

// cobbled this together - ok for PoC
[[maybe_unused]] void hook_iat()
{
    auto const h_exe = ::GetModuleHandleA(nullptr);
    if (!h_exe)
    {
        return;
    }

    auto const dos_header = reinterpret_cast<::PIMAGE_DOS_HEADER>(h_exe);
    auto const nt_headers =
        reinterpret_cast<::PIMAGE_NT_HEADERS>(reinterpret_cast<::BYTE *>(h_exe) + dos_header->e_lfanew);

    auto const &import_dir = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (import_dir.Size == 0)
    {
        return;
    }

    auto const import_desc =
        reinterpret_cast<::PIMAGE_IMPORT_DESCRIPTOR>(reinterpret_cast<::BYTE *>(h_exe) + import_dir.VirtualAddress);

    for (auto desc = import_desc; desc->Name != 0; ++desc)
    {
        auto const module_name = reinterpret_cast<::LPCSTR>(reinterpret_cast<::BYTE *>(h_exe) + desc->Name);

        if (::lstrcmpiA(module_name, "KERNEL32.dll") == 0)
        {
            auto const orig_first_thunk =
                reinterpret_cast<::PIMAGE_THUNK_DATA>(reinterpret_cast<::BYTE *>(h_exe) + desc->OriginalFirstThunk);
            auto const first_thunk =
                reinterpret_cast<::PIMAGE_THUNK_DATA>(reinterpret_cast<::BYTE *>(h_exe) + desc->FirstThunk);

            for (auto oft = orig_first_thunk, ft = first_thunk; oft->u1.AddressOfData != 0; ++oft, ++ft)
            {
                if (!(oft->u1.Ordinal & IMAGE_ORDINAL_FLAG))
                {
                    auto const import_by_name = reinterpret_cast<::PIMAGE_IMPORT_BY_NAME>(
                        reinterpret_cast<::BYTE *>(h_exe) + oft->u1.AddressOfData);

                    if (::lstrcmpA(reinterpret_cast<::LPCSTR>(import_by_name->Name), "GetProcAddress") == 0)
                    {
                        const auto auto_prot = richterite::AutoProtect{
                            reinterpret_cast<void *>(&ft->u1.Function), sizeof(::DWORD_PTR), PAGE_READWRITE};

                        Original_GetProcAddress = reinterpret_cast<GetProcAddress_t>(ft->u1.Function);

                        ft->u1.Function = reinterpret_cast<::DWORD_PTR>(&Hooked_GetProcAddress);

                        richterite::log("GetProcAddress IAT hooked");
                    }
                    else if (::lstrcmpA(reinterpret_cast<::LPCSTR>(import_by_name->Name), "LoadLibraryA") == 0)
                    {
                        const auto auto_prot = richterite::AutoProtect{
                            reinterpret_cast<void *>(&ft->u1.Function), sizeof(::DWORD_PTR), PAGE_READWRITE};

                        Original_LoadLibrary = reinterpret_cast<LoadLibrary_t>(ft->u1.Function);

                        ft->u1.Function = reinterpret_cast<::DWORD_PTR>(&Hooked_LoadLibraryA);

                        richterite::log("LoadLibrary IAT hooked");
                    }
                    else if (::lstrcmpA(reinterpret_cast<::LPCSTR>(import_by_name->Name), "FreeLibrary") == 0)
                    {
                        const auto auto_prot = richterite::AutoProtect{
                            reinterpret_cast<void *>(&ft->u1.Function), sizeof(::DWORD_PTR), PAGE_READWRITE};

                        Original_FreeLibrary = reinterpret_cast<FreeLibrary_t>(ft->u1.Function);

                        ft->u1.Function = reinterpret_cast<::DWORD_PTR>(&Hooked_FreeLibrary);

                        richterite::log("FreeLibrary IAT hooked");
                    }
                }
            }
        }
    }
}

::DWORD WINAPI DllMain(::HINSTANCE, ::DWORD fdwReason, ::LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        richterite::log("winmm.dll loaded");

        hook_iat();

        // patch();
        patch2();
    }

    return 1;
}
}
