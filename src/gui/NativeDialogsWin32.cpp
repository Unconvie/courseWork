#include "visionCore/gui/NativeDialogs.hpp"

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shobjidl.h>

#include <vector>

namespace visionCore::gui {

namespace {

std::string wideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return {};
    }
    const int size = WideCharToMultiByte(
        CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }
    std::string utf8(static_cast<std::size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), size,
                        nullptr, nullptr);
    return utf8;
}

bool pickPath(std::string& outPathUtf8, bool folder) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    const bool needUninit = SUCCEEDED(hr);

    IFileOpenDialog* dialog = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
                          IID_IFileOpenDialog,
                          reinterpret_cast<void**>(&dialog));
    if (FAILED(hr) || !dialog) {
        if (needUninit) {
            CoUninitialize();
        }
        return false;
    }

    DWORD options = 0;
    dialog->GetOptions(&options);
    if (folder) {
        dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    } else {
        dialog->SetOptions(options | FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);
        const COMDLG_FILTERSPEC filters[] = {
            {L"Изображения",
             L"*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff"},
            {L"Все файлы", L"*.*"},
        };
        dialog->SetFileTypes(2, filters);
    }

    hr = dialog->Show(nullptr);
    if (FAILED(hr)) {
        dialog->Release();
        if (needUninit) {
            CoUninitialize();
        }
        return false;
    }

    IShellItem* item = nullptr;
    hr = dialog->GetResult(&item);
    dialog->Release();
    if (FAILED(hr) || !item) {
        if (needUninit) {
            CoUninitialize();
        }
        return false;
    }

    PWSTR path = nullptr;
    hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
    item->Release();
    if (FAILED(hr) || !path) {
        if (needUninit) {
            CoUninitialize();
        }
        return false;
    }

    outPathUtf8 = wideToUtf8(path);
    CoTaskMemFree(path);

    if (needUninit) {
        CoUninitialize();
    }
    return !outPathUtf8.empty();
}

}  // anonymous namespace

bool pickImageFile(std::string& outPathUtf8) {
    return pickPath(outPathUtf8, false);
}

bool pickFolder(std::string& outPathUtf8) {
    return pickPath(outPathUtf8, true);
}

}  // namespace visionCore::gui

#else

namespace visionCore::gui {

bool pickImageFile(std::string&) { return false; }
bool pickFolder(std::string&) { return false; }

}  // namespace visionCore::gui

#endif
