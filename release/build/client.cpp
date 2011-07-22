
/// Open Source 2010

/// This program is based on an example for a TCP client from Open Source telecom.
/// This program, although somewhat different, still mentions the basis of departure
/// with deference to the GNU General Public License version 2.

/// By Copious Systems for Juelle Motors

// Copyright (C) 1999-2005 Open Source Telecom Corporation.
// Copyright (C) 2006-2008 David Sugar, Tycho Softworks.
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// As a special exception to the GNU General Public License, permission is
// granted for additional uses of the text contained in its release
// of Common C++.
//
// The exception is that, if you link the Common C++ library with other
// files to produce an executable, this does not by itself cause the
// resulting executable to be covered by the GNU General Public License.
// Your use of that executable is in no way restricted on account of
// linking the Common C++ library code into it.
//
// This exception does not however invalidate any other reasons why
// the executable file might be covered by the GNU General Public License.
//
// This exception applies only to the code released under the
// name Common C++.  If you copy code from other releases into a copy of
// Common C++, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
//
// If you write modifications of your own for Common C++, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.



#include <cc++/socket.h>
#include <iostream>
#include <cstdlib>

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <list>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
#include <sstream>

#include <signal.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <stdlib.h>


//#include "uiinterfacethread.h"

#include "arm_configuration.h"




#ifdef	CCXX_NAMESPACES
using namespace std;
using namespace ost;
#endif


//UIInterfaceReaderThread *g_user_interface_reader_thread;


extern map<string,string> 		sg_server_parameters;  // Set global
extern vector<Target *>			sg_target_storage;




///==================================================================


typedef void (*sighandler_t)(int);



unsigned int MESSAGE_WAIT_TOLLERANCE = 900;
unsigned int RESTFUL_QUERY_THRESHOLD = 2;


mqd_t g_mq_input = -1;
mqd_t g_mq_output = -1;



void
on_hangup(int sinum) {

	//g_user_interface_reader_thread->completion();
	g_user_interface_writer_thread->completion();
	//g_data_interface_thread->proper_shutdown();

}





extern UIInterfaceWriterThread *tcp_client_operation(SensorController *sensor,unsigned int port);
extern void start_sensing(UIInterfaceWriterThread *sensor_client);


void
start_sensing(UIInterfaceWriterThread *sensor_client) {

	if ( sensor_client != NULL ) {
		try {
			/// hold off on starting the thread exection.
			sensor_client->start();
		} catch(SockException& e) {
			cout << "Exception Thrown" << endl;
			cout << e.getString() << ": " << e.getSystemErrorString() << endl;
			exit(-1);
		}
	}

}



UIInterfaceWriterThread *
tcp_client_operation(SensorController *sensor,unsigned int port) {

	
	UIInterfaceWriterThread *sensor_client = NULL;

	try {

		string address = "";

		address = sensor->address();

		sensor_client = new UIInterfaceWriterThread(sensor->parameters(),port);

		Thread::setException(Thread::throwException);


//		cout << "timeout after 30 seconds inactivity, exiting" << endl;

	} catch ( ErrorOnQueue *eoq ) {
		cerr << eoq->message() << endl;
		return(NULL);
	}

//    cout << "Execution Finished" << endl;

	return(sensor_client);

}

