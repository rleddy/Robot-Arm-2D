

///=======================================================================================================


#include <iostream>
#include <unistd.h>
#include <cstdlib>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <queue>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <SerialStream.h>

#define ECHO_INI_FILE false

#include <common/parameters.h>
#include "uiinterfacethread.h"
#include "arm_configuration.h"


#include "transformations.h"


#include "app_errors.h"


///------------------------------------->>>

static Arm_Configuration			*sg_arm_construct = NULL;

static queue<string>				sg_short_command_list;
bool								sg_metrics_acquired = false;
bool								sg_parse_results = false;
string 								sg_save_last_parameter_request = "";


///=======================================================================================================

extern bool sg_use_simulations;
bool sg_use_simulations = SIM_OFF;

extern bool sg_use_tcp_with_simulation;
bool sg_use_tcp_with_simulation = false;


bool sg_reporting = false;

///=======================================================================================================


unsigned int DEFAULT_DRIVE_COUNT = 3;
unsigned int DEFAULT_SENSOR_COUNT = 1;


static unsigned int		sg_actual_drive_count = 0;
static unsigned int		sg_actual_sensor_count = 0;


map<string,string> 		sg_server_parameters;  // Set global


// Allocate static space and don't worry too much about it.  Later, make it efficient;
MotorDriver				*sg_motor_driver_array[MAX_DRIVE_COUNT];
SensorController		*sg_sensor_controller_array[MAX_SENSOR_COUNT];


vector<Target *>						sg_target_storage;
map<string,SensorController *>			sg_sensor_map;


map< string, Target *>					sg_target_map;
map<string, Target *> 					sg_travel_map;

///---------------------------------------------->>>>
bool sg_error_is_soft = false; 				/// true if software, false if hardware...
software_error_type		sg_soft_error = ARM_SOFT_ERROR_NONE;



///=======================================================================================================

extern UIInterfaceWriterThread *tcp_client_operation(SensorController *sensor,unsigned int port);
extern void start_sensing(UIInterfaceWriterThread *sensor_client);



const bool g_config_noisy = true;

void
noisy_message(string message) {
	cout << message << endl;
}


void
sleeper(int sec, int nano) {

	struct timespec req;
	struct timespec rem;

	req.tv_sec = sec;
	req.tv_nsec = nano;

	rem.tv_sec = 0;
	rem.tv_nsec = 0;

	nanosleep(&req,&rem);
}




#include "m_commands.xpp"





Target *
find_target(string id_match) {
	int n = sg_target_storage.size();

	for ( int i = 0; i < n; i++ ) {
		Target *t = sg_target_storage[i];
		if ( t != NULL ) {
			if ( t->id() == id_match ) return(t);
		}
	}

	return(NULL);
}



///=======================================================================================================

#include "serialback.xpp"

/// ==============================================================================================>>>>>>>>




void
SensorController::setup_port(unsigned int port) {
	
	UIInterfaceWriterThread *uiiw = tcp_client_operation(this,port);  // the thread is now running.. It may or may not yet be connected.

	char buffer[255];
	sprintf(buffer,"%u",port);
	string port_str = strdup(buffer);
	_open_ports[port_str] = uiiw;
		///---------------->>
}



void
SensorController::start_ports(void) {
	map< string , UIInterfaceWriterThread * >::iterator mit;

	for ( mit = _open_ports.begin(); mit != _open_ports.end(); mit++ ) {
		UIInterfaceWriterThread *sensor_client  = mit->second;
		start_sensing(sensor_client);
	}
}




bool
Target::acquired_target(void) {
			///
	if ( this->target_present() ) {

		_target_present = false;
			///
		return(true);
	}
			///
	return(false);
}






/// less bound to a subroutine... ... ... 

#define MAX_POSSIBLE_MEASUREMENT_KEYS 7
const char *
sg_set_of_possible_measurements_keys[MAX_POSSIBLE_MEASUREMENT_KEYS] =  {
	"x", "y", "angle", "center_x", "center_y", "radius", "theta"
};



void
Target::unpack_mapped_values(void) {
	///

		/// x, y, angle, center, radius, theta
	if ( _target_present ) {
		 for ( int i = 0; i < MAX_POSSIBLE_MEASUREMENT_KEYS; i++ ) {
			 if ( _reported_values.find(sg_set_of_possible_measurements_keys[i]) != _reported_values.end() ) {
				 string measurement_key = sg_set_of_possible_measurements_keys[i];
				///------------------------>>
				 string value = _reported_values[measurement_key];
				///------------------------>>
				_measurements[measurement_key] = atof(value.c_str());
cout << "_measurements[measurement_key]" << _measurements[measurement_key] << endl;
			 }
		 }
	} else {

		if ( _target_was_present ) {
			/// handle loss of target... _target_was_present

		}

	}

}


 
void
Target::parse_data(string data_str) {
		///
	char *data = (char *)data_str.c_str();

	list<string> *value_list = to_tokens(data);

	list<string>::iterator lit;
	lit = _fields->begin();

	string reporting_element = *lit;

	list<string>::iterator slit;
	for ( lit++, slit = value_list->begin(); lit != _fields->end(); (lit++,slit++) ) {
			///---------------------->>
		string field_name = *lit;
		string value = *slit;
		if ( value[0] == '(' ) {
			slit++;
			string value2 = *slit;
			value = value.substr(1);
			value2 = value2.substr(0,value2.find(')'));

			string p1 = field_name + "_x";
			string p2 = field_name + "_y";
			_reported_values[p1] = value;
			_reported_values[p2] = value2;
				///----------------->>
		}

				/// These are in particular for the Banner camera
		if ( value.size() == 0 ) {
			_target_present = false;
		} else {
			_target_present = true;
		}

		_reported_values[field_name] = value;

	}

}





void
Target::data_test(string data) {

	/// ACQUIRED STRING
if ( sg_reporting ) {
	cout << _report_id << " :: :: "  << data << endl;
}

	char *tmp = (char *)data.c_str();
	while ( *tmp ) {	// Loop through ... 
		if ( (tmp[0] == ',') &&  (tmp[1] == ',') ) { _target_present = false; return; }
		tmp++;
	}

	_target_present = true;

	string pair = data.substr(data.find('(')+1,data.find(')')-1);

	string xstr = pair.substr(0,pair.find(','));
	_measurements["X"] = atof(xstr.c_str());

	string ystr = pair.substr(pair.find(',')+1);
	_measurements["Y"] = atof(ystr.c_str());
		///----------------->>

	string rest = data.substr( data.find(",", data.find(",") + 1) + 1);
	string rotation, percent;

	split_string(rest,rotation,percent,',');

	_measurements["THETA"] = atof(rotation.c_str());
	_measurements["MATCH"] = atof(percent.c_str());

		///----------------->>
	_fov_x = _measurements["X"];
	_fov_y = _measurements["Y"];
	_fov_theta = _measurements["THETA"];
	// convert from degrees to radians
	_fov_theta = (_fov_theta/180.0)*PI;
		///----------------->>
	_fov_match = _measurements["MATCH"];
		///----------------->>

}



unsigned char
index_of(string key) {
	char *tmp = (char *)key.c_str();
	tmp = strchr(tmp,'[');
	if ( tmp != NULL ) {
		tmp++;
		int i = atoi(tmp);
		if ( i > 0 ) i--;
		return((unsigned char)i);
	}
	return(0);
}


string
check_vars(string line) {
	string value = "";

	if ( strstr(line.c_str(),"{CANISTER_STEPS}") != NULL ) {
		value = sg_arm_construct->canister_current_steps_str();
		line = subst("{CANISTER_STEPS}",value,line);
	}

	if ( strstr(line.c_str(),"{ELBOW_STEPS}") != NULL ) {
		value = sg_arm_construct->elbow_current_steps_str();
		line = subst("{ELBOW_STEPS}",value,line);
	}

	return(line);
}




void
init_motor_descriptions(void) {

	if ( !(sg_server_parameters.empty()) ) {

		DEFAULT_DRIVE_COUNT = atoi(sg_server_parameters["MOTOR_COUNT"].c_str());

		for ( int i = 0; i < DEFAULT_DRIVE_COUNT; i++ ) {
			sg_motor_driver_array[i] = new MotorDriver(i+1);
		}

		unsigned char current_index = 0;
		
		map<string,string>::iterator mit;
			///
		for ( mit = sg_server_parameters.begin(); mit != sg_server_parameters.end(); mit++ ) {
			string key = mit->first;
			string line = mit->second;

			if ( key == "drive_count" ) {
				sg_actual_drive_count = atoi(line.c_str());
			} else {

				if ( key.substr(0,3) != "(m)" ) continue;

				if ( key.find("[") ) {  // has to refer to a driver

					line = sg_server_parameters[key];
					current_index = index_of(key);

					MotorDriver *md = sg_motor_driver_array[current_index];

					key = key.substr(3); // "(m)"

					key = key.substr(0,key.find("["));

					line = check_vars(line);

					md->add_parameter(key,line);

				}
			}

		}

		for ( int i = 0; i < sg_actual_drive_count; i++ ) {
			MotorDriver *md = sg_motor_driver_array[i];
			md->process_parameters();
		}

	}

}



// At the moment this is not called.
/*
void
test_routine(void) {
	//
	for ( int i = 0; i < sg_actual_drive_count; i++ ) {
		MotorDriver *md = sg_motor_driver_array[i];
		md->run_action("single_motion_A");
	}
}
*/




void
init_sensor_descriptions(void) {

	if ( !(sg_server_parameters.empty()) ) {
		bool status = true;

		DEFAULT_SENSOR_COUNT = atoi(sg_server_parameters["SENSOR_COUNT"].c_str());

		for ( int i = 0; i < DEFAULT_SENSOR_COUNT; i++ ) {
			sg_sensor_controller_array[i] = new SensorController();
		}

		unsigned char current_index = 0;
		map<string,string>::iterator mit;
			///
		for ( mit = sg_server_parameters.begin(); mit != sg_server_parameters.end(); mit++ ) {
			string key = mit->first;
			string line = mit->second;

				///-------------------------->>>
			if ( key.substr(0,3) != "(s)" ) continue;   /// specifically create targets...

			if ( key.find("[") ) {  // has to refer to a driver, sensor, or target...
				line = sg_server_parameters[key];
				current_index = index_of(key);
			}

			SensorController *s = sg_sensor_controller_array[current_index];
			string sensor_id = key.substr(3);
			sg_sensor_map[sensor_id] = s;
			s->set_descriptor(line);  /// For certain classes of sensors, connections to hardware may be established.

		}

		/// load target descriptions and associate them with their sensors. ...

		///(t)pattern[1]

cout << "sensors initialized" << endl;
			///
		for ( mit = sg_server_parameters.begin(); mit != sg_server_parameters.end(); mit++ ) {
			string key = mit->first;
			string line = mit->second;

				///-------------------------->>>
			if ( key.substr(0,3) != "(t)" ) continue;   /// specifically create targets...

			if ( key.find("[") ) {  // has to refer to a driver, sensor, or target...
				current_index = index_of(key);
			}

			if ( strncmp("(t)definition",key.c_str(),8) == 0 ) {

				Target *target = new Target();
					///--------------->>
				if ( sg_target_storage.size() <= current_index ) {
					for ( int i = sg_target_storage.size(); ( i <= current_index ); i++ ) {
						sg_target_storage.push_back(target);	/// really have to make space for the target...
					}
				} else {
								/// should delete previous definition.
					sg_target_storage[current_index] = target;  /// write over previous definition.
				}

				target->set_descriptor(line);

				string sensor_id = target->parameter("source");
				SensorController *s = sg_sensor_map[sensor_id];
				target->set_sensor(s);

				sg_travel_map[target->parameter("id")] = target; /// store the target for target mapping.... label identifier
				sg_target_map[target->parameter("report_id")] = target; /// store the target for target mapping.... sensor report label

			} else {

				if ( current_index > 0 ) current_index--;
					///-------------->>>
				key = key.substr(3); // "(m)"
				key = key.substr(0,key.find("["));
					///-------------->>>
				Target *target = sg_target_storage[current_index];
				line = check_vars(line);
				target->add_parameter(key,line);

			}

		}


		map<string,SensorController *>::iterator smit;
		for ( smit = sg_sensor_map.begin(); smit != sg_sensor_map.end(); smit++ ) {
			SensorController *s = smit->second;
			s->start_ports();
		}

cout << "targets initialized" << endl;

	}
}


/// ==============================================================================================>>>>>>>>




/// If a target has been acquired determine if the target is
/// has a set of other possible directions targets on a target schedule,
/// so that movement can go toward a goal in stages.


/// based on the current target, rebuild the target list for a later check..

void
schedule_relative_targets(Target *target,set<Target * >  *t_set) {
	///----------------->>>

	if ( ( t_set != NULL ) && ( target != NULL ) ) {

		unsigned int current_level = target->level();

		vector<Target *>::iterator vit;
		for ( vit = sg_target_storage.begin(); vit != sg_target_storage.end(); vit++ ) {
			Target *t = *vit;
			unsigned int t_level = t->level();

/*
			if ( ( t_level == current_level ) || ( t_level == (current_level + 1))  || ( t_level == (current_level - 1)) ) {
				t_set->insert(t);
			} else
*/
			if ( t->target_present() ) {
				t_set->insert(t);
			}

		}

	}
	
	///----------------->>>
}





/// STEP 1
/// init_target_list
///
/// Input an empty list for targets.
///
///

void
perpare_first_tier_targets(set<Target * > *t_set) {
	/// ---------------------------->>>

	vector<Target *>::iterator vit;
	for ( vit = sg_target_storage.begin(); vit != sg_target_storage.end(); vit++ ) {
		Target *t = *vit;
		if ( t->level() == 1 ) {
			t_set->insert(t);
		}
	}

}



bool
init_target_list(set<Target * > *t_set) {

	bool status = true;

	if ( t_set == NULL ) {
		status = false;
	} else {

		/// Setup targets based on predetermined schedule...
		/// should be read from the ini file
		/// each refers to a particular inspection...

		perpare_first_tier_targets(t_set);  /// put the first targets in the list
	}

	return(status);

}





/// STEP 2
/// -------------------->>
/// goal_setup
///   Just say what the goal is..  This is supposed to be the final target..
///

Target *
goal_setup(void) {  
	///----------------->>
	Target *goal = find_target("goal");
	///----------------->>
	return(goal);
}




/// STEP 3 (decision)
/// at_goal
/// Return true if the goal the current point of inspection

/// check the output of sensors to see if the goal matches current information..

bool
at_goal(Target *goal,Target *current_target) {

	bool found_goal = false;

	if ( current_target == NULL ) return(false);

	{
		///---------------------->>
		
	}
	
	return(found_goal);

}







void
Target::fov_center_offset(double &x,double &y) {

	SensorController *s = this->sensor();

	double mid_x = 0.0, mid_y = 0.0;
	double scalex, scaley;
		///--------------------->>

	s->scale(scalex,scaley);
	s->fov_center(mid_x,mid_y);

	double x_hat = x - mid_x;
	double y_hat = mid_y - y;

	x = x_hat*scalex;
	y = y_hat*scaley;

	if ( g_config_noisy ) {
		cout << "fov_center_offset:: delta X: " << x << " delta  Y: " << y << " scalex,scaley " << scalex << "," << scaley << endl;
		cout << "----------------------------------------------------------" << endl;
	}

}




bool
target_accuracy(Target *current_target) {
		//-------------------->>
	bool status = true;

	/// Setup targets based on predetermined schedule...

	return(status);
}



typedef struct {
	double x;
	double y;
} seq_pos;


#define TRIALS_COUNT  0

int trials = 0;





Target *
locate_in_cell(Target *current_target,set<Target * > *t_set,double &x,double &y) {

	Target *t = NULL;

	if ( t_set == NULL ) {
		return(NULL);
	} else if ( t_set->empty() ) {
		return(NULL);
	} else {

		set<Target *>::iterator next;

		double save_delta_x = 0.0;
		double save_delta_y = 0.0;
		double distance = 10000.0;

		SensorController *s = current_target->sensor();
		double mid_x = atof(s->parameter("fov_w").c_str());
		double mid_y = atof(s->parameter("fov_h").c_str());

		Target *new_focus = NULL;

		for ( next = t_set->begin(); next != t_set->end(); next++ ) {
							///------------->>
			t = *next;
							///------------->>
			if ( t->acquired_target() ) {
					//
				double xx = t->fov_X();
				double yy = t->fov_Y();
					//
				double delta_x = mid_x - xx;
				double delta_y = mid_y - yy;

				double dd = sqrt(delta_x*delta_x + delta_y*delta_y);
cout << "check>>   <" << t->parameter("id") << "> xx: " << xx << ", yy: " << yy <<  ", delta_x: " << delta_x << ", delta_y: " << delta_y << ", dd: " << dd << endl;

				if ( dd < distance ) {
					distance = dd;
					new_focus = t;
					save_delta_x = delta_x;
					save_delta_y = delta_y;
				}

			}
				///------------------------->>
		}

		t = new_focus;
		if ( t == NULL ) t = current_target;


/*
		if ( distance < TEST_EPSILON ) {  /// if close enough to the target (close enough to center)

cout << "FOUND IT CLOSE ENOUGH TO MATCH!!!" << endl;
			t = sg_travel_map[t->parameter("travel_next")];
		}
*/

		double x_hat = t->fov_X();
		double y_hat = t->fov_Y();

cout << "<" << t->parameter("id") << "> xx: " << x_hat << ", yy: " << y_hat;

		x_hat += save_delta_x;
		y_hat += save_delta_y;

cout <<  ", delta_x: " << save_delta_x << ", delta_y: " << save_delta_y;
 
		double scalex, scaley;
		s->scale(scalex,scaley);

		x_hat = x_hat*scalex;
		y_hat = y_hat*scaley;

cout <<  ", x_hat: " << x_hat << ", y_hat: " << y_hat << endl;

		/// x_hat,y_hat is now at the center of the pattern

/*
		if ( distance < TEST_EPSILON ) {  /// if close enough to the target (close enough to center)

cout << "FOUND IT CLOSE ENOUGH TO MATCH!!!" << endl;
		}
*/

 		Target *next_t = sg_travel_map[t->parameter("travel_next")];

		string next_target_offset = t->parameter("travel_offset");

		string xystr = next_target_offset.substr(0,next_target_offset.find(","));
		double offset_x = atof(xystr.c_str());

		xystr = next_target_offset.substr(next_target_offset.find(",") + 1);
		double offset_y = atof(xystr.c_str());

cout << "next_target_offset: "  << next_target_offset << "," << offset_x << "," << offset_y << endl;

		x = x_hat + offset_x;
		y = y_hat + offset_y;

	}

	return(t);
}




void
step_n_motion(int motor, int n_steps) {

	if ( motor > 0 ) motor--;

	///
	MotorDriver *md = sg_motor_driver_array[motor];

	string step_string = "";
	char buffer[255];

	double f_steps = n_steps*md->gear_ratio();
	n_steps = (int)(f_steps);   /// perhaps round?

	sprintf(buffer,"%d",n_steps);
	step_string = strdup(buffer);

	md->run_action("single_motion_var","STEPS",step_string);

}



bool
check_motion_status(int motor) {

	if ( motor > 0 ) motor--;
	///
	MotorDriver *md = sg_motor_driver_array[motor];
	if ( md == NULL ) return(false);

	return(!(md->check_status(19)));
}


bool
check_motion_status_quiet(int motor) {

	if ( motor > 0 ) motor--;
	///
	MotorDriver *md = sg_motor_driver_array[motor];
	if ( md == NULL ) return(false);

	return(!(md->check_status(19,false)));
}




void
move_by_command(double x,double y) {
	/// Run stepper commands...

cout << "x: " << x << " y: " << y << endl;

	double dx = x, dy =y;   // motors run clockwise...

	sg_arm_construct->set_target(dx, dy);

	int csteps = sg_arm_construct->canister_current_steps();
	int esteps = sg_arm_construct->elbow_current_steps();

cout << "steps alpha: " << csteps << " steps beta: " << esteps << endl;

	step_n_motion(1, csteps);
	step_n_motion(2, esteps);

	unsigned int count = 5;
	bool in_motion = true;
	sleeper(0,10000);
	if ( sg_use_simulations == SIM_OFF ) {
		while ( in_motion ) {
				///
			bool in_motion_1;
			bool in_motion_2;
				///
			in_motion_1 = check_motion_status(1);
			in_motion_2 = check_motion_status(2);

			in_motion = in_motion_1 || in_motion_2;

			//in_motion = in_motion && (--count > 0);

			sleeper(0,100000);
		}
	} else {
		sleeper(0,1000000);
	}

	sg_current_x = x;
	sg_current_y = y;
	
}



void
stepper_by_command(int motor,int steps) {
	/// Run stepper commands...

	step_n_motion(motor, steps);

	unsigned int count = 5;
	bool in_motion = true;
	sleeper(0,10000);
	if ( sg_use_simulations == SIM_OFF ) {
		while ( in_motion ) {
				///
			in_motion = check_motion_status(motor);
				///
			sleeper(0,100000);
		}
	} else {
		sleeper(0,1000000);
	}

}





/// The assumption is that the vector is in the same scale and dimensions as the arm coordinates (which is for the motion).

void
move_by_vector(double radius, double theta) {

	///------------>>

	double x = sg_current_x;
	double y = sg_current_y;

	///------------>>

	double x1 = 0.0; double y1 = 0.0;

	x1 = radius*cos(theta);
	y1 = radius*sin(theta);
	///------------>>

	x1 = x1 + x;
	y1 = y1 + y;
	///------------>>

	move_by_command(x1,y1);
}






bool
engagement(void) {
	bool status = true;


	return(status);
}



bool
handoff(void) {
	bool status = true;


	return(status);
}




 /// run a process that hones the device to its coupling position.
 
bool
allignment(Target *goal) {
	
	return(true);
}




///--------------------------------------------------->>>

double theta_delta = TWO_PI/20000;

void
test_coords(double &x,double &y) {
		///--------------->>
	static int i = 0;
	static double current_theta = 0.0;
	static double current_radius = 1.0;
		///--------------->>

	if ( (i % 20000) ) {
		i = 0;
		current_theta = 0.0;
		current_radius += 3.0;
	}

	x = current_radius*cos(current_theta);
	y = current_radius*sin(current_theta);


	/// cout << "TEST RADIUS: R: " << current_radius << " A: " << current_theta << endl;

	i++;
	current_theta += theta_delta;
	sleeper(0,250000000);
}




double R_delta = 0.1;
double theta_delta2 = TWO_PI/20;


void
test_extender_coords(double &x,double &y) {
		///--------------->>
	static int i = 0;
	static double current_theta = 0.0;
	static double current_radius = 30.0;
		///--------------->>
	if ( !(i % 20) ) {
		current_theta = 0;
		current_radius += R_delta;
	}
		///--------------->>
	x = current_radius*cos(current_theta);
	y = current_radius*sin(current_theta);

	i++;
	current_theta += theta_delta2;

	sleeper(0,250000000);
}





extern void emit_to_target(string key,string data);

void
emit_to_target(string key,string data) {

	///------------------------->>
	Target *target = sg_target_map[key];
	///------------------------->>
	if ( target != NULL ) {
		target->data_test(data);
	}

}





/// ==============================================================================================>>>>>>>>


bool
to_target(void) {

	bool status = true;

	set<Target * > *t_set = new set<Target * >(); // create an empty list, a receptical of a target part

	status = init_target_list(t_set);

	if ( status ) {
		
		Target *goal = goal_setup();

		if ( goal != NULL ) {

			Target *current_target = NULL;

			double x = 0.0, y = 0.0;  // Initial coordinates of the apparatus

			while ( !(at_goal(goal,current_target)) ) {  // by now sensors are ready, there can be a single motion until the presence of any marker.

				if ( current_target == NULL ) current_target = goal;
					//----------->>						//----------->>
					//----------->>
				do {
					/// Where to go next.
					/// Given the list of targets, what is the best match, and what is it's x,y position?
					current_target = locate_in_cell(current_target,t_set,x,y);
				///------------------------------>>
					move_by_command(x,y);

				} while ( !(target_accuracy(current_target)) );  // check on accuracy.

				 schedule_relative_targets(current_target,t_set);  /// change the target list.
					//----------->>

			}

		}

		status = allignment(goal);  /// run a process that hones the device to its coupling position.

	}

	if ( status ) status = engagement();   /// Run the process that causes engagement.

	if ( status ) status = handoff();

	return(status);

}






/// ==============================================================================================>>>>>>>>
///---------------------------------------------------------------------------------------------------->>>




int
motor_driver_port_initialization( void ) {
		///
		///

	if ( sg_use_simulations != SIM_ON ) {

		cout << "opening /dev/ttyS0" << endl;

		//
		// Open the serial port.
		//
		sg_serial_port.Open( "/dev/ttyS0" ) ;
		if ( ! sg_serial_port.good() )
		{
			cerr << "[" << __FILE__ << ":" << __LINE__ << "] "
						<< "Error: Could not open serial port."
						<< endl ;
			exit(1) ;
		}
		//
		// Set the baud rate of the serial port.
		//
		sg_serial_port.SetBaudRate( SerialStreamBuf::BAUD_9600 ) ;
		if ( ! sg_serial_port.good() )
		{
			cerr << "Error: Could not set the baud rate." << endl ;
			exit(1) ;
		}
		//
		// Set the number of data bits.
		//
		sg_serial_port.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 ) ;
		if ( ! sg_serial_port.good() )
		{
			cerr << "Error: Could not set the character size." << endl ;
			exit(1) ;
		}
		//
		// Disable parity.
		//
		sg_serial_port.SetParity( SerialStreamBuf::PARITY_NONE ) ;
		if ( ! sg_serial_port.good() )
		{
			cerr << "Error: Could not disable the parity." << endl ;
			exit(1) ;
		}
		//
		// Set the number of stop bits.
		//
		sg_serial_port.SetNumOfStopBits( 1 ) ;
		if ( ! sg_serial_port.good() )
		{
			cerr << "Error: Could not set the number of stop bits."
						<< endl ;
			exit(1) ;
		}
		//
		// Turn off hardware flow control.
		//
		sg_serial_port.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE ) ;
		if ( ! sg_serial_port.good() )
		{
			cerr << "Error: Could not use hardware flow control."
						<< endl ;
			exit(1) ;
		}

		noisy_message("WAIT SERIAL");

		while( sg_serial_port.rdbuf()->in_avail() == 0 ) {
			sleeper(0,1000) ;
		}
		//
		// Keep reading data from serial port and print it to the screen.
		//
		while( sg_serial_port.rdbuf()->in_avail() > 0  ) {
			char next_byte;
			sg_serial_port.get(next_byte);
			cerr << hex << static_cast<int>(next_byte) << " ";
			sleeper(0,500) ;
		}
		cerr << endl;

		int port_delay = atoi(sg_server_parameters["port_ready_delay"].c_str());

		sleep(port_delay);
		noisy_message("READY");

	}

	return EXIT_SUCCESS;
}







///---------------------------------------------------------------------->>>

bool
initialize_all( int argc, char** argv ) {   /// Make sure motors and sensors are ready for operation.
		///------------------------->>
	int status = EXIT_SUCCESS;				///

	read_parameters(sg_server_parameters,true);

	{

		string sim_state = sg_server_parameters["SIMULATION"];
		lower_case(sim_state.c_str());
		if ( sim_state == "yes" ) {
			sg_use_simulations = SIM_ON;

			sim_state = sg_server_parameters["TCP_WITH_SIMULATION"];
			if ( sim_state == "yes" ) {
				sg_use_tcp_with_simulation = true;  /// defaults to false
			}

		}

	}
	
	if ( (status = motor_driver_port_initialization()) == EXIT_SUCCESS ) {

		CANISTER_RADIUS = atof(sg_server_parameters["CANISTER_RADIUS"].c_str());
		ARM_1_length = atof(sg_server_parameters["ARM_1_length"].c_str());
		ARM_2_length = atof(sg_server_parameters["ARM_2_length"].c_str());

		CANISTER_STEP =  atof(sg_server_parameters["CANISTER_STEP"].c_str());
		ELBOW_STEP = atof(sg_server_parameters["ELBOW_STEP"].c_str());

			/// Initialize 		ARM
		sg_arm_construct = new Arm_Configuration();
		double radius = atof(sg_server_parameters["MIN_RADIUS"].c_str());;
		sg_arm_construct->set_mininum_radius(radius);

			///------------------------->>
			/// Initialize 		MOTOR
		init_motor_descriptions();

			///------------------------->>
			/// Initialize 		SENSOR
		init_sensor_descriptions();

			///------------------------->>

		return(true);
	} else {
		return(false);
	}
}





///-------------------------------->>
/// COMMAND RESPONSES
///-------------------------------->>

extern void report_arm_position(void);



///===== COMMANDS (ARM positions only  )



/// (1) move to position:
///-------------------------------->>
/// Take XY coordinates and move to those coordinates.

void
move_to_position(void) {

	string XY_pair = "";

	double X, Y;

	cout << "(X,Y):"; cout.flush();
	cin >> XY_pair;

	if ( XY_pair[0] == '(' ) XY_pair = XY_pair.substr(1);
	X = atof(XY_pair.c_str());
	XY_pair = XY_pair.substr(XY_pair.find(',')+1);
	Y =  atof(XY_pair.c_str());;

	move_by_command(X,Y);

	report_arm_position();

}





/// (2) move to position list:
///-------------------------------->>
/// Take XY coordinates from a list and move to those coordinates.


void
move_to_position_list(void) {

	int trials = 0;
	Target *t = NULL;

	string positions = sg_server_parameters["TEST_POSITIONS"];


	list<string> *coords = to_items(positions,' ');
	list<string>::iterator lit;

	for ( lit = coords->begin(); lit != coords->end(); lit++ ) {
		string coord = *lit;

		string xstr, ystr;
		split_string(coord,xstr,ystr,',');
		
		double x = atof(xstr.c_str());
		double y = atof(ystr.c_str());
		sleeper(0,2500000);

		move_by_command(x,y);

		report_arm_position();

	}

}

	

static double sg_test_delta_x = 0;
static double sg_test_start_x = 0.0;
static unsigned int TEST2_TRIALS = 30;


/// (3) walk a line:
///-------------------------------->>
/// Turn on or turn off the sensor reports to console (file, etc)

void
walk_a_line(void) {

/// calculate start x X
	bool locate_x = false;

	string num_str;
	cout << "Y = ";
	cout.flush();

	cin >> num_str;

	if ( isalpha(num_str[0]) ) {
		cout << "A number is required" << endl;
		return;
	}

	double y = atof(num_str.c_str());
	double x = 0.0;

	double R = sg_arm_construct->start_radius();
	double end_x = sg_arm_construct->max_radius();

	cout << "Y: " << y << " R: " << R << " MAX R: " << end_x << endl;

	cout << "NUMBER OF TRIALS: "; cout.flush();
	string test_trials_str;
	cin >> test_trials_str;
	TEST2_TRIALS = atoi(test_trials_str.c_str());
	if ( TEST2_TRIALS == 0 ) TEST2_TRIALS = 30;

	/// figure the delta x for dividing the line into TEST2_TRIALS equal steps;
	if ( y > end_x ) { sg_test_start_x = 0; sg_test_delta_x = 0; }
	else {

		x = sqrt(R*R - y*y);
		if ( y != 0 ) {
			double ratio = x/R;
			end_x = ratio*end_x;
		}
		sg_test_delta_x = (end_x - x)/TEST2_TRIALS;
		sg_test_start_x = x;

	}

	for ( int trials = 0; trials < TEST2_TRIALS; trials++ ) {

		if ( trials >= (TEST2_TRIALS >> 1) ) {

			trials = -1;
			x = sg_test_start_x + TEST2_TRIALS*sg_test_delta_x;

			return;
		}

		if ( !locate_x ) {
				///----------------->>
			x = sg_test_start_x + trials*sg_test_delta_x;
				///----------------->>
			if ( sg_reporting ) {
				cout << "TEST (X,Y) : (" << x << "," << y << ")" << endl;
			}
		}

		move_by_command(x,y);
		//sleeper(0,2500000);

	}

	report_arm_position();

}



///===== COMMANDS (positions based on VISION )



/// (4) move to pattern:
///-------------------------------->>
/// Given a pattern appears in the field of view, move to it when it does.

void
move_to_pattern(string pattern_id) {

	// you are here
	Target *t = sg_travel_map[pattern_id];
	if ( t == NULL ) {

		cout << "No such target " << pattern_id << endl;

	} else {

		while ( !( t->acquired_target() ) ) {
			sleeper(0,1000000);
			cout << "MP" << pattern_id << endl;
		}

		/// camera has pattern in the field of view...
		/// now move the middle of the field of view to the pattern...

		double x = 0.0;
		double y = 0.0;

		x = t->X();  /// locus of the object that has been seen. In the frame Field Of View (FOV), In pixels.
		y = t->Y();

		t->fov_center_offset(x,y); /// offset from center in the global scale determined by the sensor
		x += sg_current_x;
		y += sg_current_y;
		move_by_command(x,y);

		report_arm_position();

	}
}





/// (5) move relative to pattern:
///-------------------------------->>
/// Given a pattern appears in the field of view, move to a pattern relative to it and focus on that pattern

void
move_relative_to_pattern(string pattern_id) {
	// you are here

	// you are here
	Target *t = sg_travel_map[pattern_id];
	if ( t == NULL ) {

		cout << "No such target " << pattern_id << endl;

	} else {

		while ( !( t->acquired_target() ) ) {
			sleeper(0,100000);
			cout << "MP" << pattern_id << endl;
		}

		/// camera has pattern in the field of view...
		/// now move the middle of the field of view to the pattern...

		double x = 0.0;
		double y = 0.0;
		double x_hat = t->X();
		double y_hat = t->Y();

		Target *next_t = sg_travel_map[t->parameter("travel_next")];
		string next_target_offset = t->parameter("travel_offset");

		t->fov_center_offset(x,y);

		{
			string xystr = next_target_offset.substr(0,next_target_offset.find(","));
			double offset_x = atof(xystr.c_str());

			xystr = next_target_offset.substr(next_target_offset.find(",") + 1);
			double offset_y = atof(xystr.c_str());

			x = x_hat + offset_x;
			y = y_hat + offset_y;
		}

		x += sg_current_x;
		y += sg_current_y;
		move_by_command(x,y);

		report_arm_position();

	}
}


/// (6) scan_for_pattern:
///-------------------------------->>
/// Jog forward with limits until one of several patterns appears




Target *
point_in_time_best_match(double &x_hat,double &y_hat) {
		///
	///--------------------->>
	int n_targets = sg_target_storage.size();
	double percent_match = 0;
	Target *best = NULL;

	for ( int j = 0; j < n_targets; j++ ) {
			///---------------->>>
		Target *t = sg_target_storage[j];

		if ( t->acquired_target() ) {
			if ( t->fov_match() > percent_match ) {
				percent_match = t->fov_match();
				best = t;
			}
		}
			///---------------->>>
	}

	if ( best != NULL ) {
			///---------------->>>
		x_hat = best->fov_X();
		y_hat = best->fov_Y();

	}

	return(best);
		///
}



Target *
wait_for_any_pattern(double &x_hat,double &y_hat) {
	///--------------------->>
	Target *best = NULL;

	while ( best == NULL ) {

		best = point_in_time_best_match(x_hat,y_hat);
		sleeper(0,1000000);

	}

}




void
scan_for_pattern(void) {

	///
	MotorDriver *md = sg_motor_driver_array[0];
	md->run_action("scan_jog_action");

	///
	md = sg_motor_driver_array[1];
	md->run_action("scan_jog_action");
		///----------------------->>>
	double x_hat = 0.0, y_hat = 0.0;
	Target *t = wait_for_any_pattern(x_hat,y_hat); /// fov coordinates
	t->fov_center_offset(x_hat,y_hat);

	md = sg_motor_driver_array[0];
	md->run_action("stop_motion");
			///
	md = sg_motor_driver_array[1];
	md->run_action("stop_motion");

	double x, y;
	double rad1,rad2;

	md = sg_motor_driver_array[0];
	rad1 = md->current_rotation();
			///
	md = sg_motor_driver_array[1];
	rad2 = md->current_rotation();

	sg_arm_construct->rotation_to_cartesian(rad1,rad2,x,y);

	x += x_hat;
	y += y_hat;

cout << x << "," << y << endl;

	move_by_command(x,y);
	///

}



/// (7) scan_for_pattern with directed motion:
///-------------------------------->>
/// Jog forward with limits until one of several patterns appears


void
go_to_max_edge_test(double current_r,double &x,double &y) {
	x = 0;
	y = current_r;
	move_by_command(x,y);
}



void
scan_for_pattern_with_directed_motion(void) {

	///
	MotorDriver *md1 = sg_motor_driver_array[0];
	MotorDriver *md2 = sg_motor_driver_array[1];

	double end_r = sg_arm_construct->max_radius();
	double start_r = sg_arm_construct->start_radius();
	int n = (int)(end_r - start_r);


	int base_steps = (CANISTER_STEP/2.0);

	int n_targets = sg_target_storage.size();
	double current_r = start_r;

	Target *t_acquired = NULL;

	double x = 0.0, y = 0.0;
	double x_hat = 0.0, y_hat = 0.0;


cout << end_r << "," << start_r << ", " << n << "==>> " << n_targets << "..." << endl;
cout << "----------------------------------------------------------" << endl;

string wait_for_me;

	for ( int i = 0; i < n; i++ ) {
			///
		go_to_max_edge_test(current_r++,x,y);
/*
cout << "wait prompt (test):" << x << "," << y; cout.flush();
cin >> wait_for_me;
*/
sleeper(1,1000);

		cout  << "<next>" << endl;
		step_n_motion(1, base_steps);

		unsigned int count = 5;
		bool in_motion = true;
		sleeper(0,10000);
			///
		if ( sg_use_simulations == SIM_OFF ) {
			while ( in_motion ) {
					///
				bool in_motion_1;
				bool in_motion_2;
					///
				in_motion_1 = check_motion_status(1);

				in_motion = in_motion_1;
				t_acquired = point_in_time_best_match(x_hat,y_hat);
				if ( t_acquired != NULL ) {
					if ( in_motion ) {
cout << "STOP MOTION... " << endl;
						md1->run_action("stop_motion");
					}
					break;
				}
				sleeper(0,100000);
			}
		} else {
			sleeper(0,1000000);
		}
		if ( t_acquired != NULL ) break;
	}

	///----------------------------------->>
	double rad1,rad2;

	rad1 = md1->current_rotation()*(md1->gear_ratio());
	rad2 = md2->current_rotation()*(md2->gear_ratio());

	sg_arm_construct->rotation_to_cartesian(rad1,rad2,x,y);

	x += x_hat;
	y += y_hat;

cout << "FINAL: " << x << "," << y << endl;
	move_by_command(x,y);
	///

}







///===== COMMANDS (positions)

/// (8) report arm position:
///-------------------------------->>
/// Report the position of the arm right now

void
report_arm_position(void) {

	cout << endl;
	cout << "(sg_current_x, sg_current_y): (" << sg_current_x << "," << sg_current_y << ") ,, ";

	double alpha_theta, beta;

	alpha_theta = sg_arm_construct->base_angle();
	beta = sg_arm_construct->elbow_angle();
	
	cout << "(alpha_theta, beta): (" << alpha_theta << "," << beta << ") ,, ";

	cout << endl;
	cout << endl;

	/// now report the motor rotations....

}



/// (9) toggle sensor reports:
///-------------------------------->>
/// Turn on or turn off the sensor reports to console (file, etc)

void
toggle_sensor_reports(void) {
	sg_reporting = !sg_reporting;
}





/// (10) transformation_tests:
///-------------------------------->>
/// Turn on or turn off the sensor reports to console (file, etc)

void
transformation_tests(void) {

	JointTransformation transformer;
	
	double X, Y;
	double pi_part;
	string XY_pair;
	string pipart;

	cout << "(X,Y):"; cout.flush();
	cin >> XY_pair;
	cout << "Rotate base coordinates by (PI/?):"; cout.flush();
	cin >> pipart;

	pi_part =  atof(pipart.c_str());
	pi_part = -pi_part; /// transform the vectors in the inverse of the transformation

	if ( XY_pair[0] == '(' ) XY_pair = XY_pair.substr(1);
	X = atof(XY_pair.c_str());
	XY_pair = XY_pair.substr(XY_pair.find(',')+1);
	Y =  atof(XY_pair.c_str());;

	/// Report length and rotation in base coordinates...

	{
		double R1 = sqrt(X*X + Y*Y);
		double theta = acos(fabs(X)/R1);

		if ( X < 0 || Y < 0 ) {
			if ( X > 0 ) {
				theta = -theta;
			} else if ( Y > 0 ) {
				theta = PI/2 + theta;
			} else {
				theta = -theta - PI/2;
			}
		}

		double ppreport = PI/theta;

		cout << "BASED COORDS:: X: " << X << " Y: " << Y << endl;
		cout << "BASED COORDS:: Radius: " << R1 << " theta: " << theta << " pi part " << ppreport << endl;
	}

	///------------------------------------>>

	double theta = 0.0;
	double abs_pi_part = fabs(pi_part);
	if ( ( abs_pi_part > 0.0005 ) &&  ( abs_pi_part < 50000 )  ) theta = (PI/pi_part);

	///------------------------------------>>
	transformer.rotate_2d(theta,X,Y);
	///------------------------------------>>

	{
		double R1 = sqrt(X*X + Y*Y);
		double theta = acos(fabs(X)/R1);

		if ( X < 0 || Y < 0 ) {
			if ( X > 0 ) {
				theta = -theta;
			} else if ( Y > 0 ) {
				theta = PI/2 + theta;
			} else {
				theta = -theta - PI/2;
			}
		}

		double ppreport = PI/theta;

		cout << "TRANSFORMED:: X: " << X << " Y: " << Y << endl;
		cout << "TRANSFORMED:: Radius: " << R1 << " theta: " << theta << " pi part " << ppreport << endl;
	}

}



/// (11) target_to_arm:
///-------------------------------->>
/// Given a sensor, move the arm to the goal position based on the information from the pattern...

void
target_to_arm(string pattern_id,double &x, double &y) {

	Target *t = sg_travel_map[pattern_id];
	if ( t != NULL ) {

		double theta = 0.0;
		double match = 0.0;

		JointTransformation		transformer;
		SensorController		*s = t->sensor();

		x = t->fov_X();  /// locus of the object that has been seen. In the frame Field Of View (FOV), In pixels.
		y = t->fov_Y();
		theta = t->fov_theta();
		match = t->fov_match();

		if ( g_config_noisy ) {
			cout << "FROM CAMERA:: X: " << x << " Y: " << y << " theta: " << theta << " match: " << match << endl;
			cout << "CURRENT (X,Y): (" << sg_current_x << "," << sg_current_y << ")" << endl;
			cout << "----------------------------------------------------------" << endl;
		}

		t->fov_center_offset(x,y); 		/// (offset, scale) offset from center in the global scale determined by the sensor

		double elbow_translate_theta = sg_arm_construct->elbow_angle();
		double base_translate_theta = sg_arm_construct->base_angle();
			///-----------------------------------------------------------------

		double coord_rotate_1 = PI - elbow_translate_theta;
		transformer.rotate_2d(-coord_rotate_1,x,y);  /// transform the coordinate system to second ARM2 ... apply inverse to vectors
			///-----------------------------------------------------------------

		if ( g_config_noisy ) {
			cout << "(rotate_2d):: .... X: " << x << " .... Y: " << y << " coord_rotate_1: " << coord_rotate_1 << " elbow_translate_theta:" << elbow_translate_theta << endl;
			cout << "----------------------------------------------------------" << endl;
		}

		double coord_rotate_2 = PI/2 - base_translate_theta;
		transformer.rotate_2d(-coord_rotate_2,x,y);  /// transform the coordinate system to base ... apply inverse to vectors
			///-----------------------------------------------------------------

		if ( g_config_noisy ) {
			cout << "(rotate_2d):: .... X: " << x << " .... Y: " << y << " coord_rotate_2: " << coord_rotate_2 << " base_translate_theta:" << base_translate_theta << endl;
			cout << "----------------------------------------------------------" << endl;
		}

		/// The current coordinates of the center of focus are known in the base system
		//sg_current_x << "," << sg_current_y

		x += sg_current_x;
		y += sg_current_y;

		if ( g_config_noisy ) {
			cout << "(translate to focus):: .... X: " << x << " .... Y: " << y << endl;
			cout << "----------------------------------------------------------" << endl;
		}

		/// Now X,Y are in the coordinates of the second arm part
		/// Get the goal into the same process... The goal (presumed) has already been scaled to the base scale.

		double goal_x = 0.0;
		double goal_y = 0.0;

		t->goal_offset(goal_x,goal_y);

		if ( g_config_noisy ) {
			cout << "(goal_offset):: X: " << goal_x << " Y: " << goal_y << endl;
			cout << "Pattern rotation: " << theta << endl;
			/// theta is reported clockwise, but positive is counter clock wise. However, coordinate transformation is clockwise.
			cout << "----------------------------------------------------------" << endl;
		}

		transformer.rotate_2d(theta,goal_x,goal_y);  /// transform the coordinate system to FOV ... apply inverse to vectors
		transformer.rotate_2d(-coord_rotate_1,goal_x,goal_y);  /// transform the coordinate system to second ARM2 ... apply inverse to vectors
		transformer.rotate_2d(-coord_rotate_2,goal_x,goal_y);  /// transform the coordinate system to base ... apply inverse to vectors

		if ( g_config_noisy ) {
			cout << "(goal_offset after rotation):: X: " << goal_x << " Y: " << goal_y << endl;
			cout << "----------------------------------------------------------" << endl;
		}

		x += goal_x;
		y += goal_y;

		if ( g_config_noisy ) {
			cout << "(translate to goal):: .... X: " << x << " .... Y: " << y << endl;
			cout << "----------------------------------------------------------" << endl;
		}

	} else {

		software_error("NULL target to target_to_arm",ARM_SOFT_ERROR_NULL_POINTER);

	}

}




void
test_target_to_arm(void) {
		///------------------->>
	string controller = "yes";
	string pattern_id = "alpha_Q";
		///------------------->>
	double test_x = 0.0;
	double test_y = 0.0;
	double test_theta = 0.0;
	double test_match = 0.0;
		///------------------->>

	Target *t = NULL;
	string value;

	while ( t == NULL ) {

		cout << "pattern(id): "; cout.flush();
		cin >> pattern_id;

		t = sg_travel_map[pattern_id];

		if ( t == NULL ) {
			cout << "Pattern not found ------------>>  " << endl;
			cout << "Try again? "; cout.flush();
			string answer;
			cin >> answer;
			if  ( answer[0] == 'y' ) continue;
			else return;
		}

		bool use_data = ( sg_use_simulations == SIM_OFF );

		if ( use_data ) {
			cout << "Use Data From Camera? " << endl;
			string answer;
			cin >> answer;
			if  ( answer[0] != 'y' ) use_data = false;
		}
		
		if ( !(use_data) || ( sg_use_simulations == SIM_ON ) ) {

				///----------------->>>

			cout << "X: "; cout.flush();
			cin >> value;
			test_x = atof(value.c_str());

			cout << "Y: "; cout.flush();
			cin >> value;
			test_y = atof(value.c_str());

			cout << "Theta: "; cout.flush();
			cin >> value;
			test_theta = atof(value.c_str());

			cout << "Match: "; cout.flush();
			cin >> value;
			test_match = atof(value.c_str());

			t->set_test_measurements(test_x, test_y, test_theta, test_match);
				///----------------->>>

		} else if ( !(t->acquired_target()) ) {
				///----------------->>>
			cout << "Pattern Not In VIEW ------------>>  " << endl;
			cout << "Try again? "; cout.flush();
			string answer;
			cin >> answer;
			if  ( answer[0] == 'y' ) continue;
			else return;
				///----------------->>>
		}

		double x = 0.0;
		double y = 0.0;
		double theta = 0.0;
		double match = 0.0;

		x = t->fov_X();  /// locus of the object that has been seen. In the frame Field Of View (FOV), In pixels.
		y = t->fov_Y();
		theta = t->fov_theta();
		match = t->fov_match();

		cout << "FROM CAMERA:: X: " << x << " Y: " << y << " theta: " << theta << " match: " << match << endl;
		cout << "----------------------------------------------------------" << endl;

	}

	bool quit = false;
	do {

		double x = 0.0;
		double y = 0.0;
			///------------------------------------->>>
		target_to_arm(pattern_id,x,y);
			///------------------------------------->>>
		move_by_command(x,y);
			///------------------------------------->>>

		cout << "Repeat? "; cout.flush();
		cin >> controller;

		if ( controller[0] != 'q' ) {

			bool use_data = ( sg_use_simulations == SIM_OFF );

			if ( use_data ) {
				cout << "Use Data From Camera? " << endl;
				string answer;
				cin >> answer;
				if  ( answer[0] != 'y' ) use_data = false;
			}

			if ( !(use_data) || ( sg_use_simulations == SIM_ON ) ) {

				string var_set;
				cout << "list vars [X,Y,T,M]: " << endl;
				cin >> var_set;
							///---------------------------->>>
					///---------------------------->>>
				list<string> *var_list = to_items(var_set,',');
				list<string>::iterator lit;
				for ( lit = var_list->begin(); lit != var_list->end(); lit++ ) {
						///------------------>>
					string var = *lit;
					cout << ">>" << var << ": "; cout.flush();
					cin >> value;
					if ( var == "X" ) {
						test_x = atof(value.c_str());
					} else if ( var == "Y" ) {
						test_y = atof(value.c_str());
					} else if ( var == "T" ) {
						test_theta = atof(value.c_str());
					} else if ( var == "M" ) {
						test_match = atof(value.c_str());
					}
						///------------------>>
				}

				t->set_test_measurements(test_x, test_y, test_theta, test_match);

			}

		} else quit = true;

	} while ( !quit );

}




///======================================================================>>


void
target_to_arm_quiet(string pattern_id,double &x, double &y) {

	Target *t = sg_travel_map[pattern_id];
		///--------------------->>>>
	if ( t != NULL ) {

		double theta = 0.0;
		double match = 0.0;

		JointTransformation		transformer;
		SensorController		*s = t->sensor();

		x = t->fov_X();  /// locus of the object that has been seen. In the frame Field Of View (FOV), In pixels.
		y = t->fov_Y();
		theta = t->fov_theta();
		match = t->fov_match();

		t->fov_center_offset(x,y); 		/// (offset, scale) offset from center in the global scale determined by the sensor

		double elbow_translate_theta = sg_arm_construct->elbow_angle();
		double base_translate_theta = sg_arm_construct->base_angle();
			///-----------------------------------------------------------------

		double coord_rotate_1 = PI - elbow_translate_theta;
		transformer.rotate_2d(-coord_rotate_1,x,y);  /// transform the coordinate system to second ARM2 ... apply inverse to vectors
			///-----------------------------------------------------------------

		double coord_rotate_2 = PI/2 - base_translate_theta;
		transformer.rotate_2d(-coord_rotate_2,x,y);  /// transform the coordinate system to base ... apply inverse to vectors
			///-----------------------------------------------------------------

		/// The current coordinates of the center of focus are known in the base system

		x += sg_current_x;
		y += sg_current_y;

		/// Now X,Y are in the coordinates of the second arm part
		/// Get the goal into the same process... The goal (presumed) has already been scaled to the base scale.

		double goal_x = 0.0;
		double goal_y = 0.0;

		t->goal_offset(goal_x,goal_y);

		transformer.rotate_2d(theta,goal_x,goal_y);  /// transform the coordinate system to FOV ... apply inverse to vectors
		transformer.rotate_2d(-coord_rotate_1,goal_x,goal_y);  /// transform the coordinate system to second ARM2 ... apply inverse to vectors
		transformer.rotate_2d(-coord_rotate_2,goal_x,goal_y);  /// transform the coordinate system to base ... apply inverse to vectors

		x += goal_x;
		y += goal_y;

	} else {
		software_error("NULL target to target_to_arm",ARM_SOFT_ERROR_NULL_POINTER);
	}

}




void
clear_targets(void) {
	int n_targets = sg_target_storage.size();

	for ( int j = 0; j < n_targets; j++ ) {
			///---------------->>>
		Target *t = sg_target_storage[j];
		t->set_target_not_acquired();
			///---------------->>>
	}
}






Target *
stepper_by_commands_acquire(int motor,int steps, double &x_hat, double &y_hat) {
	/// Run stepper commands...

	step_n_motion(motor, steps);

	unsigned int count = 5;
	bool in_motion = true;
	sleeper(0,10000);

	Target *t_acquired = NULL;
	x_hat = 0.0;
	y_hat = 0.0;

	MotorDriver *md = sg_motor_driver_array[0];

	if ( sg_use_simulations == SIM_OFF ) {
		while ( in_motion ) {
				///
			in_motion = check_motion_status_quiet(motor);
				///
			t_acquired = point_in_time_best_match(x_hat,y_hat);
			if ( t_acquired != NULL ) {
				if ( in_motion ) {
					md->run_action("stop_motion");
				}
				break;
			}
			sleeper(0,100000);
		}
	} else {
		sleeper(0,1000000);
	}

	return(t_acquired);
}




void
scan_for_pattern_with_rotation_motion(void) {

			///----------------------------------->>
	double rad1,rad2;
			///
	clear_targets();
	stepper_by_command(1,0); // wait for motor 1 to zero
	stepper_by_command(2,0); // wait for motor 2 to zero
	///

	MotorDriver *md1 = sg_motor_driver_array[0];
	MotorDriver *md2 = sg_motor_driver_array[1];

	sleeper(1,10000);

	double end_r = sg_arm_construct->max_radius();
	double start_r = sg_arm_construct->start_radius();
	int n = (int)(end_r - start_r);

	n *= 2;  // make the steps smaller 

	int base_steps = (CANISTER_STEP/2.0);

	string range_factor_str = sg_server_parameters["FIELD_RANGE_FACTOR"];
	double range_factor = atof(range_factor_str.c_str());
	
	base_steps /= range_factor;

	double current_r = start_r;

	Target *t_acquired = NULL;

	double x = 0.0, y = 0.0;
	double x_hat = 0.0, y_hat = 0.0;

	double elbow_start = sg_arm_construct->elbow_current_steps();
	double base_start = sg_arm_construct->canister_current_steps();
	
	double extend_delta = (ELBOW_STEP-elbow_start)/(2*n);


	string field_arc_partions_str = sg_server_parameters["FIELD_ARC_PARTITIONS"];
	int m = atof(field_arc_partions_str.c_str());
	int base_steps_delta = base_steps/m;
	int step_pos = base_steps;

	stepper_by_command(1,base_steps); // wait for motor 1 to zero

	for ( int i = 0; i < n; i++ ) {
			///
		int elbow_steps = -(i*extend_delta + elbow_start);
			///
		stepper_by_command(2,elbow_steps);

		while ( step_pos > -base_steps ) {
			step_pos -= base_steps_delta;
			t_acquired = stepper_by_commands_acquire(1,step_pos,x_hat,y_hat);

			cout  << "<next> (" << x_hat << "," << y_hat<< ") ," << step_pos << " whence: " << i << "::" << n << endl;
			if ( t_acquired != NULL ) break;
			sleeper(1,10000);
								///----------------------------------->>
		}
		if ( t_acquired != NULL ) break;

			///---------------------------->>
			///---------------------------->>>

		i++;
		elbow_steps = -(i*extend_delta + elbow_start);
		stepper_by_command(2,elbow_steps);

		while ( step_pos < base_steps ) {
			step_pos += base_steps_delta;
			t_acquired = stepper_by_commands_acquire(1,step_pos,x_hat,y_hat);

			cout  << "<next> (" << x_hat << "," << y_hat<< ") ," << step_pos << " whence: " << i << "::" << n << endl;
			if ( t_acquired != NULL ) break;
			sleeper(1,10000);
								///----------------------------------->>
		}
		if ( t_acquired != NULL ) break;
			///
	}

	rad1 = md1->current_rotation();
	rad2 = md2->current_rotation();
	cout << "rad1: " << rad1 << " rad2: " << rad2; cout.flush();
								///----------------------------------->>
	sg_arm_construct->rotation_to_cartesian(-rad1,-rad2,x,y);
cout << " x: " << x << " y: " << y << endl;

	move_by_command(x,y);
	string pattern_id = t_acquired->parameter("id");
	target_to_arm_quiet(pattern_id,x,y);
	move_by_command(x,y);
	///

}


void
arm_to_home(void) {
	clear_targets();
	stepper_by_command(1,0); // wait for motor 1 to zero
	stepper_by_command(2,0); // wait for motor 2 to zero
}



void
application_cycle(void) { // At this point there is an engagement and this will result in calculations having to do with consumption.

}






bool
disengaged(void) {
	bool status = true;
	return(true);
}





void
issue_alerts(void) {
	
}






void
travel_home(void) {
	
}




void
shutdown(void) {
			//
	if ( sg_use_simulations == SIM_OFF ) {
		//
		sg_serial_port.Close();
		//
	}
			//
}





int
main( int argc, char** argv ) {

	if ( initialize_all( argc, argv ) ) {   /// Make sure motors and sensors are ready for operation.

		noisy_message("INITIALIZED");

		bool (* exec_option)(void);

		exec_option = to_target;
		string run_option = sg_server_parameters["run_mode"];

		if ( run_option == "application" ) {

			if ( (* exec_option)() ) {

				application_cycle(); // At this point there is an engagement and this will result in calculations having to do with consumption.

				while ( !disengaged() ) {
					issue_alerts();
				}

				travel_home();
			}
		} else {
			/// command mode option
			string command = "q";

			do {
				command = "";
					///--------->>
				cout << "#arm test operation#>>"; cout.flush();
				cin >> command;

				if ( command == "MP" ) {
					string pattern_id = "";
					cout << "Pattern ID:"; cout.flush();
					cin >> pattern_id;
					//-------------------->
					move_to_pattern(pattern_id);
					//-------------------->
				} else if ( command == "MR" ) {
					string pattern_id = "";
					cout << "Pattern ID (relative):"; cout.flush();
					cin >> pattern_id;
					//-------------------->
					move_relative_to_pattern(pattern_id);
					//-------------------->
				} else if ( command == "X" ) {
					//-------------------->
					report_arm_position();
					//-------------------->
				} else if ( command == "MV" ) {
					//-------------------->
					move_to_position();
					//-------------------->
				} else if ( command == "ML" ) {
					//-------------------->
					move_to_position_list();
					//-------------------->
				} else if ( command == "WL" ) {
					//-------------------->
					walk_a_line();
					//-------------------->
				} else if ( command == "R" ) {
					//-------------------->
					toggle_sensor_reports();
					//-------------------->
				} else if ( command == "T" ) {
					//-------------------->
					to_target();
					//-------------------->
				} else if ( command == "S" ) {
					//-------------------->
					scan_for_pattern();
					//-------------------->
				} else if ( command == "SPD" ) {
					scan_for_pattern_with_directed_motion();
				} else if ( command == "SPR" ) {
					scan_for_pattern_with_rotation_motion();
				} else if ( command == "TT" ) {
					transformation_tests();
				} else if ( command == "TA" ) {
					test_target_to_arm();
				} else if ( command == "HOME" ) {
					arm_to_home();
				}

			} while ( command[0] != 'q' && command[0] != 'Q' );

		}

	}

	shutdown();
	cout << "END OF PROCESSING" << endl;
}

