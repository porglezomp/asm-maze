#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/ioctl.h>

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo orig_vinfo;
int kbfd;

int global_fbfd;
void *global_fbp;
int global_screensize;

struct easy_access {
    int xres;
    int yres;
    int bits_per_pixel;
    int line_length;
    int screensize;
};



void close_fb(int fbfd, void *fbp, int screensize);

void handle_interrupt() {
    close_fb(global_fbfd, global_fbp, global_screensize);
}

int setup_fb(char **fbp, struct easy_access *easy) {
    int fbfd = 0;

    fbfd = open("/dev/fb0", O_RDWR);
    if (!fbfd) {
        fprintf(stderr, "Error: cannot open framebuffer device.\n");
        close_fb(fbfd, NULL, 0);
        return 0;
    }
    global_fbfd = fbfd;

    signal(SIGTERM, handle_interrupt);
    signal(SIGINT, handle_interrupt);

    // Hide the cursor
    kbfd = open("/dev/tty", O_WRONLY);
    if (kbfd < 0) {
        fprintf(stderr, "Could not open /dev/tty.\n");
        close_fb(fbfd, NULL, 0);
        return 0;
    }
    ioctl(kbfd, KDSETMODE, KD_GRAPHICS);

    printf("The framebuffer device opened.\n");

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        fprintf(stderr, "Error reading variable screen info.\n");
        close_fb(fbfd, NULL, 0);
        return 0;
    }
    // Make a copy of the vinfo to restore later
    memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));

    // Update the framebuffer mode
    vinfo.bits_per_pixel = easy->bits_per_pixel;
    vinfo.xres = easy->xres;
    vinfo.yres = easy->yres;
    vinfo.xres_virtual = vinfo.xres;
    vinfo.yres_virtual = vinfo.yres;

    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo)) {
        fprintf(stderr, "Error setting variable information.\n");
        close_fb(fbfd, NULL, 0);
        return 0;
    }

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        fprintf(stderr, "Error reading fixed screen info.\n");
        close_fb(fbfd, NULL, 0);
        return 0;
    }

    easy->line_length = finfo.line_length;

    printf("Display info %dx%d, %d bpp\n",
           vinfo.xres, vinfo.yres,
           vinfo.bits_per_pixel);

    // Map the framebuffer into memory
    easy->screensize = finfo.smem_len;
    *fbp = (char*) mmap(0, easy->screensize,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fbfd, 0);
    global_fbp = *fbp;
    global_screensize = easy->screensize;

    if ((int)fbp == -1) {
        printf("Failed to mmap.\n");
        close_fb(fbfd, fbp, easy->screensize);
        return 0;
    }

    return fbfd;
}

void close_fb(int fbfd, void *fbp, int screensize) {
    if (fbp != NULL) {
        if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
            fprintf(stderr, "Error resetting variable information.\n");
        }
        munmap(fbp, screensize);
    }
    // Reset cursor
    if (kbfd >= 0) {
        ioctl(kbfd, KDSETMODE, KD_TEXT);
    }
    close(fbfd);
}
