
void kernel_main() {
    const char *str = "VoidBlade OS: Ready for world domination";
    char *vidptr = (char*)0xb8000;  // VGA text mode memory
    unsigned int i = 0;
    while(str[i] != '\0') {
        vidptr[i*2] = str[i];
        vidptr[i*2+1] = 0x07;  // Light gray on black
        i++;
    }
    while (1);
}
