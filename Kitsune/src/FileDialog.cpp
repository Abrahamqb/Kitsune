#include "Other/portable-file-dialogs.h"

bool OpenFileDialog(std::string& outPath, std::string title, std::string filters) {
    auto dialog = pfd::open_file(title, "", { filters });

    if (!dialog.result().empty()) {
        outPath = dialog.result()[0];
        return true;
    }
    return false;
}