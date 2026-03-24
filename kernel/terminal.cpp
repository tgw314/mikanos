#include "terminal.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

#include "elf.hpp"
#include "fat.hpp"
#include "font.hpp"
#include "graphics.hpp"
#include "layer.hpp"
#include "message.hpp"
#include "pci.hpp"
#include "task.hpp"
#include "window.hpp"

namespace {
std::vector<char *> MakeArgVector(char *command, char *first_arg) {
    std::vector<char *> argv;
    argv.push_back(command);

    if (first_arg == nullptr) {
        return argv;
    }

    char *p = first_arg;
    for (;;) {
        while (isspace(*p)) p++;
        if (*p == '\0') break;
        argv.push_back(p);

        while (*p != '\0' && !isspace(*p)) p++;
        if (*p == '\0') break;
        *p++ = '\0';
    }

    return argv;
}

void CalcLoadAddressRange(const Elf64_Ehdr *ehdr, uint64_t *first,
                          uint64_t *last) {
    auto phdr = reinterpret_cast<const Elf64_Phdr *>(
        reinterpret_cast<uintptr_t>(ehdr) + ehdr->e_phoff);
    *first = UINT64_MAX;
    *last = 0;

    for (Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) continue;

        *first = std::min(*first, phdr[i].p_vaddr);
        *last = std::max(*last, phdr[i].p_vaddr + phdr[i].p_memsz);
    }
}

void CopyLoadSegments(const Elf64_Ehdr *ehdr, uint8_t *load_image,
                      uint64_t image_base_vaddr) {
    auto phdr = reinterpret_cast<const Elf64_Phdr *>(
        reinterpret_cast<uintptr_t>(ehdr) + ehdr->e_phoff);
    for (Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) continue;

        auto dst = load_image + (phdr[i].p_vaddr - image_base_vaddr);
        auto src = reinterpret_cast<const uint8_t *>(
            reinterpret_cast<uintptr_t>(ehdr) + phdr[i].p_offset);
        memcpy(dst, src, phdr[i].p_filesz);
        memset(dst + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
    }
}
}  // namespace

Terminal::Terminal() {
    window_ = std::make_shared<ToplevelWindow>(
        kColumns * 8 + 8 + ToplevelWindow::kMarginX,
        kRows * 16 + 8 + ToplevelWindow::kMarginY, screen_config.pixel_format,
        "MikanTerm");
    DrawTerminal(*window_->InnerWriter(), {0, 0}, window_->InnerSize());

    layer_id_ =
        layer_manager->NewLayer().SetWindow(window_).SetDraggable(true).ID();

    Print(">");
    cmd_history_.resize(8);
}

Rectangle<int> Terminal::BlinkCursor() {
    cursor_visible_ = !cursor_visible_;
    DrawCursor(cursor_visible_);

    return {CalcCursorPos(), {7, 15}};
}

void Terminal::DrawCursor(bool visible) {
    const auto color = ToColor(visible ? 0xffffff : 0);
    FillRectangle(*window_->Writer(), CalcCursorPos(), {7, 15}, color);
}

Vector2D<int> Terminal::CalcCursorPos() const {
    return ToplevelWindow::kTopLeftMargin +
           Vector2D<int>{4 + 8 * cursor_.x, 4 + 16 * cursor_.y};
}

Rectangle<int> Terminal::InputKey(uint8_t modifier, uint8_t keycode,
                                  char ascii) {
    DrawCursor(false);

    Rectangle<int> draw_area{CalcCursorPos(), {8 * 2, 16}};

    if (ascii == '\n') {
        linebuf_[linebuf_index_] = '\0';
        if (linebuf_index_ > 0) {
            cmd_history_.pop_back();
            cmd_history_.push_front(linebuf_);
        }
        linebuf_index_ = 0;
        cmd_history_index_ = -1;

        cursor_.x = 0;
        if (cursor_.y < kRows - 1) {
            cursor_.y++;
        } else {
            Scroll1();
        }
        ExecuteLine();
        Print(">");
        draw_area.pos = ToplevelWindow::kTopLeftMargin;
        draw_area.size = window_->InnerSize();
    } else if (ascii == '\b') {
        if (cursor_.x > 0) {
            cursor_.x--;
            FillRectangle(*window_->Writer(), CalcCursorPos(), {8, 16},
                          {0, 0, 0});
            draw_area.pos = CalcCursorPos();
            if (linebuf_index_ > 0) linebuf_index_--;
        }
    } else if (ascii == 0) {
        if (keycode == 0x51) {
            draw_area = HistoryUpDown(-1);
        } else if (keycode == 0x52) {
            draw_area = HistoryUpDown(1);
        }
    } else {
        if (cursor_.x < kColumns - 1 && linebuf_index_ < kLineMax - 1) {
            linebuf_[linebuf_index_++] = ascii;
            WriteAscii(*window_->Writer(), CalcCursorPos(), ascii,
                       {255, 255, 255});
            cursor_.x++;
        }
    }
    DrawCursor(true);
    return draw_area;
}

void Terminal::Scroll1() {
    Rectangle<int> move_src{
        ToplevelWindow::kTopLeftMargin + Vector2D<int>{4, 4 + 16},
        {8 * kColumns, 16 * (kRows - 1)}};
    window_->Move(ToplevelWindow::kTopLeftMargin + Vector2D<int>{4, 4},
                  move_src);
    FillRectangle(*window_->InnerWriter(), {4, 4 + 16 * cursor_.y},
                  {8 * kColumns, 16}, {0, 0, 0});
}

void Terminal::ExecuteLine() {
    char *command = &linebuf_[0];
    char *first_arg = strchr(command, ' ');
    if (first_arg) *(first_arg++) = '\0';

    if (command[0] == '\0') return;

    if (strcmp(command, "echo") == 0) {
        if (first_arg) Print(first_arg);
        Print("\n");
        return;
    }

    if (strcmp(command, "clear") == 0) {
        FillRectangle(*window_->InnerWriter(), {4, 4},
                      {8 * kColumns, 16 * kRows}, {0, 0, 0});
        cursor_.y = 0;
        return;
    }

    if (strcmp(command, "lspci") == 0) {
        char s[64];
        for (int i = 0; i < pci::num_device; ++i) {
            const auto &dev = pci::devices[i];
            auto vendor_id =
                pci::ReadVendorId(dev.bus, dev.device, dev.function);
            sprintf(s,
                    "%02x:%02x.%d vend=%04x head=%02x class=%02x.%02x.%02x\n",
                    dev.bus, dev.device, dev.function, vendor_id,
                    dev.header_type, dev.class_code.base, dev.class_code.sub,
                    dev.class_code.interface);
            Print(s);
        }
        return;
    }

    if (strcmp(command, "ls") == 0) {
        auto root_dir_entries = fat::GetSectorByCluster<fat::DirectoryEntry>(
            fat::boot_volume_image->root_cluster);
        auto entries_per_cluster =
            fat::bytes_per_cluster / sizeof(fat::DirectoryEntry);
        char base[9], ext[4];
        char s[64];
        for (int i = 0; i < entries_per_cluster; i++) {
            fat::ReadName(root_dir_entries[i], base, ext);
            if (base[0] == 0x00) return;
            if (static_cast<uint8_t>(base[0]) == 0xe5) continue;
            if (root_dir_entries[i].attr == fat::Attribute::kLongName) continue;

            if (ext[0]) {
                sprintf(s, "%s.%s\n", base, ext);
            } else {
                sprintf(s, "%s\n", base);
            }
            Print(s);
        }
        return;
    }

    if (strcmp(command, "cat") == 0) {
        auto file_entry = fat::FindFile(first_arg);
        if (!file_entry) {
            char s[64];
            sprintf(s, "no such file: %s\n", first_arg);
            Print(s);
            return;
        }

        auto cluster = file_entry->FirstCluster();
        auto remain_bytes = file_entry->file_size;

        DrawCursor(false);
        while (cluster != 0 && cluster != fat::kEndOfClusterchain) {
            char *p = fat::GetSectorByCluster<char>(cluster);
            const int bytes_to_read =
                std::min<unsigned long>(fat::bytes_per_cluster, remain_bytes);

            for (int i = 0; i < bytes_to_read; i++, p++) {
                Print(*p);
            }
            remain_bytes -= bytes_to_read;

            cluster = fat::NextCluster(cluster);
        }
        DrawCursor(true);

        return;
    }

    auto file_entry = fat::FindFile(command);
    if (file_entry) {
        ExecuteFile(*file_entry, command, first_arg);
        return;
    }

    Print("no such command: ");
    Print(command);
    Print("\n");
}

void Terminal::ExecuteFile(const fat::DirectoryEntry &file_entry, char *command,
                           char *first_arg) {
    auto cluster = file_entry.FirstCluster();
    auto remain_bytes = file_entry.file_size;

    std::vector<uint8_t> file_buf(remain_bytes);
    auto p = &file_buf[0];

    while (cluster != 0 && cluster != fat::kEndOfClusterchain) {
        const auto copy_bytes =
            std::min<unsigned long>(fat::bytes_per_cluster, remain_bytes);
        memcpy(p, fat::GetSectorByCluster<uint8_t>(cluster), copy_bytes);

        remain_bytes -= copy_bytes;
        p += copy_bytes;
        cluster = fat::NextCluster(cluster);
    }

    auto elf_header = reinterpret_cast<const Elf64_Ehdr *>(&file_buf[0]);
    if (memcmp(elf_header->e_ident,
               "\x7f"
               "ELF",
               4) != 0) {
        using Func = void();
        auto f = reinterpret_cast<Func *>(&file_buf[0]);
        f();
        return;
    }

    uint64_t load_first_vaddr, load_last_vaddr;
    CalcLoadAddressRange(elf_header, &load_first_vaddr, &load_last_vaddr);
    if (load_first_vaddr >= load_last_vaddr ||
        elf_header->e_entry < load_first_vaddr ||
        load_last_vaddr <= elf_header->e_entry) {
        Print("invalid elf entry\n");
        return;
    }

    std::vector<uint8_t> load_image(load_last_vaddr - load_first_vaddr);
    CopyLoadSegments(elf_header, &load_image[0], load_first_vaddr);

    auto argv = MakeArgVector(command, first_arg);

    auto entry_addr = reinterpret_cast<uintptr_t>(&load_image[0]);
    entry_addr += elf_header->e_entry - load_first_vaddr;
    using Func = int(int, char **);
    auto f = reinterpret_cast<Func *>(entry_addr);
    auto ret = f(argv.size(), &argv[0]);

    char s[64];
    sprintf(s, "app exited. ret = %d\n", ret);
    Print(s);
}

void Terminal::Print(char c) {
    auto newline = [this]() {
        cursor_.x = 0;
        if (cursor_.y < kRows - 1) {
            cursor_.y++;
            return;
        }
        Scroll1();
    };

    if (c == '\n') {
        newline();
        return;
    }

    WriteAscii(*window_->Writer(), CalcCursorPos(), c, {255, 255, 255});
    if (cursor_.x == kColumns - 1) {
        newline();
        return;
    }

    cursor_.x++;
}

void Terminal::Print(const char *s) {
    DrawCursor(false);
    for (; *s; s++) Print(*s);
    DrawCursor(true);
}

Rectangle<int> Terminal::HistoryUpDown(int direction) {
    if (direction == -1 && cmd_history_index_ >= 0) {
        cmd_history_index_--;
    } else if (direction == 1 && cmd_history_index_ + 1 < cmd_history_.size()) {
        cmd_history_index_++;
    }

    cursor_.x = 1;
    const auto first_pos = CalcCursorPos();

    Rectangle<int> draw_area{first_pos, {8 * (kColumns - 1), 16}};
    FillRectangle(*window_->Writer(), draw_area.pos, draw_area.size, {0, 0, 0});

    const char *history = "";
    if (cmd_history_index_ >= 0) {
        history = &cmd_history_[cmd_history_index_][0];
    }

    strcpy(&linebuf_[0], history);
    linebuf_index_ = strlen(history);

    WriteString(*window_->Writer(), first_pos, history, {255, 255, 255});
    cursor_.x = linebuf_index_ + 1;
    return draw_area;
}

void TaskTerminal(uint64_t task_id, int64_t data) {
    __asm__("cli");
    Task &task = task_manager->CurrentTask();
    Terminal *terminal = new Terminal;
    layer_manager->Move(terminal->LayerID(), {100, 200});
    active_layer->Activate(terminal->LayerID());
    layer_task_map->insert(std::make_pair(terminal->LayerID(), task_id));
    __asm__("sti");

    for (;;) {
        __asm__("cli");
        auto msg = task.ReceiveMessage();
        if (!msg) {
            task.Sleep();
            __asm__("sti");
            continue;
        }
        __asm__("sti");

        switch (msg->type) {
            case Message::kTimerTimeout: {
                const auto area = terminal->BlinkCursor();
                Message msg = MakeLayerMessage(task_id, terminal->LayerID(),
                                               LayerOperation::DrawArea, area);
                __asm__("cli");
                task_manager->SendMessage(1, msg);
                __asm__("sti");
                break;
            }
            case Message::kKeyPush: {
                const auto area = terminal->InputKey(msg->arg.keyboard.modifier,
                                                     msg->arg.keyboard.keycode,
                                                     msg->arg.keyboard.ascii);
                Message msg = MakeLayerMessage(task_id, terminal->LayerID(),
                                               LayerOperation::DrawArea, area);
                __asm__("cli");
                task_manager->SendMessage(1, msg);
                __asm__("sti");
                break;
            }
            default:
                break;
        }
    }
}
