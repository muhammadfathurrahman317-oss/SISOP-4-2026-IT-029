# Praktikum Sistem Operasi - Modul 3

**Dikerjakan oleh:** Muhammad Rifqi Fathurrahman (5027251029)

---

# Soal 1 - Present Day, Present Time

## Penjelasan

Program `wired.c` (server) dan `navi.c` (client) merupakan implementasi sistem komunikasi berbasis **socket programming** menggunakan bahasa C. Program ini memungkinkan beberapa client terhubung ke server dan saling berkomunikasi secara real-time menggunakan mekanisme **broadcast**.

Sistem ini menggunakan **multi-threading (`pthread`)** untuk menangani banyak client secara bersamaan serta memastikan komunikasi berjalan asynchronous.

---

## Cara Kompilasi & Menjalankan

```bash
# Kompilasi
gcc wired.c -o wired -lpthread
gcc navi.c -o navi -lpthread

# Jalankan server (terminal 1)
./wired

# Jalankan client (terminal 2 & 3)
./navi
```

---

## Penjelasan Per Langkah

### a) Inisialisasi Server

Server dibuat menggunakan socket TCP:

```c
server_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(server_fd, (struct sockaddr *)&address, sizeof(address));
listen(server_fd, 10);
```

Fungsi:
- `socket()` → membuat socket
- `bind()` → menghubungkan ke port 8080
- `listen()` → menunggu koneksi client

Output:
```
Server running on port 8080...
```

---

### b) Koneksi Client ke Server

Client melakukan koneksi ke server:

```c
connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
```

Output:
```
Connected to server
```

---

### c) Multi Client dengan Thread

Server menggunakan `pthread` untuk menangani banyak client:

```c
pthread_create(&tid, NULL, handle_client, (void *)new_sock);
```

Fungsi:
- Setiap client ditangani oleh thread terpisah
- Server tidak blocking

---

### d) Pengiriman & Penerimaan Pesan

Client mengirim pesan:
```c
send(sock, message, strlen(message), 0);
```

Server menerima:
```c
read(sock, buffer, 1024);
```

---

### e) Broadcast ke Semua Client

Server mengirim pesan ke semua client lain:

```c
void broadcast(char *message, int sender) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender) {
            send(clients[i], message, strlen(message), 0);
        }
    }
}
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
