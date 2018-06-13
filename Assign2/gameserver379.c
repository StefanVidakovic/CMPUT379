#include "gameserver379.h"

#define MIN_PORT 1025  // the minimum port allowed
#define MAX_PORT 65535 // the maximum port allowed
int MAX_CLIENTS; //the max players, calculated based on board size
extern int errno; //for comprehensive error messages
#define DOWN 0
#define RIGHT 1
#define UP 2
#define LEFT 3


int overlord_flag = 0; //synchronization flag between game overlord and client threads
//lists of threads and sockets for graceful_exit
int* threads;
int* sockets;
int* clients;
pthread_t overlord_thread;

struct Player_Input{ //This structure is poorly named, but holds the state of a player
	int sprite_id;
	int x;
	int y;
	int facing;
	int new;
	int fire;
	int alive;
	int kill_count;
};

struct Client_Create {//structure used for passing information to client threads
	int sock_fd;
	pthread_t thread_id;
	int sprite_id;
	int* client_count;
	struct Player_Input** move_requests; //list of pointers to move requests
	struct Player_Input** game_state; //the validated game state
	int dimension;
	int rand_seed;
};

struct Overlord_Create {//structure for passing info to overlord thread
	struct Player_Input** move_requests;
	struct Player_Input** game_state;
	int* client_count;
	int dimension;
	int rand_seed;
	int usecs;
};


pthread_mutex_t move_requests_mutex;
pthread_mutex_t count_mutex;
pthread_mutex_t client_info_mutex;

void graceful_exit(){
	pthread_cancel(overlord_thread);
	shutdown(sockets[0],SHUT_RDWR);
	close(sockets[0]);
	for(int i = 0;i<*clients;i++){
		pthread_cancel(threads[i]);
		shutdown(sockets[i],SHUT_RDWR);
		close(sockets[i]);
	}
	exit(EXIT_FAILURE);
}


int sig_handler(int signal) {
	if (signal == SIGINT|signal == SIGTERM) {
		graceful_exit();
	}
}

void daemonization(){
	pid_t pid;
	pid = fork();
	if(pid<0){
		exit(EXIT_FAILURE);
	}
	if(pid>0){
		exit(EXIT_SUCCESS);
	}
	if (setsid() < 0){
		exit(EXIT_FAILURE);
	}
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	pid = fork();
	if(pid<0){
		exit(EXIT_FAILURE);
	}
	if(pid>0){
		exit(EXIT_SUCCESS);
	}
	umask(0);
}

int input_check(int dimension, float update_period, int port, int pseudo_rand_seed) {
	if (dimension < 4) { // check that the dimension is not less than 1
		printf("Dimension was less than 4, \nPlease enter a playable dimension and try again.\n");
		return -1;
	}
	if (update_period <= 0) { // check that the update_period is greater than 0
		printf("Update period was not greater than 0, \nPlease check your Update period and try again.\n");
		return -1;
	}
	if (!((port >= MIN_PORT) && (port <= MAX_PORT))) { // Check that the port is in range
		printf("Port was either less than ", MIN_PORT, " or greater than ", MAX_PORT, "\nPlease check your port and try again.\n");
		return -1;
	}
	if (pseudo_rand_seed < 1) { // check that the pseudo_rand_seed is not less than 1
		printf("Seed for the pseudo random number generator was less than 1, \nPlease check your seed and try again.\n");
		return -1;
	}
}


int out_of_bounds(struct Player_Input* move,int dimension){
	int out = 0;
	if(move->x<=0){
		move->x += 1;
		out = 1;
	}
	if(move->x>=dimension){
		move->x -= 1;
		out = 1;
	}
	if(move->y<=0){
		move->y += 1;
		out = 1;
	}
	if(move->y>=dimension){
		move->y -= 1;
		out = 1;
	}
	return out;
}


int collision(struct Player_Input** move_requests,int i,int client_count){
	int collision = 0;
	if(client_count==1){
		return collision;
	}
	int x1 = move_requests[i]->x;
	int x2 = 0;
	int y1 = move_requests[i]->y;
	int y2 = 0;
		for(int j = 0;j<client_count;j++){
			if(i==j) continue;
			if(move_requests[j]==NULL) continue;

			x2 = move_requests[j]->x;
			y2 = move_requests[j]->y;
			if((x1==x2)&(y1==y2)){
				collision = 1;
			}
		}
		return collision;
}


int client_collision(struct Player_Input** game_state,struct Player_Input* move,int i,int client_count){
	int collision = 0;
	if(client_count==1){
		return collision;
	}
	int x1 = move->x;
	int x2 = 0;
	int y1 = move->y;
	int y2 = 0;
		for(int j = 0;j<client_count;j++){
			if(i==j) continue;
			if(game_state[j]==NULL) continue;

			x2 = game_state[j]->x;
			y2 = game_state[j]->y;
			if((x1==x2)&(y1==y2)){
				collision = 1;
			}
		}
		return collision;
}


int shot(struct Player_Input** move_requests,struct Player_Input** game_state,int i,int client_count){
	int dead = -1;
	int bulletx1;
	int bullety1;
	int bulletx2;
	int bullety2;
	for(int j = 0;j<client_count;j++){
		if(j==i) continue;
		if(move_requests[j]==NULL) continue;

		bulletx1 = move_requests[j]->x;
		bullety1 = move_requests[j]->y;
		bulletx2 = bulletx1;
		bullety2 = bullety1;

		if(move_requests[j]->fire==1){

			if(move_requests[j]->facing==DOWN){
				bullety1 = bullety1+1;
				bullety2 = bullety2+2;
			}

			if(move_requests[j]->facing==RIGHT){
				bulletx1 = bulletx1+1;
				bulletx2 = bulletx2+2;
			}

			if(move_requests[j]->facing==UP){
				bullety1 = bullety1-1;
				bullety2 = bullety2-2;
			}

			if(move_requests[j]->facing==LEFT){
				bulletx1 = bulletx1-1;
				bulletx2 = bulletx2-2;
			}

			if(move_requests[i]->x==bulletx1&move_requests[i]->y==bullety1){
				return j;
			}

			if(move_requests[i]->x==bulletx2&move_requests[i]->y==bullety2){
				return j;
			}
		}
	}
	return dead;
}


void player_init(struct Player_Input* move,int seed,int dimension){
	srand(seed);
	move->x = rand()%dimension;
	move->y = rand()%dimension;
}


void encode_game_state(char* out_buf,struct Player_Input** game_state){
	for(int i = 0;i<MAX_CLIENTS;i++){
		if(game_state[i]==NULL){
			out_buf[i*5] = 'X';
		 	out_buf[i*5+1] = 'X';
			out_buf[i*5+2] = 'X';
			out_buf[i*5+3] = 'X';
			out_buf[i*5+4] = 'X';
			continue;
		}
		out_buf[i*5] = (char)game_state[i]->sprite_id;
		out_buf[i*5+1] = (char)game_state[i]->x;
		out_buf[i*5+2] = (char)game_state[i]->y;
		out_buf[i*5+3] = (char)game_state[i]->facing;
		out_buf[i*5+4] = (char)game_state[i]->fire;
	}
	return;
}



void decode_input(struct Player_Input* move,char* buf){
	if(buf[0]=='X'){
		move->x = move->x;
		move->y = move->y;
		move->fire = 0;
	}

	if(buf[0]=='j'){
		move->x = move->x-1;
		move->facing = LEFT;
		move->fire = 0;
	}

	if(buf[0]=='l'){
		move->x = move->x+1;
		move->facing = RIGHT;
		move->fire = 0;
	}

	if(buf[0]=='k'){
		move->y = move->y+1;
		move->facing = DOWN;
		move->fire = 0;
	}

	if(buf[0]=='i'){
		move->y = move->y-1;
		move->facing = UP;
		move->fire = 0;
	}

	if(buf[0]==' '){
		move->fire = 1;
	}
}


void copy_Player_Input(struct Player_Input* move,struct Player_Input* validated_move){
	validated_move->x = move->x;
	validated_move->y = move->y;
	validated_move->facing = move->facing;
	validated_move->new = move->new;
	validated_move->fire = move->fire;
	validated_move->alive = move->alive;
	validated_move->kill_count = move->kill_count;
}


void* player_thread(void* arg){

	pthread_mutex_lock(&client_info_mutex);
	struct Client_Create* player = (struct Client_Create *) arg; //accept input structure
	int sprite_id = player->sprite_id; //create thread local variables before struct is overwritten
	int* client_count = player->client_count;
	int seed = player->rand_seed;

	int player_socket = player->sock_fd;

	fcntl(player_socket,F_SETFL,O_NONBLOCK);

	pthread_t thread_id = player->thread_id;
	pthread_detach(thread_id);

	int dimension = player->dimension;

	struct Player_Input** move_requests = player->move_requests;
	struct Player_Input** game_state = player->game_state;
	pthread_mutex_unlock(&client_info_mutex);

	char buffer[2] = {(char)sprite_id,(char)dimension};
	send(player_socket,(void*)buffer,sizeof(buffer),MSG_NOSIGNAL);//initialization communication
	if(errno==EPIPE) {
		shutdown(player_socket,SHUT_RDWR);
		close(player_socket);
		pthread_mutex_lock(&count_mutex);
		*client_count = *client_count-1;
		pthread_mutex_unlock(&count_mutex);
		pthread_exit(NULL);
	}

	char in_buf[1];

	struct Player_Input move;
	move.sprite_id = sprite_id;
	move.x = 0;
	move.y = 0;
	move.facing = UP;
	move.new = 1;
	move.fire = 0;
	move.alive = 1;
	move.kill_count = 0;

	struct Player_Input validated_move;
	copy_Player_Input(&move,&validated_move);

	int collides = 1;
	while(collides){
		player_init(&move,seed,dimension); //For genuinely random init if done later.
		collides = client_collision(game_state,&move,sprite_id,*client_count);
		if(collides) continue;
	}

	move.new=0; //if we initialized then tell overlord player is no longer new
	int inital_write = 0;

	pthread_mutex_lock(&move_requests_mutex);
	game_state[sprite_id] = &validated_move;
	move_requests[sprite_id] = &move;
	inital_write = 1;
	pthread_mutex_unlock(&move_requests_mutex);


	unsigned int usecs = 300;
	int write_flag = 0;
	int send_flag = 0;
	char out_buf[MAX_CLIENTS*5];

	while(1){
		if(overlord_flag==1&inital_write==1){

			if(recv(player_socket,in_buf,sizeof(in_buf),0)<=0){
				if(errno == EPIPE|errno==ECONNRESET) break;
				write_flag = 1;
				move_requests[sprite_id]->fire = 0;
			}

			if(write_flag==0){
				pthread_mutex_lock(&move_requests_mutex);
				decode_input(&move,in_buf);
				move_requests[sprite_id] = &move;
				write_flag = 1;
				pthread_mutex_unlock(&move_requests_mutex);
			}

			if(game_state[sprite_id]!=NULL){

				write_flag = 0;
				send_flag = 1;

				if(game_state[sprite_id]->alive==0){
					char buf[2] = {(char)game_state[sprite_id]->sprite_id,(char)game_state[sprite_id]->kill_count};
					send(player_socket,(void*)buf,sizeof(buf),MSG_NOSIGNAL);
					break;
				}

				encode_game_state(out_buf,game_state);
				send(player_socket,(void*)out_buf,sizeof(out_buf),MSG_NOSIGNAL);
				overlord_flag = 0; //this ensures writing and reading only happen once per interval

				if(errno==EPIPE|errno==ECONNRESET) break;
			}
		}
	}
	// essentially a specialized graceful_exit
	shutdown(player_socket,SHUT_RDWR);
	close(player_socket);
	pthread_mutex_lock(&move_requests_mutex);
	move_requests[sprite_id] = NULL;
	game_state[sprite_id] = NULL;
	pthread_mutex_unlock(&move_requests_mutex);

	pthread_exit(NULL);
}


void* game_overlord(void* arg){
	struct Overlord_Create* overlord = (struct Overlord_Create*) arg;

	struct Player_Input** game_state = overlord->game_state;
	int* client_count = overlord->client_count;
	int dimension = overlord->dimension;
	int seed = overlord->rand_seed;
	struct Player_Input** move_requests = overlord->move_requests;

	int write_flag = 1;
	unsigned int usecs = overlord->usecs;

	while(1){

		overlord_flag = 0;
		pthread_mutex_lock(&move_requests_mutex);

		for(int i = 0;i<*client_count;i++){
			write_flag = 1;

			if(move_requests[i]==NULL){
				write_flag = 0;
				continue;
			}

			int out = 0;
			out = out_of_bounds(move_requests[i],dimension-1);
			if(out){ //if out of bounds don't update game state and reset the move
				write_flag = 0;
				continue;
			}

			int collides = 0;
			collides = collision(move_requests,i,*client_count);
			if(collides){ //if theres a collision then don't write to game state
				copy_Player_Input(game_state[i],move_requests[i]); //reset the move_request
				write_flag = 0;
				continue;
			}

			int shooter_index = shot(move_requests,game_state,i,*client_count);
			if(shooter_index>=0){ //kill the guy that got shot
				move_requests[i]->alive = 0;
				game_state[i]->alive = 0;
				move_requests[shooter_index]->kill_count = move_requests[shooter_index]->kill_count+1;
				game_state[shooter_index]->kill_count = move_requests[shooter_index]->kill_count+1;
			}

			if(write_flag>0){//if no collisions or out of bounds, update the game_state for this player
				copy_Player_Input(move_requests[i],game_state[i]);
			}
		}

		overlord_flag = 1; //tell the clients they can read/write
		pthread_mutex_unlock(&move_requests_mutex); //allow writing
		usleep(usecs); //sleep for a period
	}
}


int main(int argc, char **argv) {
	if (argc != 5) { // check that there are 4 arguments
		printf("Wrong number of arguments given, please enter \n");
		printf("the dimension of the square grid \na update period in seconds \n");
		printf("the port on which the server should be listening \n");
		printf("and a seed for the pseudo random number generator.\n");
		return -1;
	}

	int dimension = atoi(argv[1]);
	float update_period = atof(argv[2]);
	int usecs = 1000000*update_period; //conversion to microseconds
	int port = atoi(argv[3]);
	int pseudo_rand_seed = atoi(argv[4]);
	if(!input_check(dimension,update_period,port,pseudo_rand_seed)){
		return -1;
	}

	daemonization();

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0); //socket initialization
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int addr_len = sizeof(serverAddr);

	if(bind(sock_fd, (struct sockaddr *) &serverAddr, sizeof(serverAddr))==-1){
		perror("Bind Error: ");
	}

	MAX_CLIENTS = dimension/2;//setting the max number of players

	pthread_t temp[MAX_CLIENTS];//declaring thread list
	threads = temp;

	struct Player_Input* move_requests[MAX_CLIENTS];//move list initialization
	memset(&move_requests,NULL,MAX_CLIENTS*sizeof(struct Player_Input*));

	struct Player_Input* game_state[MAX_CLIENTS];//game list initialization
	memset(&game_state,NULL,MAX_CLIENTS*sizeof(struct Player_Input*));

	pthread_mutex_init(&move_requests_mutex,NULL);//mutex initialization
	pthread_mutex_init(&count_mutex,NULL);
	pthread_mutex_init(&client_info_mutex,NULL);

	int count = 0;
	clients = &count;
	int temp_sockets[MAX_CLIENTS];
	sockets = temp_sockets;
	sockets[0] = sock_fd;

	struct Overlord_Create overlord_create;
	overlord_create.move_requests = move_requests;//creation of overlord thread
	overlord_create.game_state = game_state;
	overlord_create.client_count = &count;
	overlord_create.dimension = dimension;
	overlord_create.rand_seed = pseudo_rand_seed;
	overlord_create.usecs = usecs;
	pthread_create(&overlord_thread,NULL,game_overlord,(void*)&overlord_create);

	struct Client_Create client_create; //pre-loop initialization
	client_create.move_requests = move_requests; //point client to list where it will write its moves
	client_create.game_state = game_state;
	client_create.dimension = dimension;
	client_create.client_count = &count; //give it a server specific ID
	client_create.rand_seed = pseudo_rand_seed;

	struct sigaction interrupt;
	interrupt.sa_handler = &sig_handler;
	sigaddset(&interrupt.sa_mask, SIGINT);  // Add SIGINT to the sigaction mask
	sigaddset(&interrupt.sa_mask,SIGTERM);
	sigaction(SIGINT, &interrupt, NULL);

	fd_set readSet;

	while(count<MAX_CLIENTS){
		FD_ZERO(&readSet);
		FD_SET(sock_fd,&readSet);

		if(!listen(sock_fd, MAX_CLIENTS) == 0){
			perror("Listen Error: ");
		}
		if(select(sock_fd+1,&readSet,NULL,NULL,NULL)<=0){
			perror("Select Error: ");
			continue;	//wait for connect through socket
		}

		int clientsock_id = accept(sock_fd,(struct sockaddr*)&serverAddr, &addr_len);//connect to client
		if(clientsock_id==-1){
			perror("Accept Error: "); //faulty connection, try again
			continue;
		}
		sockets[count+1] = clientsock_id;
		usleep(50);

		client_create.sock_fd = clientsock_id; //give the new client its own socket
		for(int i = 0;i<MAX_CLIENTS;i++){
			if(move_requests[i]==NULL){
				client_create.thread_id = threads[i];
				client_create.sprite_id = i;
				break;
			}
		}
		pthread_create(&threads[count],NULL,player_thread,(void*) &client_create); //create client thread

		pthread_mutex_lock(&count_mutex);
		count++; //increment count for next thread id and sprite_id
		pthread_mutex_unlock(&count_mutex);
	}

	graceful_exit();
}
