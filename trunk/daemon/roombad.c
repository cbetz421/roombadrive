#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>

#include <glib.h>

#include "roomba.h"

#define SERVER_PORT 4000
#define MAX_CLIENTS 4
#define MAXSTRLEN 1024

struct _client {
	int fd;
	time_t connect_time;
	GString *data;
} PACKSTRUCT;

struct _client clients[MAX_CLIENTS];

int serial_fd = 0;
_sensor_data sensor_data;

static int socket_init() {
	struct sockaddr_in server_addr;
	int server_fd = 0;
	int true = 1;
	int i;
	
	if(server_fd > 0)
		return server_fd;
		
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd < 0) {
		fprintf(stderr, "roombad: CAN NOT OPEN SOCKET\n");
		return -1;
	}
  
	if(setsockopt(server_fd , SOL_SOCKET, SO_REUSEADDR, (void *) &true, sizeof(true)) == -1) {
		fprintf(stderr, "roombad: CAN NOT OPEN SET REUSE ADDRESS?\n");
		shutdown(server_fd, SHUT_RDWR);
		close(server_fd);
		return -1;
	}
	
	/* bind server port */
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	if(bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) <0 ) {
		perror(NULL);
		fprintf(stderr, "roombad: CAN NOT BIND TO PORT!\n");
		shutdown(server_fd, SHUT_RDWR);
		close(server_fd);
		return -1;
	}
	
	if(fcntl(server_fd, F_SETFL, FNDELAY) < 0) {
		shutdown(server_fd, SHUT_RDWR);
		close(server_fd);
		fprintf(stderr, "roombad: ERROR SETTING SOCKET TO NON BLOCKING\n");
		return -1 ;
	}

	if(fcntl(server_fd, F_SETFL, FNDELAY) < 0) {
		shutdown(server_fd, SHUT_RDWR);
		close(server_fd);
		fprintf(stderr, "roombad: ERROR SETTING SOCKET TO NON BLOCKING\n");
		return -1;
	}	
	
	listen(server_fd,9);
	fprintf(stderr, "roombad: SUCCESSFULLY LISTENING ON PORT %d\n", SERVER_PORT);
	
	for(i=0;i<MAX_CLIENTS;i++) {
		clients[i].fd = -1;
		clients[i].data = NULL;
	}
	
	return server_fd;
}

static int client_close(int client) {
	if(clients[client].fd > 0) {
		fprintf(stderr, "roombad: GOODBYE CLIENT %d\n", client);
		shutdown(clients[client].fd, SHUT_RDWR);
		close(clients[client].fd);
		clients[client].fd = -1;
		g_string_free(clients[client].data, TRUE);
	}
	return TRUE;
}

static int client_getter(int server_fd) {
	struct sockaddr_in client_addr;
	unsigned int tmp = 0;
	int current_client = -1, i;
	
	bzero(&client_addr, sizeof(client_addr));
	
	if(server_fd == -1)
		return FALSE;

	for(i=0;i<MAX_CLIENTS;i++) {
		if(clients[i].fd < 0) {
			current_client = i;
			break;
		}
	}

	if(current_client == -1)
		return TRUE;

	clients[current_client].fd = accept(server_fd, (struct sockaddr *) &client_addr, &tmp);

	if(clients[current_client].fd > 0) {
		if (fcntl(clients[current_client].fd, F_SETFL, FNDELAY) < 0) {
			fprintf(stderr, "roombad: ERROR SETTING CLIENT %d SOCKET TO NON BLOCKING\n", current_client);
			client_close(current_client);
			return FALSE;
		}	

		fprintf(stderr, "roombad: RECEIVED NEW CLIENT %d TO TALK TO\n", current_client);
		time(&clients[current_client].connect_time);
		clients[current_client].data = g_string_new(NULL);
		return TRUE;
	}

	clients[current_client].fd = -1;
	return FALSE;
}

static int client_monitor() {
	int res; 
	char buf[MAXSTRLEN];
	int i,j;
	
	for(i=0;i<MAX_CLIENTS;i++) {
		if(clients[i].fd > 0) {

			res = recv(clients[i].fd,buf,MAXSTRLEN-1,0);

			if(res == 0) {
				fprintf(stderr, "roombad: CLIENT %d DISCONNECTED\n", i);
				client_close(i);
				continue;
			}

			if(res > 0) {
				buf[res] = 0;

				fprintf(stderr, "FROM SOCKET {%d} => ", res);
				for(j=0;j<res;j++) {
					fprintf(stderr, "[%02x:%03d] ", (unsigned char)buf[j], (unsigned char)buf[j]);	
				}
				fprintf(stderr, "\n");

				g_string_append_len(clients[i].data, buf, res);
				continue;
			}
		}
	}

	return FALSE;
}

static int send_client(char *data) {
	int i;
	
	for(i=0;i<MAX_CLIENTS;i++) {
		if(clients[i].fd > 0) {
			write(clients[i].fd, data, strlen(data));
		}
	}
	return TRUE;
}


void drive_wheels(int left, int right) {
	char buf[1024];
	roomba_wheels(serial_fd, left, right);
	sprintf(buf, "W|%05d|%05d\n", left, right);
	send_client(buf);
}

static int client_responder() {
	int i;
	
	for(i=0;i<MAX_CLIENTS;i++) {
		if(clients[i].fd > 0) {
			if((gint)clients[i].data->len > 0) {
				GString *string = clients[i].data;
				if(string->str[0] == 'F' && string->len >= 2) {
					char data[2];
					data[0] = string->str[1];
					data[1] = 0;
					fprintf(stderr, "roombad: CMD: F [%s]\n", data);
					g_string_erase(string, 0, 2);
					drive_wheels(50*atoi(data), 50*atoi(data));
				} else if(string->str[0] == 'W' && string->len >= 5) {
					short l,r;
					memcpy(&l, string->str+1, 2);
					memcpy(&r, string->str+3, 2);
					fprintf(stderr, "roombad: CMD: W [%d][%d]\n", l, r);
					g_string_erase(string, 0, 5);
					drive_wheels(l, r);
				} else if(string->str[0] == 'S' && string->len >= 1) {
					fprintf(stderr, "roombad: CMD: S\n");
					g_string_erase(string, 0, 1);
					drive_wheels(0, 0);
				} else if(string->str[0] == '\n' || string->str[0] == '\r') {
					g_string_erase(string, 0, 1);	
				} else {
					fprintf(stderr, "UNKNOWN COMMAND!!!! [%c] LEN =%d\n", string->str[0], string->len);
					g_string_erase(string, 0, 1);	
				}
			}

		}
	}
	return FALSE;
}


static int client_send_stream() {
	char buf[1024];

	if(sensor_data.bumps_and_wheel_drops != 0 || sensor_data.wall != 0 || sensor_data.distance != 0) {
		sprintf(buf, "S|%02d|%02d|%04d|%04d\n", sensor_data.bumps_and_wheel_drops, sensor_data.wall,
			sensor_data.distance, sensor_data.battery_capacity);
		send_client(buf);
	}
	return TRUE;
}

static void pipes_are_yummy (int signum, siginfo_t *si, void *aptr) {}

int main(int argc, char **argv) {
	int socket_fd = socket_init();
	struct sigaction sa;
	bzero(&sa, sizeof(struct sigaction));
	sa.sa_handler = (void(*)(int))pipes_are_yummy;
	sigaction (SIGPIPE, &sa, NULL);

	if(argc != 2) {
		fprintf(stderr, "roombad: Serial port as first param!\n");
		exit(1);	
	}
	
	serial_fd = serial_open(argv[1]);

	if(serial_fd <= 0) {
		fprintf(stderr, "roombad: NO roomba!\n");
		exit(1);	
	}

	if(socket_fd <= 0) {
		fprintf(stderr, "roombad: NO socket!\n");
		exit(1);	
	}

	roomba_start_communication(serial_fd);
	roomba_init_safe(serial_fd);
	roomba_get_sensor_data(serial_fd, &sensor_data);
	roomba_start_stream(serial_fd);
	
	while(1) {	
		int didit;
		client_getter(socket_fd);
		client_monitor();
		client_responder();
		didit = roomba_read_stream(serial_fd, &sensor_data);
		if(didit == TRUE)
			client_send_stream();
		else
			usleep(20000);
		
	}
	close(socket_fd);
	return 0;
}

