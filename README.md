| Nama | NRP |
| :--- | :--- |
| Muhammad Rifqi Fathurrahman | 5027241029 |

---

## Soal 1 - Penyelamatan Mas Amba (Virtual File System menggunakan FUSE)

**Dikerjakan oleh:** Muhammad Rifqi Fathurrahman (5027241029)

### Penjelasan

Program `kenz_rescue.c` dibuat menggunakan library **FUSE (Filesystem in Userspace)** di dalam bahasa C untuk mengimplementasikan virtualisasi berkas log koordinat. Program ini bertugas menyisir sekumpulan berkas log fisik (`1.txt` sampai `7.txt`) di dalam folder `amba_files`, mengekstrak string koordinat tepat setelah kata kunci `"KOORD:"`, membersihkan spasi/newline, dan menggabungkannya secara *on-the-fly* menjadi sebuah file virtual berwujud `tujuan.txt` di dalam direktori mount `mnt` tanpa memakan ruang penyimpanan fisik baru.

#### Struktur Kode Utama FUSE:

* **a) Fungsi `do_getattr` (Mengamankan Atribut Root dan Berkas Virtual)**
  Mencegat pengecekan atribut sistem operasi agar folder root FUSE (`/`) dan berkas virtual `tujuan.txt` dikenali memiliki permission dan tipe hak akses yang valid.
Output kode
File README.md berhasil dibuat.

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
b) Fungsi do_readdir (Menampilkan Daftar Isi Direktori mnt)
Membaca isi direktori asal amba_files menggunakan fungsi opendir() dan readdir(), lalu menyuntikkan entri nama file virtual "tujuan.txt" secara manual ke dalam buffer bawaan FUSE.

C
DIR *dp = opendir(source_path);
while ((de = readdir(dp)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    filler(buf, de->d_name, NULL, 0);
}
closedir(dp);
filler(buf, "tujuan.txt", NULL, 0);
c) Fungsi do_read (Ekstraksi Berkas dan Penggabungan Koordinat Otomatis)
Melakukan looping terstruktur untuk menyisir berkas 1.txt hingga 7.txt secara berurutan, mencari kecocokan token "KOORD:" dengan strstr(), memotong string, membersihkan trailing whitespace/newline, dan menggabungkannya ke buffer virtual_content.

C
for (int i = 1; i <= 7; i++) {
    sprintf(filepath, "%s/%d.txt", source_path, i);
    FILE *fp = fopen(filepath, "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            char *match = strstr(line, "KOORD:");
            if (match) {
                char *coord_val = match + 6;
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
Output Ekstraksi Terakhir (Sukses):

Plaintext
root@07acdd547e4f:/# cat /workspace/mnt/tujuan.txt
Tujuan Mas Amba: -7.279444, 112.798333 (Lokasi Rahasia)
Kendala & Penyelesaian

Error "mount_macfuse: the file system is not available (1)" di macOS
Eksekusi binary FUSE secara native di terminal MacBook Pro gagal karena sistem keamanan kernel macOS terbaru memblokir ekstensi macFUSE secara default.

Penyelesaian: Mengalihkan environment eksekusi ke dalam kontainer Docker Ubuntu berbasis Linux dengan mengaktifkan flag keamanan tinggi menggunakan perintah:

Bash
docker run --privileged -it -v "$(pwd)":/workspace ubuntu:latest /bin/bash
Compile Error FUSE & Command Not Found di Container Baru
Di dalam image ubuntu:latest dasar, utilitas penting seperti gcc, libfuse-dev, pkg-config, make, fusermount, hingga editor nano belum terpasang sehingga program gagal dicompile.

Penyelesaian: Melakukan pembaharuan package list apt dan menginstal manual seluruh dependensi pembangunan FUSE:

Bash
apt-get update && apt-get install -y gcc libfuse-dev pkg-config make tree fuse nano
Kompilasi sprintf() memicu warning "-Wformat-overflow="
Kompilasi memunculkan catatan warning karena alokasi ukuran buffer array filepath berpotensi kelebihan muatan string jika source_path bernilai terlalu panjang.

Penyelesaian: Mengabaikan warning tersebut karena alokasi lokal statis masih berada di bawah batas memori aman, atau memperbesar ukuran array tujuan menjadi char filepath[1500];.

Error "ls: cannot access 'mnt': No such file or directory" saat Container di-restart
Ketika keluar dari container dan masuk kembali lewat docker exec, status filesystem mount FUSE sering kali terputus (detached), menyebabkan kernel OS kehilangan referensi ke direktori mnt.

Penyelesaian: Membersihkan sisa zombie mount lama menggunakan lazy unmount dan mengeksekusi ulang binary dengan argumen yang benar untuk memasang ulang filesystem:

Bash
fusermount -uz mnt 2>/dev/null
./kenz_rescue amba_files mnt -f
Folder amba_files kosong melompong (total 0) saat awal dijalankan
Folder amba_files di dalam Docker ternyata belum dibuat atau file asli dari MacBook belum masuk sehingga program FUSE membaca folder kosong dan menghasilkan output string kosong.

Penyelesaian: Mengisi folder amba_files secara langsung di dalam kontainer menggunakan utilitas echo untuk menyimulasikan data log koordinat dari berkas 1.txt hingga 7.txt.

Bash Error "syntax error near unexpected token ')'" di Terminal
Terjadi karena ketidaksengajaan melakukan copy-paste baris output mentah hasil pembacaan FUSE kembali ke dalam command prompt Bash, sehingga tanda kurung pada teks (Lokasi Rahasia) dianggap sebagai token perintah oleh shell Bash.

Penyelesaian: Mengabaikan pesan error tersebut karena tidak memengaruhi program, lalu menjalankan ulang instruksi pemanggilan file virtual secara bersih melalui perintah cat /workspace/mnt/tujuan.txt.
