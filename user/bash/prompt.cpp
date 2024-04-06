#include "prompt.hpp"
#include <stdlib.h>
#include <string.h>

#define INPUT_BSP 0x08
#define INPUT_ENT 0x0A
#define INPUT_UP  'A'
#define INPUT_DWN 'B'
#define INPUT_FWD 'C'
#define INPUT_BCK 'D'

CommandPrompt::CommandPrompt(int stdout, int stdin)
    : stdout(stdout), stdin(stdin)
{
    buffer = nullptr;
}

CommandPrompt::~CommandPrompt() {
    if (buffer)
        free(buffer);

    for (int i = 0; i < PROMPT_HISTORY_SIZE; i++) {
        if (history[i])
            free(history[i]);
    }
}

void CommandPrompt::input_write(const char* buf, size_t size) {
    while (size) {
        input_put(*buf);
        size--;
        buf++;
    }
}

void CommandPrompt::input_put(char ch) {
    if (csi_enabled) {
        special_key(ch);
        csi_enabled = false;
        return;
    }

    if (esc_enabled) {
        if (ch == '[')
            csi_enabled = true;
        esc_enabled = false;
        return;
    }

    if (ch == '\e') {
        esc_enabled = true;
        return;
    }

    if (ch == INPUT_BSP || ch == INPUT_ENT)
        special_key(ch);

    if (ch < 0x20)
        return;

    if (cursor < command_size) {
        set_fdcursor(cursor + 1);
        fdwrite(&buffer[cursor], command_size - cursor);
        set_fdcursor(cursor);
    }

    fdwrite(&ch, 1);
    
    set_command_size(command_size + 1);

    if (cursor < command_size) {
        for (int i = command_size; i > cursor; i--)
            buffer[i] = buffer[i - 1];
    }

    buffer[cursor] = ch;

    cursor++;
    set_fdcursor(cursor);
    history_selector = -1;
}

void CommandPrompt::special_key(char ch) {
    if (ch == INPUT_BCK) {
        if (cursor <= 0)
            return;
        
        cursor--;
        set_fdcursor(cursor);
    } else if (ch == INPUT_FWD) {
        if (cursor >= command_size)
            return;

        cursor++;
        set_fdcursor(cursor);
    } else if (ch == INPUT_BSP) {
        if (cursor <= 0)
            return;

        set_fdcursor(cursor - 1);
        fdwrite(&buffer[cursor], command_size - cursor);
        char zero = 0;
        fdwrite(&zero, 1);
        cursor--;
        set_fdcursor(cursor);

        for (int i = cursor; i < command_size - 1; i++)
            buffer[i] = buffer[i + 1];
        
        set_command_size(command_size - 1);
        history_selector = -1;
    } else if (ch == INPUT_UP) {
        if (history_selector == -1)
            history_selector = history_ptr;
        
        int nsel = history_selector - 1;
        if (nsel < 0)
            nsel = PROMPT_HISTORY_SIZE - 1;

        if (nsel == history_ptr)
            return;

        if (history[nsel] == nullptr)
            return;

        history_selector = nsel;
        restore_command(history[history_selector]);
    } else if (ch == INPUT_DWN) {
        if (history_selector == -1)
            return;
        
        int nsel = (history_selector + 1) % PROMPT_HISTORY_SIZE;

        if (nsel == history_ptr) {
            history_selector = -1;
            restore_command("");
            return;
        }

        if (history[nsel] == nullptr)
            return;

        history_selector = nsel;
        restore_command(history[history_selector]);
    } else if (ch == INPUT_ENT) {
        reading = false;
        save_command(buffer);
    }
}

const char* CommandPrompt::prompt() {
    cursor = 0;
    fdcursor = 0;

    esc_enabled = false;
    csi_enabled = false;

    buffer_capacity = 128;
    if (buffer)
        free(buffer);
    buffer = (char*)malloc(buffer_capacity);
    memset(buffer, 0, buffer_capacity);
    command_size = 0;
    history_selector = -1;
    
    reading = true;
    while (reading) {
        char ch;
        read(STDIN_FILENO, &ch, 1);
        input_put(ch);
    }

    return buffer;
}

void CommandPrompt::set_fdcursor(int ncursor) {
    int diff = ncursor - fdcursor;
    if (diff == 0)
        return;

    fdcursor = ncursor;

    char cmd[] = { '\e', '[', INPUT_FWD };
    if (diff < 0) {
        cmd[2] = INPUT_BCK;
        diff = -diff;
    }
    
    for (int i = 0; i < diff; i++)
        write(stdout, cmd, 3);
}

void CommandPrompt::fdwrite(const char* buf, size_t size) {
    write(stdout, buf, size);
    fdcursor += size;
}

void CommandPrompt::set_command_size(size_t size) {
    command_size = size;
    buffer[command_size] = 0;
    while (command_size + 1 >= buffer_capacity)
        expand_buffer();
}

void CommandPrompt::expand_buffer() {
    buffer_capacity *= 2;
    buffer = (char*)realloc(buffer, buffer_capacity);
}

void CommandPrompt::save_command(const char* cmd) {
    size_t len = strlen(cmd);
    char* ncmd = (char*)malloc(len + 1);
    memcpy(ncmd, cmd, len);
    ncmd[len] = 0;

    if (history[history_ptr])
        free(history[history_ptr]);

    history[history_ptr] = ncmd;
    history_ptr = (history_ptr + 1) % PROMPT_HISTORY_SIZE;
}

void CommandPrompt::restore_command(const char* cmd) {
    size_t prev_size = command_size;

    size_t len = strlen(cmd);
    set_command_size(len);
    memcpy(buffer, cmd, len);

    set_fdcursor(0);
    fdwrite(cmd, len);
    char zero = 0;
    for (size_t i = len; i < prev_size; i++)
        fdwrite(&zero, 1);
    set_fdcursor(len);
    cursor = len;
}