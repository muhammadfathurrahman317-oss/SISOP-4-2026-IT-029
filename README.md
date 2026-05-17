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
>>>>>>> db68bb61bc47afbf6093ec27bd50b07d58028318
        }
    }
}
<<<<<<< HEAD
```

Semua client menerima pesan kecuali pengirim.

---

### f) Client Asynchronous (Thread)

Client menggunakan thread untuk menerima pesan:

```c
pthread_create(&tid, NULL, receive_msg, &sock);
```

Client bisa kirim dan terima pesan bersamaan.

---

## Cara Testing

### 1. Jalankan Server
```bash
./wired
```

### 2. Jalankan Minimal 2 Client
```bash
./navi
```

### 3. Kirim Pesan
```
halo
```

### 4. Output yang Diharapkan

Server:
```
Client connected!
Client connected!
Message: halo
```

Client lain:
```
halo
```

---

## Kendala & Penyelesaian

### 1. Error Compile (`expected '}'`)
**Masalah:** Kurung kurawal tidak lengkap

**Penyelesaian:** Memastikan setiap `{` memiliki pasangan `}`

---

### 2. `strlen undeclared`
**Masalah:** Tidak menyertakan library string

**Penyelesaian:**
```c
#include <string.h>
```

---

### 3. Client Tidak Menampilkan Pesan
**Masalah:** Client hanya mengirim tanpa menerima

**Penyelesaian:** Menambahkan thread untuk `read()` dari server

---

### 4. Tidak Ada Output di Client
**Masalah:** Hanya menjalankan 1 client

**Penyelesaian:** Menjalankan minimal 2 client untuk melihat broadcast

---

### 5. Server Tidak Menerima Pesan
**Masalah:** Server dihentikan (`Ctrl + C`) sebelum client mengirim

**Penyelesaian:** Menjalankan server terlebih dahulu dan membiarkannya aktif

---

## Hasil Akhir

Program berhasil:
- Menangani banyak client secara bersamaan
- Mengirim dan menerima pesan secara real-time
- Melakukan broadcast antar client
- Menggunakan multi-threading untuk efisiensi

---

## Kesimpulan

Program ini berhasil mengimplementasikan sistem komunikasi client-server berbasis socket dengan fitur multi-client dan broadcast. Penggunaan thread memungkinkan sistem berjalan secara asynchronous dan efisien dalam menangani banyak koneksi sekaligus.

---

---

# Soal 2 - The Battle of Eterion

## Penjelasan

Program `orion.c` (server) dan `eternal.c` (client) merupakan implementasi sistem battle arena berbasis **IPC (Inter-Process Communication)** menggunakan bahasa C. Program ini memungkinkan beberapa pemain terhubung ke server, melakukan register/login, matchmaking, dan bertarung secara real-time.

Sistem ini menggunakan **Shared Memory** untuk menyimpan data user dan battle room, serta **Message Queue** untuk komunikasi antara client dan server. **Pthread Mutex** digunakan untuk mencegah race condition.

---

## Struktur File

```
soal_2/
├── arena.h      # Definisi struct, config, IPC keys
├── orion.c      # Server
├── eternal.c    # Client
└── Makefile
```

---

## Cara Kompilasi & Menjalankan

```bash
# Kompilasi
gcc -Wall -pthread orion.c -o orion
gcc -Wall -pthread eternal.c -o eternal

# Jalankan server (terminal 1)
./orion

# Jalankan client (terminal 2 & 3)
./eternal
```

---

## Penjelasan Per Langkah

### a) Inisialisasi Server

Server membuat Shared Memory dan Message Queue:

```c
int shm_users = shmget(SHM_KEY_USERS, sizeof(UserDB), IPC_CREAT | 0666);
db = (UserDB*)shmat(shm_users, NULL, 0);

int shm_rooms = shmget(SHM_KEY_ROOMS, sizeof(RoomDB), IPC_CREAT | 0666);
rooms = (RoomDB*)shmat(shm_rooms, NULL, 0);

mqid = msgget(MQ_KEY, IPC_CREAT | 0666);
```

Output:
```
Orion is ready (PID: xxxxx)
```

---

### b) Koneksi Client ke Server

Client mencoba terhubung ke Message Queue yang sudah dibuat server:

```c
mqid = msgget(MQ_KEY, 0666);
if (mqid == -1) {
    printf("Orion are you there?\n");
    return 1;
}
```

Jika server belum jalan, client akan menampilkan pesan error.

---

### c) Register & Login

Client mengirim request register/login via Message Queue:

```c
req.cmd = CMD_REGISTER;
strcpy(req.username, uname);
strcpy(req.password, pass);
send_msg(&req);
```

Ketentuan:
- Username harus unik
- Data tersimpan di Shared Memory (persistent selama server hidup)
- Tidak bisa login di 2 sesi bersamaan

---

### d) Matchmaking

Saat memilih Battle, client mengirim request matchmaking:

```c
req.cmd = CMD_MATCHMAKE;
send_msg(&req);
```

Alur matchmaking:
- Jika ada pemain lain yang menunggu → langsung battle
- Jika tidak ada dalam **35 detik** → otomatis lawan **WildBeast (bot)**

---

### e) Battle Realtime

Battle berjalan secara **asynchronous** (tidak turn-based). Setiap pemain bisa menyerang kapan saja dengan cooldown 1 detik:

```c
if (ch == 'a') atk.cmd = CMD_ATTACK;
if (ch == 'u') atk.cmd = CMD_ULTIMATE;
send_msg(&atk);
```

Kontrol:
- Tekan `a` → Attack (damage normal)
- Tekan `u` → Ultimate (damage x3, hanya jika punya senjata)

---

### f) Armory

Pemain bisa membeli senjata menggunakan gold:

| No | Senjata     | Harga  | Bonus DMG |
|----|-------------|--------|-----------|
| 1  | Wood Sword  | 100 G  | +5        |
| 2  | Iron Sword  | 300 G  | +15       |
| 3  | Steel Axe   | 600 G  | +30       |
| 4  | Demon Blade | 1500 G | +60       |
| 5  | God Slayer  | 5000 G | +150      |

Sistem otomatis menggunakan senjata dengan damage terbesar.

---

### g) Match History

Server menyimpan riwayat pertandingan setiap pemain di Shared Memory:

```c
void add_history(int idx, const char* opp, int result, int xp) {
    User* u = &db->users[idx];
    if (u->history_count < MAX_HISTORY) {
        MatchRecord* r = &u->history[u->history_count++];
        strcpy(r->opponent, opp);
        r->result    = result;
        r->xp_gained = xp;
        r->timestamp = time(NULL);
    }
}
```

Output:
```
MATCH HISTORY
Time   Opponent     Res    XP
--------------------------------
17:30  bob          WIN    +50 XP
17:29  WildBeast    LOSS   +15 XP
```

---

## Cara Testing

### 1. Jalankan Server
```bash
./orion
```

### 2. Jalankan 2 Client
```bash
./eternal
```

### 3. Register & Login di Kedua Client
```
Choice: 1
Username: alice
Password: 123
Account created!

Choice: 2
Username: alice
Password: 123
Welcome!
```

### 4. Kedua Client Pilih Battle
```
> Choice: 1
Searching for an opponent... (35s)
```

### 5. Saat Battle Berlangsung
- Tekan `a` untuk menyerang
- Tekan `u` untuk ultimate

### 6. Output yang Diharapkan
```
ARENA

alice           HP: 100/100
bob             HP: 70/100

Combat Log:
> alice hit for 10 damage!

[a] Attack  [u] Ultimate
```

---

## Konfigurasi Default Akun Baru

| Atribut | Nilai |
|---------|-------|
| Gold    | 150   |
| Level   | 1     |
| XP      | 0     |
| Senjata | -     |

---

## Formula Stats

| Stat   | Formula |
|--------|---------|
| Damage | BASE_DAMAGE (10) + (total XP / 50) + bonus senjata |
| Health | BASE_HEALTH (100) + (total XP / 10) |
| Level  | 1 + (total XP / 100) |

---

## Reward Per Pertandingan

| Kondisi | XP  | Gold |
|---------|-----|------|
| Menang  | +50 | +120 |
| Kalah   | +15 | +30  |

---

## Kendala & Penyelesaian

### 1. Error Compile (`iostream not found`)
**Masalah:** Awalnya file menggunakan sintaks C++ namun dikompile dengan `gcc`

**Penyelesaian:** Semua file dikonversi ke C murni dan dikompile dengan `gcc`
```bash
gcc -Wall -pthread orion.c -o orion
gcc -Wall -pthread eternal.c -o eternal
```

---

### 2. Segmentation Fault saat jalankan server
**Masalah:** Shared Memory dari session sebelumnya masih tersisa di sistem

**Penyelesaian:** Bersihkan IPC sebelum menjalankan server
```bash
ipcs -m | awk '{print $2}' | xargs -I{} ipcrm -m {} 2>/dev/null
ipcs -q | awk '{print $2}' | xargs -I{} ipcrm -q {} 2>/dev/null
```

---

### 3. Client menampilkan "Orion are you there?"
**Masalah:** Server belum dijalankan sebelum client

**Penyelesaian:** Jalankan `./orion` terlebih dahulu sebelum `./eternal`

---

### 4. Username already taken saat register
**Masalah:** Username yang sama sudah terdaftar di Shared Memory

**Penyelesaian:** Gunakan username yang berbeda, atau restart server dan bersihkan IPC

---

### 5. Race condition saat battle
**Masalah:** Dua pemain menyerang bersamaan bisa menyebabkan data corrupt

**Penyelesaian:** Menggunakan `pthread_mutex_lock` dan `pthread_mutex_unlock` pada setiap akses ke battle room

---

## Hasil Akhir

Program berhasil:
- Menangani register dan login dengan validasi username unik
- Mencegah multi-login pada akun yang sama
- Melakukan matchmaking antar pemain secara realtime
- Menjalankan battle asynchronous dengan cooldown sistem
- Menyimpan dan menampilkan match history
- Menggunakan mutex untuk mencegah race condition

---

## Kesimpulan

Program ini berhasil mengimplementasikan sistem battle arena berbasis IPC dengan fitur lengkap mulai dari autentikasi, matchmaking, battle realtime, armory, hingga match history. Penggunaan Shared Memory dan Message Queue memungkinkan komunikasi antar proses yang efisien tanpa memerlukan koneksi jaringan.
=======
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
>>>>>>> db68bb61bc47afbf6093ec27bd50b07d58028318
