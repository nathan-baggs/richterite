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

namespace
{

auto patch() -> void
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

::DWORD WINAPI DllMain(::HINSTANCE, ::DWORD fdwReason, ::LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        richterite::log("winmm.dll loaded");

        patch();
    }

    return 1;
}
}
