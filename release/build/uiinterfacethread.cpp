/***************************************************************************
 *   Copyright (C) 2010 by Copious Systems for Open Source   *
 *   richardaleddy@gmail.com   *

	Data Manager project :: CAN bus negotiations

 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <cc++/socket.h>
#include "uiinterfacethread.h"
#include "test_control.h"


#ifdef	CCXX_NAMESPACES
using namespace std;
using namespace ost;
#endif


#define SIM_ON true
#define SIM_OFF false


extern bool sg_use_simulations;
extern bool sg_use_tcp_with_simulation;

extern void command_from_ui_response_function(union sigval sv);

/// ---------  CURRENT PANEL
static string sg_current_panel = "MAIN";


/// TCP INTERFACE  --- done in constructor

bool
SysComInterface::initialize_tcp(map<string,string> &server_parameters,unsigned int port) {

	string ip_addr_str = server_parameters["address"];

	_port = port;

	string port_str = server_parameters["port"];
	IPV4Host visserver(ip_addr_str.c_str());

	_tcp = NULL;
#ifndef DEBUGGING_QUEUE
	if ( (sg_use_simulations == SIM_OFF) || sg_use_tcp_with_simulation ) {
		while ( _tcp == NULL ) {
			try {

	cout << "INITILIZE TCP " <<  ip_addr_str << ":" << _port << endl;
				_tcp = new TCPStream(visserver,_port);
	cout << "TCP GOING" << endl;

			} catch ( ... ) {
				if ( _tcp == NULL ) {
					cout << ".";
					cout.flush();
					{

						struct timespec tspec;
						tspec.tv_sec = 0;
						tspec.tv_nsec = 100*1000000L;

						int e = nanosleep(&tspec,NULL);

					}
				}
			}

		}

	} else {
		cout << "initialize_tcp: simulation: " << port_str << endl;
	}
#endif

}



/// QUEUE INTERFACE --- done in subclass constructor

bool
SysComInterface::initialize_queue(string q_name) {

	_queue_name = q_name;

	string qname = "/"; qname += q_name;

	_mq_io = mq_open(qname.c_str(),_use_flags);	/// create it as a blocking process.
	if ( _mq_io == (mqd_t)(-1) ) { _queue_name + " :  mq_open failed on " + _ui_direction + " queue..."; die(_queue_name.c_str()); }

	return(true);
}



// called by the start method of the notifier response subclass
bool
SysComInterface::reset_notifier(void) {

	_notifier.sigev_notify = SIGEV_THREAD;
	_notifier.sigev_notify_function = command_from_ui_response_function;
	_notifier.sigev_value.sival_ptr = (void *)g_user_interface_reader_thread;   /// Arg. to thread func.
	if ( mq_notify(_mq_io, &_notifier) == -1 ) { _queue_name + " : mq_open failed on setting up notifier..."; die(_queue_name.c_str()); }

	return(true);
}



void
SysComInterface::clear_out_queue(void) {
	struct mq_attr attr;

	if ( mq_getattr(_mq_io, &attr) == -1 ) die("mq_getattr");

	char buffer[256];
	memset(buffer,0,256);
	int nr = 0;
	while ( nr > -1 ) {
		nr = mq_receive(_mq_io, buffer, attr.mq_msgsize, NULL);  // This will respond even if the thread is blocked on a writer.

		if ( nr == -1 ) {
			if ( errno == EINTR ) continue;
			else if ( errno == EBADF ) {
				if ( (long int)nr == -1) die("mq_receive: bad file descriptor");
			} else if ( errno == EMSGSIZE ) {
				if ( nr <= (long int)sizeof(OutputMessage) ) continue;
			} else if ( errno == EAGAIN ) break;
		}
	}
}



#ifdef SIM_TEST_DEBUG
static int q_hits = 0;
#endif


bool
SysComInterface::data_queue_writer(OutputMessage *om) {
	if ( _ui_ready_for_data  ) {
		mqd_t result;

		struct timeval tv;
		gettimeofday(&(tv), NULL);

		om->abs_timeout.tv_sec += tv.tv_sec + 5;
		om->abs_timeout.tv_nsec = 1500;


#ifdef SIM_TEST_DEBUG
	q_hits++;
cout << "--" << q_hits << "--Writing to queue " <<  om->message << endl;
#endif


		result = mq_timedsend(_mq_io, (const char *)om, sizeof(OutputMessage), om->priority, &(om->abs_timeout));
		if ( result == (mqd_t)(-1) ) {
			/// what to do about the error
			status_message("data_queue_writer:: queue writer error");
			return(false);
		}

//cout << "WRITING TO DATA QUEUE RETURNS TRUE: " <<  om->message << endl;

	}

	return(true);
}





////////////////////////  READER



UIInterfaceReaderThread *g_user_interface_reader_thread = NULL;


#define INPUT_BUFFER_SIZE 2048


static OutputMessage sg_message_input[20];
static char sg_queue_response_buffer[INPUT_BUFFER_SIZE];

void                     // Thread start function
command_from_ui_response_function(union sigval sv) {
	struct mq_attr attr;
	ssize_t nr;
	void *input = NULL;

	UIInterfaceReaderThread *tmp_ui_th = (UIInterfaceReaderThread *)g_user_interface_reader_thread;

	if ( tmp_ui_th != NULL ) {
			///-------------------------------------------
		mqd_t mqdes = tmp_ui_th->queue_reader_descriptor();

		// Determine max. msg size; allocate buffer to receive msg

		if ( mq_getattr(mqdes, &attr) == -1 ) die("mq_getattr");
		    nr = 0;
		    while ( nr > -1 ) {

			memset((void *)sg_queue_response_buffer,0,INPUT_BUFFER_SIZE);

			nr = mq_receive(mqdes, (char *)sg_queue_response_buffer, attr.mq_msgsize, NULL);  // This will respond even if the thread is blocked on a writer.
			if ( nr != -1 ) {
			    ///----------------------------->
			    tmp_ui_th->handle_user_command((char *)sg_queue_response_buffer);
			    ///----------------------------->
			} else {
			    if ( errno == EINTR ) continue;
			    else if ( errno == EBADF ) {
				if ( (long int)nr == -1) die("mq_receive: bad file descriptor");
			    } else if ( errno == EMSGSIZE ) {
				if ( nr <= (long int)sizeof(OutputMessage) ) continue;
			    } else if ( errno == EAGAIN ) break;
			}
		    }

		tmp_ui_th->reset_notifier();
	} else {
		die("mq_receive:  no User Interface Thread In sigval");
	}
}




bool
UIInterfaceReaderThread::wait_acknowledge(void) {

	if ( _tcp == NULL ) return(false);

		if ( _tcp->isPending(Socket::pendingInput, 20000) ) {

		char buffer[TCP_BUFFER_SIZE];

		string server_input = "";
		*_tcp >> server_input;

#ifdef DEBUGGING
cout << server_input << endl;
#endif

	}

	return(true);
}



/// ---------  CURRENT PANEL
    /// -- tell the interfaces to start reporting to the panel...
void
UIInterfaceReaderThread::set_panel_parameters(string panel) {
		/// ------------------->>
	this->set_ui_receiving();
	g_user_interface_writer_thread->set_ui_receiving();

	cout << panel << endl;
}


void
UIInterfaceReaderThread::ui_shutdown(void) {
		/// ------------------->>
	this->set_ui_not_receiving();
	g_user_interface_writer_thread->set_ui_not_receiving();
}




char *
UIInterfaceReaderThread::command_transformer(char *tmp) {
	return(tmp);
}



void
UIInterfaceReaderThread::handle_user_command(char *buf) {
	/// Drive the message to the CAN Bus interface...
	/// Load message to the appropriate interface handler...

/// TYPE I
/// SELECT REPORT MODE ON THE CONTROLLER CARD...

/// BUCK STOPS HERE...  (DATA RETURNS THROUGH THE TCP/IP SERVER)


/// TYPE II
/// SEND DESIRED ACTUATION STATE OF AN EFFECTOR

	if ( strncmp(buf,PANEL_CHANGER_COM,6) == 0 ) {
		// panels
		char *tmp = buf + 6;

	/// ---------  CURRENT PANEL
		sg_current_panel = strdup(tmp);
		set_panel_parameters(sg_current_panel);

	} else if ( strncmp(buf,DATA_HANGUP_COM,6) == 0 ) {
		sg_current_panel = strdup("MAIN");
		ui_shutdown();
	} else if ( strncmp(buf,OBJECT_COM,6) == 0 ) {

		char *tmp = buf + 6;
		tmp = command_transformer(tmp);
		cout << tmp << endl;
		if ( _tcp != NULL ) {
			*_tcp << tmp << endl;
			wait_acknowledge();
		}

	} else {
		char *tmp = buf + 6;
		cout << tmp << endl;
	}


/*
#ifdef DEBUGGING
    if ( buf == NULL ) {
        // This is a user test branch.
        while ( true ) {

            string command_line;

cout << "HANDLE USER COMMAND == ENTER COMMAND: ";
cin >> command_line;
cout << "SENDING: "<< command_line << endl;

            if ( _tcp != NULL ) {
                *_tcp << command_line << endl;
                if ( strncmp(command_line.c_str(),"QUIT",4) == 0 ) break;
                wait_acknowledge();
            }

      }

    } else {
        //OutputMessage *om = (OutputMessage *)(buf);
        //char *tmp = om->message;
        cout << tmp << endl;
    }
#endif
*/

}





void
UIInterfaceReaderThread::completion(void) {

cout << endl << "UIInterfaceReaderThread completion" << endl;

	_data_handled_event->signal();
	_running = false;
}




/// ======================================================================================================================================================
/// ======================================================================================================================================================



////////////////////////  WRITER


UIInterfaceWriterThread *g_user_interface_writer_thread = NULL;





/// DATA INTEFACE THREAD  - Interface writes to the UI process
int sg_wait_tick = 10000;

data_handler_event_type
UIInterfaceWriterThread::wait_on_data_event(unsigned int &event_id,OutputMessage *data_region) {

	data_handler_event_type dhet = MAX_DATA_HANDLER_EVENTS;
	string server_input = "";

	if ( _tcp != NULL ) {
		if ( _tcp->isPending(Socket::pendingInput, 20000) ) {

			//char buffer[TCP_BUFFER_SIZE];

			*_tcp >> server_input;
			data_region->message[0] = 0;
			strcpy(data_region->message,server_input.c_str());

		}

	} else {

		if ( sg_use_simulations == SIM_ON ) {
			data_region->message[0] = 0;
			strcpy(data_region->message,"MT_ABC,3.9,4.9,(5.9,10),9.0,0.123");
		}

	}

	return(dhet);
}




string
UIInterfaceWriterThread::wait_on_data(void) {

	data_handler_event_type dhet = MAX_DATA_HANDLER_EVENTS;
	string server_input = "";

	if ( _tcp != NULL ) {
		if ( _tcp->isPending(Socket::pendingInput, 20000) ) {
			*_tcp >> server_input;

		}

	} else {

		if ( sg_use_simulations == SIM_ON ) {
			server_input = strdup("MT_ABC,3.9,4.9,(5.9,10),9.0,0.123");
		}

	}

	return(server_input);
}





void
UIInterfaceWriterThread::provider_acknowldege(void) {
	if ( _tcp != NULL ) {
		*_tcp << "OK" << endl;
	}
}




static char message_buffer[255];




bool
is_header(string token) {
	char c1 = token[0];
	char c2 = token[1];
	char c3 = token[2];

	if ( isalpha(c1) && isalpha(c2) && c3 == '_' ) return(true);
	return(false);
}


bool
complete_record(string prev_data) {
	return(is_header(prev_data));
}


extern void emit_to_target(string key,string data);

void
UIInterfaceWriterThread::emit_record(string data) {

	string key = data.substr(0,data.find(","));
	data = data.substr(data.find(",")+1);

	emit_to_target(key,data);

}




void
UIInterfaceWriterThread::run(void) {

		///---------------------------------->>
	_running = true;


	while ( _running ) {

		{
			sleep(100);
			//provider_acknowldege();
		}

			/// GET ONE MESSAGE AT A TIME (or wait for a new message to arrive.  (i.e. wait if queue is empty))
		string data = wait_on_data();

		data = trim_string(data);
		if ( data.size() == 0 ) continue;

		if ( data[data.size()-1] == ',' ) data = data.substr(0,data.size()-1);

		string sep = ",";

		while ( data.size() > 0 ) {
				///---------------------->>>
			string token = data.substr(0,data.find(','));
			if ( token == data ) {
				token = data;
				data = "";
			} else {
				data = data.substr(data.find(',')+1);
			}
				///---------------------->>>
			if ( is_header(token) ) {
				if ( _found_header ) {
					this->emit_record(_previous_partial);
				}
				_found_header = true;
				_previous_partial = token;
			} else if ( _found_header ) {
				_previous_partial += sep + token;
			}
				///---------------------->>>
		}

	}

cout << "DONE WITH ALL WRITING" << endl;

}



/// FORAMT MESSAGE FOR INTER PROCESS QUEUE


bool
UIInterfaceWriterThread::data_ready_to_ui(unsigned int event_id,OutputMessage *om) {

	om->action = ACTION_PANEL;
	return(data_queue_writer(om));

}



bool
UIInterfaceWriterThread::alert_to_ui(unsigned int event_id,OutputMessage *om) {

	om->action = ACTION_FAST_CONTROL;

//	ACTION_OVERRIDE_ALL,

	return(data_queue_writer(om));

}





/// ============================================================================

void
UIInterfaceWriterThread::completion(void) {
	_running = false;
	if ( _tcp != NULL ) {
		_tcp->disconnect();
		delete _tcp;
		_tcp = NULL;
	}
//	_data_interface->end_of_reading();
}


