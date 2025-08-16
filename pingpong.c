/**
 
 Boilerplate code implementing the GUI for the pingpong game. Update the file accordingly as necessary. 
 You are allowed to update functions, add new functions, modify the stuctures etc. Keep the output graphics intact.
 
 CS3205 - Assignment 2 (Holi'25)
 Instructor: Ayon Chakraborty
 **/
 
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

#define WIDTH 80
#define HEIGHT 30
#define OFFSETX 10
#define OFFSETY 5

typedef struct {
    int x, y;
    int dx, dy;
} Ball;

typedef struct {
    int x; 
    int width;
} Paddle;

typedef struct {
    Ball ball;
    Paddle paddle_A;
    Paddle paddle_B;
    int score_A;
    int score_B;
} GameState;

Ball ball;
Paddle paddle_A, paddle_B;
int game_running = 1;
int score_A = 0, score_B = 0;
int is_server = -1 ; 
int server_socket = -1 , client_socket = -1 ;
struct sockaddr_in server_addr, client_addr ;
__socklen_t addr_size ; 

pthread_mutex_t game_state_mutex = PTHREAD_MUTEX_INITIALIZER ; 

void init();
void end_game();
void draw(WINDOW *win);
void *move_ball(void *args);
void update_paddle_A(int ch);
void update_paddle_B(int ch);
void reset_ball_A();
void reset_ball_B();
void *network_loop(void *args);
void send_game_state();
void receive_game_state();


int main(int argc, char *argv[]) {
    int port = 0;
    char* server_ip ;
    ball = (Ball){WIDTH / 2, HEIGHT / 2, 1, 1};
    paddle_A = (Paddle){WIDTH / 2 - 3, 10};
    paddle_B = (Paddle){WIDTH / 2 - 3, 10};
    if(strcmp(argv[1], "server") == 0){
        is_server = 1;
        port = atoi(argv[2]);

    }
    else if(strcmp(argv[1], "client") == 0){
        is_server = 0;
        server_ip = argv[2];
    }
    else
    {
        printf("Invalid argument\n");
        return 0;
    }

    if(is_server)
    {
        // Create socket
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            printf("Socket creation failed");
            
        }
        printf("Server socket created successfully.\n");

        // Configure server address
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        // Bind socket to the address
        if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            printf("Socket bind failed");
            close(server_socket);
            
        }
        printf("Socket successfully bound to port %d.\n", port);

        // Listen for incoming connections
        if (listen(server_socket, 1) < 0) {
            printf("Socket listen failed");
            close(server_socket);
            
        }
        printf("Server is listening for incoming connections...\n");

        // Accept a client connection
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            printf("Client connection failed");
            close(server_socket);
            
        }
        printf("Client connected successfully.\n");
    }
    else 
    {
        // Create socket
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket < 0) {
            printf("Socket creation failed");
            
        }
        printf("Client socket created successfully.\n");

        // Configure server address
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(server_ip);
        server_addr.sin_port = htons(12345);

        // Connect to server
        if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            printf("Connection to server failed");
            close(client_socket);
            
        }
        printf("Connected to server successfully.\n");
    }

    init();

    pthread_t ball_thread;
    
    pthread_t network_thread;

// Create the network thread
if (pthread_create(&network_thread, NULL, network_loop, NULL) != 0) {
    printf("Failed to create send thread");
    
}



// Wait for threads to finish

    if(is_server)
    {
    pthread_create(&ball_thread, NULL, move_ball, NULL);
    while (game_running) {
        int ch = getch();
        if (ch == 'q') {
            game_running = 0; /* if we want to exit by pressing 'q' */
            break;
        }
        update_paddle_A(ch);
        draw(stdscr);
    }
}
else
{
    while(game_running)
    {
        int ch = getch();
        if (ch == 'q') {
            game_running = 0; /* if we want to exit by pressing 'q' */
            break;
        }
        update_paddle_B(ch);
        
        draw(stdscr);
   }
}

    if(is_server)
    {
    pthread_join(ball_thread, NULL);
    }
    pthread_join(network_thread, NULL);
    
    end_game();

    return 0;
}

void *network_loop(void *args) {
while(game_running)
{
    if(is_server)
    {
        send_game_state();
        receive_game_state();
    }
    else
    {
        receive_game_state();
        send_game_state();
    }
    usleep(80000);
}
return NULL ; 


}

void send_game_state()
{
    GameState send_state;
    pthread_mutex_lock(&game_state_mutex);
    if(is_server)
    {
        send_state = (GameState){ball, paddle_A, paddle_B, score_A, score_B};
    }
    else
    {
        send_state.paddle_B = paddle_B;
    }
    pthread_mutex_unlock(&game_state_mutex);
    if(send(client_socket, &send_state, sizeof(GameState), 0) < 0)
    {
        printf("Failed to send game state\n");
        game_running = 0 ; 
    }
}

void receive_game_state()
{
    GameState recv_state ; 
    int recv_size = recv(client_socket, &recv_state, sizeof(GameState), 0);
    if(recv_size <= 0)
    {
        printf("Failed to receive game state\n");
        game_running = 0 ; 
    }
    else
{    pthread_mutex_lock(&game_state_mutex);
    if(is_server)
    {
        paddle_B = recv_state.paddle_B;
    }
    else
    {
        ball = recv_state.ball;
        paddle_A = recv_state.paddle_A;
        score_A = recv_state.score_A;
        score_B = recv_state.score_B;
    }
    pthread_mutex_unlock(&game_state_mutex);
}
}



void init() {
    initscr();
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_WHITE);
    init_pair(2, COLOR_YELLOW, COLOR_YELLOW);
    timeout(10);                    
    keypad(stdscr, TRUE);  
    curs_set(FALSE);
    noecho(); 
}

void end_game() {
    endwin();  // End curses mode
}

void draw(WINDOW *win) {
    clear();  // Clear the screen

    // Draw the border
    attron(COLOR_PAIR(1));
    for (int i = OFFSETX; i <= OFFSETX + WIDTH; i++) {
        mvprintw(OFFSETY-1, i, " ");
    }
    mvprintw(OFFSETY-1, OFFSETX + 3, "CS3205 NetPong, Ball: %d, %d", ball.x, ball.y);
    mvprintw(OFFSETY-1, OFFSETX + WIDTH-25, "Player A: %d, Player B: %d", score_A, score_B);
        
    for (int i = OFFSETY; i < OFFSETY + HEIGHT; i++) {
        mvprintw(i, OFFSETX, "  ");
        mvprintw(i, OFFSETX + WIDTH - 1, "  ");
    }
    for (int i = OFFSETX; i < OFFSETX + WIDTH; i++) {
        mvprintw(OFFSETY, i, " ");
        mvprintw(OFFSETY + HEIGHT - 1, i, " ");
    }
    attroff(COLOR_PAIR(1));
    
    // Draw the ball
    mvprintw(OFFSETY + ball.y, OFFSETX + ball.x, "o");

    // Draw the paddle
    attron(COLOR_PAIR(2));
    for (int i = 0; i < paddle_A.width; i++) {
        mvprintw(OFFSETY + HEIGHT - 4, OFFSETX + paddle_A.x + i, " ");
    }
    for (int i = 0; i < paddle_B.width; i++) {
        mvprintw(OFFSETY + 2, OFFSETX + paddle_B.x + i, " ");
    }
    attroff(COLOR_PAIR(2));

    refresh();
}

void *move_ball(void *args) {
    while (game_running) {
        // Move the ball
        pthread_mutex_lock(&game_state_mutex);
        ball.x += ball.dx;
        ball.y += ball.dy;

        // Ball bounces off left and right walls
        if (ball.x <= 2 || ball.x >= WIDTH - 2) {
            ball.dx = -ball.dx;
        }

        // Ball hits the bottom paddle
        if (ball.y == HEIGHT - 5 && ball.x >= paddle_A.x - 1 && ball.x < paddle_A.x + paddle_A.width + 1) {
            ball.dy = -ball.dy;
        }

        // Ball hits the top paddle
        if (ball.y == 3 && ball.x >= paddle_B.x - 1 && ball.x < paddle_B.x + paddle_B.width + 1) {
            ball.dy = -ball.dy;
        }

        // Ball goes past the bottom paddle (Game Over)
        if (ball.y >= HEIGHT - 3) {
            reset_ball_A();
        }

        // Ball goes past the top paddle (Game Over)
        if (ball.y <= 1) {
            reset_ball_B();
        }

        // Slow down ball movement
        pthread_mutex_unlock(&game_state_mutex);
        usleep(80000); 
    }
    return NULL;
}

void update_paddle_A(int ch) {
    pthread_mutex_lock(&game_state_mutex);
    if (ch == KEY_LEFT && paddle_A.x > 2) {
        paddle_A.x--;  // Move paddle left
    }
    if (ch == KEY_RIGHT && paddle_A.x < WIDTH - paddle_A.width - 2) {
        paddle_A.x++;  // Move paddle right
    }
    pthread_mutex_unlock(&game_state_mutex);
    
}
void update_paddle_B(int ch) {
    pthread_mutex_lock(&game_state_mutex);
    if (ch == KEY_LEFT && paddle_B.x > 2) {
        paddle_B.x--;  // Move paddle left
    }
    if (ch == KEY_RIGHT && paddle_B.x < WIDTH - paddle_B.width - 2) {
        paddle_B.x++;  // Move paddle right
    }
    pthread_mutex_unlock(&game_state_mutex);
    
}

void reset_ball_A() {
    ball.x = OFFSETX + WIDTH / 2;
    ball.y = OFFSETY + HEIGHT / 2;
    ball.dx = 1;
    ball.dy = 1;
    score_B++ ; 
    usleep(1000000);
}

void reset_ball_B() {
    ball.x = OFFSETX + WIDTH / 2;
    ball.y = OFFSETY + HEIGHT / 2;
    ball.dx = 1;
    ball.dy = 1;
    score_A++ ; 
    usleep(1000000);
}
