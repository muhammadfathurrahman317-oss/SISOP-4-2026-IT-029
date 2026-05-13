#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

// Fungsi untuk mendapatkan atribut file/folder (ls -l)
static int do_getattr(const char *path, struct stat *st) {
    memset(st, 0, sizeof(struct stat));

    // Mendapatkan ID user dan group yang menjalankan program
    st->st_uid = getuid();
    st->st_gid = getgid();
    st->st_atime = time(NULL);
    st->st_mtime = time(NULL);

    if (strcmp(path, "/") == 0) {
        // Atribut untuk root directory
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
    } else {
        // Atribut untuk file di dalam mount point (dummy file)
        st->st_mode = S_IFREG | 0644;
        st->st_nlink = 1;
        st->st_size = 1024;
    }
    return 0;
}

// Fungsi untuk membaca isi direktori (ls)
static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    filler(buffer, ".", NULL, 0);  // Direktori saat ini
    filler(buffer, "..", NULL, 0); // Direktori sebelumnya

    if (strcmp(path, "/") == 0) {
        // Menampilkan file dummy bernama "kenz_was_here"
        filler(buffer, "kenz_was_here", NULL, 0);
    }
    return 0;
}

// Struktur operasi FUSE
static struct fuse_operations ops = {
    .getattr = do_getattr,
    .readdir = do_readdir,
};

int main(int argc, char *argv[]) {
    // Memulai sistem FUSE
    return fuse_main(argc, argv, &ops, NULL);
}
