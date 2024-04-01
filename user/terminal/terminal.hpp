#pragma once

class Terminal {
public:
    virtual ~Terminal();

    virtual void put(char ch, char bg, char fg) = 0;

    virtual void set_cursor(int x, int y) = 0;

    virtual void scroll(char bg, char fg) = 0;

    virtual int get_width() = 0;

    virtual int get_height() = 0;
};

class VGATerminal : public Terminal {
public:
    VGATerminal();

    virtual ~VGATerminal();

    void put(char ch, char bg, char fg) override;

    void set_cursor(int x, int y) override;

    void scroll(char bg, char fg) override;

    int get_width() override;

    int get_height() override;

private:
    char map_color(char bg, char fg);

private:
    int fd;
};