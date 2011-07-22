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



#define TCP_PORT_CHECK 1
#include <common/parameters.h>
#include "test_control.h"

using namespace std;

void
status_reporter::_status_message_wrapped(string message_command) {

	cerr << message_command << endl;

}



/*

*/



char *
OutputMessage::extract_service_type(char *tmp) {

	service_type = MAX_DATA_HANDLER_EVENTS;	// default to bipass.

	if  ( strncmp(tmp,"CEE",3) == 0 ) {
		service_type = DH_CRITICAL_ERROR_EVENT;
		action = ACTION_OVERRIDE_ALL;
	} else if  ( strncmp(tmp,"TIM",3) == 0 ) {
		service_type = DH_TIMER_EVENT;
		action = ACTION_OVERRIDE_ALL;
	} else if  ( strncmp(tmp,"FGE",3) == 0 ) {
		service_type = DH_FAST_GAUGE_EVENT;
		action = ACTION_FAST_CONTROL;
	} else if  ( strncmp(tmp,"FPC",3) == 0 ) {
		service_type = DH_FAST_PANEL_CHANGE;
		action = ACTION_FAST_CONTROL;
	} else if  ( strncmp(tmp,"SGE",3) == 0 ) {
		service_type = DH_SLOW_GAUGE_EVENT;
		action = ACTION_PANEL;
	} else if  ( strncmp(tmp,"EEV",3) == 0 ) {
		service_type = DH_ERROR_EVENT;
		action = ACTION_AJAX;
	} else if  ( strncmp(tmp,"SOD",3) == 0 ) {
		service_type = DH_STATUS_ON_DEMAND;
		action = ACTION_AJAX;
	} else if  ( strncmp(tmp,"SRP",3) == 0 ) {
		service_type = DH_STATE_REPORT;
		action = ACTION_PANEL;
	} else if  ( strncmp(tmp,"PAN",3) == 0 ) {
		service_type = DH_PANEL_CHANGE;
		action = ACTION_PANEL;
	}

	return(tmp+4);
}



void
OutputMessage::parse_data(void *data) {
	char *tmp = (char *)data;

	quasi_url_decode(tmp);

	tmp = extract_service_type(tmp);

	this->priority = 20 - (int)(this->service_type) + (int)(action);

	strcpy(message,tmp);
	msg_len = strlen(tmp);

cout << message << endl;

}


