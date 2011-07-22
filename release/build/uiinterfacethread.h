/***************************************************************************
 *   Copyright (C) 2010 by Copious Systems for Open Source   *
 *   richardaleddy@gmail.com   *

      This software is part of the car interface platform.
      In particular this is a subproject for the Data Manager.
      The Data Manager is the program for interfacing the CAN bus.

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

#ifndef UIINTERFACETHREAD_H
#define UIINTERFACETHREAD_H


#define TCP_PORT_CHECK 1
#include <common/parameters.h>


#define TCP_BUFFER_SIZE  2048


#include <cc++/socket.h>

#ifdef	CCXX_NAMESPACES
using namespace std;
using namespace ost;
#endif



//class DataInterface;

class UIInterfaceReaderThread;
class UIInterfaceWriterThread;  /// write to the user interface


extern UIInterfaceReaderThread *g_user_interface_reader_thread;
extern UIInterfaceWriterThread *g_user_interface_writer_thread;

/// =========================================================================================
/// =========================================================================================



class SysComInterface : public virtual status_reporter {

public:

	SysComInterface(map<string,string> &server_parameters,unsigned int port) {

		_port = 0;
		_tcp = NULL;
		_ui_ready_for_data = false;
		_address = "";
			///
		if ( initialize_tcp(server_parameters,port) ) {
			cout << "Socket Interface Ready" << endl;
		}

	}

    virtual ~SysComInterface(void) {}


protected:

	/// The queue reader reades the commands from the UI
	/// The queue writer writes allerts to the UI.

	bool				initialize_tcp(map<string,string> &server_parameters,unsigned int port);
	bool				initialize_queue(string q_name);
		///----------------------------------------------------->>>
	bool				data_queue_writer(OutputMessage *om);
        ///----------------------------------------------------->>>

	void				clear_out_queue(void);

public:

	bool 				reset_notifier(void);

	inline mqd_t			queue_reader_descriptor(void) { return(_mq_io); }
	inline mqd_t			queue_writer_descriptor(void) { return(_mq_io); }

	inline void 			set_ui_receiving(void) { _ui_ready_for_data = true; }
	inline void				set_ui_not_receiving() { _ui_ready_for_data = false; }


public:

	data_handler_event_type			_event_case;

	string                          _queue_name;
	mqd_t                           _mq_io;
	struct sigevent 				_notifier;

	int                             _use_flags;
	string                          _ui_direction;

	tpport_t                        _port;
	TCPStream                       *_tcp;

	bool                            _ui_ready_for_data;

	string 							_address;

};



/// =========================================================================================
/// =========================================================================================
/// =========================================================================================



// notifier response subclass

class UIInterfaceReaderThread : public virtual SysComInterface {

public:

	UIInterfaceReaderThread(map<string,string> &server_parameters,unsigned int port) : SysComInterface(server_parameters,port), _running(false)  {

		_queue_name = server_parameters["UI_READER_QUEUE"];

		_use_flags = O_RDONLY | O_NONBLOCK;
		_ui_direction = "READER";

		if ( initialize_queue(_queue_name) ) {
			cout << "Data Manager Process Reader Queue Ready" << endl;
		} else {
		    die("Queue initialization failed");
		}

		g_user_interface_reader_thread = this;
		_data_handled_event = new Event();

	}


	virtual ~UIInterfaceReaderThread() {
	}

	void                start(void) {
	    clear_out_queue();
	    this->reset_notifier();
	}


	void				handle_user_command(char *buf);

protected:

	void				completion(void);
	bool				wait_acknowledge(void);
	void				set_panel_parameters(string panel);
	void				ui_shutdown(void);
	char				*command_transformer(char *tmp);

protected:

//	DataInterface			    *_data_interface;

	data_handler_event_type			_event_case;
	bool					_running;
	Event					*_data_handled_event;


};





/// =========================================================================================
/// =========================================================================================
/// =========================================================================================



	/// The queue writer writes allerts to the UI.

class UIInterfaceWriterThread : public virtual Thread, public virtual SysComInterface {

public:

	UIInterfaceWriterThread(map<string,string> &server_parameters,unsigned int port) : SysComInterface(server_parameters,port), _running(false)  {
/*
		_queue_name = server_parameters["UI_WRITER_QUEUE"];

		_use_flags = O_WRONLY;
		_ui_direction = "WRITER";

		if ( initialize_queue(_queue_name) ) {
			cout << "Data Manager Process Writer Queue Ready" << endl;
		}
*/

		g_user_interface_writer_thread = this;

		_previous_partial = "";
		_found_header = false;

	}


	virtual ~UIInterfaceWriterThread(void) {

	}


public:

	//void 		                set_data_interface_thread(DataInterface *di) { _data_interface = di; }

	void		                run(void);
	void		                completion(void);

protected:

		///----------------------------------------------------->>>
	data_handler_event_type		wait_on_data_event(unsigned int &event_id,OutputMessage *om);
	string						wait_on_data(void);
		///----------------------------------------------------->>>
	bool						data_ready_to_ui(unsigned int event_id,OutputMessage *om);	/// Change panel most likely -- data taken through server

	bool						alert_to_ui(unsigned int event_id,OutputMessage *om);
							/// anything from innocent time stamp to critical errors -- updates via JavaScript on current panel, or window controls
							/// carries data in queue message.


	void			 			provider_acknowldege(void);
	void						emit_record(string data);

	
public:

	bool						target_present(void) { return(_target_present); }
	double						get_value(string value_key) { double m = _measurements[value_key]; return(m); }

protected:

	bool				        _running;
	string						_report_id;
	string						_reported_id;
	string						_report_fields;

	map<string,string>			_reported_values;
	map<string,double>			_measurements;
	
	bool						_target_present;
	bool						_target_was_present;


	string						_previous_partial;
	bool						_found_header;

};



class SensorController;
///-------------->>
extern UIInterfaceWriterThread *tcp_client_operation(SensorController *sensor,unsigned int port);



#endif
