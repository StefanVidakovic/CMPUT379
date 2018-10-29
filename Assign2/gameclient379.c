#include "gameclient379.h"

#define MIN_PORT 1025  // the minimum port allowed
#define MAX_PORT 65535 // the maximum port allowed
int MAX_CLIENTS;

struct Input {
  int sock_fd;
  char* input;
};

int server_sock;
pthread_t thread;
pthread_mutex_t input_mutex;
char input[1];
WINDOW* window;

void graceful_exit(){
  pthread_cancel(thread);
  shutdown(server_sock,SHUT_RDWR);
  close(server_sock);
  // curs_set(1);
  // clear();
  werase(window);
  wrefresh(window);
  delwin(window);
  endwin();
  exit(EXIT_FAILURE);
}

int sig_handler(int signal,int fd,char* buffer,int count) {
  if (signal == SIGINT|signal == SIGTERM) {
		graceful_exit();
	}
}

void* user_input(void* arg){
  struct Input* in = (struct Input*) arg;
  // char* input = in->input;
  int ch1;
  nodelay(stdscr, TRUE);
  while(1) {
    ch1 = getch();
    if (ch1 == ERR) {
      continue;
    }
    pthread_mutex_lock(&input_mutex);
    input[0] = (char)ch1;
    pthread_mutex_unlock(&input_mutex);
    // printf("user input: %c\n",input[0]);
  }
}

void print_boundary(int dimension,WINDOW* win){
  for(int i = 0;i<dimension;i++){
    wmove(win,i,0);
    waddch(win,'-');
    wmove(win,i,dimension-1);
    waddch(win,'-');
  }
  for(int j = 0;j<dimension;j++){
    wmove(win,0,j);
    waddch(win,'-');
    wmove(win,dimension-1,j);
    waddch(win,'-');
  }
  wrefresh(win);
}

void decode_and_print(char* buf,WINDOW* win,int sprite_id){
  int bold = 0;
  // printf("SPRITE X: %c", buf[1]);
  // printf("SPRITE Y: %d\n", (int)buf[2]);
  for(int i = 0;i<(5*MAX_CLIENTS);i+=5){

    if(buf[i]=='X') {
      // printf("SPRITE ID%c\n",buf[32]);
      // wrefresh(win);
      continue;
    }
    bold = 0;
    if(buf[i]==sprite_id){
      bold = 1;
    }

    wmove(win,(int)buf[i+2],(int)buf[i+1]);
    if((int)buf[i+3]==0){

      if(bold) {
        waddch(win,'v'|A_BOLD);
        wrefresh(win);
      }
      else{
        waddch(win,'v');
        wrefresh(win);
      }

      if((int)buf[i+4]==1){
        wmove(win,(int)buf[i+2]+1,(int)buf[i+1]);
        if(bold) {
          waddch(win,'o'|A_BOLD);
          wrefresh(win);
        }
        else {
          waddch(win,'o');
          wrefresh(win);
        }

        wmove(win,(int)buf[i+2]+2,(int)buf[i+1]);
        if(bold) {
          waddch(win,'o'|A_BOLD);
          wrefresh(win);
        }
        else{
         waddch(win,'o');
         wrefresh(win);
       }
      }
      continue;
    }

    if((int)buf[i+3]==1) {
      if(bold) {
        waddch(win,'>'|A_BOLD);
        wrefresh(win);
      }
      else {
      waddch(win,'>');
      wrefresh(win);
      }

      if((int)buf[i+4]==1) {

        wmove(win,(int)buf[i+2],(int)buf[i+1]+1);
        if(bold) {
          waddch(win,'o'|A_BOLD);
          wrefresh(win);
        }
        else {
          waddch(win,'o');
          wrefresh(win);
        }

        wmove(win,(int)buf[i+2],(int)buf[i+1]+2);
        if(bold) {
          waddch(win,'o'|A_BOLD);
          wrefresh(win);
        }
        else {
          waddch(win,'o');
          wrefresh(win);
        }
      }
      continue;
    }

    if((int)buf[i+3]==2){

      if(bold) {
        waddch(win,'^'|A_BOLD);
        wrefresh(win);
      }
      else {
        waddch(win,'^');
        wrefresh(win);
      }

      if((int)buf[i+4]==1){

        wmove(win,(int)buf[i+2]-1,(int)buf[i+1]);
        if(bold){
        waddch(win,'o'|A_BOLD);
        wrefresh(win);
        }
        else {
        waddch(win,'o');
        wrefresh(win);
        }

        wmove(win,(int)buf[i+2]-2,(int)buf[i+1]);
        if(bold) {
          waddch(win,'o'|A_BOLD);
          wrefresh(win);
        }
        else {
        waddch(win,'o');
        }
      }
      continue;
    }

    if((int)buf[i+3]==3){

      if(bold) {
        waddch(win,'<'|A_BOLD);
        wrefresh(win);
      }
      else {
        waddch(win,'<');
        wrefresh(win);
      }

      if((int)buf[i+4]==1){

        wmove(win,(int)buf[i+2],(int)buf[i+1]-1);
        if(bold) {
          waddch(win,'o'|A_BOLD);
          wrefresh(win);
        }
        else {
          waddch(win,'o');
          wrefresh(win);
        }

        wmove(win,(int)buf[i+2],(int)buf[i+1]-2);
        if(bold) {
          waddch(win,'o'|A_BOLD);
          wrefresh(win);
        }
        else {
          waddch(win,'o');
          wrefresh(win);
        }
      }
      continue;
    }
  }
}

void dead(char sprite_id,char kill_count,WINDOW* win,int dimension){
    werase(win);
    wrefresh(win);
    delwin(window);
    endwin();
    printf("You have tragically died. Better luck next time.\n");
    printf("Player ID: %d\n",(int)sprite_id);
    printf("Kill count: %d\n",(int)kill_count);
    printf("Enter q to quit.\n");
    // mvprintw(win,dimension/2+1,dimension/2,str1);
    // wrefresh(win);
    // usleep(8000000);
    char stringbuf[2];

    printf(">");
    fgets(stringbuf, 3, stdin);
    stringbuf[strlen(stringbuf)-1] = '\0';
    printf("%c\n",stringbuf[0]);
    while(stringbuf[0]!='q');
    graceful_exit();

}

int main(int argc, char **argv) {
  if(argc != 3){
    printf("Wrong number of arguments given, please enter \n");
    printf("Please enter the server IP address\n");
    printf("and the port on which the server should be listening \n");
  }
  int port = atoi(argv[2]);
	if (!((port >= MIN_PORT) && (port <= MAX_PORT))) { // Check that the port is in range
		printf("Port was either less than ", MIN_PORT, " or greater than ", MAX_PORT, "\nPlease check your port and try again.\n");
		return -1;
	}

  int sock_fd = socket(AF_INET,SOCK_STREAM,0);
  server_sock = sock_fd;
  fcntl(sock_fd,F_SETFL,O_NONBLOCK);

  struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

  struct sigaction interrupt;
	interrupt.sa_handler = &sig_handler;
	sigaddset(&interrupt.sa_mask, SIGINT);  // Add SIGINT to the sigaction mask
  sigaddset(&interrupt.sa_mask, SIGTERM);
	sigaction(SIGINT, &interrupt, NULL);

	/* Set up the structure to specify user_sig. */
	struct sigaction terminate;
	terminate.sa_handler = &sig_handler;
	sigaddset(&terminate.sa_mask, SIGINT);
  sigaddset(&terminate.sa_mask, SIGTERM);
	sigaction(SIGTERM, &terminate, NULL);

  if(connect(sock_fd, (struct sockaddr*) &serverAddr, sizeof(serverAddr))==-1){
    perror("Connect Error: ");
  }
  if(errno==EPIPE|errno==ECONNRESET){
    printf("The server has disconnected. Exiting.\n");
    graceful_exit();
  }

  fd_set readSet;
  int server_init = 0;

  while(!server_init){
    FD_ZERO(&readSet);
    FD_SET(sock_fd,&readSet);
    server_init = select(sock_fd+1,&readSet,NULL,NULL,NULL);
    printf("server_init: %d\n",server_init);
  }

  char init_buf[2];
  if(recv(sock_fd,(void*)init_buf,2,0)<=0){
    perror("Receive Error: ");
    graceful_exit();
  }

  int sprite_id = init_buf[0];
  int dimension = init_buf[1];
  MAX_CLIENTS = dimension/2;

  printf("Sprite ID: %d\n", sprite_id);
  printf("Dimension: %d\n", dimension);

  initscr();
  cbreak();
  noecho();
  curs_set(0);
  WINDOW* win = newwin(dimension,dimension,0,0);
  wrefresh(win);
  window = win;

  // char input[1];
  memset(&input,NULL,sizeof(char));
  pthread_mutex_init(&input_mutex,NULL);

  struct Input input_struct;
  input_struct.sock_fd = sock_fd;
  // input_struct.input = input;
  struct Input* inputptr = &input_struct;

  pthread_t thread_id;
  thread = thread_id;
  pthread_create(&thread_id,NULL,user_input,(void*)&input_struct);

  int server_game_state = 0;
  char game_state_buf[5*MAX_CLIENTS];
  int nbytes;
  FD_ZERO(&readSet);
  FD_SET(sock_fd,&readSet);

  while(1){

    while(server_game_state==0){
      FD_ZERO(&readSet);
      FD_SET(sock_fd,&readSet);
      if(select(sock_fd+1,&readSet,NULL,NULL,NULL)<=0){
        if(errno==EPIPE|errno==ECONNRESET) graceful_exit();
        perror("Select Error: ");
        continue;
      }
      break;
    }

    nbytes = recv(sock_fd,(void*)game_state_buf,sizeof(game_state_buf),0);
    if(errno==EPIPE|errno==ECONNRESET){
      printf("The server has disconnected. Exiting\n");
      graceful_exit();
    }

    if(nbytes==2){
      // while(1) printf("WE GET HERE\n");
      dead(game_state_buf[0],game_state_buf[1],win,dimension);
    }

    werase(win);
    print_boundary(dimension,win);
    decode_and_print(game_state_buf,win,sprite_id);


    if(input[0]=='X'){
      continue;
    }

    pthread_mutex_lock(&input_mutex);
    if(input[0]==NULL){
      input[0] = 'X';
    }

    if(input[0]=='q') graceful_exit();

    send(sock_fd,(void*)input,sizeof(input),MSG_NOSIGNAL);
    if(errno==EPIPE|errno==ECONNRESET){
      printf("Server has disconnected. Exiting\n");
      graceful_exit();
    }
    input[0] = 'X';
    pthread_mutex_unlock(&input_mutex);

    wrefresh(win);
  }
  graceful_exit();
}
