 


#include <common/parameters.h>
#include "test_control.h"
#include <unistd.h>

#include <pthread.h>
#include <time.h>


/*

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int    pthread_cond_init(pthread_cond_t	  *cond,    pthread_condattr_t
*cond_attr);

int pthread_cond_signal(pthread_cond_t *cond);

int pthread_cond_broadcast(pthread_cond_t *cond);

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

int   pthread_cond_timedwait(pthread_cond_t   *cond,    pthread_mutex_t
*mutex, const struct timespec *abstime);

int pthread_cond_destroy(pthread_cond_t *cond);

	      int x,y;
	      pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
	      pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	      struct timeval now;
	      struct timespec timeout;
	      int retcode;

	      pthread_mutex_lock(&mut);
	      gettimeofday(&now);
	      timeout.tv_sec = now.tv_sec + 5;
	      timeout.tv_nsec = now.tv_usec * 1000;
	      retcode = 0;
	      while (x <= y && retcode != ETIMEDOUT) {
		      retcode = pthread_cond_timedwait(&cond, &mut, &timeout);
	      }
	      if (retcode == ETIMEDOUT) {
		      /* timeout occurred 
	      } else {
		      /* operate on x and y 
	      }
	      pthread_mutex_unlock(&mut);


*/

     

long int sim_step = 0;
long int sim_odo = 123455;

const char *gear_states[5];


double last_battery_percentage_report = 0.0;
double last_tempurature_report_avg = 0.0;

double
read_battery_stats(void) {
	
	FILE *fp = fopen("/home/juelle/data_manager/battery_report.txt","r");
	if ( fp != NULL ) {
		char filebuffer[20000L];
		memset(filebuffer,0,20000L);

		size_t cread = fread(filebuffer,20000L,1L,fp);
		fclose(fp);

		char *tmp = filebuffer;
		tmp = strstr(tmp, "percent");
		if ( tmp != NULL ) {
			tmp += 8;
			tmp = skip_white(tmp);
			float ff;
			sscanf(tmp,"%f,",&ff);
			last_battery_percentage_report = ff;
		}


		float t1 = 0.0;
		float t2 = 0.0;
		float t3 = 0.0;

		tmp = filebuffer;
		tmp = strstr(tmp, "1t.:");
		if ( tmp != NULL ) {
			tmp += 5;
			tmp = skip_white(tmp);
			tmp = skip_to_white(tmp);
			tmp = skip_white(tmp);
			sscanf(tmp,"%f,",&t1);
		}

		tmp = filebuffer;
		tmp = strstr(tmp, "9t.:");
		if ( tmp != NULL ) {
			tmp += 5;
			tmp = skip_white(tmp);
			tmp = skip_to_white(tmp);
			tmp = skip_white(tmp);
			sscanf(tmp,"%fF,",&t2);
		}

		tmp = filebuffer;
		tmp = strstr(tmp, "1t.:");
		if ( tmp != NULL ) {
			tmp += 5;
			tmp = skip_white(tmp);
			tmp = skip_to_white(tmp);
			tmp = skip_white(tmp);
			sscanf(tmp,"%fF,",&t3);
		}

		last_tempurature_report_avg = (t1 + t2 + t3)/3.0;


	}

	return(last_battery_percentage_report);
}



bool g_set_global_speed = false;
double sg_rspeed = 0.0;
double sg_tach = 0.0;
string sg_gear_label = "";

double sg_rspeed_next = 0;
double sg_tach_next = 0;

double sg_rspeed_step = 0;
double sg_tach_step = 0;



void
read_command_sims(void) {

	FILE *fp = fopen("/home/juelle/data_manager/command_sims.txt","r");
	if ( fp != NULL ) {
		char filebuffer[20000L];
		memset(filebuffer,0,20000L);

		size_t cread = fread(filebuffer,20000L,1L,fp);
		fclose(fp);

		char *tmp = filebuffer;

		float speed;
		float tach;
		char buffer[255];
		sscanf(tmp,"SPEED:%f,TACH:%f,GEAR:%s",&speed,&tach,buffer);

		g_set_global_speed = true;

		sg_rspeed_next  = speed;
		sg_tach_next = tach;
		sg_gear_label = strdup(buffer);

		if ( sg_rspeed != sg_rspeed_next ) {
			double diff = sg_rspeed_next - sg_rspeed;
			sg_rspeed_step = diff/10;
		}

		if ( sg_tach != sg_tach_next ) {
			double diff = sg_tach_next - sg_tach;
			sg_tach_step = diff/10;
		}

	}
}





char *
simulation_output(void) {

    static char buffer[1024];


    if ( sim_step == 0 ) {
        srand(45);
        gear_states[0] = "park";
        gear_states[1] = "reverse";
        gear_states[2] = "neutral";
        gear_states[3] = "drive";
        gear_states[4] = "overdrive";
    }


    if ( sg_rspeed_next != sg_rspeed ) {
		if ( ( sg_rspeed_step > 0 ) && ( sg_rspeed >= sg_rspeed_next) ) {
			sg_rspeed = sg_rspeed_next;
		} else if ( ( sg_rspeed_step < 0 ) && ( sg_rspeed <= sg_rspeed_next) ) {
			sg_rspeed = sg_rspeed_next;
		} else {
			sg_rspeed += sg_rspeed_step;
		}
	}

    if ( sg_rspeed_next != sg_tach ) {
		if ( ( sg_tach_step > 0 ) && ( sg_tach >= sg_tach_next) ) {
			sg_tach = sg_tach_next;
		} else if ( ( sg_tach_step < 0 ) && ( sg_tach <= sg_tach_next) ) {
			sg_tach = sg_tach_next;
		} else {
			sg_tach += sg_tach_step;
		}
	}


    if ( !(sim_step%3) ) {
		double rspeed = rand();
		if ( ! g_set_global_speed ) {
			rspeed = (rspeed/(1.0*RAND_MAX))*180;
		} else {
			rspeed = sg_rspeed;
		} 
		sprintf(buffer,"SOD { name : 'SPEED', speed: '%d' }",(int)(rspeed));
    }
    if ( !(sim_step%5) ) {
		double num = rand();
		if ( ! g_set_global_speed ) {
			num = (num/(1.0*RAND_MAX))*8000;
		} else {
			num = sg_tach;
		}
        sprintf(buffer,"SOD { name : 'TACH', rpm: '%d' }",(int)(num));
    }
    if ( !(sim_step%9) ) {

/*
		double num = rand();
		num = (num/(1.0*RAND_MAX))*250;
*/

        sprintf(buffer,"SOD { name : 'TEMP', degrees: '%f' }",last_tempurature_report_avg);
    }
    if ( !(sim_step%16) ) {
        double num = rand();
        num = (num/(1.0*RAND_MAX))*8;
        sprintf(buffer,"SOD { name : 'OIL', level: '%f' }",num);
    }
    if ( !(sim_step%17) ) {
        sprintf(buffer,"SOD { name : 'ODOMETER', distance: '%ld', units: 'mi'  }",sim_odo);
    }
    if ( !(sim_step%19) ) {
        sprintf(buffer,"SOD { name : 'TRIP_ODO', distance: '%ld', units: 'mi'  }",(sim_odo - 123455));
    }
    if ( !(sim_step%113) ) {
		string which_gear = "park";
		
  		if ( ! g_set_global_speed ) {
			double num = rand();
			num = (num/(1.0*RAND_MAX))*4;
			int i = (int)(num);
			which_gear = gear_states[i];
		} else {
			which_gear = sg_gear_label;
		}

        sprintf(buffer,"SOD { name : 'GEAR', state: '%s' }",which_gear.c_str());
    }
    if ( !(sim_step%10) ) {

		read_command_sims();

        double num = read_battery_stats();
        sprintf(buffer,"SOD { name : 'BATTERY', percent: '%f' }",num);
    }

    sim_step++;

//cout << sim_step << " " << buffer;
//useconds_t tt = 10000;

    struct timespec tspec;
    tspec.tv_sec = 0;
    tspec.tv_nsec = 100*1000000L;
    
    int e = nanosleep(&tspec,NULL);
   
//cout << " " << e << endl;

    return(buffer);
}


