


#ifndef _ARM_CONFIGURATION_H_
#define _ARM_CONFIGURATION_H_

#include <iostream>
#include <unistd.h>
#include <cstdlib>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <queue>
#include <map>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>

#include <SerialStream.h>

#define ECHO_INI_FILE false

#include <common/parameters.h>

#include "uiinterfacethread.h"


using namespace std;
using namespace ost;
using namespace LibSerial;



#define SIM_ON true
#define SIM_OFF false


class MotorDriver;
class SensorController;
class Arm_Configuration;
class ParameterSink;
class Target;

/// ==============================================================================================>>>>>>>>


extern void run_commands(queue<string> *short_command_list,bool &going,SerialStream &serial_port);
extern void run_commands2(queue<string> *short_command_list, SerialStream &serial_port, ParameterSink *ps);


///=======================================================================================================

#define MAX_DRIVE_COUNT			200
#define MAX_SENSOR_COUNT		200



const double PI = 3.14159;
const double TWO_PI = PI*2;



		///----------------------->>
extern map<string,string> 		sg_server_parameters;  // Set global
extern SensorController			*sg_sensor_controller_array[MAX_SENSOR_COUNT];
		///----------------------->>
extern vector<Target *>			sg_target_storage;


extern bool sensor_interface_initializer(Target *t);


///=======================================================================================================




///=======================================================================================================
static SerialStream sg_serial_port;   /// Serial port for the motor interfaces.
///=======================================================================================================




class ParameterSink {

	public:

		ParameterSink(void) : _short_command_list(NULL) {
			_short_command_list = new queue<string>();
		}

		virtual ~ParameterSink(void) {}


	public:

		virtual void 				handle_command_response(string response) = 0;
		virtual void				process_parameters(void)  = 0;

		void	add_line(string line) {
					_short_command_list->push(line);
				}

		void	add_parameter(string key, string line) {
					_parameters[key] = line;
				}


		void	set_double_pair(string key,float &p1,float &p2) {
					string values = _parameters[key];
					sscanf(values.c_str(),"%f, %f",&p1,&p2);
				}

		bool	has_parameters(string param_key) {
					if ( _parameters.find(param_key) != _parameters.end() ) {
						return(true);
					}
					return(false);
				}

		map<string,string> 		&parameters(void) { return(_parameters); }

		string					parameter(string key) { return(_parameters[key]); }


		void set_descriptor(string goal_description) {
				//----------------------->>
			list<string> *items = NULL;
			list<string>::iterator lit;
				//----------------------->>
			{

				items = to_items(goal_description,';');

				if ( items != NULL ) {
					lit = items->begin();
					while ( lit != items->end() ) {
						string pardef = *lit++;
					//----------------------->>
						string key, term;
						split_string(pardef,key,term,':');
					//----------------------->>
						key = trim_string(key);
						term = trim_string(term);
					//----------------------->>
						this->add_parameter(key,term);
					//----------------------->>
					}
				}

				specialization();


			}
				//----------------------->>
		}

	protected:

		virtual void			specialization(void) {}


	protected:

		queue<string>				*_short_command_list;
		map<string,string>			_parameters;

};




/// ==============================================================================================>>>>>>>>



#define TOO_SMALL 0.005


class MotorDriver : public ParameterSink {

	public:

		MotorDriver(int index) : _index_str(""), _parameter("") {
			char buffer[255];

			sprintf(buffer,"%d",index);
			_index_str = strdup(buffer);

			_checking_status = true;
			_checking_position = false;
			_gear_ratio = 1.0;

		}

		virtual ~MotorDriver(void) {

		}

		MotorDriver(const MotorDriver &mdref)  :  _index_str(mdref._index_str), _parameter(mdref._parameter) {
		}

		MotorDriver &operator=(const MotorDriver &mdref)  {
			return(*this);
		}


		void	set_motor_parameter(string param_cmd,string data) {
				}


		void	run_lines(void) {

					while ( !(_short_command_list->empty() ) ) {
						run_commands2(_short_command_list,sg_serial_port,dynamic_cast<ParameterSink *>(this));
					}

				}


		void 	handle_command_response(string response) {

					_parameter = response;  //stop gap
					if ( _checking_status ) {
						cout << _parameter << endl;
					}

					if ( _checking_position ) {
						/// response should have steps after '='
						string nada = "";
						split_string(response,nada,_reported_steps,'=');
						_reported_steps = trim_string(_reported_steps);
					}

				}

		void	process_parameters(void)  {

					this->specialization();

					list<string> *tokens = NULL;
					list<string>::iterator lit;

					list<string> *commands = NULL;
					list<string>::iterator literal;
					static char *initializers = (char *)"read_power_up,set_run_parameters,read_reset";
					commands =  to_items(initializers,',');

					for ( literal = commands->begin(); literal != commands->end(); literal++ ) {
						string command = *literal;
								///--------------------------->>
							/// power up parameters :: READ
						string seq = _parameters[command];
						tokens = to_items(seq,';');
						if ( tokens != NULL ) {
							lit = tokens->begin();
							while ( lit != tokens->end() ) {
								string line = _index_str + *lit++;
								this->add_line(line);
							}
						}

						this->run_lines();

					}

				}


		void specialization(void) {
					string value = "";

					if ( has_parameters("gear_ratio") ) {
						value = _parameters["gear_ratio"];
						_gear_ratio = atof(value.c_str());
						if ( _gear_ratio < TOO_SMALL ) {
							_gear_ratio = 1.0;  // expect gears to be multipliers of the motor capability.
						}
					}

				}

			/// call up a macro

		void	run_action(string action_key) {

					list<string> *items = NULL;
					list<string>::iterator lit;

					{
						string seq = _parameters[action_key];
						items = to_items(seq,';');
						if ( items != NULL ) {
							lit = items->begin();
							while ( lit != items->end() ) {
								string line = _index_str + *lit++;
								this->add_line(line);
							}
						}

						this->run_lines();

					}

				}


		void	run_action(string action_key, string var, string value) {

					list<string> *items = NULL;
					list<string>::iterator lit;

					{

						string seq = _parameters[action_key];

						string var_marker = "{"; var_marker += var; var_marker += "}";

						seq = subst(var_marker,value,seq);

						items = to_items(seq,';');

						if ( items != NULL ) {
							lit = items->begin();
							while ( lit != items->end() ) {
								string line = _index_str + *lit++;
								this->add_line(line);
							}
						}

						this->run_lines();

					}

				}


		bool	check_status(unsigned int code = 10,bool check_noisy = true) {
			bool status = false;

			_checking_status = check_noisy;

				///--------------------->>
			string line = _index_str + "SC";
			this->add_line(line);
			this->run_lines();
				///--------------------->>
			//_checking_status = false;

			string code_str = _parameter.substr(_parameter.find("=") + 1);
			code_str = trim_string(code_str);
			if ( code_str == "0001" ) {
				status = true;
			}

			return(status);
		}


		double	current_rotation(void) {
			_checking_position = true;
			this->run_action("rotation_query");
			_checking_position = false;
			double steps = atof(_reported_steps.c_str());
			steps = steps/_gear_ratio;
			return(steps);
		}


		inline double gear_ratio(void) { return(_gear_ratio); }


	protected:

		string					_index_str;
		string					_parameter;

		bool					_checking_status;
		bool					_checking_position;

		string 					_reported_steps;

		double					_gear_ratio;

};



class UIInterfaceWriterThread;



class SensorController : public ParameterSink {

	public:

		SensorController(void) {
			_port_count = 1;
			_scale_x = 0.0;
			_scale_y = 0.0;
		}

		void	set_sensor_parameter(string param_cmd,string data) {
					cout << param_cmd << endl;
				}

	public:

		string					address(void) { if ( this != NULL ) { return(this->parameter("address")); } else { return(""); } }
		
		void 					handle_command_response(string response) { }
		void					process_parameters(void)  { }
		void					start_ports(void);

		void					scale(double &x, double &y) { x = _scale_x; y = _scale_y; }

		void					fov_center(double &x, double &y) { x = _fov_center_x; y = _fov_center_y; }

	protected:

		void					specialization(void) {

			string value = this->parameter("model");
			_product_model = value;
			value = this->parameter("address");
			_address = value;
			value = this->parameter("connect");
			_connect = value;
			value = this->parameter("base_port");
			_base_port = value;

			if ( _connect == "multiport" ) {
						///--------------------->>
				value = this->parameter("port_count");
				_port_count = atoi(value.c_str());

				unsigned int base_port = atoi(_base_port.c_str());
				for ( int i = 0; i < _port_count; i++ ) {
					int port = base_port + i;
					setup_port(port);
				}
						///--------------------->>
			}

			_scale_x = atof(this->parameter("scale_x").c_str());
			_scale_y = atof(this->parameter("scale_y").c_str());

			_fov_w = atof(this->parameter("fov_w").c_str());
			_fov_h = atof(this->parameter("fov_h").c_str());

			_scale_x = _scale_x/_fov_w;
			_scale_y = _scale_y/_fov_h;

			_fov_center_x = _fov_w/2;
			_fov_center_y = _fov_h/2;

		}


		void								setup_port(unsigned int base_port);


	protected:

		string											_product_model;
		string											_address;
		string											_connect;
		string											_base_port;

		unsigned int									_port_count;

		map< string , UIInterfaceWriterThread * >		_open_ports;

		double											_scale_x;
		double											_scale_y;
		double											_fov_w;
		double											_fov_h;

		double											_fov_center_x;
		double											_fov_center_y;

};



static double sg_current_x = 0.0;
static double sg_current_y = 0.0;



class Target : public ParameterSink {

	public:

		Target(void) {

			_x_offset_total = 0.0;
			_y_offset_total = 0.0;

			_id = "nada";
			_sensor = NULL;
			_level =  0;

			_target_was_present = false;
			_target_present = false;

			_reported_id = "";

			_fov_theta = 0.0;
			_fov_x = 0.0;
			_fov_y = 0.0;
			_fov_match = 0.0;

		}

		virtual ~Target(void) { }

		bool matches(void) {
			return(true);
		}


		inline double X(void) {
			return(_x_offset_total);
		}

		inline double Y(void) {
			return(_y_offset_total);
		}

				///------------------------->>
		inline double fov_X(void) { return(_fov_x); }
		inline double fov_Y(void) { return(_fov_y); }
				///------------------------->>
		inline double fov_theta(void) { return(_fov_theta); }
		inline double fov_match(void) { return(_fov_match); }

			///------------------------->>

		inline void set_test_measurements(double x, double y, double theta, double match) {
				///----------------->>
			_fov_x = x;
			_fov_y = y;
			_fov_theta = theta;
			_fov_match = match;
				///----------------->>
			_fov_theta = (_fov_theta/180.0)*PI;
		}

		void 				handle_command_response(string response) { }
		void				process_parameters(void)  { }

		string 				id(void) { return(_id); }

		SensorController	*sensor(void) { return(_sensor); }
		void				set_sensor(SensorController *s) { _sensor = s; }

		unsigned int		level(void) { return(_level); }

		void				set_sensor_interface(UIInterfaceWriterThread *si) { _sensor_interface = si; }

		bool 				acquired_target(void);
		void 				data_test(string data);
		bool				target_present(void) { return(_target_present); }

		void				fov_center_offset(double &x,double &y);
		void				goal_offset(double &x,double &y) { x = _goal_x; y = _goal_y; }

		void 				set_target_not_acquired(void) { _target_present = false; _target_was_present = false; }

	protected:

		void				specialization(void) {
				//
			string param_key = "offset_total(x,y)";
			this->set_double_pair(param_key,_x_offset_total,_y_offset_total);

			string value = this->parameter("level");
			_level = atoi(value.c_str());

			_id = this->parameter("id");
				//
			_report_id = this->parameter("report_id");
			_report_fields = this->parameter("fields");

			_fields = to_tokens(_report_fields);

			if ( _fields != NULL ) {
				list<string>::iterator lit;
				for ( lit = _fields->begin(); lit != _fields->end(); lit++ ) {
					string field_name = *lit;
					_reported_values[field_name] = "0";
					_measurements[field_name] = 0.0;
				}
			}
				//

			string goal_offset = this->parameter("goal_offset");
			{
				string xystr = goal_offset.substr(0,goal_offset.find(","));
				_goal_x = atof(xystr.c_str());
					///----------------------------->>>
				xystr = goal_offset.substr(goal_offset.find(",") + 1);
				_goal_y = atof(xystr.c_str());
			}

		}

		void				parse_data(string data_str);
		void				unpack_mapped_values(void);

	protected:

		string								_id;
		SensorController					*_sensor;
		unsigned int						_level;
		UIInterfaceWriterThread				*_sensor_interface;
		list<string>						*_fields;

		string								_report_id;
		string								_reported_id;
		string								_report_fields;

		map<string,string>					_reported_values;
		map<string,double>					_measurements;

		bool								_target_present;
		bool								_target_was_present;

		float 								_x_offset_total;
		float 								_y_offset_total;

		double								_fov_x;
		double								_fov_y;
				///------------------------->>
		double								_fov_theta;
		double								_fov_match;

		double 								_goal_x;
		double 								_goal_y;

};



#endif


