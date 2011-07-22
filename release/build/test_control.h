
#ifndef TEST_CONTROL_H
#define TEST_CONTROL_H





//#define SIM_TEST_DEBUG 1
//#define DEBUGGING_QUEUE 1
#define DEBUGGING 1




extern char *simulation_output(void);




static inline void quasi_url_encode(char *str) {
	char buffer[2048];

	char *tmp = str;
	char *dst = buffer;

	memset(buffer,0,2048);
	while ( *tmp ) {
		if ( *tmp == '&' ) {
			strcat(dst,"&038;");
			dst += 6;
		} else if ( *tmp == '+' ) {
			strcat(dst,"&043;");
			dst += 6;
		} else if ( *tmp == ' ' ) {
			*dst++ = '+';
		} else {
			*dst++ = *tmp;
		}
		tmp++;
	}
	*dst = 0;
	strcpy(str,buffer);
}



static inline void quasi_url_decode(char *str) {
	char buffer[2048];

	char *tmp = str;
	char *dst = buffer;
	while ( *tmp ) {
		if ( *tmp == '&' ) {
			tmp++;
			if ( strncmp(tmp,"038",3) == 0 ) {
				*dst++ = '&';
			} else if ( strncmp(tmp,"043",3) == 0 ) {
				*dst++ = '+';
			} else {
				*dst = '?';
			}
			tmp += 3;
		} else if ( *tmp == '+' ) {
			*dst++ = ' ';
		} else {
			*dst++ = *tmp;
		}
		tmp++;
	}
	*dst = 0;
	strcpy(str,buffer);
}






#endif