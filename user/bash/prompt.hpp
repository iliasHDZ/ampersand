#pragma once

#include <unistd.h>

#define PROMPT_HISTORY_SIZE 16

class CommandPrompt {
public:
    CommandPrompt(int stdout, int stdin);

    ~CommandPrompt();

    void input_write(const char* buf, size_t size);

    void input_put(char ch);

    void special_key(char ch);

    const char* prompt();

private:
    void set_fdcursor(int ncursor);

    void fdwrite(const char* buf, size_t size);

    void set_command_size(size_t size);

    void expand_buffer();

    void save_command(const char* cmd);

    void restore_command(const char* cmd);

private:
    int stdout;
    int stdin;

    bool reading = false;

    int cursor = 0;
    int fdcursor = 0;

    bool esc_enabled = false;
    bool csi_enabled = false;

    char* buffer;
    size_t buffer_capacity = 128;
    size_t command_size = 0;

    char* history[PROMPT_HISTORY_SIZE] = { 0 };
    int history_ptr = 0;
    int history_selector = -1;
};