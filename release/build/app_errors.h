

#ifndef _H_ARM_APPLICATION_ERRORS
#define _H_ARM_APPLICATION_ERRORS


typedef enum {

	ARM_SOFT_ERROR_NONE,
	ARM_SOFT_ERROR_NULL_POINTER,


	ARM_SOFT_ERROR_MAX

}  software_error_type;






extern bool 						sg_error_is_soft; 				/// true if software, false if hardware...
extern software_error_type			sg_soft_error;




static inline void
software_error(string error_msg,software_error_type sert) {
	sg_error_is_soft = true;
	sg_soft_error = sert;
	cout << error_msg << endl;
}








#endif

