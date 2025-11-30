// client.c  -- Tetris online client (terminal graphics)
// Bien dich:  gcc client.c -o client
// Chay:      ./client 127.0.0.1 5555

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <time.h>

#define BUF_SIZE 4096

/* ------------------------- tien ich chung ------------------------- */

static void trim_newline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        len--;
    }
}

/* Ket noi TCP co buffer de tach dong (truyen dong) */
typedef struct {
    int  fd;
    char buf[BUF_SIZE];
    int  len;
} Conn;

static void conn_init(Conn *c, int fd) {
    c->fd = fd;
    c->len = 0;
}

/* Doc 1 dong (blocking) - dung cho lobby */
static int conn_read_line(Conn *c, char *out, size_t out_size) {
    while (1) {
        for (int i = 0; i < c->len; ++i) {
            if (c->buf[i] == '\n') {
                int line_len = i + 1;
                if ((size_t)line_len >= out_size) line_len = (int)out_size - 1;
                memcpy(out, c->buf, line_len);
                out[line_len] = '\0';
                int remaining = c->len - (i+1);
                memmove(c->buf, c->buf + i + 1, remaining);
                c->len = remaining;
                return 1;
            }
        }
        if (c->len >= BUF_SIZE) c->len = 0;
        ssize_t n = recv(c->fd, c->buf + c->len, BUF_SIZE - c->len, 0);
        if (n < 0) {
            perror("recv");
            return -1;
        } else if (n == 0) {
            return 0; // server dong
        } else {
            c->len += (int)n;
        }
    }
}

/* Doc 1 dong (non-blocking) - dung trong game loop */
static int conn_read_line_nonblock(Conn *c, char *out, size_t out_size) {
    // kiem tra buffer hien co
    for (int i = 0; i < c->len; ++i) {
        if (c->buf[i] == '\n') {
            int line_len = i + 1;
            if ((size_t)line_len >= out_size) line_len = (int)out_size - 1;
            memcpy(out, c->buf, line_len);
            out[line_len] = '\0';
            int remaining = c->len - (i+1);
            memmove(c->buf, c->buf + i + 1, remaining);
            c->len = remaining;
            return 1;
        }
    }
    // doc them du lieu (non-blocking)
    char tmp[1024];
    ssize_t n = recv(c->fd, tmp, sizeof(tmp), MSG_DONTWAIT);
    if (n <= 0) return 0;
    if (c->len + n >= BUF_SIZE) c->len = 0;
    memcpy(c->buf + c->len, tmp, n);
    c->len += (int)n;

    for (int i = 0; i < c->len; ++i) {
        if (c->buf[i] == '\n') {
            int line_len = i + 1;
            if ((size_t)line_len >= out_size) line_len = (int)out_size - 1;
            memcpy(out, c->buf, line_len);
            out[line_len] = '\0';
            int remaining = c->len - (i+1);
            memmove(c->buf, c->buf + i + 1, remaining);
            c->len = remaining;
            return 1;
        }
    }
    return 0;
}

/* Gui 1 dong, tu them '\n' neu thieu */
static int conn_send_line(Conn *c, const char *line) {
    char buf[BUF_SIZE];
    size_t len = strlen(line);
    if (len + 1 >= sizeof(buf)) len = sizeof(buf) - 2;
    memcpy(buf, line, len);
    if (len == 0 || buf[len-1] != '\n') buf[len++] = '\n';
    ssize_t n = send(c->fd, buf, len, 0);
    if (n < 0) { perror("send"); return -1; }
    return 0;
}

/* ---------------------------- TETRIS ---------------------------- */

#define BOARD_W 10
#define BOARD_H 20

typedef struct {
    int x, y;
    int shape;
    int rot;
} Piece;

static int board[BOARD_H][BOARD_W];

/* 7 tetromino, moi cai 4 rotation, moi rotation la ma tran 4x4 */
static const int tetrominoes[7][4][4][4] = {
    {   // I
        {
            {0,0,0,0},
            {1,1,1,1},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,0,1,0},
            {0,0,1,0},
            {0,0,1,0},
            {0,0,1,0}
        },
        {
            {0,0,0,0},
            {0,0,0,0},
            {1,1,1,1},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,1,0,0}
        }
    },
    {   // O
        {
            {0,0,0,0},
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0}
        }
    },
    {   // T
        {
            {0,0,0,0},
            {1,1,1,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,1,0},
            {0,1,0,0},
            {0,0,0,0}
        }
    },
    {   // L
        {
            {0,0,0,0},
            {1,1,1,0},
            {1,0,0,0},
            {0,0,0,0}
        },
        {
            {1,1,0,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,0,1,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        }
    },
    {   // J
        {
            {0,0,0,0},
            {1,1,1,0},
            {0,0,1,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {1,1,0,0},
            {0,0,0,0}
        },
        {
            {1,0,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,1,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        }
    },
    {   // S
        {
            {0,0,0,0},
            {0,1,1,0},
            {1,1,0,0},
            {0,0,0,0}
        },
        {
            {1,0,0,0},
            {1,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {0,1,1,0},
            {1,1,0,0},
            {0,0,0,0}
        },
        {
            {1,0,0,0},
            {1,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        }
    },
    {   // Z
        {
            {0,0,0,0},
            {1,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,0,0},
            {1,0,0,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {1,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,0,0},
            {1,0,0,0},
            {0,0,0,0}
        }
    }
};

static void board_clear() {
    for (int y = 0; y < BOARD_H; ++y)
        for (int x = 0; x < BOARD_W; ++x)
            board[y][x] = 0;
}

static int piece_fits(const Piece *p, int nx, int ny, int nrot) {
    for (int py = 0; py < 4; ++py) {
        for (int px = 0; px < 4; ++px) {
            if (tetrominoes[p->shape][nrot][py][px]) {
                int bx = nx + px;
                int by = ny + py;
                if (bx < 0 || bx >= BOARD_W || by < 0 || by >= BOARD_H) return 0;
                if (board[by][bx]) return 0;
            }
        }
    }
    return 1;
}

static void piece_lock(const Piece *p) {
    for (int py = 0; py < 4; ++py) {
        for (int px = 0; px < 4; ++px) {
            if (tetrominoes[p->shape][p->rot][py][px]) {
                int bx = p->x + px;
                int by = p->y + py;
                if (bx >= 0 && bx < BOARD_W && by >= 0 && by < BOARD_H)
                    board[by][bx] = 1;
            }
        }
    }
}

static int board_clear_lines() {
    int lines = 0;
    for (int y = 0; y < BOARD_H; ++y) {
        int full = 1;
        for (int x = 0; x < BOARD_W; ++x)
            if (!board[y][x]) { full = 0; break; }
        if (full) {
            lines++;
            for (int yy = y; yy > 0; --yy)
                for (int x = 0; x < BOARD_W; ++x)
                    board[yy][x] = board[yy-1][x];
            for (int x = 0; x < BOARD_W; ++x) board[0][x] = 0;
        }
    }
    return lines;
}

/* ve bang + khoi hien tai + scoreboard */
static void draw_board(const Piece *p, int score, const char *scoreboard_text) {
    printf("\033[H\033[J"); // clear screen
    printf("TETRIS ONLINE - controls: a=left, d=right, s=down, w=rotate, q=quit\n");
    printf("Your score: %d\n", score);
    printf("--------------------------------\n");

    for (int y = 0; y < BOARD_H; ++y) {
        printf("|");
        for (int x = 0; x < BOARD_W; ++x) {
            int filled = board[y][x];
            if (p) {
                for (int py = 0; py < 4; ++py) {
                    for (int px = 0; px < 4; ++px) {
                        if (tetrominoes[p->shape][p->rot][py][px]) {
                            int bx = p->x + px;
                            int by = p->y + py;
                            if (bx == x && by == y) filled = 1;
                        }
                    }
                }
            }
            printf(filled ? "[]" : "  ");
        }
        printf("|\n");
    }
    printf("--------------------------------\n");
    printf("LEADERBOARD (Room):\n");
    printf("%s\n", scoreboard_text ? scoreboard_text : "(waiting for scores...)");
    fflush(stdout);
}

/* doc phim tu stdin (non-blocking) bang select */
static int read_input_char(int timeout_ms) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    int ret = select(STDIN_FILENO+1, &rfds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(STDIN_FILENO, &rfds)) {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1) return (int)ch;
    }
    return -1;
}

/* vong lap choi Tetris - gui GAME_SCORE, nhan SCORE_UPDATE */
static void tetris_game_loop(Conn *conn) {
    board_clear();
    srand((unsigned)time(NULL));

    Piece cur;
    cur.shape = rand() % 7;
    cur.rot   = 0;
    cur.x     = BOARD_W/2 - 2;
    cur.y     = 0;

    int score = 0;
    char scoreboard_text[512] = "Waiting for scores...";

    /* chuyen stdin sang che do doc tung ky tu, khong echo */
    system("stty cbreak -echo");

    int game_over = 0;
    long last_drop = 0;
    struct timespec ts;

    while (!game_over) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        long now_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        if (last_drop == 0) last_drop = now_ms;

        /* nhan tin SCORE_UPDATE (neu co) */
        char line[BUF_SIZE];
        while (conn_read_line_nonblock(conn, line, sizeof(line)) == 1) {
            trim_newline(line);
            if (strncmp(line, "SCORE_UPDATE", 12) == 0) {
                // phan tich bang xep hang: SCORE_UPDATE user1:score1 user2:score2 ...
                char formatted[512] = "";
                char *ptr = line + 12; // bo qua "SCORE_UPDATE"
                int rank = 1;
                while (*ptr != '\0') {
                    while (*ptr == ' ') ptr++; // bo qua khoang trang
                    if (*ptr == '\0') break;
                    
                    char username[64];
                    int uscore = 0;
                    if (sscanf(ptr, "%63[^:]:%d", username, &uscore) == 2) {
                        char entry[128];
                        snprintf(entry, sizeof(entry), "#%d: %s - %d points\n", 
                                rank++, username, uscore);
                        strncat(formatted, entry, sizeof(formatted) - strlen(formatted) - 1);
                        
                        // chuyen den user tiep theo
                        while (*ptr != ' ' && *ptr != '\0') ptr++;
                    } else {
                        break;
                    }
                }
                if (strlen(formatted) > 0) {
                    snprintf(scoreboard_text, sizeof(scoreboard_text), "%s", formatted);
                }
            }
        }

        /* xu ly input nguoi choi */
        int ch = read_input_char(50); // cho toi da 50ms
        if (ch == 'q') {
            game_over = 1;
            break;
        } else if (ch == 'a') {
            if (piece_fits(&cur, cur.x-1, cur.y, cur.rot)) cur.x--;
        } else if (ch == 'd') {
            if (piece_fits(&cur, cur.x+1, cur.y, cur.rot)) cur.x++;
        } else if (ch == 's') {
            if (piece_fits(&cur, cur.x, cur.y+1, cur.rot)) cur.y++;
        } else if (ch == 'w') {
            int nrot = (cur.rot + 1) % 4;
            if (piece_fits(&cur, cur.x, cur.y, nrot)) cur.rot = nrot;
        }

        /* roi xuong theo thoi gian */
        clock_gettime(CLOCK_MONOTONIC, &ts);
        now_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        if (now_ms - last_drop >= 500) { // 500ms roi 1 o
            if (piece_fits(&cur, cur.x, cur.y+1, cur.rot)) {
                cur.y++;
            } else {
                // cham day: co dinh + clear line + spawn khoi moi
                piece_lock(&cur);
                int lines = board_clear_lines();
                if (lines > 0) {
                    score += lines * 100;
                    char cmd[64];
                    snprintf(cmd, sizeof(cmd), "GAME_SCORE %d", score);
                    conn_send_line(conn, cmd); // gui diem len server
                }
                Piece next;
                next.shape = rand() % 7;
                next.rot   = 0;
                next.x     = BOARD_W/2 - 2;
                next.y     = 0;
                if (!piece_fits(&next, next.x, next.y, next.rot)) {
                    game_over = 1; // khong con cho: thua
                }
                cur = next;
            }
            last_drop = now_ms;
        }

        draw_board(&cur, score, scoreboard_text);
    }

    system("stty sane");
    
    // Gui GAME_END den server de reset ready
    conn_send_line(conn, "GAME_END");
    char response[128];
    // Doc response (non-blocking, khong cho lau)
    conn_read_line_nonblock(conn, response, sizeof(response));
    
    // Clear stdin buffer de tranh doc nham ky tu
    int ch;
    while ((ch = getchar()) != EOF && ch != '\n');
    
    printf("\n===========================================\n");
    printf("Game over! Final score: %d\n", score);
    printf("===========================================\n");
    printf("Press ENTER to continue...");
    getchar(); // cho nguoi choi nhan Enter
}

/* --------------------- menu lobby (console) --------------------- */

static void print_menu(int in_room) {
    printf("\n==== TETRIS ONLINE CLIENT ====\n");
    printf("1. Register\n");
    printf("2. Login\n");
    printf("3. List rooms\n");
    printf("4. Create room\n");
    printf("5. Join room\n");
    if (in_room) {
        printf("6. Ready (start game when all ready)\n");
    } else {
        printf("6. (Must be in a room first)\n");
    }
    printf("0. Exit\n");
    printf("Select: ");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }
    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
        perror("inet_pton"); close(fd); return 1;
    }
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect"); close(fd); return 1;
    }

    Conn conn;
    conn_init(&conn, fd);

    char line[BUF_SIZE];
    int r = conn_read_line(&conn, line, sizeof(line));
    if (r <= 0) {
        printf("Server closed connection.\n");
        close(fd);
        return 1;
    }
    printf("Server: %s", line);

    int running   = 1;
    int logged_in = 0;
    int in_room   = 0;
    int waiting_for_game = 0;

    while (running) {
        // Clear buffer cua connection truoc khi vao menu (tru khi dang cho game)
        if (!waiting_for_game) {
            char tmp_discard[BUF_SIZE];
            while (conn_read_line_nonblock(&conn, tmp_discard, sizeof(tmp_discard)) == 1) {
                // Bo qua cac tin nhan cu sau game
            }
        }
        
        // kiem tra neu server gui START_GAME khi dang cho
        if (waiting_for_game) {
            char tmp_line[BUF_SIZE];
            int should_display = 1;
            while (conn_read_line_nonblock(&conn, tmp_line, sizeof(tmp_line)) == 1) {
                trim_newline(tmp_line);
                if (strncmp(tmp_line, "START_GAME", 10) == 0) {
                    printf("\n=== ALL PLAYERS READY! STARTING GAME... ===\n");
                    sleep(1);
                    waiting_for_game = 0;
                    tetris_game_loop(&conn);
                    // Sau khi game ket thuc, clear buffer lan nua
                    while (conn_read_line_nonblock(&conn, tmp_line, sizeof(tmp_line)) == 1);
                    break;
                } else if (strncmp(tmp_line, "READY_STATUS", 12) == 0) {
                    // Hien thi trang thai ready: READY_STATUS user1:1 user2:0 ...
                    printf("\n--- Ready Status ---\n");
                    char *ptr = tmp_line + 12;
                    while (*ptr != '\0') {
                        while (*ptr == ' ') ptr++;
                        if (*ptr == '\0') break;
                        
                        char username[64];
                        int is_ready = 0;
                        if (sscanf(ptr, "%63[^:]:%d", username, &is_ready) == 2) {
                            printf("  %s: %s\n", username, is_ready ? "[READY]" : "[NOT READY]");
                            while (*ptr != ' ' && *ptr != '\0') ptr++;
                        } else {
                            break;
                        }
                    }
                    printf("--------------------\n");
                    should_display = 0;
                }
            }
            if (waiting_for_game && should_display) {
                printf("Waiting for other players to be ready...\n");
                sleep(1);
                continue;
            } else if (waiting_for_game) {
                sleep(1);
                continue;
            }
        }
        
        print_menu(in_room);
        char choice[16];
        if (!fgets(choice, sizeof(choice), stdin)) break;
        int opt = atoi(choice);

        if (opt == 0) {
            running = 0;
            break;

        } else if (opt == 1) {          // REGISTER
            char user[64], pass[64];
            printf("Username: "); fflush(stdout);
            if (!fgets(user, sizeof(user), stdin)) break;
            printf("Password: "); fflush(stdout);
            if (!fgets(pass, sizeof(pass), stdin)) break;
            trim_newline(user);
            trim_newline(pass);
            char cmd[BUF_SIZE];
            snprintf(cmd, sizeof(cmd), "REGISTER %s %s", user, pass);
            conn_send_line(&conn, cmd);
            if (conn_read_line(&conn, line, sizeof(line)) <= 0) { printf("Disconnected.\n"); break; }
            printf("Server: %s", line);

        } else if (opt == 2) {          // LOGIN
            char user[64], pass[64];
            printf("Username: "); fflush(stdout);
            if (!fgets(user, sizeof(user), stdin)) break;
            printf("Password: "); fflush(stdout);
            if (!fgets(pass, sizeof(pass), stdin)) break;
            trim_newline(user);
            trim_newline(pass);
            char cmd[BUF_SIZE];
            snprintf(cmd, sizeof(cmd), "LOGIN %s %s", user, pass);
            conn_send_line(&conn, cmd);
            if (conn_read_line(&conn, line, sizeof(line)) <= 0) { printf("Disconnected.\n"); break; }
            printf("Server: %s", line);
            if (strncmp(line, "LOGIN_OK", 8) == 0) logged_in = 1;

        } else if (opt == 3) {          // LIST_ROOMS
            conn_send_line(&conn, "LIST_ROOMS");
            while (1) {
                int rr = conn_read_line(&conn, line, sizeof(line));
                if (rr <= 0) { printf("Disconnected.\n"); running = 0; break; }
                printf("Server: %s", line);
                if (strncmp(line, "ROOM_LIST_END", 13) == 0) break;
            }

        } else if (opt == 4) {          // CREATE_ROOM
            if (!logged_in) { printf("You must login first.\n"); continue; }
            conn_send_line(&conn, "CREATE_ROOM");
            if (conn_read_line(&conn, line, sizeof(line)) <= 0) { printf("Disconnected.\n"); break; }
            printf("Server: %s", line);
            if (strncmp(line, "ROOM_CREATED", 12) == 0) in_room = 1;

        } else if (opt == 5) {          // JOIN_ROOM
            char tmp[32];
            printf("Enter room id to join: "); fflush(stdout);
            if (!fgets(tmp, sizeof(tmp), stdin)) break;
            trim_newline(tmp);
            char cmd[BUF_SIZE];
            snprintf(cmd, sizeof(cmd), "JOIN_ROOM %s", tmp);
            conn_send_line(&conn, cmd);
            if (conn_read_line(&conn, line, sizeof(line)) <= 0) { printf("Disconnected.\n"); break; }
            printf("Server: %s", line);
            if (strncmp(line, "JOIN_OK", 7) == 0) in_room = 1;

        } else if (opt == 6) {          // READY
            if (!in_room) {
                printf("You must be in a room first (create or join).\n");
                continue;
            }
            conn_send_line(&conn, "READY");
            if (conn_read_line(&conn, line, sizeof(line)) <= 0) { printf("Disconnected.\n"); break; }
            printf("Server: %s", line);
            if (strncmp(line, "READY_OK", 8) == 0) {
                waiting_for_game = 1;
                printf("You are ready! Waiting for other players...\n");
            }

        } else {
            printf("Unknown option.\n");
        }
    }

    close(fd);
    return 0;
}

