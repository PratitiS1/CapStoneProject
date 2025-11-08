#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>     //for chmod and access()
#include <sys/stat.h>  // for file Permission
#include <sstream>    // for parsing copy/move commands
#include <regex>

namespace fs = std::filesystem;

// ---------- Utility Functions ---------- //

void listFiles(const fs::path& path) {
    std::cout << "\n📂 Files in: " << path << "\n";
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_directory())
            std::cout << "[DIR]  ";
        else
            std::cout << "       ";
        std::cout << entry.path().filename().string() << "\n";
    }
}

void changeDirectory(fs::path& currentPath, const std::string& folder) {
    fs::path newPath = currentPath / folder;
    if (fs::exists(newPath) && fs::is_directory(newPath))
        currentPath = fs::canonical(newPath);
    else
        std::cout << "❌ Folder not found!\n";
}

void createFolder(const fs::path& path) {
    if (fs::create_directory(path))
        std::cout << "✅ Folder created: " << path << "\n";
    else
        std::cout << "❌ Could not create folder.\n";
}

void createFile(const fs::path& path) {
    std::ofstream file(path);
    if (file)
        std::cout << "✅ File created: " << path << "\n";
    else
        std::cout << "❌ Could not create file.\n";
}

void deleteItem(const fs::path& path) {
    if (fs::remove_all(path))
        std::cout << "🗑️ Deleted: " << path << "\n";
    else
        std::cout << "❌ Item not found.\n";
}

void copyItem(const fs::path& src, const fs::path& dest) {
    try {
        fs::copy(src, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        std::cout << "✅ Copied to " << dest << "\n";
    } catch (const std::exception& e) {
        std::cout << "❌ Copy failed: " << e.what() << "\n";
    }
}

void moveItem(const fs::path& src, const fs::path& dest) {
    try {
        fs::rename(src, dest);
        std::cout << "✅ Moved to " << dest << "\n";
    } catch (const std::exception& e) {
        std::cout << "❌ Move failed: " << e.what() << "\n";
    }
}

void searchFile(const fs::path& dir, const std::string& name) {
    std::cout << "🔍 Searching for \"" << name << "\" in " << dir << "\n";
    for (auto& entry : fs::recursive_directory_iterator(dir)) {
        if (entry.path().filename().string().find(name) != std::string::npos)
            std::cout << "   Found: " << entry.path() << "\n";
    }
}

// ---- Numeric-only chmod ---- //
void setPermissions(const fs::path& path, const std::string& modeStr) {
    if (!std::regex_match(modeStr, std::regex("^[0-7]{3}$"))) {
        std::cout << "❌ Invalid mode! Use numeric values like 400, 644, 755, 777.\n";
        return;
    }

    mode_t mode = std::stoi(modeStr, nullptr, 8); // Convert octal string to integer

    if (chmod(path.c_str(), mode) == 0)
        std::cout << "✅ Permissions updated for " << path << " (mode " << modeStr << ")\n";
    else
        perror("❌ chmod failed");
}

void showHelp() {
    std::cout << R"(
            ==============================================
              📘  Linux File Explorer Application (C++)
            ==============================================

                         ~Commands:~
          ls                         - List files and folders
          ls -l <file>               - View details
          cd <folder>                - Go inside a folder
          back                       - Go to previous folder
          mkdir <name>               - Create new folder
          touch <name>               - Create new file
          del <name>                 - Delete file or folder
          copy <src> <dest>          - Copy file/folder
          move <src> <dest>          - Move file/folder
          search <name>              - Search file/folder
          chmod <file> <mode>        - Change file permissions
                                       (e.g., 400, 644, 755, 777)
          clear                      - Clear screen
          help                       - Show commands
          exit                       - Quit program
                   

                  ==============================
)";
}

// ---------- Main Program ---------- //

int main() {
    fs::path currentPath = fs::current_path();
    std::string command;

    std::cout << "         🧭 Welcome to Linux File Explorer Application!       \n";
    std::cout << "Type 'help' for available commands.\n";

    while (true) {
        std::cout << "\n[" << currentPath << "]$ ";
        std::getline(std::cin, command);

        if (command == "exit") break;
        else if (command == "help") showHelp();
        else if (command == "ls") listFiles(currentPath);
        else if (command.rfind("ls ", 0) == 0) {
            std::string fullCmd = "cd \"" + currentPath.string() + "\" && " + command;
            system(fullCmd.c_str());
        }
        else if (command.rfind("cd ", 0) == 0) changeDirectory(currentPath, command.substr(3));
        else if (command == "back") currentPath = currentPath.parent_path();
        else if (command.rfind("mkdir ", 0) == 0) createFolder(currentPath / command.substr(6));
        else if (command.rfind("touch ", 0) == 0) createFile(currentPath / command.substr(6));
        else if (command.rfind("del ", 0) == 0) deleteItem(currentPath / command.substr(4));
        else if (command.rfind("copy ", 0) == 0) {
            std::istringstream iss(command.substr(5));
            std::string src, dest; iss >> src >> dest;
            copyItem(currentPath / src, currentPath / dest);
        }
        else if (command.rfind("move ", 0) == 0) {
            std::istringstream iss(command.substr(5));
            std::string src, dest; iss >> src >> dest;
            moveItem(currentPath / src, currentPath / dest);
        }
        else if (command.rfind("search ", 0) == 0) searchFile(currentPath, command.substr(7));
        else if (command.rfind("chmod ", 0) == 0) {
            std::istringstream iss(command.substr(6));
            std::string name, mode; iss >> name >> mode;
            setPermissions(currentPath / name, mode);
        }
        else if (command == "clear") system("clear");
        else {
            std::string fullCmd = "cd \"" + currentPath.string() + "\" && " + command;
            int ret = system(fullCmd.c_str());
            if (ret != 0)
                std::cout << "❓ Unknown or failed command.\n";
        }
    }

    std::cout << "👋 Exiting File Explorer. Goodbye!\n";
    return 0;
}
