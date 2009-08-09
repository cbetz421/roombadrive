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
	
	fd = serial_open(argv[1]);
	if(fd <= 0) {
		printf("NO roomba!\n");
		exit(1);	
	}

	roomba_start_communication(fd);
	roomba_init_safe(fd);
/*
	for(i=0;i<255;i++) {
		usleep(20000);
		data[0] = 139;
		data[1] = 0;
		data[2] = i;
		data[3] = 255;
		res = write(fd, data, 4);
	}
*/
	
	/*
	usleep(20000);
	roomba_wheels(fd, 50, 50);
	sleep(4);
	roomba_wheels(fd, -50, -50);
	sleep(4);
	roomba_wheels(fd, 25, -25);
	sleep(4);
	roomba_wheels(fd, -50, 50);
	sleep(4);
	roomba_wheels(fd, 25, -25);
	sleep(4);
	roomba_stop(fd);
	sleep(4);
	*/
	
		
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
	long last = -1;
	while(1) {    
		long now = time(NULL);
		mvprintw(1, 0, "TIME=%s", ctime(&now));
		
		c = wgetch(menu_win);
		mvprintw(3, 0, "KEY=%d  ", c);

		if(last != now) {
			_sensor_data sensor_data;
			last = now;
			if(roomba_get_sensor_data(fd, &sensor_data) != NULL) {
				mvprintw(4, 0, "SENSOR DATA  -----------");	
				mvprintw(5, 0, " bumps_and_wheel_drops: %02d wall:%02d cliff_left:%02d cliff_front_left:%02d", 
					sensor_data.bumps_and_wheel_drops, sensor_data.wall, sensor_data.cliff_left, sensor_data.cliff_front_left);	
				mvprintw(6, 0, " cliff_front_right:%02d cliff_right:%02d virtual_wall:%02d overcurrents:%02d", 
					sensor_data.cliff_front_right, sensor_data.cliff_right, sensor_data.virtual_wall, sensor_data.overcurrents);
				mvprintw(7, 0, " ir:%02d buttons:%02d distance:%05d angle:%05d charging_state:%02d voltage:%02d", 
					sensor_data.ir, sensor_data.buttons, sensor_data.distance, sensor_data.angle, sensor_data.charging_state,
					sensor_data.voltage);
				mvprintw(8, 0, " current:%02d battery_temperature:%02d battery_charge:%05d, battery_capacity:%05d", 
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
			} else {
				mvprintw(4, 0, "BAD SENSOR DATA  --------");					
			}
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
				roomba_wheels(fd, 50*speed, 50*speed);
				break;
			case KEY_DOWN:
				mvprintw(2, 0, "DOWN ");
				roomba_wheels(fd, -50*speed, -50*speed);
				break;
			case KEY_LEFT:
				mvprintw(2, 0, "LEFT ");
				roomba_wheels(fd, 25*speed, -25*speed);
				break;
			case KEY_RIGHT:
				mvprintw(2, 0, "RIGHT");
				roomba_wheels(fd, -25*speed, 25*speed);
				break;
			case 'S':
			case 's':
				mvprintw(2, 0, "STOP");
				roomba_stop(fd);
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




/*
 * BEFORE:
 * speed 9600 baud; line = 0;
intr = <undef>; quit = <undef>; erase = <undef>; kill = <undef>; eof = <undef>;
start = <undef>; stop = <undef>; susp = <undef>; rprnt = <undef>;
werase = <undef>; lnext = <undef>; flush = <undef>; min = 0; time = 0;
-brkint -icrnl -imaxbel
-opost -onlcr
-isig -icanon -iexten -echo -echoe -echok -echoctl -echoke

 * 
 * 
 */


/* AFTER
 * 
 * speed 57600 baud; line = 0;
intr = <undef>; quit = <undef>; erase = <undef>; kill = <undef>; eof = <undef>;
start = <undef>; stop = <undef>; susp = <undef>; rprnt = <undef>;
werase = <undef>; lnext = <undef>; flush = <undef>; min = 26; time = 2;
-brkint -icrnl -imaxbel
-opost -onlcr
-isig -icanon -iexten -echo -echoe -echok -echoctl -echoke
*/
