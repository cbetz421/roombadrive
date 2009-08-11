#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <string.h>
#include <arpa/inet.h>

#include <glib.h>

#include "roomba.h"



/* Open the serial port, set the params, etc */

/*
 * 
 * 
 * speed 57600 baud; line = 0;
intr = <undef>; quit = <undef>; erase = <undef>; kill = <undef>; eof = <undef>; start = <undef>; stop = <undef>; susp = <undef>;
rprnt = <undef>; werase = <undef>; lnext = <undef>; flush = <undef>; min = 0; time = 0;
-brkint -icrnl -imaxbel
-opost -onlcr
-isig -icanon -iexten -echo -echoe -echok -echoctl -echoke

*/

#define SPEED B57600
//#define SPEED B9600

int serial_open(char *device) {
	int fd;
	static struct termios newtio;
	int status;
	
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		return -1;
	}

	tcgetattr(fd, &newtio);
	


	newtio.c_cflag = SPEED | CS8 | PARENB | CLOCAL | CREAD;

	cfsetispeed(&newtio, SPEED);
	cfsetospeed(&newtio, SPEED);


	/*
	IGNPAR : ignore bytes with parity errors
	otherwise make device raw (no other input processing)
	*/
	newtio.c_iflag = IGNPAR;

	/*
	Raw output.
	*/
	newtio.c_oflag = 0;

	/*
	set input mode (non-canonical, no echo ... )
	*/

	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 0;  /* no blocking */

	newtio.c_cflag &= ~PARENB;

	tcflush(fd, TCIFLUSH);
	if(tcsetattr(fd,TCSANOW,&newtio) < 0) {
		exit(1);
		return -1;	
	}

	/* make sure DTR & RTS outputs are ON */    
	ioctl(fd, TIOCMGET, &status);
	status |= (TIOCM_DTR | TIOCM_RTS);
	ioctl(fd, TIOCMSET, &status);
	
	usleep(20000);
	return fd;	
}

int serial_close(int fd) {
	return close(fd);	
}


int serial_write(int fd, char *data, int len) {
	int i;
	int res;
	fprintf(stderr, "TX {%d} => ", len);
	for(i=0;i<len;i++) {
		fprintf(stderr, "[%02x:%03d] ", (unsigned char)data[i], (unsigned char)data[i]);	
	}
	res = write(fd, data, len);
	fprintf(stderr, "= {%d}\n", res);
	return res;
}

int roomba_wheels(int fd, short left, short right) {
	char data[5];
	data[0] = 145;
	data[1] = ((char *)&left)[1];
	data[2] = ((char *)&left)[0];
	data[3] = ((char *)&right)[1];
	data[4] = ((char *)&right)[0];
	return serial_write(fd, data, 5);
}

int roomba_stop(int fd) {
	return roomba_wheels(fd, 0, 0);
}

int roomba_start_communication(int fd) {
	char data[1];
	int res;
	data[0] = 128;
	res = serial_write(fd, data, 1);
	usleep(20000);
	return res;
}

int roomba_init_safe(int fd) {
	char data[1];
	int res;
	data[0] = 131;
	res = serial_write(fd, data, 1);
	usleep(20000);
	return res;
}


_sensor_data* roomba_get_sensor_data(int fd, _sensor_data *sensor_data) {
	unsigned char data[2];
	int res;
	data[0] = 142;
	data[1] = 6; /*This is all packets*/
	res = serial_write(fd, (char *)data, 2);
	usleep(40000);
	res = read(fd, sensor_data, 52);
	if(res == 52) {
		sensor_data->distance = htons(sensor_data->distance);
		sensor_data->angle = htons(sensor_data->angle);
		sensor_data->battery_charge = htons(sensor_data->battery_charge);

		sensor_data->battery_capacity = htons(sensor_data->battery_capacity);
		sensor_data->wall_signal = htons(sensor_data->wall_signal);
		sensor_data->cliff_left_signal = htons(sensor_data->cliff_left_signal);
		sensor_data->cliff_front_left_signal = htons(sensor_data->cliff_front_left_signal);
		sensor_data->cliff_font_right_signal = htons(sensor_data->cliff_font_right_signal);
		sensor_data->cliff_right_signal = htons(sensor_data->cliff_right_signal);
		sensor_data->user_analog_input = htons(sensor_data->user_analog_input);
		sensor_data->velocity = htons(sensor_data->velocity);
		sensor_data->radius = htons(sensor_data->radius);
		sensor_data->right_velocity = htons(sensor_data->right_velocity);
		sensor_data->left_velocity = htons(sensor_data->left_velocity);
		
		return sensor_data;
	}
	
	return NULL;
}

int roomba_start_stream(int fd) {
	char data[6];
	data[0] = 148;
	data[1] = 4;
	data[2] = 7; //bumps and wheel drop [1]
	data[3] = 8; //wall [1]
	data[4] = 19; //distance [2]
	data[5] = 26; //battery capacity [2]
	return serial_write(fd, data, 6);
}

int roomba_stop_stream(int fd) {
	char data[2];
	data[0] = 150;
	data[1] = 0;
	return serial_write(fd, data, 2);
}

int roomba_read_stream(int fd, _sensor_data *sensor_data) {
	static GString *string = NULL;
	char buf[512];
	int res;
	int didit= FALSE;
	
	if(string == NULL) {
		string = g_string_new(NULL);	
	}
	
	res = read(fd, buf, 512);
	if(res > 0) {
		string = g_string_append_len(string, buf, res);	
	}
	
	while(string->len >= 13) { //[19] + [len] + [pakcet id 1] + [packet data] ... [checksum]
		if(string->str[0] == 19) {
			short tmp;
			sensor_data->bumps_and_wheel_drops = string->str[3];
			sensor_data->wall = string->str[5];
			memcpy(&tmp, string->str+7, 2);
			sensor_data->distance = htons(tmp);
			memcpy(&tmp, string->str+10, 2);
			sensor_data->battery_capacity = htons(tmp);
			didit = TRUE;
			g_string_erase(string, 0, 13);
		} else {
			//All packets need to start with 19
			g_string_erase(string, 0, 1);	
		}
	}
	
	return didit;
}

