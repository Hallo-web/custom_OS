__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,   // magic
    0x00,         // flags
    -(0x1BADB002) // checksum = -(magic + flags)
};

void clear_screen()
{
    char *video_memory = (char *)0xb8000;
    for (int i = 0; i < 80 * 25 * 2; i += 2)
    {
        video_memory[i] = ' ';
        video_memory[i + 1] = 0x07;
    }
}

void print_centered(const char *str, int row)
{
    int len = 0;
    while (str[len])
        len++;

    int col = (80 - len) / 2;
    int offset = (row * 80 + col) * 2;
    char *video_memory = (char *)0xb8000;
}

void splashScreen()
{
    const char *message = "O.S.I.R.I.S";
    int offset = 0;

    print_centered(message, 5);
}

void kernel_main()
{
    clear_screen();
    const char *message = "Booting...";
    
    char *video_memory = (char *)0xb8000;
    splashScreen();
    int offset = 0;

    for (int i = 0; message[i] != '\0'; ++i)
    {
        video_memory[i * 2] = message[i];
        video_memory[i * 2 + 1] = 0x0F; // white on black
    }

    while (1)
    {
    } // Halt the CPU
}
