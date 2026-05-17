#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

// Kita set langsung path absolut folder amba_files di Docker
const char *source_path = "/workspace/amba_files";

// Fungsi pembantu untuk menggabungkan path
void get_full_path(char *fpath, const char *path) {
    sprintf(fpath, "%s%s", source_path, path);
}

// 1. Callback getattr (Dibuat aman untuk Root dan tujuan.txt)
static int do_getattr(const char *path, struct stat *stbuf) {
    char fpath[1000];
    get_full_path(fpath, path);

    memset(stbuf, 0, sizeof(struct stat));

    // Jika yang diakses adalah ROOT folder mnt itu sendiri
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    // Jika yang diakses adalah file virtual tujuan.txt
    if (strcmp(path, "/tujuan.txt") == 0) {
        stbuf->st_mode = S_IFREG | 0444; // Read-only
        stbuf->st_nlink = 1;
        stbuf->st_size = 200;            // Beri buffer size agar teks panjang muat
        return 0;
    }

    // Untuk file 1.txt - 7.txt, gunakan lstat asli dari amba_files
    int res = lstat(fpath, stbuf);
    if (res == -1) return -errno;

    return 0;
}

// 2. Callback readdir
static int do_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    // Kita hanya mengizinkan readdir di root "/"
    if (strcmp(path, "/") != 0) return -ENOENT;

    // Isi dasar direktori
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    // Buka folder asli amba_files
    DIR *dp = opendir(source_path);
    if (dp == NULL) return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        // Lewati "." dan ".." dari folder asli agar tidak double
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }
        filler(buf, de->d_name, NULL, 0);
    }
    closedir(dp);

    // Suntikkan file virtual tujuan.txt
    filler(buf, "tujuan.txt", NULL, 0);

    return 0;
}

// 3. Callback open
static int do_open(const char *path, struct fuse_file_info *fi) {
    char fpath[1000];
    get_full_path(fpath, path);

    if (strcmp(path, "/tujuan.txt") == 0) {
        return 0;
    }

    int res = open(fpath, fi->flags);
    if (res == -1) return -errno;

    close(res);
    return 0;
}

// 4. Callback read
static int do_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    (void) fi;

    if (strcmp(path, "/tujuan.txt") == 0) {
        char virtual_content[2000] = "Tujuan Mas Amba: ";
        char coord_buffer[1000] = "";
        
        // Loop membaca file 1.txt sampai 7.txt secara berurutan
        for (int i = 1; i <= 7; i++) {
            char filepath[1500];
            sprintf(filepath, "%s/%d.txt", source_path, i);
            
            FILE *fp = fopen(filepath, "r");
            if (fp) {
                char line[256];
                while (fgets(line, sizeof(line), fp)) {
                    char *match = strstr(line, "KOORD:");
                    if (match) {
                        char *coord_val = match + 6;
                        
                        // Bersihkan spaces/newlines
                        size_t len = strlen(coord_val);
                        while(len > 0 && (coord_val[len-1] == '\n' || coord_val[len-1] == '\r' || coord_val[len-1] == ' ')) {
                            coord_val[len-1] = '\0';
                            len--;
                        }
                        
                        strcat(coord_buffer, coord_val);
                        break; 
                    }
                }
                fclose(fp);
            }
        }
        
        strcat(virtual_content, coord_buffer);
        strcat(virtual_content, "\n");

        size_t len = strlen(virtual_content);
        if (offset < len) {
            if (offset + size > len) size = len - offset;
            memcpy(buf, virtual_content + offset, size);
        } else {
            size = 0;
        }
        return size;
    }

    // Passthrough untuk file biasa
    char fpath[1000];
    get_full_path(fpath, path);
    
    int fd = open(fpath, O_RDONLY);
    if (fd == -1) return -errno;

    int res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;

    close(fd);
    return res;
}

static struct fuse_operations kenz_ops = {
    .getattr = do_getattr,
    .readdir = do_readdir,
    .open    = do_open,
    .read    = do_read,
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <source_dir> <mount_dir> [fuse_options]\n", argv[0]);
        return 1;
    }

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    // Buang argumen ke-1 (amba_files)
    for (int i = 1; i < args.argc - 1; i++) {
        args.argv[i] = args.argv[i + 1];
    }
    args.argc--;

    return fuse_main(args.argc, args.argv, &kenz_ops, NULL);
}
