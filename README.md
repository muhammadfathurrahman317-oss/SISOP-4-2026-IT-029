# Soal 1 - Penyelamatan Mas Amba (Virtual File System menggunakan FUSE)

| Nama | NRP |
| :--- | :--- |
| Muhammad Rifqi Fathurrahman | 5027241029 |

---

# Penjelasan Program

Program `kenz_rescue.c` dibuat menggunakan library **FUSE (Filesystem in Userspace)** dalam bahasa C untuk membuat virtual file system yang dapat membaca dan menggabungkan data koordinat dari beberapa file log.

Program akan membaca file:

- `1.txt`
- `2.txt`
- `3.txt`
- `4.txt`
- `5.txt`
- `6.txt`
- `7.txt`

yang berada di dalam folder `amba_files`.

Setiap file berisi log tertentu dan program akan mencari string dengan format:

```txt
KOORD:
```

Setelah menemukan keyword tersebut, program akan mengambil isi koordinatnya lalu menggabungkan seluruh hasil menjadi file virtual:

```txt
tujuan.txt
```

yang muncul pada folder mount FUSE.

Filesystem virtual ini bekerja secara dinamis (*on-the-fly*) tanpa membuat file fisik baru di storage.

---

# Struktur Fungsi Utama

## 1. Fungsi `do_getattr`

Fungsi ini digunakan untuk memberikan atribut filesystem terhadap direktori root dan file virtual `tujuan.txt`.

```c
if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    return 0;
}

if (strcmp(path, "/tujuan.txt") == 0) {
    stbuf->st_mode = S_IFREG | 0444;
    stbuf->st_nlink = 1;
    stbuf->st_size = 200;
    return 0;
}
```

Penjelasan:
- `S_IFDIR` digunakan untuk menandai direktori.
- `0755` merupakan permission direktori.
- `S_IFREG` digunakan untuk file biasa.
- `0444` membuat file hanya dapat dibaca.

---

## 2. Fungsi `do_readdir`

Fungsi ini membaca isi folder sumber menggunakan `opendir()` dan `readdir()` lalu menambahkan file virtual `tujuan.txt`.

```c
DIR *dp = opendir(source_path);

while ((de = readdir(dp)) != NULL) {

    if (strcmp(de->d_name, ".") == 0 ||
        strcmp(de->d_name, "..") == 0)
        continue;

    filler(buf, de->d_name, NULL, 0);
}

closedir(dp);

filler(buf, "tujuan.txt", NULL, 0);
```

Penjelasan:
- `opendir()` membuka folder sumber.
- `readdir()` membaca isi folder.
- `filler()` digunakan untuk menampilkan file pada mount FUSE.

---

## 3. Fungsi `do_read`

Fungsi ini melakukan proses utama yaitu membaca file log dan mengekstrak koordinat.

```c
for (int i = 1; i <= 7; i++) {

    sprintf(filepath, "%s/%d.txt", source_path, i);

    FILE *fp = fopen(filepath, "r");

    if (fp) {

        while (fgets(line, sizeof(line), fp)) {

            char *match = strstr(line, "KOORD:");

            if (match) {

                char *coord_val = match + 6;

                size_t len = strlen(coord_val);

                while (len > 0 &&
                      (coord_val[len-1] == '\n' ||
                       coord_val[len-1] == '\r' ||
                       coord_val[len-1] == ' ')) {

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
```

Penjelasan:
- `strstr()` mencari keyword `"KOORD:"`.
- `match + 6` digunakan untuk mengambil isi setelah keyword.
- Karakter newline dan spasi dibersihkan.
- Semua koordinat digabung ke `coord_buffer`.

---

# Output Program

Contoh hasil ketika file virtual dibaca:

```txt
root@container:/workspace# cat mnt/tujuan.txt

Tujuan Mas Amba: -7.279444, 112.798333 (Lokasi Rahasia)
```

---

# Cara Compile

```bash
gcc kenz_rescue.c `pkg-config fuse --cflags --libs` -o kenz_rescue
```

---

# Cara Menjalankan

```bash
mkdir mnt

./kenz_rescue amba_files mnt -f
```

Untuk membaca file virtual:

```bash
cat mnt/tujuan.txt
```

---

# Kendala dan Penyelesaian

## 1. macFUSE Tidak Berjalan di macOS

### Kendala
Kernel macOS memblokir filesystem FUSE sehingga program gagal dijalankan secara native.

### Penyelesaian

Menggunakan Docker Ubuntu:

```bash
docker run --privileged -it -v "$(pwd)":/workspace ubuntu:latest /bin/bash
```

---

## 2. Dependency FUSE Belum Terpasang

### Kendala
Container Ubuntu belum memiliki package penting seperti:
- gcc
- fuse
- libfuse-dev
- make

### Penyelesaian

```bash
apt-get update && apt-get install -y gcc libfuse-dev pkg-config make fuse nano
```

---

## 3. Error Mount Hilang Setelah Restart

### Kendala
Filesystem FUSE detached ketika container restart.

### Penyelesaian

```bash
fusermount -uz mnt 2>/dev/null

./kenz_rescue amba_files mnt -f
```

---

## 4. Folder `amba_files` Kosong

### Kendala
File log koordinat belum tersedia.

### Penyelesaian
Membuat file simulasi menggunakan `echo`.

---

# Kesimpulan

Program `kenz_rescue.c` berhasil mengimplementasikan virtual filesystem berbasis FUSE yang mampu membaca beberapa file log, mengekstrak koordinat tertentu, dan menggabungkannya menjadi file virtual secara otomatis tanpa membuat file fisik baru.

