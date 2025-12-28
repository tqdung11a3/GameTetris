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

/* ve bang + khoi hien tai + next piece + scoreboard */
static void draw_board(const Piece *p, const Piece *next_p, int score, int level, const char *scoreboard_text, int time_left, int time_limit) {
    printf("\033[H\033[J"); // clear screen
    
    // Header với màu sắc
    printf("\033[1;36m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;33m🎮 TETRIS ONLINE 🎮\033[0m                                    \033[1;36m║\033[0m\n");
    printf("\033[1;36m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;32mScore:\033[0m \033[1;37m%-10d\033[0m  \033[1;34mLevel:\033[0m \033[1;37m%-3d\033[0m", score, level);
    if (time_limit > 0) {
        printf("  \033[1;31m⏱ Time:\033[0m \033[1;37m%3d/%3d sec\033[0m", time_left, time_limit);
    }
    printf("        \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;35mControls:\033[0m \033[0;37ma=←  d=→  s=↓  w=↻  q=quit\033[0m              \033[1;36m║\033[0m\n");
    printf("\033[1;36m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    
    // Vẽ board với border đẹp
    printf("\033[1;36m║\033[0m  \033[1;37m┌");
    for (int x = 0; x < BOARD_W; ++x) printf("──");
    printf("┐\033[0m  \033[1;36m║\033[0m\n");
    
    for (int y = 0; y < BOARD_H; ++y) {
        printf("\033[1;36m║\033[0m  \033[1;37m│\033[0m");
        for (int x = 0; x < BOARD_W; ++x) {
            int filled = board[y][x];
            int is_current_piece = 0;
            if (p) {
                for (int py = 0; py < 4; ++py) {
                    for (int px = 0; px < 4; ++px) {
                        if (tetrominoes[p->shape][p->rot][py][px]) {
                            int bx = p->x + px;
                            int by = p->y + py;
                            if (bx == x && by == y) {
                                filled = 1;
                                is_current_piece = 1;
                            }
                        }
                    }
                }
            }
            
            if (filled) {
                if (is_current_piece) {
                    // Màu sáng cho khối đang rơi
                    printf("\033[1;43m  \033[0m"); // Nền vàng sáng
                } else {
                    // Màu cho khối đã đặt
                    int color = (y + x) % 6 + 1; // Màu thay đổi theo vị trí
                    printf("\033[1;4%dm██\033[0m", color);
                }
            } else {
                printf("  ");
            }
        }
        printf("\033[1;37m│\033[0m  \033[1;36m║\033[0m\n");
    }
    
    printf("\033[1;36m║\033[0m  \033[1;37m└");
    for (int x = 0; x < BOARD_W; ++x) printf("──");
    printf("┘\033[0m  \033[1;36m║\033[0m\n");
    printf("\033[1;36m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    
    // Next Piece
    if (next_p) {
        printf("\033[1;36m║\033[0m  \033[1;35mNext:\033[0m ");
        // Ve next piece (4x4 grid)
        for (int py = 0; py < 4; ++py) {
            for (int px = 0; px < 4; ++px) {
                if (tetrominoes[next_p->shape][0][py][px]) {
                    printf("\033[1;43m██\033[0m");
                } else {
                    printf("  ");
                }
            }
            if (py < 3) {
                printf("\033[1;36m║\033[0m\n\033[1;36m║\033[0m        ");
            }
        }
        printf("                    \033[1;36m║\033[0m\n");
        printf("\033[1;36m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    }
    
    // Leaderboard
    printf("\033[1;36m║\033[0m  \033[1;33m🏆 LEADERBOARD (Room)\033[0m                              \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m  ");
    if (scoreboard_text && strlen(scoreboard_text) > 0) {
        // In từng dòng của scoreboard với màu
        char text_copy[512];
        strncpy(text_copy, scoreboard_text, sizeof(text_copy) - 1);
        text_copy[sizeof(text_copy) - 1] = '\0';
        char *line = strtok(text_copy, "\n");
        int first_line = 1;
        while (line) {
            if (!first_line) printf("\033[1;36m║\033[0m  ");
            printf("\033[0;37m%s\033[0m", line);
            int padding = 50 - strlen(line);
            if (padding > 0) {
                for (int i = 0; i < padding; i++) printf(" ");
            }
            printf("  \033[1;36m║\033[0m\n");
            line = strtok(NULL, "\n");
            first_line = 0;
        }
    } else {
        printf("\033[0;37m(waiting for scores...)\033[0m");
        int padding = 50 - 23;
        for (int i = 0; i < padding; i++) printf(" ");
        printf("  \033[1;36m║\033[0m\n");
    }
    printf("\033[1;36m╚════════════════════════════════════════════════════════════╝\033[0m\n");
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
static void tetris_game_loop(Conn *conn, int time_limit) {
    board_clear();
    srand((unsigned)time(NULL));


    int score = 0;
    int level = 1;              // Level bat dau tu 1
    int total_lines_cleared = 0; // Tong so hang da xoa
    int drop_interval = 500;     // Toc do roi ban dau (ms)
    char scoreboard_text[512] = "";

    /* chuyen stdin sang che do doc tung ky tu, khong echo */
    system("stty cbreak -echo");

    int game_over = 0;
    long last_drop = 0;
    long start_time = 0;
    struct timespec ts;

    // Khoi tao khối đầu tiên và khối tiếp theo
    Piece cur;
    Piece next;
    cur.shape = rand() % 7;
    cur.rot   = 0;
    cur.x     = BOARD_W/2 - 2;
    cur.y     = 0;
    next.shape = rand() % 7;
    next.rot   = 0;

    while (!game_over) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        long now_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        if (last_drop == 0) {
            last_drop = now_ms;
            start_time = now_ms;
        }
        
        // Kiem tra het thoi gian (neu co time limit)
        if (time_limit > 0) {
            long elapsed_sec = (now_ms - start_time) / 1000;
            if (elapsed_sec >= time_limit) {
                game_over = 1;
                break;
            }
        }

        /* nhan tin SCORE_UPDATE, PLAYER_FINISHED, PLAYER_DISCONNECTED (neu co) */
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
            } else if (strncmp(line, "PLAYER_FINISHED", 15) == 0) {
                // Thong bao co nguoi ket thuc game
                char username[64];
                int final_score = 0;
                if (sscanf(line, "PLAYER_FINISHED %63s %d", username, &final_score) == 2) {
                    // Them thong bao vao scoreboard_text
                    char notify[128];
                    snprintf(notify, sizeof(notify), "\n>>> %s GAME OVER <<<", username);
                    strncat(scoreboard_text, notify, sizeof(scoreboard_text) - strlen(scoreboard_text) - 1);
                }
            } else if (strncmp(line, "PLAYER_DISCONNECTED", 19) == 0) {
                // Thong bao co nguoi disconnect
                char username[64];
                int final_score = 0;
                if (sscanf(line, "PLAYER_DISCONNECTED %63s %d", username, &final_score) == 2) {
                    // Them thong bao vao scoreboard_text
                    char notify[128];
                    snprintf(notify, sizeof(notify), "\n>>> %s OUT GAME <<<", username);
                    strncat(scoreboard_text, notify, sizeof(scoreboard_text) - strlen(scoreboard_text) - 1);
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

        /* roi xuong theo thoi gian - toc do tang dan theo level */
        clock_gettime(CLOCK_MONOTONIC, &ts);
        now_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
        if (now_ms - last_drop >= drop_interval) {
            if (piece_fits(&cur, cur.x, cur.y+1, cur.rot)) {
                cur.y++;
            } else {
                // cham day: co dinh + clear line + spawn khoi moi
                piece_lock(&cur);
                int lines = board_clear_lines();
                if (lines > 0) {
                    // Cap nhat tong so hang da xoa va level (tang level moi 5 hang de ro rang hon)
                    total_lines_cleared += lines;
                    int new_level = (total_lines_cleared / 5) + 1;
                    if (new_level > level) {
                        level = new_level;
                        // Tinh toan toc do roi moi: giam 20ms moi level, toi thieu 50ms
                        drop_interval = 500 - (level - 1) * 20;
                        if (drop_interval < 50) drop_interval = 50;
                        
                        // Hien thi thong bao level up
                        printf("\n\033[1;35m╔════════════════════════════════════════════════════════════╗\033[0m\n");
                        printf("\033[1;35m║\033[0m              \033[1;33m🚀 LEVEL UP! LEVEL %d 🚀\033[0m              \033[1;35m║\033[0m\n", level);
                        printf("\033[1;35m║\033[0m         \033[1;37mTốc độ rơi tăng! (%dms/ô)\033[0m         \033[1;35m║\033[0m\n", drop_interval);
                        printf("\033[1;35m╚════════════════════════════════════════════════════════════╝\033[0m\n");
                        fflush(stdout);
                        usleep(800000); // Hien thi 0.8 giay
                    }
                    
                    // He thong diem Tetris goc: an nhieu hang cung luc = nhieu diem hon
                    int points = 0;
                    const char *line_name = "";
                    switch(lines) {
                        case 1: 
                            points = 100;
                            line_name = "SINGLE!";
                            break;
                        case 2: 
                            points = 300;
                            line_name = "DOUBLE!!";
                            break;
                        case 3: 
                            points = 500;
                            line_name = "TRIPLE!!!";
                            break;
                        case 4: 
                            points = 800;
                            line_name = "TETRIS!!!!";
                            break;
                        default: 
                            points = lines * 100;
                            line_name = "AMAZING!";
                            break;
                    }
                    score += points;
                    
                    // Hien thi thong bao tam thoi với hiệu ứng
                    printf("\n\033[1;33m╔════════════════════════════════════════════════════════════╗\033[0m\n");
                    printf("\033[1;33m║\033[0m                    \033[1;32m%s\033[0m                    \033[1;33m║\033[0m\n", line_name);
                    printf("\033[1;33m║\033[0m                  \033[1;37m+%d points!\033[0m                  \033[1;33m║\033[0m\n", points);
                    printf("\033[1;33m╚════════════════════════════════════════════════════════════╝\033[0m\n");
                    fflush(stdout);
                    usleep(500000); // Hiển thị 0.5 giây
                    
                    char cmd[64];
                    snprintf(cmd, sizeof(cmd), "GAME_SCORE %d", score);
                    conn_send_line(conn, cmd); // gui diem len server
                }
                // Su dung next piece da co lam current piece
                cur = next;
                cur.x = BOARD_W/2 - 2;
                cur.y = 0;
                cur.rot = 0;
                // Tao next piece moi
                next.shape = rand() % 7;
                next.rot = 0;
                if (!piece_fits(&cur, cur.x, cur.y, cur.rot)) {
                    game_over = 1; // khong con cho: thua
                }
            }
            last_drop = now_ms;
        }

        // Tinh thoi gian con lai
        int time_left = 0;
        if (time_limit > 0) {
            long elapsed_sec = (now_ms - start_time) / 1000;
            time_left = time_limit - elapsed_sec;
            if (time_left < 0) time_left = 0;
        }
        
        draw_board(&cur, &next, score, level, scoreboard_text, time_left, time_limit);
    }

    system("stty sane");
    
    // Gui diem cuoi cung truoc khi ket thuc (ke ca 0 diem)
    char final_score_cmd[64];
    snprintf(final_score_cmd, sizeof(final_score_cmd), "GAME_SCORE %d", score);
    conn_send_line(conn, final_score_cmd);
    
    // Gui GAME_END den server de reset ready
    conn_send_line(conn, "GAME_END");
    
    // Doi nhan ket qua cuoi cung tu server
    printf("\n\033[1;32m===========================================\033[0m\n");
    printf("\033[1;33m           MATCH RESULTS\033[0m\n");
    printf("\033[1;32m===========================================\033[0m\n");
    printf("Your final score: \033[1;36m%d\033[0m\n\n", score);
    
    // Doi FINAL_RESULTS hoac GAME_END_OK
    int got_results = 0;
    for (int attempts = 0; attempts < 50; attempts++) {  // Doi toi da 5s
        char response[BUF_SIZE];
        if (conn_read_line_nonblock(conn, response, sizeof(response)) == 1) {
            trim_newline(response);
            if (strncmp(response, "FINAL_RESULTS", 13) == 0) {
                // Hien thi ket qua cuoi cung
                printf("\033[1;33mFinal Ranking:\033[0m\n");
                printf("-------------------------------------------\n");
                
                char *ptr = response + 13; // Bo qua "FINAL_RESULTS"
                int rank = 1;
                while (*ptr != '\0') {
                    while (*ptr == ' ') ptr++;
                    if (*ptr == '\0') break;
                    
                    char username[64];
                    int uscore = 0;
                    if (sscanf(ptr, "%63[^:]:%d", username, &uscore) == 2) {
                        if (rank == 1) {
                            printf("\033[1;33m  🏆 #%d: %-15s %d points [WINNER]\033[0m\n", 
                                   rank, username, uscore);
                        } else if (rank == 2) {
                            printf("\033[1;37m  🥈 #%d: %-15s %d points\033[0m\n", 
                                   rank, username, uscore);
                        } else if (rank == 3) {
                            printf("\033[1;31m  🥉 #%d: %-15s %d points\033[0m\n", 
                                   rank, username, uscore);
                        } else {
                            printf("     #%d: %-15s %d points\n", 
                                   rank, username, uscore);
                        }
                        rank++;
                        
                        // Chuyen den user tiep theo
                        while (*ptr != ' ' && *ptr != '\0') ptr++;
                    } else {
                        break;
                    }
                }
                got_results = 1;
                break;
            } else if (strncmp(response, "GAME_END_OK", 11) == 0) {
                break;
            }
        }
        usleep(100000);  // Doi 100ms
    }
    
    if (!got_results) {
        printf("Waiting for other players to finish...\n");
    }
    
    printf("\033[1;32m===========================================\033[0m\n");
    
    // Clear stdin buffer
    int ch;
    while ((ch = getchar()) != EOF && ch != '\n');
    
    printf("Press ENTER to continue...");
    getchar();
}

/* Hien thi trang thai READY_STATUS */
static void print_ready_status_line(const char *line) {
    printf("\n--- Ready Status ---\n");
    const char *ptr = line + 12; // bo qua "READY_STATUS"
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
    fflush(stdout);
}

/* Cho den khi nhan START_GAME, xu ly COUNTDOWN/READY_STATUS */
static void wait_for_game_start_blocking(Conn *conn) {
    char line[BUF_SIZE];
    while (1) {
        int rr = conn_read_line(conn, line, sizeof(line));
        if (rr <= 0) {
            printf("Disconnected while waiting for game start.\n");
            exit(1);
        }
        trim_newline(line);

        if (strncmp(line, "GAME_MODE", 9) == 0) {
            // Nhan thong bao mode da duoc chon
            char mode_name[64];
            int time_limit = 0;
            if (sscanf(line, "GAME_MODE %63s %d", mode_name, &time_limit) >= 1) {
                printf("\n>>> Game Mode: %s", mode_name);
                if (time_limit > 0) {
                    printf(" (%d seconds)", time_limit);
                }
                printf(" <<<\n");
            }
        } else if (strncmp(line, "COUNTDOWN", 9) == 0) {
            int count = 0;
            if (sscanf(line, "COUNTDOWN %d", &count) == 1) {
                printf("\r*** GAME STARTING IN %d... ***", count);
                fflush(stdout);
            }
        } else if (strncmp(line, "START_GAME", 10) == 0) {
            int mode = 0, time_limit = 0;
            sscanf(line, "START_GAME %d %d", &mode, &time_limit);
            
            printf("\n\n=== GO! GO! GO! ===\n");
            if (time_limit > 0) {
                printf("=== TIME LIMIT: %d seconds ===\n", time_limit);
            }
            fflush(stdout);
            sleep(1);
            tetris_game_loop(conn, time_limit);
            break;
        } else if (strncmp(line, "READY_STATUS", 12) == 0) {
            print_ready_status_line(line);
        } else {
            printf("\nServer: %s\n", line);
            fflush(stdout);
        }
    }

    // Clear buffer sau khi tro lai lobby
    char discard[BUF_SIZE];
    while (conn_read_line_nonblock(conn, discard, sizeof(discard)) == 1);
}

/* --------------------- menu lobby (console) --------------------- */

static void print_menu(int in_room) {
    printf("\n");
    printf("\033[1;36m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║\033[0m                    \033[1;33m🎮 TETRIS ONLINE 🎮\033[0m                    \033[1;36m║\033[0m\n");
    printf("\033[1;36m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;32m1.\033[0m \033[0;37mRegister\033[0m                                          \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;32m2.\033[0m \033[0;37mLogin\033[0m                                             \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;32m3.\033[0m \033[0;37mList rooms\033[0m                                        \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;32m4.\033[0m \033[0;37mCreate room\033[0m                                       \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;32m5.\033[0m \033[0;37mJoin room\033[0m                                         \033[1;36m║\033[0m\n");
    if (in_room) {
        printf("\033[1;36m║\033[0m  \033[1;32m6.\033[0m \033[1;33m✅ Ready\033[0m \033[0;37m(start game when all ready)\033[0m              \033[1;36m║\033[0m\n");
    } else {
        printf("\033[1;36m║\033[0m  \033[1;32m6.\033[0m \033[0;30m(Must be in a room first)\033[0m                      \033[1;36m║\033[0m\n");
    }
    printf("\033[1;36m║\033[0m  \033[1;32m7.\033[0m \033[0;37mView Match History\033[0m                                \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;32m8.\033[0m \033[0;37mView Player Records\033[0m                               \033[1;36m║\033[0m\n");
    printf("\033[1;36m║\033[0m  \033[1;31m0.\033[0m \033[0;37mExit\033[0m                                              \033[1;36m║\033[0m\n");
    printf("\033[1;36m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\033[1;33mSelect:\033[0m ");
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

    while (running) {
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
            
            int ready_ack_received = 0;
            int game_started_inline = 0;
            while (!ready_ack_received && !game_started_inline) {
                int rr = conn_read_line(&conn, line, sizeof(line));
                if (rr <= 0) {
                    printf("Disconnected.\n");
                    running = 0;
                    break;
                }
                trim_newline(line);

                if (strncmp(line, "CHOOSE_MODE", 11) == 0) {
                    // Nguoi ready dau tien chon mode
                    printf("\n=== SELECT GAME MODE ===\n");
                    printf("0. SURVIVAL (Play until game over)\n");
                    printf("1. TIME ATTACK - 60 seconds\n");
                    printf("2. TIME ATTACK - 180 seconds (3 min)\n");
                    printf("3. TIME ATTACK - 300 seconds (5 min)\n");
                    printf("Select mode: ");
                    fflush(stdout);
                    
                    int mode = 0;
                    if (scanf("%d", &mode) == 1 && mode >= 0 && mode <= 3) {
                        char mode_cmd[64];
                        snprintf(mode_cmd, sizeof(mode_cmd), "SET_MODE %d", mode);
                        conn_send_line(&conn, mode_cmd);
                    } else {
                        printf("Invalid mode, using SURVIVAL mode\n");
                        conn_send_line(&conn, "SET_MODE 0");
                    }
                    // Clear input buffer
                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF);
                } else if (strncmp(line, "READY_OK", 8) == 0) {
                    ready_ack_received = 1;
                    printf("You are ready! Waiting for other players...\n");
                } else if (strncmp(line, "GAME_MODE", 9) == 0) {
                    // Nhan thong bao mode da duoc chon
                    char mode_name[64];
                    int time_limit = 0;
                    if (sscanf(line, "GAME_MODE %63s %d", mode_name, &time_limit) >= 1) {
                        printf("\n>>> Game Mode: %s", mode_name);
                        if (time_limit > 0) {
                            printf(" (%d seconds)", time_limit);
                        }
                        printf(" <<<\n");
                    }
                } else if (strncmp(line, "COUNTDOWN", 9) == 0) {
                    int count = 0;
                    if (sscanf(line, "COUNTDOWN %d", &count) == 1) {
                        printf("\r*** GAME STARTING IN %d... ***", count);
                        fflush(stdout);
                    }
                } else if (strncmp(line, "START_GAME", 10) == 0) {
                    int mode = 0, time_limit = 0;
                    sscanf(line, "START_GAME %d %d", &mode, &time_limit);
                    
                    printf("\n\n=== GO! GO! GO! ===\n");
                    if (time_limit > 0) {
                        printf("=== TIME LIMIT: %d seconds ===\n", time_limit);
                    }
                    fflush(stdout);
                    sleep(1);
                    game_started_inline = 1;
                    tetris_game_loop(&conn, time_limit);
                    // Clear buffer
                    char discard[BUF_SIZE];
                    while (conn_read_line_nonblock(&conn, discard, sizeof(discard)) == 1);
                } else if (strncmp(line, "READY_STATUS", 12) == 0) {
                    print_ready_status_line(line);
                } else {
                    printf("Server: %s\n", line);
                }
            }

            if (!running) break;
            if (game_started_inline) continue;
            if (ready_ack_received) {
                wait_for_game_start_blocking(&conn);
            }

        } else if (opt == 7) {          // VIEW MATCH HISTORY
            printf("\n========== MATCH HISTORY ==========\n");
            FILE *f = fopen("match_history.txt", "r");
            if (!f) {
                printf("No match history found yet.\n");
            } else {
                char buf[256];
                int count = 0;
                while (fgets(buf, sizeof(buf), f) != NULL) {
                    printf("%s", buf);
                    count++;
                    if (count > 100) break; // Gioi han hien thi 100 dong
                }
                fclose(f);
                if (count == 0) {
                    printf("No matches recorded yet.\n");
                }
            }
            printf("===================================\n");
            
        } else if (opt == 8) {          // VIEW PLAYER RECORDS
            printf("\n========== PLAYER RECORDS ==========\n");
            FILE *f = fopen("player_records.txt", "r");
            if (!f) {
                printf("No player records found yet.\n");
            } else {
                // Doc tat ca records vao array
                typedef struct {
                    char username[64];
                    int score;
                } RecordEntry;
                RecordEntry records[128];
                int count = 0;
                
                while (count < 128 && fscanf(f, "%63s %d", records[count].username, &records[count].score) == 2) {
                    count++;
                }
                fclose(f);
                
                // Sap xep theo diem giam dan
                for (int i = 0; i < count - 1; ++i) {
                    for (int j = 0; j < count - i - 1; ++j) {
                        if (records[j].score < records[j+1].score) {
                            RecordEntry tmp = records[j];
                            records[j] = records[j+1];
                            records[j+1] = tmp;
                        }
                    }
                }
                
                // Hien thi top 20
                printf("TOP PLAYERS (Best Scores):\n");
                printf("------------------------------------\n");
                int display_count = count < 20 ? count : 20;
                for (int i = 0; i < display_count; ++i) {
                    printf("#%d: %-20s %d points\n", i+1, records[i].username, records[i].score);
                }
                
                if (count == 0) {
                    printf("No records yet.\n");
                } else if (count > 20) {
                    printf("... and %d more players\n", count - 20);
                }
            }
            printf("====================================\n");
            
        } else {
            printf("Unknown option.\n");
        }
    }

    close(fd);
    return 0;
}

