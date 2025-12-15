// server.c  -- Tetris online backend (khong do hoa)
// Bien dich (Linux / WSL):  gcc server.c -o server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>

#define MAX_CLIENTS   64
#define MAX_ROOMS     32
#define BUF_SIZE      4096
#define USERNAME_LEN  32
#define PASSWORD_LEN  32
#define MAX_ACCOUNTS  128

typedef struct {
    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
} Account;

static Account accounts[MAX_ACCOUNTS];
static int account_count = 0;
static const char *ACCOUNTS_FILE = "accounts.txt";
static const char *MATCH_HISTORY_FILE = "match_history.txt";
static const char *PLAYER_RECORDS_FILE = "player_records.txt";

typedef struct {
    int  used;
    int  fd;
    char recv_buf[BUF_SIZE];
    int  recv_len;
    char username[USERNAME_LEN];
    int  logged_in;
    int  room_id; // -1 neu chua o phong nao
} Client;

typedef struct {
    int used;
    int id;               // roomId
    int clients[4];       // index trong mang clients[]
    int num_players;
    int playing;          // 0: cho, 1: dang choi
    int ready[4];         // trang thai ready cua moi nguoi choi
    int scores[4];        // diem so cua moi nguoi choi
    int game_started;     // 1 neu game da bat dau (tranh reset ready sai thoi diem)
    int game_mode;        // 0: survival, 1: 60s, 2: 180s, 3: 300s
    int time_limit;       // thoi gian gioi han (giay), 0 = khong gioi han
    int mode_selected;    // 1 neu mode da duoc chon, 0 neu chua
    int mode_chooser;     // client index cua nguoi duoc phep chon mode, -1 neu chua co
} Room;

static Client clients[MAX_CLIENTS];
static Room   rooms[MAX_ROOMS];
static int    next_room_id = 1;

/* ===================== helpers chung ===================== */

static void trim_newline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        len--;
    }
}

/* ---------- Tai khoan ---------- */

static void load_accounts() {
    FILE *f = fopen(ACCOUNTS_FILE, "r");
    if (!f) return; // chua co file

    char u[USERNAME_LEN], p[PASSWORD_LEN];
    while (fscanf(f, "%31s %31s", u, p) == 2) {
        if (account_count >= MAX_ACCOUNTS) break;
        strncpy(accounts[account_count].username, u, USERNAME_LEN);
        accounts[account_count].username[USERNAME_LEN-1] = '\0';
        strncpy(accounts[account_count].password, p, PASSWORD_LEN);
        accounts[account_count].password[PASSWORD_LEN-1] = '\0';
        account_count++;
    }
    fclose(f);
}

static int find_account(const char *username) {
    for (int i = 0; i < account_count; ++i) {
        if (strcmp(accounts[i].username, username) == 0) return i;
    }
    return -1;
}

// tra 0 = ok, -2 = da ton tai, -1 = het cho
static int add_account(const char *username, const char *password) {
    if (account_count >= MAX_ACCOUNTS) return -1;
    if (find_account(username) != -1)  return -2;

    strncpy(accounts[account_count].username, username, USERNAME_LEN);
    accounts[account_count].username[USERNAME_LEN-1] = '\0';
    strncpy(accounts[account_count].password, password, PASSWORD_LEN);
    accounts[account_count].password[PASSWORD_LEN-1] = '\0';
    account_count++;

    FILE *f = fopen(ACCOUNTS_FILE, "a");
    if (f) {
        fprintf(f, "%s %s\n", username, password);
        fclose(f);
    }
    return 0;
}

/* ---------- Ki luc nguoi choi ---------- */

// Lay diem cao nhat cua nguoi choi tu file
static int get_player_record(const char *username) {
    FILE *f = fopen(PLAYER_RECORDS_FILE, "r");
    if (!f) return 0;
    
    char u[USERNAME_LEN];
    int score;
    while (fscanf(f, "%31s %d", u, &score) == 2) {
        if (strcmp(u, username) == 0) {
            fclose(f);
            return score;
        }
    }
    fclose(f);
    return 0;
}

// Cap nhat ki luc nguoi choi (neu score moi cao hon)
static void update_player_record(const char *username, int new_score) {
    int old_record = get_player_record(username);
    if (new_score <= old_record && old_record > 0) return; // khong can cap nhat
    
    // Doc tat ca records
    typedef struct {
        char username[USERNAME_LEN];
        int score;
    } Record;
    Record records[MAX_ACCOUNTS];
    int count = 0;
    
    FILE *f = fopen(PLAYER_RECORDS_FILE, "r");
    if (f) {
        while (count < MAX_ACCOUNTS && fscanf(f, "%31s %d", records[count].username, &records[count].score) == 2) {
            count++;
        }
        fclose(f);
    }
    
    // Cap nhat hoac them moi
    int found = 0;
    for (int i = 0; i < count; ++i) {
        if (strcmp(records[i].username, username) == 0) {
            records[i].score = new_score;
            found = 1;
            break;
        }
    }
    if (!found && count < MAX_ACCOUNTS) {
        strncpy(records[count].username, username, USERNAME_LEN);
        records[count].username[USERNAME_LEN-1] = '\0';
        records[count].score = new_score;
        count++;
    }
    
    // Ghi lai file
    f = fopen(PLAYER_RECORDS_FILE, "w");
    if (f) {
        for (int i = 0; i < count; ++i) {
            fprintf(f, "%s %d\n", records[i].username, records[i].score);
        }
        fclose(f);
    }
}

/* ---------- Lich su dau ---------- */

static void save_match_history(int room_id, const char *player_names[], int player_scores[], int num_players) {
    FILE *f = fopen(MATCH_HISTORY_FILE, "a");
    if (!f) return;
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    
    fprintf(f, "=== Match Room %d - %s ===\n", room_id, timestamp);
    
    // Sap xep theo diem giam dan
    typedef struct {
        char name[USERNAME_LEN];
        int score;
    } PlayerData;
    PlayerData sorted[4];
    for (int i = 0; i < num_players; ++i) {
        strncpy(sorted[i].name, player_names[i], USERNAME_LEN);
        sorted[i].score = player_scores[i];
    }
    
    // Bubble sort
    for (int i = 0; i < num_players - 1; ++i) {
        for (int j = 0; j < num_players - i - 1; ++j) {
            if (sorted[j].score < sorted[j+1].score) {
                PlayerData tmp = sorted[j];
                sorted[j] = sorted[j+1];
                sorted[j+1] = tmp;
            }
        }
    }
    
    for (int i = 0; i < num_players; ++i) {
        fprintf(f, "#%d: %s - %d points", i+1, sorted[i].name, sorted[i].score);
        if (i == 0) fprintf(f, " [WINNER]");
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
    fclose(f);
}

/* ---------- Khoi tao client / room ---------- */

static void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].used       = 0;
        clients[i].fd         = -1;
        clients[i].recv_len   = 0;
        clients[i].logged_in  = 0;
        clients[i].room_id    = -1;
        clients[i].username[0]= '\0';
    }
}

static void init_rooms() {
    for (int i = 0; i < MAX_ROOMS; ++i) {
        rooms[i].used        = 0;
        rooms[i].id          = 0;
        rooms[i].num_players = 0;
        rooms[i].playing     = 0;
        rooms[i].game_started = 0;
        rooms[i].game_mode   = 0;
        rooms[i].time_limit  = 0;
        rooms[i].mode_selected = 0;
        rooms[i].mode_chooser = -1;
        for (int j = 0; j < 4; ++j) {
            rooms[i].clients[j] = -1;
            rooms[i].ready[j] = 0;
            rooms[i].scores[j] = 0;
        }
    }
}

/* ---------- Gui / broadcast ---------- */

static void send_to_client(int idx, const char *msg) {
    if (!clients[idx].used) return;
    size_t len = strlen(msg);
    send(clients[idx].fd, msg, len, 0);   // don gian, bo qua loi
}

static void broadcast_room(int room_index, const char *msg) {
    if (!rooms[room_index].used) return;
    for (int i = 0; i < rooms[room_index].num_players; ++i) {
        int ci = rooms[room_index].clients[i];
        if (ci >= 0 && clients[ci].used) send_to_client(ci, msg);
    }
}

static int find_empty_client_slot() {
    for (int i = 0; i < MAX_CLIENTS; ++i)
        if (!clients[i].used) return i;
    return -1;
}

/* ---------- Roi phong / disconnect ---------- */

static void remove_client_from_room(int client_index) {
    int rid = clients[client_index].room_id;
    if (rid < 0) return;

    int rindex = -1;
    for (int i = 0; i < MAX_ROOMS; ++i)
        if (rooms[i].used && rooms[i].id == rid) { rindex = i; break; }

    if (rindex < 0) {
        clients[client_index].room_id = -1;
        return;
    }

    Room *r = &rooms[rindex];
    const char *dc_username = clients[client_index].username[0] ? clients[client_index].username : "guest";
    int dc_score = 0;
    
    for (int i = 0; i < r->num_players; ++i) {
        if (r->clients[i] == client_index) {
            dc_score = r->scores[i];
            
            // Neu dang choi game, broadcast thong bao disconnect
            if (r->game_started) {
                char notify_msg[256];
                snprintf(notify_msg, sizeof(notify_msg), "PLAYER_DISCONNECTED %s %d\n", 
                         dc_username, dc_score);
                broadcast_room(rindex, notify_msg);
                
                // Cap nhat ki luc neu da dang nhap
                if (clients[client_index].logged_in) {
                    update_player_record(dc_username, dc_score);
                }
            }
            
            // Xoa nguoi choi nay
            for (int j = i; j < r->num_players - 1; ++j) {
                r->clients[j] = r->clients[j+1];
                r->ready[j] = r->ready[j+1];
                r->scores[j] = r->scores[j+1];
            }
            r->num_players--;
            r->clients[r->num_players] = -1;
            r->ready[r->num_players] = 0;
            r->scores[r->num_players] = 0;
            break;
        }
    }
    clients[client_index].room_id = -1;

    if (r->num_players == 0) {
        r->used = 0;
        r->id   = 0;
        r->playing = 0;
        r->game_started = 0;
        r->game_mode = 0;
        r->time_limit = 0;
        r->mode_selected = 0;
        r->mode_chooser = -1;
    }
}

static void disconnect_client(int idx) {
    if (!clients[idx].used) return;
    printf("Client %d (%s) disconnected\n", idx,
           clients[idx].logged_in ? clients[idx].username : "guest");

    remove_client_from_room(idx);
    close(clients[idx].fd);
    clients[idx].used      = 0;
    clients[idx].fd        = -1;
    clients[idx].recv_len  = 0;
    clients[idx].logged_in = 0;
    clients[idx].username[0]= '\0';
}

/* ---------- Quan ly phong ---------- */

static int create_room_for_client(int client_idx) {
    int slot = -1;
    for (int i = 0; i < MAX_ROOMS; ++i)
        if (!rooms[i].used) { slot = i; break; }

    if (slot < 0) return -1;

    Room *r = &rooms[slot];
    r->used        = 1;
    r->id          = next_room_id++;
    r->num_players = 1;
    r->playing     = 0;
    r->game_started = 0;
    for (int i = 0; i < 4; ++i) {
        r->clients[i] = -1;
        r->ready[i] = 0;
        r->scores[i] = 0;
    }
    r->clients[0]  = client_idx;

    clients[client_idx].room_id = r->id;
    return r->id;
}

static int find_room_by_id(int room_id) {
    for (int i = 0; i < MAX_ROOMS; ++i)
        if (rooms[i].used && rooms[i].id == room_id) return i;
    return -1;
}

static int join_room(int client_idx, int room_id) {
    int rindex = find_room_by_id(room_id);
    if (rindex < 0) return -1;

    Room *r = &rooms[rindex];
    if (r->num_players >= 4) return -2;

    // tranh them trung
    for (int i = 0; i < r->num_players; ++i)
        if (r->clients[i] == client_idx) return 0;

    int idx = r->num_players;
    r->clients[idx] = client_idx;
    r->ready[idx] = 0;
    r->scores[idx] = 0;
    r->num_players++;
    clients[client_idx].room_id = room_id;
    return 0;
}

static void send_room_list(int client_idx) {
    send_to_client(client_idx, "ROOM_LIST_BEGIN\n");
    for (int i = 0; i < MAX_ROOMS; ++i) {
        if (!rooms[i].used) continue;
        char line[128];
        snprintf(line, sizeof(line), "ROOM %d %d\n",
                 rooms[i].id, rooms[i].num_players);
        send_to_client(client_idx, line);
    }
    send_to_client(client_idx, "ROOM_LIST_END\n");
}

/* ===================== Xu ly lenh tu client ===================== */

static void handle_command(int cindex, char *line) {
    trim_newline(line);
    printf("From client %d: %s\n", cindex, line);

    char cmd[32];
    if (sscanf(line, "%31s", cmd) != 1) return;

    if (strcmp(cmd, "REGISTER") == 0) {
        char user[USERNAME_LEN], pass[PASSWORD_LEN];
        if (sscanf(line, "REGISTER %31s %31s", user, pass) != 2) {
            send_to_client(cindex, "REGISTER_FAIL Invalid_format\n");
            return;
        }
        int r = add_account(user, pass);
        if (r == -2)      send_to_client(cindex, "REGISTER_FAIL Username_exists\n");
        else if (r != 0)  send_to_client(cindex, "REGISTER_FAIL Internal_error\n");
        else              send_to_client(cindex, "REGISTER_OK\n");

    } else if (strcmp(cmd, "LOGIN") == 0) {
        char user[USERNAME_LEN], pass[PASSWORD_LEN];
        if (sscanf(line, "LOGIN %31s %31s", user, pass) != 2) {
            send_to_client(cindex, "LOGIN_FAIL Invalid_format\n");
            return;
        }
        int idx = find_account(user);
        if (idx < 0 || strcmp(accounts[idx].password, pass) != 0) {
            send_to_client(cindex, "LOGIN_FAIL Wrong_credentials\n");
            return;
        }
        clients[cindex].logged_in = 1;
        strncpy(clients[cindex].username, user, USERNAME_LEN);
        clients[cindex].username[USERNAME_LEN-1] = '\0';
        send_to_client(cindex, "LOGIN_OK\n");

    } else if (strcmp(cmd, "CREATE_ROOM") == 0) {
        if (!clients[cindex].logged_in) {
            send_to_client(cindex, "ERROR Not_logged_in\n");
            return;
        }
        int rid = create_room_for_client(cindex);
        if (rid < 0)      send_to_client(cindex, "ERROR Cannot_create_room\n");
        else {
            char buf[64];
            snprintf(buf, sizeof(buf), "ROOM_CREATED %d\n", rid);
            send_to_client(cindex, buf);
        }

    } else if (strcmp(cmd, "LIST_ROOMS") == 0) {
        send_room_list(cindex);

    } else if (strcmp(cmd, "JOIN_ROOM") == 0) {
        int rid;
        if (sscanf(line, "JOIN_ROOM %d", &rid) != 1) {
            send_to_client(cindex, "JOIN_FAIL Invalid_format\n");
            return;
        }
        int r = join_room(cindex, rid);
        if (r == -1)      send_to_client(cindex, "JOIN_FAIL No_such_room\n");
        else if (r == -2) send_to_client(cindex, "JOIN_FAIL Room_full\n");
        else {
            char buf[64];
            snprintf(buf, sizeof(buf), "JOIN_OK %d\n", rid);
            send_to_client(cindex, buf);
        }

    } else if (strcmp(cmd, "LEAVE_ROOM") == 0) {
        remove_client_from_room(cindex);
        send_to_client(cindex, "LEFT_ROOM\n");

    } else if (strcmp(cmd, "READY") == 0) {
        int rid = clients[cindex].room_id;
        if (rid < 0) {
            send_to_client(cindex, "ERROR Not_in_room\n");
            return;
        }
        int rindex = find_room_by_id(rid);
        if (rindex < 0) return;
        
        Room *r = &rooms[rindex];
        
        // Khong cho ready neu game da bat dau
        if (r->game_started) {
            send_to_client(cindex, "ERROR Game_already_started\n");
            return;
        }
        
        // tim vi tri cua client trong phong
        int pos = -1;
        for (int i = 0; i < r->num_players; ++i) {
            if (r->clients[i] == cindex) {
                pos = i;
                break;
            }
        }
        if (pos < 0) return;
        
        // Kiem tra xem da co nguoi ready chua (truoc khi set ready)
        int ready_count_before = 0;
        for (int i = 0; i < r->num_players; ++i) {
            if (r->ready[i]) ready_count_before++;
        }
        
        // Set ready
        r->ready[pos] = 1;
        
        // Kiem tra xem day co phai nguoi ready dau tien khong
        if (ready_count_before == 0) {
            // Nguoi ready dau tien (du la solo hay multiplayer) chon game mode
            r->mode_chooser = cindex;  // Luu nguoi duoc phep chon mode
            send_to_client(cindex, "CHOOSE_MODE\n");
        } else {
            send_to_client(cindex, "READY_OK\n");
        }
        
        // Broadcast trang thai ready den tat ca
        char ready_status[BUF_SIZE];
        int offset = 0;
        offset += snprintf(ready_status + offset, BUF_SIZE - offset, "READY_STATUS");
        for (int i = 0; i < r->num_players; ++i) {
            int ci = r->clients[i];
            const char *name = clients[ci].username[0] ? clients[ci].username : "guest";
            offset += snprintf(ready_status + offset, BUF_SIZE - offset, " %s:%d",
                             name, r->ready[i]);
        }
        offset += snprintf(ready_status + offset, BUF_SIZE - offset, "\n");
        broadcast_room(rindex, ready_status);
        
        // kiem tra neu tat ca san sang
        int all_ready = 1;
        for (int i = 0; i < r->num_players; ++i) {
            if (!r->ready[i]) {
                all_ready = 0;
                break;
            }
        }
        
        // Dieu kien bat dau game:
        // - Tat ca ready
        // - VA mode da duoc chon (bat buoc cho ca solo va multiplayer)
        int can_start = all_ready && r->num_players > 0 && !r->game_started && r->mode_selected;
        
        if (can_start) {
            r->playing = 1;
            r->game_started = 1;
            
            // Gui countdown de dong bo
            broadcast_room(rindex, "COUNTDOWN 3\n");
            sleep(1);
            broadcast_room(rindex, "COUNTDOWN 2\n");
            sleep(1);
            broadcast_room(rindex, "COUNTDOWN 1\n");
            sleep(1);
            // Gui START_GAME voi mode va time limit
            char start_msg[256];
            snprintf(start_msg, sizeof(start_msg), "START_GAME %d %d\n", r->game_mode, r->time_limit);
            broadcast_room(rindex, start_msg);
            
            // Reset diem (nhung GIU ready = 1 de tranh race condition)
            for (int i = 0; i < r->num_players; ++i) {
                r->scores[i] = 0;
            }
            
            // Gui SCORE_UPDATE ban dau voi tat ca nguoi choi diem 0
            char initial_score_msg[BUF_SIZE];
            int offset = 0;
            offset += snprintf(initial_score_msg + offset, BUF_SIZE - offset, "SCORE_UPDATE");
            for (int i = 0; i < r->num_players; ++i) {
                int ci = r->clients[i];
                const char *username = clients[ci].username[0] ? clients[ci].username : "guest";
                offset += snprintf(initial_score_msg + offset, BUF_SIZE - offset, " %s:0", username);
            }
            offset += snprintf(initial_score_msg + offset, BUF_SIZE - offset, "\n");
            broadcast_room(rindex, initial_score_msg);
        }

    } else if (strcmp(cmd, "GAME_SCORE") == 0) {
        int score = 0;
        if (sscanf(line, "GAME_SCORE %d", &score) != 1) {
            send_to_client(cindex, "ERROR Invalid_score_format\n");
            return;
        }
        int rid = clients[cindex].room_id;
        if (rid < 0) {
            send_to_client(cindex, "ERROR Not_in_room\n");
            return;
        }
        int rindex = find_room_by_id(rid);
        if (rindex < 0) return;
        
        Room *r = &rooms[rindex];
        // cap nhat diem cua nguoi choi nay
        int pos = -1;
        for (int i = 0; i < r->num_players; ++i) {
            if (r->clients[i] == cindex) {
                pos = i;
                break;
            }
        }
        if (pos >= 0) {
            r->scores[pos] = score;
        }
        
        // tao bang xep hang day du
        // sao chep du lieu de sap xep
        typedef struct {
            char username[USERNAME_LEN];
            int score;
        } PlayerScore;
        PlayerScore rankings[4];
        for (int i = 0; i < r->num_players; ++i) {
            int ci = r->clients[i];
            strncpy(rankings[i].username, 
                    clients[ci].username[0] ? clients[ci].username : "guest",
                    USERNAME_LEN);
            rankings[i].score = r->scores[i];
        }
        
        // sap xep theo diem giam dan (bubble sort)
        for (int i = 0; i < r->num_players - 1; ++i) {
            for (int j = 0; j < r->num_players - i - 1; ++j) {
                if (rankings[j].score < rankings[j+1].score) {
                    PlayerScore tmp = rankings[j];
                    rankings[j] = rankings[j+1];
                    rankings[j+1] = tmp;
                }
            }
        }
        
        // tao tin SCORE_UPDATE voi bang xep hang day du
        char msg[BUF_SIZE];
        int offset = 0;
        offset += snprintf(msg + offset, BUF_SIZE - offset, "SCORE_UPDATE");
        for (int i = 0; i < r->num_players; ++i) {
            offset += snprintf(msg + offset, BUF_SIZE - offset, " %s:%d",
                             rankings[i].username, rankings[i].score);
        }
        offset += snprintf(msg + offset, BUF_SIZE - offset, "\n");
        
        broadcast_room(rindex, msg);

    } else if (strcmp(cmd, "SET_MODE") == 0) {
        int mode = 0;
        if (sscanf(line, "SET_MODE %d", &mode) != 1) {
            send_to_client(cindex, "ERROR Invalid_mode\n");
            return;
        }
        
        int rid = clients[cindex].room_id;
        if (rid < 0) {
            send_to_client(cindex, "ERROR Not_in_room\n");
            return;
        }
        int rindex = find_room_by_id(rid);
        if (rindex < 0) return;
        
        Room *r = &rooms[rindex];
        
        // Chi nguoi duoc chon (mode_chooser) moi duoc set mode
        if (r->mode_chooser != cindex) {
            send_to_client(cindex, "ERROR Not_authorized\n");
            return;
        }
        
        // Set mode va time limit
        r->game_mode = mode;
        switch(mode) {
            case 0: r->time_limit = 0; break;      // Survival
            case 1: r->time_limit = 60; break;     // 60s
            case 2: r->time_limit = 180; break;    // 180s
            case 3: r->time_limit = 300; break;    // 300s
            default: r->time_limit = 0; break;
        }
        
        r->mode_selected = 1;  // Danh dau mode da duoc chon
        
        send_to_client(cindex, "READY_OK\n");
        
        // Broadcast mode den tat ca
        char mode_msg[256];
        const char *mode_name = "";
        switch(mode) {
            case 0: mode_name = "SURVIVAL"; break;
            case 1: mode_name = "TIME_ATTACK_60s"; break;
            case 2: mode_name = "TIME_ATTACK_180s"; break;
            case 3: mode_name = "TIME_ATTACK_300s"; break;
        }
        snprintf(mode_msg, sizeof(mode_msg), "GAME_MODE %s %d\n", mode_name, r->time_limit);
        broadcast_room(rindex, mode_msg);
        
        // Kiem tra xem tat ca da ready chua -> bat dau game
        int all_ready = 1;
        for (int i = 0; i < r->num_players; ++i) {
            if (!r->ready[i]) {
                all_ready = 0;
                break;
            }
        }
        
        if (all_ready && !r->game_started) {
            r->playing = 1;
            r->game_started = 1;
            
            // Gui countdown de dong bo
            broadcast_room(rindex, "COUNTDOWN 3\n");
            sleep(1);
            broadcast_room(rindex, "COUNTDOWN 2\n");
            sleep(1);
            broadcast_room(rindex, "COUNTDOWN 1\n");
            sleep(1);
            // Gui START_GAME voi mode va time limit
            char start_msg[256];
            snprintf(start_msg, sizeof(start_msg), "START_GAME %d %d\n", r->game_mode, r->time_limit);
            broadcast_room(rindex, start_msg);
            
            // Reset diem (nhung GIU ready = 1 de tranh race condition)
            for (int i = 0; i < r->num_players; ++i) {
                r->scores[i] = 0;
            }
            
            // Gui SCORE_UPDATE ban dau voi tat ca nguoi choi diem 0
            char initial_score_msg[BUF_SIZE];
            int offset = 0;
            offset += snprintf(initial_score_msg + offset, BUF_SIZE - offset, "SCORE_UPDATE");
            for (int i = 0; i < r->num_players; ++i) {
                int ci = r->clients[i];
                const char *username = clients[ci].username[0] ? clients[ci].username : "guest";
                offset += snprintf(initial_score_msg + offset, BUF_SIZE - offset, " %s:0", username);
            }
            offset += snprintf(initial_score_msg + offset, BUF_SIZE - offset, "\n");
            broadcast_room(rindex, initial_score_msg);
        }

    } else if (strcmp(cmd, "GAME_END") == 0) {
        // Nguoi choi ket thuc game (quit)
        int rid = clients[cindex].room_id;
        if (rid < 0) return;
        
        int rindex = find_room_by_id(rid);
        if (rindex < 0) return;
        
        Room *r = &rooms[rindex];
        
        // Lay thong tin nguoi choi nay
        const char *finished_username = clients[cindex].username[0] ? clients[cindex].username : "guest";
        int finished_score = 0;
        
        // tim vi tri va reset ready cua nguoi nay
        for (int i = 0; i < r->num_players; ++i) {
            if (r->clients[i] == cindex) {
                finished_score = r->scores[i];
                r->ready[i] = 0;
                
                // Cap nhat ki luc nguoi choi (neu da dang nhap)
                if (clients[cindex].logged_in) {
                    update_player_record(finished_username, finished_score);
                }
                break;
            }
        }
        
        // Broadcast cho cac client khac biet nguoi nay da ket thuc
        char notify_msg[256];
        snprintf(notify_msg, sizeof(notify_msg), "PLAYER_FINISHED %s %d\n", 
                 finished_username, finished_score);
        broadcast_room(rindex, notify_msg);
        
        // Kiem tra neu tat ca deu ket thuc (ready = 0), reset game_started
        int all_finished = 1;
        for (int i = 0; i < r->num_players; ++i) {
            if (r->ready[i]) {
                all_finished = 0;
                break;
            }
        }
        
        if (all_finished && r->game_started) {
            // Tao bang ket qua cuoi cung
            typedef struct {
                char username[USERNAME_LEN];
                int score;
            } PlayerScore;
            PlayerScore rankings[4];
            for (int i = 0; i < r->num_players; ++i) {
                int ci = r->clients[i];
                strncpy(rankings[i].username, 
                        clients[ci].username[0] ? clients[ci].username : "guest",
                        USERNAME_LEN);
                rankings[i].score = r->scores[i];
            }
            
            // Sap xep theo diem giam dan
            for (int i = 0; i < r->num_players - 1; ++i) {
                for (int j = 0; j < r->num_players - i - 1; ++j) {
                    if (rankings[j].score < rankings[j+1].score) {
                        PlayerScore tmp = rankings[j];
                        rankings[j] = rankings[j+1];
                        rankings[j+1] = tmp;
                    }
                }
            }
            
            // Gui ket qua cuoi cung den tat ca
            char final_msg[BUF_SIZE];
            int offset = 0;
            offset += snprintf(final_msg + offset, BUF_SIZE - offset, "FINAL_RESULTS");
            for (int i = 0; i < r->num_players; ++i) {
                offset += snprintf(final_msg + offset, BUF_SIZE - offset, " %s:%d",
                                 rankings[i].username, rankings[i].score);
            }
            offset += snprintf(final_msg + offset, BUF_SIZE - offset, "\n");
            broadcast_room(rindex, final_msg);
            
            // Luu lich su dau
            const char *player_names[4];
            int player_scores[4];
            for (int i = 0; i < r->num_players; ++i) {
                int ci = r->clients[i];
                player_names[i] = clients[ci].username[0] ? clients[ci].username : "guest";
                player_scores[i] = r->scores[i];
            }
            save_match_history(r->id, player_names, player_scores, r->num_players);
            
            r->playing = 0;
            r->game_started = 0;
            r->game_mode = 0;
            r->time_limit = 0;
            r->mode_selected = 0;
            r->mode_chooser = -1;
        }
        
        send_to_client(cindex, "GAME_END_OK\n");

    } else if (strcmp(cmd, "VIEW_HISTORY") == 0) {
        // Gửi lịch sử trận đấu
        FILE *f = fopen(MATCH_HISTORY_FILE, "r");
        if (!f) {
            send_to_client(cindex, "HISTORY_BEGIN\n");
            send_to_client(cindex, "HISTORY_END\n");
            return;
        }
        
        send_to_client(cindex, "HISTORY_BEGIN\n");
        char buf[512];
        int line_count = 0;
        while (fgets(buf, sizeof(buf), f) != NULL && line_count < 200) {
            // Gửi từng dòng, thay \n bằng | để tránh conflict với protocol
            size_t len = strlen(buf);
            if (len > 0 && buf[len-1] == '\n') {
                buf[len-1] = '\0';
            }
            char line[520];
            snprintf(line, sizeof(line), "HISTORY_LINE %s\n", buf);
            send_to_client(cindex, line);
            line_count++;
        }
        fclose(f);
        send_to_client(cindex, "HISTORY_END\n");

    } else if (strcmp(cmd, "VIEW_RECORDS") == 0) {
        // Gửi kỷ lục người chơi
        FILE *f = fopen(PLAYER_RECORDS_FILE, "r");
        if (!f) {
            send_to_client(cindex, "RECORDS_BEGIN\n");
            send_to_client(cindex, "RECORDS_END\n");
            return;
        }
        
        // Đọc tất cả records vào array
        typedef struct {
            char username[USERNAME_LEN];
            int score;
        } RecordEntry;
        RecordEntry records[128];
        int count = 0;
        
        while (count < 128 && fscanf(f, "%31s %d", records[count].username, &records[count].score) == 2) {
            count++;
        }
        fclose(f);
        
        // Sắp xếp theo điểm giảm dần
        for (int i = 0; i < count - 1; ++i) {
            for (int j = 0; j < count - i - 1; ++j) {
                if (records[j].score < records[j+1].score) {
                    RecordEntry tmp = records[j];
                    records[j] = records[j+1];
                    records[j+1] = tmp;
                }
            }
        }
        
        // Gửi kết quả
        send_to_client(cindex, "RECORDS_BEGIN\n");
        int display_count = count < 50 ? count : 50; // Top 50
        for (int i = 0; i < display_count; ++i) {
            char line[128];
            snprintf(line, sizeof(line), "RECORD %s %d\n", records[i].username, records[i].score);
            send_to_client(cindex, line);
        }
        send_to_client(cindex, "RECORDS_END\n");

    } else {
        send_to_client(cindex, "ERROR Unknown_command\n");
    }
}

/* Tach buffer theo dong (xu ly truyen dong) */
static void process_client_buffer(int idx) {
    Client *c = &clients[idx];
    int start = 0;
    for (int i = 0; i < c->recv_len; ++i) {
        if (c->recv_buf[i] == '\n') {
            int line_len = i - start + 1;
            char line[BUF_SIZE];
            if (line_len >= BUF_SIZE) line_len = BUF_SIZE - 1;
            memcpy(line, c->recv_buf + start, line_len);
            line[line_len] = '\0';
            handle_command(idx, line);
            start = i + 1;
        }
    }
    if (start > 0) {
        int remaining = c->recv_len - start;
        memmove(c->recv_buf, c->recv_buf + start, remaining);
        c->recv_len = remaining;
    }
}

/* ===================== main server ===================== */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(listen_fd); return 1;
    }
    if (listen(listen_fd, 16) < 0) {
        perror("listen"); close(listen_fd); return 1;
    }

    printf("Server listening on port %d\n", port);

    load_accounts();
    init_clients();
    init_rooms();

    fd_set readfds;
    int maxfd = listen_fd;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        maxfd = listen_fd;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].used) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > maxfd) maxfd = clients[i].fd;
            }
        }

        int ret = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        // Ket noi moi
        if (FD_ISSET(listen_fd, &readfds)) {
            struct sockaddr_in cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            int cfd = accept(listen_fd, (struct sockaddr*)&cliaddr, &clilen);
            if (cfd < 0) {
                perror("accept");
            } else {
                int slot = find_empty_client_slot();
                if (slot < 0) {
                    const char *msg = "SERVER_FULL\n";
                    send(cfd, msg, strlen(msg), 0);
                    close(cfd);
                } else {
                    clients[slot].used      = 1;
                    clients[slot].fd        = cfd;
                    clients[slot].recv_len  = 0;
                    clients[slot].logged_in = 0;
                    clients[slot].room_id   = -1;
                    clients[slot].username[0] = '\0';
                    printf("New client at index %d (fd=%d)\n", slot, cfd);
                    send_to_client(slot, "WELCOME TETRIS SERVER\n");
                }
            }
        }

        // Du lieu tu client
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (!clients[i].used) continue;
            int fd = clients[i].fd;
            if (FD_ISSET(fd, &readfds)) {
                char buf[1024];
                ssize_t n = recv(fd, buf, sizeof(buf), 0);
                if (n <= 0) {
                    disconnect_client(i);
                } else {
                    if (clients[i].recv_len + n >= BUF_SIZE) {
                        // tran buffer -> reset cho an toan
                        clients[i].recv_len = 0;
                    } else {
                        memcpy(clients[i].recv_buf + clients[i].recv_len, buf, n);
                        clients[i].recv_len += (int)n;
                        process_client_buffer(i);
                    }
                }
            }
        }
    }

    close(listen_fd);
    return 0;
}

