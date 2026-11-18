/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "kode.h"

KODE_APP_MANIFEST("Storage Manager", "maker");

#define PAGE_SIZE KODE_FILE_LIST_MAX
#define VIEW_ROWS 5U

static void show_storage(kode_widget_t widget, const char *volume) {
    uint64_t total = 0U, free_bytes = 0U;
    if (kode_file_space(volume, &total, &free_bytes)) {
        (void)kode_widget_set_text(widget, "%s  free %llu / %llu MB",
            volume,
            (unsigned long long)(free_bytes / (1024U * 1024U)),
            (unsigned long long)(total / (1024U * 1024U)));
    } else {
        (void)kode_widget_set_text(widget, "%s  unavailable: %.28s", volume, kode_error());
    }
}

void app_main(void) {
    const char *volumes[] = {"flash", "sd"};
    unsigned volume = 0U;
    uint32_t page = 0U;
    uint32_t selected = 0U;
    bool confirm_delete = false;
    bool dirty = true;
    char path[KODE_FILE_PATH_MAX];
    char lines[220];
    char message[100];
    kode_widget_t storage, listing, info, delete_button;

    kode_init();
    if (!kode_screen_new("Storage Manager")) {
        kode_print("screen failed: %s", kode_error());
        kode_exit();
    }

    storage = kode_screen_add_text("Checking storage...");
    listing = kode_screen_add_text("Loading files...");
    info = kode_screen_add_text("UP/DOWN select  LEFT/RIGHT page  TOP volume");
    delete_button = kode_screen_add_button("Delete selected");
    if (!storage || !listing || !info || !delete_button) {
        kode_print("widget creation failed: %s", kode_error());
        kode_exit();
    }

    for (;;) {
        const kode_file_list_t *items = NULL;

        if (kode_button_was_pressed("bottom")) kode_exit();

        if (kode_button_was_pressed("top")) {
            volume ^= 1U;
            page = 0U;
            selected = 0U;
            confirm_delete = false;
            dirty = true;
        }
        if (kode_button_was_pressed("up") && selected > 0U) {
            --selected;
            confirm_delete = false;
            dirty = true;
        }
        if (kode_button_was_pressed("down")) {
            items = kode_file_list_from(volumes[volume], page * PAGE_SIZE);
            if (items && selected + 1U < items->count) {
                ++selected;
                confirm_delete = false;
                dirty = true;
            }
        }
        if (kode_button_was_pressed("left") && page > 0U) {
            --page;
            selected = 0U;
            confirm_delete = false;
            dirty = true;
        }
        if (kode_button_was_pressed("right")) {
            items = kode_file_list_from(volumes[volume], page * PAGE_SIZE);
            if (items && items->more) {
                ++page;
                selected = 0U;
                confirm_delete = false;
                dirty = true;
            }
        }

        if (kode_widget_was_pressed(delete_button)) {
            items = kode_file_list_from(volumes[volume], page * PAGE_SIZE);
            if (!items || selected >= items->count) {
                (void)snprintf(message, sizeof(message), "Nothing selected.");
                confirm_delete = false;
            } else if (items->entries[selected].is_folder) {
                (void)snprintf(message, sizeof(message), "Folder deletion disabled.");
                confirm_delete = false;
            } else {
                int n = snprintf(path, sizeof(path), "%s/%s", volumes[volume],
                                 items->entries[selected].name);
                if (n < 0 || (size_t)n >= sizeof(path)) {
                    (void)snprintf(message, sizeof(message), "Path too long.");
                    confirm_delete = false;
                } else if (!confirm_delete) {
                    confirm_delete = true;
                    (void)snprintf(message, sizeof(message), "Tap again to delete selected file.");
                } else if (kode_file_delete(path)) {
                    confirm_delete = false;
                    (void)snprintf(message, sizeof(message), "File deleted.");
                    dirty = true;
                } else {
                    confirm_delete = false;
                    (void)snprintf(message, sizeof(message), "Delete failed: %.55s", kode_error());
                }
            }
            (void)kode_widget_set_text(info, "%s", message);
            (void)kode_widget_set_text(delete_button, "%s",
                confirm_delete ? "Confirm delete" : "Delete selected");
        }

        if (dirty) {
            size_t used = 0U;
            uint32_t first;
            uint32_t end;
            show_storage(storage, volumes[volume]);
            items = kode_file_list_from(volumes[volume], page * PAGE_SIZE);
            if (!items || items->count == 0U) {
                (void)kode_widget_set_text(listing, "%s", "(empty or unavailable)");
                (void)kode_widget_set_text(info, "No files, or storage unavailable: %.40s", kode_error());
            } else {
                if (selected >= items->count) selected = items->count - 1U;
                first = selected >= VIEW_ROWS ? selected - (VIEW_ROWS - 1U) : 0U;
                end = first + VIEW_ROWS;
                if (end > items->count) end = items->count;
                lines[0] = '\0';
                for (uint32_t i = first; i < end; ++i) {
                    int n = snprintf(lines + used, sizeof(lines) - used, "%c %s%s  %lu B\n",
                        i == selected ? '>' : ' ',
                        items->entries[i].name,
                        items->entries[i].is_folder ? "/" : "",
                        (unsigned long)items->entries[i].size);
                    if (n < 0 || (size_t)n >= sizeof(lines) - used) break;
                    used += (size_t)n;
                }
                (void)kode_widget_set_text(listing, "%s", lines);
                (void)kode_widget_set_text(info, "Page %lu  %lu item(s)%s",
                    (unsigned long)(page + 1U),
                    (unsigned long)items->count,
                    items->more ? "  RIGHT: next" : "");
            }
            (void)kode_widget_set_text(delete_button, "%s",
                confirm_delete ? "Confirm delete" : "Delete selected");
            dirty = false;
        }
        kode_delay(80U);
    }
}
