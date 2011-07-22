#ifndef _INCLUDED_PARAMETERS_common
#define _INCLUDED_PARAMETERS_common

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <functional>
#include <iostream>
#include <sstream>

#include <common/static_utils.h>
#include <iostream>
#include <sstream>
#include <list>


#define BUILD_SET_NOISY (true)

#define MAX_TITLE_SIZE 64

#ifndef ECHO_INI_FILE
#define ECHO_INI_FILE true
#endif


using namespace std;


inline bool read_parameters(map<string,string> &server_parameters, bool no_comments = false) {
	cout << "Reading Parameter File: parameters.ini" << endl;

	/// try loading a default configuration file...
	vector<string> command_list;

	char *default_server_parameters_file = (char *)"parameters.ini";

	FILE *fp = fopen(default_server_parameters_file,"r");

	if ( fp  != NULL ) {
			char filebuffer[20000L];
			memset(filebuffer,0,20000L);

		size_t cread = fread(filebuffer,20000L,1L,fp);
		if ( ECHO_INI_FILE ) {
			cout << "[" << cread << "] " << filebuffer << endl;
		}
		fclose(fp);

		list<string> *lines = to_no_empty_lines(filebuffer);

		list<string>::iterator lit;
		for ( lit = lines->begin(); lit != lines->end(); lit++ ) {

			string pardef = *lit;
			if ( no_comments && pardef[0] == '#' ) continue;

			string key, term;
			split_string(pardef,key,term,':');
			key = trim_string(key);
			term = trim_string(term);
			if ( term != "{" ) {
				server_parameters[key] = term;
			} else {
				term = "";
				string line = "";
				string sep = "";
				while ( line != "}" ) {
					lit++;
					line = *lit;
					line = trim_string(line);
					if ( line[0] == '#' ) continue;
					if ( line !=  "}" ) {
						term += sep + line;
						sep = ";";
					}
				}

				server_parameters[key] = term;
			}

		}

#ifdef TCP_PORT_CHECK
		string sport = server_parameters["TCP_PORT"];
		int port = atoi(sport.c_str());

		if ( port >= 1024 && port <= 9999 ) {
			cout << "Connecting at port " << port << endl;
		} else {
			cerr << "Port number needs to be between 1024 and 9999" << endl;
			exit(1);
		}
#endif
		lines->clear();
		delete lines;

	} else {
		cerr << "NO SUCH FILE: " << default_server_parameters_file << endl;
	}

	return(true);
}



//#define _XOPEN_SOURCE 600
#include <time.h>

#include <pthread.h>
#include <mqueue.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


#define MIN_USER_INPUT_MESSAGE_SIZE	(16)
#define MAX_TCP_MESSAGE_SIZE 		64000   /// a rather larg buffer for image frames.
	///-------------->>>


class ErrorOnQueue {

public:
	ErrorOnQueue(void) {}
	ErrorOnQueue(string s) { _message = s; }

	string message(void) {
		string errout = "Queue Management in Data Manager Process: " + _message;
		perror(errout.c_str());
		return(_message);
	}

protected:

	string _message;

};


inline void die(const char *msg) { perror(msg); throw new ErrorOnQueue(msg);  }


#define CMP_COM_LEN 32
#define PANEL_CHANGER_COM "PANEL:"
#define OBJECT_COM "OBJCT:"
#define STRING_COM "STRNG:"
#define DATA_HANGUP_COM "HNGUP:"


/// Data going outward from the CAN Bus



typedef enum {
	DH_CRITICAL_ERROR_EVENT,
	DH_TIMER_EVENT,
	DH_FAST_GAUGE_EVENT,
	DH_FAST_PANEL_CHANGE,
	DH_SLOW_GAUGE_EVENT,
	DH_ERROR_EVENT,
	DH_STATUS_ON_DEMAND,
	DH_STATE_REPORT,
	DH_PANEL_CHANGE,
	DH_PANEL_SIMPLE_MESSAGE,
	DH_CLIENT_CONTROL_CLICK,
	DH_CLIENT_CONTROL_SLIDE,
	DH_CLIENT_CONTROL_SET_MAX,
	DH_CLIENT_CONTROL_SET_MIN,
	DH_CLIENT_CONTROL_DISMISS,
	MAX_DATA_HANDLER_EVENTS
} data_handler_event_type;  /// POSSIBLE THAT FEEDBACK ON A DEVICE STATE MAY NEED TO BE HANDLED...


typedef enum {
	ACTION_PANEL,
	ACTION_AJAX,
	ACTION_FAST_CONTROL,
	ACTION_OVERRIDE_ALL,
	ACTION_UNDEFINED,
	ACTION_NEEDS_ACTUATION,
	ACTION_NEEDS_SET_VALUE,
	ACTION_NEEDS_REPORT,
	ACTION_NEEDS_ABORT,
	ACTION_MAX_ACTION_TYPES
} action_type;



class OutputMessage  {

public:

	OutputMessage() {
		service_type = MAX_DATA_HANDLER_EVENTS;
		action = ACTION_UNDEFINED;

			memset(message,0,MAX_TCP_MESSAGE_SIZE);
			msg_len = 0;
			priority = 0;
			abs_timeout.tv_sec = 0;
			abs_timeout.tv_nsec = 0;
			hit_count = 0;
			memset(this->panel,0,MAX_TITLE_SIZE);
			strcpy(panel,"PANEL");

	}

	OutputMessage(OutputMessage *message) {

		service_type = MAX_DATA_HANDLER_EVENTS;
		action = ACTION_UNDEFINED;
		abs_timeout.tv_sec = 0;
		abs_timeout.tv_nsec = 0;

		this->expected_length = message->expected_length;
		this->msg_len = message->msg_len;

		memset(this->message,0,MAX_TCP_MESSAGE_SIZE);
		memcpy(this->message,message->message,message->msg_len);

		this->source_string = message->source_string;

	}

public:

	void 			parse_data(void *data);
	char			*extract_service_type(char *tmp);

	bool			append_data(char *data,int length) {


						if ( MAX_TCP_MESSAGE_SIZE > (msg_len + length) ) {

							if ( empty() ) {
								expected_length = (* calc_expected_length)(data);
							}

							char *tmp = &message[msg_len];
							memcpy(tmp,data,length);
							msg_len += length;
						}

						return(true);
					}

	void			clear_data(void) {
						memset(this->message,0,MAX_TCP_MESSAGE_SIZE);
						msg_len = 0;
					}

	bool			empty(void) { return( msg_len == 0 ); }

	size_t			length(void) { return(this->msg_len); }

	void			set_expected_length_calculator(int (* cel)(char *)) {  calc_expected_length = cel;  }
	
	void			set_source(string s1, string s2) { source_string = s1; source_string += ":"; source_string += s2; }


public:

	data_handler_event_type		service_type;
	action_type					action;

	char						message[MAX_TCP_MESSAGE_SIZE];
	char						panel[MAX_TITLE_SIZE];

	size_t						msg_len;
	size_t						expected_length;

	unsigned int				priority;
	struct timespec				abs_timeout;
	unsigned int				hit_count;

	int							(* calc_expected_length)(char *);

	string						source_string;
	

};


class status_reporter {

public:

	status_reporter(void) {
		_noisy = BUILD_SET_NOISY;
	}

	void set_noisy(bool state) { _noisy = state; }

	inline void status_message(string message_command) {
		if ( _noisy ) _status_message_wrapped(message_command);
	}

protected:

	void _status_message_wrapped(string message_command);

	bool			_noisy;


};



#endif


