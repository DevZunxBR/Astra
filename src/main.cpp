#include "cli/commands.hpp"
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#endif

namespace {
#ifdef _WIN32
HANDLE g_out = INVALID_HANDLE_VALUE;

void init_console() {
    g_out = GetStdHandle(STD_OUTPUT_HANDLE);
}

void set_cursor_visible(bool visible) {
    if (g_out == INVALID_HANDLE_VALUE) return;
    CONSOLE_CURSOR_INFO info{};
    if (!GetConsoleCursorInfo(g_out, &info)) return;
    info.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(g_out, &info);
}

void clear_all() {
    if (g_out == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (!GetConsoleScreenBufferInfo(g_out, &csbi)) return;

    DWORD cell_count = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD written = 0;
    COORD home = {0, 0};
    FillConsoleOutputCharacterA(g_out, ' ', cell_count, home, &written);
    FillConsoleOutputAttribute(g_out, csbi.wAttributes, cell_count, home, &written);
    SetConsoleCursorPosition(g_out, home);
}

void move_home() {
    if (g_out == INVALID_HANDLE_VALUE) return;
    COORD home = {0, 0};
    SetConsoleCursorPosition(g_out, home);
}

void print_line(const std::string& text, int width = 60) {
    std::string out = text;
    if ((int)out.size() < width) out += std::string(width - out.size(), ' ');
    std::cout << out << "\n";
}

void draw_menu(const std::vector<std::string>& items, int selected) {
    move_home();
    print_line("Astra CLI");
    print_line("");
    for (int i = 0; i < (int)items.size(); ++i) {
        std::string line = (i == selected ? "> " : "  ") + items[i];
        print_line(line);
    }
    print_line("");
    print_line("Use setas e Enter");
    std::cout.flush();
}

std::string ask_project_name() {
    clear_all();
    set_cursor_visible(true);
    std::cout << "Nome do projeto: ";
    std::string value;
    std::getline(std::cin, value);
    set_cursor_visible(false);
    return value;
}
#endif

int run_setup_wizard() {
#ifdef _WIN32
    init_console();
    set_cursor_visible(false);
    clear_all();

    std::vector<std::string> items = {
        "Create Project",
        "Init Astra Here",
        "Build",
        "Deploy",
        "Exit"
    };

    int selected = 0;
    while (true) {
        draw_menu(items, selected);

        int ch = _getch();
        if (ch == 224 || ch == 0) {
            int key = _getch();
            if (key == 72) selected = (selected - 1 + (int)items.size()) % (int)items.size();
            if (key == 80) selected = (selected + 1) % (int)items.size();
            continue;
        }
        if (ch != 13) continue;

        if (selected == 0) {
            std::string project = ask_project_name();
            if (project.empty()) continue;
            clear_all();
            set_cursor_visible(true);
            return cli::create_command({project});
        }
        if (selected == 1) {
            clear_all();
            set_cursor_visible(true);
            return cli::init_command({});
        }
        if (selected == 2) {
            clear_all();
            set_cursor_visible(true);
            return cli::build_command({});
        }
        if (selected == 3) {
            clear_all();
            set_cursor_visible(true);
            return cli::deploy_command({});
        }

        clear_all();
        set_cursor_visible(true);
        return 0;
    }
#else
    std::cout << "Astra CLI\n";
    std::cout << "1) Create Project\n2) Init Astra Here\n3) Build\n4) Deploy\n5) Exit\nEscolha: ";
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "1") {
        std::cout << "Nome do projeto: ";
        std::string name;
        std::getline(std::cin, name);
        if (name.empty()) return 1;
        return cli::create_command({name});
    }
    if (choice == "2") {
        return cli::init_command({});
    }
    if (choice == "3") {
        return cli::build_command({});
    }
    if (choice == "4") return cli::deploy_command({});
    return 0;
#endif
}
}

int main(int argc, char** argv) {
    if (argc < 2) return run_setup_wizard();

    std::string cmd = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) args.emplace_back(argv[i]);

    if (cmd == "create") return cli::create_command(args);
    if (cmd == "init") return cli::init_command(args);
    if (cmd == "dev") return cli::dev_command(args);
    if (cmd == "build") return cli::build_command(args);
    if (cmd == "deploy") return cli::deploy_command(args);
    if (cmd == "run") return cli::run_command(args);
    if (cmd == "ui") return run_setup_wizard();

    std::cout << "Comando desconhecido: " << cmd << "\n";
    return 1;
}
