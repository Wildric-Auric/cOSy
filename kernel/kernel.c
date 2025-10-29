#define VIDEO_MEMORY  0xb8000

char* txt = "Hello From C!";
void main() {
    volatile char* vm = (volatile char*)VIDEO_MEMORY;
    char c;
    int  i = 0;
    while (c = txt[i++]) {
        *(vm+(i-1)*2)     = c;
        *(vm+(i-1)*2+1)   = 0xA0;
    }
}
