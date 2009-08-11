/* roomba.h */

struct _sensor_data {
	char bumps_and_wheel_drops;
	char wall;
	char cliff_left;
	char cliff_front_left;
	char cliff_front_right;
	char cliff_right;
	char virtual_wall;
	char overcurrents;
	char unused1;
	char unused2;
	char ir;
	char buttons;
	short distance; //high byte first
	short angle;
	char charging_state;
	char voltage;
	char current;
	char battery_temperature;
	short battery_charge;
	short battery_capacity;
	short wall_signal;
	short cliff_left_signal;
	short cliff_front_left_signal;
	short cliff_font_right_signal;
	short cliff_right_signal;
	char user_digial_input;
	short user_analog_input;
	char charging_source_available;
	char OI_mode;
	char song_number;
	char song_playing;
	char number_stream_packets;
	short velocity;
	short radius;
	short right_velocity;
	short left_velocity;
};
typedef struct _sensor_data _sensor_data;

int roomba_init_safe(int fd);
int serial_open(char *device);
int serial_close(int fd);
int serial_write(int fd, char *data, int len);
int roomba_wheels(int fd, short left, short right);
int roomba_start_communication(int fd);
int roomba_stop(int fd);
_sensor_data* roomba_get_sensor_data(int fd, _sensor_data *sensor_data);
int roomba_start_stream(int fd);
int roomba_stop_stream(int fd);
int roomba_read_stream(int fd, _sensor_data *sensor_data);
