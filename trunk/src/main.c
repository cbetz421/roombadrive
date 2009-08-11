#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <curses.h>

#include "roomba.h"

int main(int argc, char **argv) {
	int fd, res;
	char data[20];
	//int i;	
	_sensor_data sensor_data;
	
	fd = serial_open(argv[1]);
	if(fd <= 0) {
		printf("NO roomba!\n");
		exit(1);	
	}

	roomba_start_communication(fd);
	roomba_init_safe(fd);
	
	#define WIDTH 30
	#define HEIGHT 10 


	WINDOW *menu_win;
	int c;


	initscr();
	clear();
	noecho();
	cbreak();    
	int speed = 1;
	menu_win = newwin(HEIGHT, WIDTH, (24-HEIGHT)/2, (80-WIDTH)/2);
	nodelay(menu_win, TRUE);
	keypad(menu_win, TRUE);
	mvprintw(0, 0, "DRIVE THE ROBOT IF YOU DARE!");
	refresh();

	roomba_get_sensor_data(fd, &sensor_data);
	roomba_start_stream(fd);

	while(1) {    
		int didit;
		
		c = wgetch(menu_win);
		mvprintw(3, 0, "KEY=%d  ", c);
		didit = roomba_read_stream(fd, &sensor_data);
		if(didit) {
			long now = time(NULL);
			mvprintw(1, 0, "TIME=%s", ctime(&now));
			mvprintw(4, 0, "SENSOR DATA  ----------- ");	
			mvprintw(5, 0, " bumps_and_wheel_drops:%03d wall:%03d cliff_left:%03d cliff_front_left:%03d", 
				sensor_data.bumps_and_wheel_drops, sensor_data.wall, sensor_data.cliff_left, sensor_data.cliff_front_left);	
			mvprintw(6, 0, " cliff_front_right:%03d cliff_right:%03d virtual_wall:%03d overcurrents:%03d", 
				sensor_data.cliff_front_right, sensor_data.cliff_right, sensor_data.virtual_wall, sensor_data.overcurrents);
			mvprintw(7, 0, " ir:%03d buttons:%03d distance:%06d angle:%05d charging_state:%03d voltage:%03d", 
				sensor_data.ir, sensor_data.buttons, sensor_data.distance, sensor_data.angle, sensor_data.charging_state,
				sensor_data.voltage);
			mvprintw(8, 0, " current:%03d battery_temperature:%02d battery_charge:%05d, battery_capacity:%05d", 
				sensor_data.current, sensor_data.battery_temperature, sensor_data.battery_charge, sensor_data.battery_capacity);
			mvprintw(9, 0, " wall_signal:%05d cliff_left_signal:%05d cliff_front_left_signal:%05d cliff_font_right_signal:%05d", 
				sensor_data.wall_signal, sensor_data.cliff_left_signal, sensor_data.cliff_front_left_signal, 
				sensor_data.cliff_font_right_signal);
			mvprintw(10, 0, " cliff_font_right_signal:%05d cliff_right_signal:%05d user_digial_input:%02d", 
				sensor_data.cliff_font_right_signal, sensor_data.cliff_right_signal, sensor_data.user_digial_input);
			mvprintw(11, 0, " user_analog_input:%05d charging_source_available:%02d OI_mode:%02d song_number:%02d", 
				sensor_data.user_analog_input, sensor_data.charging_source_available, sensor_data.OI_mode, sensor_data.song_number);
			mvprintw(12, 0, " song_playing:%02d number_stream_packets:%02d velocity:%05d radius:%05d",
				sensor_data.song_playing, sensor_data.number_stream_packets, sensor_data.velocity, sensor_data.radius);
			mvprintw(13, 0, " right_velocity:%05d left_velocity:%05d",
				sensor_data.right_velocity, sensor_data.left_velocity);
		}
		
		switch(c) {    
			case '1':
				speed = 1;
				break;
			case '2':
				speed = 2;
				break;
			case '3':
				speed = 3;
				break;
			case '4':
				speed = 4;
				break;
			case '5':
				speed = 5;
				break;
			case '6':
				speed = 6;
				break;
			case '7':
				speed = 7;
				break;
			case '8':
				speed = 8;
				break;
			case '9':
				speed = 9;
				break;
			case '0':
				speed = 20;
				break;

			case KEY_UP:
				mvprintw(2, 0, "UP   ");
				roomba_start_stream(fd);
				roomba_wheels(fd, 50*speed, 50*speed);
				break;
			case KEY_DOWN:
				mvprintw(2, 0, "DOWN ");
				roomba_start_stream(fd);
				roomba_wheels(fd, -50*speed, -50*speed);
				break;
			case KEY_LEFT:
				mvprintw(2, 0, "LEFT ");
				roomba_start_stream(fd);
				roomba_wheels(fd, 25*speed, -25*speed);
				break;
			case KEY_RIGHT:
				mvprintw(2, 0, "RIGHT");
				roomba_start_stream(fd);
				roomba_wheels(fd, -25*speed, 25*speed);
				break;
			case 'S':
			case 's':
				mvprintw(2, 0, "STOP");
				roomba_stop(fd);
				roomba_stop_stream(fd);
				break;
		}
		refresh();
	}    
	clrtoeol();
	refresh();
	endwin();



	usleep(20000);
	data[0] = 136;
	data[1] = 8;
	res = write(fd, data, 2);

	serial_close(fd);
	return 0;
}
