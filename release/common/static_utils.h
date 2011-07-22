/*

  static utils.h

  copyright (c) 2001 Copious Systems

*/


#ifndef _H_STATIC_UTILS_COPIOUS_2001_
#define _H_STATIC_UTILS_COPIOUS_2001_

//%$%  HEADERS
#ifdef _WINDOWS_
#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4503 )
#endif

#include <string>
#include <list>
#include <stack>
#include <vector>
#include <set>
#include <map>
#include <iostream>

#include <cstdlib>
#include <string.h>
#include <stdio.h>


using namespace std;

//%$% DEFS
#ifndef max
#define max(a,b) ( (a) > (b) ? (a) : (b) )
#define min(a,b) ( (a) < (b) ? (a) : (b) )
#endif
///////
#ifdef _DEBUG
#define c_abs(a) ( (a) < 0 ? (a & 0x7f) : (a) )
#else
#define c_abs(a) (a)
#endif

inline char *
ctr_strcdup(const char *x)
{
	char *_zz_stmp = (char *)x; 
	size_t sz = strlen(x);

	_zz_stmp = (char *)malloc(sz+1);
	strcpy(_zz_stmp,x);
	return(_zz_stmp);
}

//%$%
static int gs_tab_depth = 0;

static inline string
tabs()
{
	string front_tab = "";
	int i = 0;

	while ( ++i < gs_tab_depth ) {
		front_tab += "\t";
	}

	return(front_tab);
}

//%$%
static inline string
tabs(int depth)
{
	string front_tab = "";
	int i = 0;

	while ( ++i < depth ) {
		front_tab += "\t";
	}

	return(front_tab);
}

//%$%
static inline bool empty_line(char *line_buffer)
{
	char *tmp = line_buffer;

	while ( *tmp && isspace(c_abs(*tmp)) ) tmp++;
	if ( (*tmp == '\n') || !(*tmp) ) return(true);
	return(false);
}

//%$%
static inline bool empty_content(char *line_buffer)
{
	char *tmp = line_buffer;

	while ( *tmp && isspace(c_abs(*tmp)) ) tmp++;
	if ( !(*tmp) ) return(true);
	return(false);
}

//%$%
static inline char *end_line(char *str) {
	while ( *str && ( *str != '\n' ) ) {
		str++;
	}
	if ( *str ) str++;
	return(str);
}

//%$%
static inline string extract_line(char **line) {
	string sline;
	char *end = end_line(*line);
	end--;
	char c = *end;
	if ( c == '\n' ) *end = 0;

	sline = ctr_strcdup((char *)*line);

	if ( c == '\n' ) *end = c;
	*line = end + 1;

	return(sline);
}


//%$%
static inline char *
skip_white(char *tmp)
{
	while ( *tmp && isspace(c_abs(*tmp)) ) tmp++;

	return(tmp);
}

//%$%
static inline char *
skip_to_white(char *tmp)
{
	while ( *tmp && !isspace(c_abs(*tmp)) ) tmp++;

	return(tmp);
}

//%$%
static inline char *
skip_identifier(char *tmp)
{
	while ( *tmp ) {
		if ( isspace(c_abs(*tmp)) ) break;
		if ( (*tmp != '_') && !isalnum(c_abs(*tmp)) ) break;
		tmp++;
	}

	return( tmp );
}

//%$%
static inline char *
skip_to_char(char *tmp, char c )
{
	while ( *tmp ) {
		if ( *tmp == c ) return(tmp);
		tmp++;
	}

	return(NULL);
}


//%$%
static inline char *
skip_quoted(char *tmp, char c )
{
	if ( *tmp == c ) tmp++;
	while ( *tmp ) {
		tmp = skip_to_char(tmp,c);
		if ( tmp == NULL ) break;
		tmp--;
		if ( *tmp == ( c == '\"' ? '\'' : '\"') ) {
			tmp += 2;
		} else {
			tmp++;
			return(tmp);
		}
	}

	return(NULL);
}


//%$%
static inline char *
skip_to_char_or_white(char *tmp, char c )
{
	while ( *tmp ) {
		if ( *tmp == c ) return(tmp);
		if ( isspace(c_abs(*tmp)) ) return(tmp);
		tmp++;
	}

	return(NULL);
}


//%$%
static inline char *
skip_to_char_condition(char *tmp, char c, bool (*a)(char) )
{
	while ( *tmp ) {
		if ( *tmp == c ) return(tmp);
		if ( (*a)(*tmp) ) return(NULL);
		tmp++;
	}

	return(NULL);
}
//%$%
static inline char *
skip_white_and_star(char *tmp)
{
	while ( *tmp && (isspace(*tmp)  ||  (*tmp == '*')) ) tmp++;

	return(tmp);
}

//%$%
static inline char *
skip_to_white_or_star(char *tmp)
{
	while ( *tmp && !isspace(*tmp) && (*tmp != '*') ) tmp++;

	return(tmp);
}

//%$%
static inline string
extract_string(char *start, char *end)
{
	string str;
	start = skip_to_char(start,'\"');
	end = skip_to_char(end,'\"');
	end++;
	char c = *end;
	*end = 0;

	str = ctr_strcdup(start);
	*end = c;
	return(str);
}

//%$%
static inline bool
is_id_char(char c) {
	return( (c == '_') || isalnum(c_abs(c)) );
}

//%$%
static inline char *
clear_to_white(char *tmp)
{
	// write spaces until the test char is a space.
	while ( *tmp && !isspace(c_abs(*tmp)) ) *tmp++ = ' ';

	return(tmp);
}


//%$%
static inline void
upper_case(char *tmp)
{
	while ( *tmp ) {
		char c = *tmp;

		if ( c >= 'a' && c <= 'z' ) *tmp++ = toupper(c);
		else tmp++;
	}
}
//%$%
static inline void
upper_case(const char *tmp)
{
	char *utmp = (char *)tmp;
	upper_case(utmp);
}
//%$%
static inline void
capitalize(char *tmp)
{
	while ( *tmp ) {

		tmp = skip_white(tmp);
		if ( !(*tmp) ) break;
		char c = *tmp;

		if ( c >= 'a' && c <= 'z' ) {
			*tmp++ = toupper(c);
			tmp = skip_to_white(tmp);
		} else tmp = skip_to_white(tmp);

	}
}
//%$%
static inline void
lower_case(char *tmp)
{
	while ( *tmp ) {
		char c = *tmp;
		if ( c >= 'A' && c <= 'Z' ) *tmp++ = tolower(c);
		else tmp++;
	}
}
//%$%
static inline void
lower_case(const char *tmp)
{
	lower_case((char *)tmp);
}

//%$%
static inline void
lower_case(string &str)
{
	lower_case(str.c_str());
}

//%$%
static inline char *
skip_to_token(char *tmp)
{
	return(skip_white(skip_to_white(tmp)));
}
//%$%
static inline char *
trim_end(char *tmp)
{
	char *ender;
	ender = tmp;
	while ( *ender ) *ender++;
	*ender--;

	while ( (*ender == '\n') || isspace(c_abs(*ender)) ) {
		*ender-- = 0;
		if ( ender == tmp ) return(tmp);
	}

	return(ender);
}
//%$%
static inline char *
trim_from_end(char *lb, char *tmp)
{
	char *ender = tmp;

	while ( (*ender == '\n') || isspace(c_abs((unsigned char)(*ender))) ) {
		*ender-- = 0;
		if ( ender == lb ) return(lb);
	}
	return(ender);
}

//%$%
static inline char *
trim_front(char *buf)
{
	char *front = buf;
	char *tmp = buf;

	while ( *tmp && isspace(c_abs((unsigned char)(*tmp))) ) tmp++;
	while ( *tmp ) *front++ = *tmp++;
	*front = 0;
	if ( front > buf ) front--;
	return(front);
}


//%$%
static inline char *
trim(char *buf)
{
	trim_from_end(buf,trim_front(buf));
	return(buf);
}

//%$%
static inline string
trim_string(string data)
{
	data = ctr_strcdup(trim((char *)data.c_str()));
	return(data);
}



//%$%
static inline unsigned long int
mask(unsigned long int r)
{
	unsigned long int i = 0;
	unsigned long int out = 0;

	while ( i++ < r ) {
		out = (out << 1) | 1;
	}

	return(out);
}

//%$%
static inline bool
is_token(char *s, char *token, unsigned int token_len)
{
	if ( !strncmp((char *)s,token,token_len) ) {
		if ( !isspace(s[token_len]) ) {
			return(false);
		}
		return(true);
	}

	return(false);
}
//%$%
static inline bool
is_token(char *s, char *token, unsigned int token_len, unsigned char delim)
{
	if ( !strncmp((char *)s,token,token_len) ) {
		if ( !isspace(c_abs(s[token_len])) || ( s[token_len] == delim ) ) {
			return(false);
		}
		return(true);
	}

	return(false);
}

//%$%
static inline char *
remove_spaces(char *str)
{
	char *src, *dst;

	src = dst = str;

	while ( char c = *src ) {
		if ( isspace(c_abs(c)) ) src++;
		else *dst++ = *src++;
	}
	*dst = *src;

	return(str);
}


//%$%
static inline char *
replace_spaces(char *str, char rc)
{
	char *src, *dst;

	src = dst = str;

	while ( char c = *src ) {
		if ( isspace(c_abs(c)) ) *dst++ = rc, src++;
		else *dst++ = *src++;
	}
	*dst = *src;

	return(str);
}

//%$%
static inline char *
remove_character(char *str, char sc)
{
	char *src, *dst;

	src = dst = str;

	while ( char c = *src ) {
		if ( c == sc ) src++;
		else *dst++ = *src++;
	}
	*dst = *src;

	return(str);
}



//%$%
static inline string
unquote(string qs)
{
	string s = "";

	char *tmp = (char *)qs.c_str();
	tmp = remove_character(tmp,'\"');

	s = ctr_strcdup(tmp);
	return(s);
}



//%$%
static inline char *
replace_character(char *str, char sc, char rc)
{
	char *src, *dst;

	src = dst = str;

	while ( char c = *src ) {
		if ( c == sc ) *dst++ = rc, src++;
		else *dst++ = *src++;
	}
	*dst = *src;

	return(str);
}

//%$%
static inline bool
in_set(char c,char *sc)
{
	char *tmp = strchr(sc,c);
	if ( tmp == NULL ) return(false);
	else return(true);
}
//%$%
static inline char *
replace_characters(char *str, char *sc, char rc)
{
	char *src, *dst;

	src = dst = str;

	while ( char c = *src ) {
		if ( in_set(c,sc) ) *dst++ = rc, src++;
		else *dst++ = *src++;
	}
	*dst = *src;

	return(str);
}


//%$%
static inline string
replace_between(char c,char *replacer,string target)
{
	char buffer[256];

	buffer[0] = 0;
	char *next = (char *)target.c_str();

	while ( (next = strchr(next,c)) != NULL ) {
		strcat(buffer,replacer);
		next++;
	}

	string replaced = ctr_strcdup(buffer);
	return(replaced);
}

//%$%
static inline string
dir_depth_string(string dir_stem)
{
	return( replace_between('\\',(char *)"../",dir_stem) );
}
//%$%
static inline string
uri_depth_string(string dir_stem)
{
	return( replace_between('/',(char *)"../",dir_stem) );
}

/*
//%$%
static inline char *
normalize_nested_string(char *src, char qc, bool toggle)
{
        char c;

	if ( toggle ) {
		if ( qc == '\'' ) *src++ = '\"';
		else *src = '\'';
	}

	while ( c = *src ) {
		if ( c == qc ) break;
		if ( ( c == '\"' ) || ( c == '\'' ) ) {
			src = normalize_nested_string(src,c,toggle);
			while ( src < end_src ) *dst++ = *src++;
		} else *src++;
	}

	if ( toggle ) {
		if ( qc == ''\' ) *src++ = '\"';
		else *src++ = '\'';
	}

	return( src );
}
*/

//%$%
static inline char *
remove_spaces_special(char *str, char _tag, char stop)
{
	char *src, *dst;

	src = dst = str;

	while ( char c = *src ) {
		if ( (c > 0) && isspace(c_abs(c)) ) src++;
		else if ( c == _tag ) {
			while ( *src && (*src != stop) ) *dst++ = *src++;
		} else if ( ( c == '\"' ) || ( c == '\'' ) ) {
                        char *end_src = (char *)""; //normalize_nested_string(src,c,( c != '\'' ));
			while ( src < end_src ) *dst++ = *src++;
		} else *dst++ = *src++;
	}
	*dst = *src;
	return(str);
}


//%$%
static inline string
to_url(string title)
{
	char *str = replace_spaces((char *)title.c_str(),'_');
	lower_case(str);
	string url_str = str;
	return(url_str);
}

//%$%
static inline string
dir_concat(string dir, string sub_item)
{
	string deeper = dir;

	deeper += "/";
	deeper += sub_item;
	return(deeper);
}


//%$%
template<class T>
list<string> *
token_list(string s,T *divider)
{
	list<string> *l = new list<string>();

	T *tmp = (T *)(s.c_str());
	T *end = tmp + (s.size()/sizeof(T));
	T *prev = tmp;

	while ( tmp < end ) {
		if ( memcmp(tmp,divider,sizeof(T)) == 0 ) {
			if ( tmp > (prev + 1) ) {
				char buffer[128];
				memset(buffer,0,128);
				size_t sz = (size_t)(tmp) - (size_t)(prev);
				memcpy(buffer,prev,sz);
				l->push_back(ctr_strcdup(buffer));
			}
			prev = tmp + 1;
		}
		tmp++;
	}

	l->push_back(prev);
		
	return(l);
}



//%$%
static inline unsigned long int
count_lines(string file_list)
{
	char *tmp = (char *)(file_list.c_str());

	int n = 0;
	while ( *tmp ) {
		if ( *tmp == '\n' ) n++;
		tmp++;
	}
	return(n);
}

//%$%
static inline string 
single_subst(string key,string value,string form)
{
	long int where = (long int)form.find(key);
	if ( where >= 0 ) {
		size_t len = key.size();
		string o_page = form.replace(where,len,value);
		return(o_page);
	} else {
		return(form);
	}
}


//%$%
static inline string 
subst(string key,string value,string form)
{
	long int where = 0;
	long int prev_where = 0;
	string o_page = "";
	
	while ( where >= 0 ) {
		where = (long int)form.find(key,prev_where);
		if ( where >= 0 ) {
			size_t len = key.size();
			o_page = form.replace(where,len,value);
			form = o_page;
			prev_where = where + (long int)value.size();
		} else {
			return(form);
		}
	}
	
	return(o_page);
}


//%$%
static inline void
split_string(string data,string &name_token,string &accessor_token,char c)
{
	bool space_div = (c == ' ') ? true : false;

	char *tmp = ctr_strcdup(data.c_str());

	tmp = trim(tmp);
	char *next;
	char c_hat;

	if ( space_div ) {
		next = skip_to_white(tmp);
		if ( *next ) {
			c_hat = *next;
			*next = 0;

			name_token = ctr_strcdup(tmp);
			*next++ = c_hat;
			next = skip_white(next);
			if ( *next ) {
				accessor_token = ctr_strcdup(next);
			}
		} else {
			name_token = ctr_strcdup(tmp);
			accessor_token = "";
		}
	} else {
		next = strchr(tmp,c);
		if ( next == NULL ) {
			name_token =  ctr_strcdup(tmp);
			accessor_token = "";
			return;
		}

		if ( *next ) {
			c_hat = *next;
			*next = 0;

			name_token = ctr_strcdup(tmp);
			*next++ = c_hat;
			if ( *next ) {
				accessor_token = ctr_strcdup(next);
			} else accessor_token = "";
		} else {
			name_token = ctr_strcdup(tmp);
			accessor_token = "";
		}
	}

	delete tmp;

}

//%$%
static inline void
string_split_string(string data,string &name_token,string &accessor_token,string split_key)
{
	char *tmp = (char *)data.c_str();

	tmp = strstr(tmp,split_key.c_str());
	if ( tmp == NULL ) {
		name_token = data;
		accessor_token = "";
	} else {
		char c = *tmp;
		*tmp = 0;
		name_token = ctr_strcdup(data.c_str());
		*tmp = c;
		tmp += split_key.size();
		accessor_token = ctr_strcdup(tmp);
	}
}

//%$%
static inline void
split_string_quotes(string data,string &name_token,string &accessor_token,char c)
{
	char *tmp = (char *)data.c_str();

	tmp = skip_white(tmp);
	if ( *tmp == '\"' ) {
		char *end = strchr(tmp + 1,'\"');
		if ( end == NULL ) {
			split_string(data,name_token,accessor_token,c);
		} else {

			end++;
			char c = *end;
			*end = 0;
			string front = ctr_strcdup(tmp);
			*end = c;
			data = ctr_strcdup(end);

			split_string(data,name_token,accessor_token,c);

			front += name_token;
			name_token = front;

		}
	} else {
		split_string(data,name_token,accessor_token,c);
	}
}


//%$%
static inline void
split_string_end(string data,string &name_token,string &accessor_token,char c)
{
	bool space_div = isspace(c_abs(c)) ? true : false;

	char *tmp = ctr_strcdup(data.c_str());

	tmp = trim(tmp);
	char *next, *prev;
	char c_hat;

	if ( space_div ) {
		prev = tmp;
		do {
			next = skip_to_white(tmp);
			if ( !(*next) ) {
				prev = next;
				next = skip_white(tmp);
			}
		} while ( *next );

		if ( prev == tmp ) {
			name_token = ctr_strcdup(tmp);
			accessor_token = "";
		} else {
			c_hat = *prev;
			*prev = 0;
			name_token = ctr_strcdup(tmp);
			*prev++ = c_hat;
			prev = skip_white(prev);
			accessor_token = ctr_strcdup(prev);
		}

	} else {
		int pos = (int)data.find_last_of(c);
		if ( pos < 0 ) {
			name_token = tmp;
			accessor_token = "";
			return;
		} else {
			next = tmp + pos;
			c_hat = *next;
			*next = 0;

			name_token = ctr_strcdup(tmp);
			*next++ = c_hat;
			if ( *next ) {
				accessor_token = ctr_strcdup(next);
			}
		}
	}

	delete tmp;

}
//%$%
static inline void
front_string(string data,string &sfront,string divider)
{
	/////

	char *tmp = ctr_strcdup(data.c_str());

	tmp = trim(tmp);
	char *next;
	char c_hat;

	next = strstr(tmp,divider.c_str());
	//
	if ( next == NULL ) {
		sfront = tmp;
		return;
	}
	//
	if ( *next ) {
		c_hat = *next;
		*next = 0;

		sfront = ctr_strcdup(tmp);
		*next = c_hat;

	} else {
		sfront = ctr_strcdup(tmp);
	}
	//
}


// The second parameter is the directory part.
// the first part should contain the output stem
// plus subdirectories.
//%$%
static inline string
extract_topic(string dir,string o_dir_stem)
{
	char *tmp1;
	char *tmp2;

	tmp1 = (char *)(dir.c_str());
	tmp2 = (char *)(o_dir_stem.c_str());

	while ( *tmp1 == *tmp2 ) {
		if ( !(*tmp2) ) break;
		tmp1++;
		tmp2++;
	}
	tmp1++;

	string topic_str = ctr_strcdup(tmp1);

	return(topic_str);
}

//%$%
static inline bool
mark_section(char **begin,char **end,const char *tag_b,const char *tag_e)
{
	int b_tag_len = (int)strlen(tag_b);
	int e_tag_len = (int)strlen(tag_e);

	char *tmp = *begin;
	char *e = NULL, *b = NULL;

	int start_count = 0;

	while ( *tmp ) {
		if ( !strncmp(tmp,tag_b,b_tag_len) ) {
			start_count++;
			tmp += b_tag_len;
			if ( b == NULL ) b = tmp; // Mark the first one only.
		} else if ( !strncmp(tmp,tag_e,e_tag_len) ) {
			start_count--;
			if ( !start_count ) {
				e = tmp; // e is pointing at the start of the tag.
				break;
			}
			tmp += e_tag_len;
		} else tmp++;
		if ( start_count < 0 ) return(false);
	}

	if ( b == NULL ) return(false);
	if ( strlen(b) == 0 ) return(false);
	*begin = b;
	*end = e;

	return(true);
}



//%$%
template<class T>
static inline void
mark_section_same(T **begin,T **end,T *tag_eq,T *quote)
{
	int eq_tag_len = strlen(tag_eq);
	int q_len = strlen(quote);

	char *tmp = *begin;
	char *e = NULL, *b = NULL;

	bool in_form = false;

	while ( *tmp ) {
		if ( !memcmp(quote,tmp,q_len) ) {
			tmp += q_len;
			if ( !memcmp(tmp,tag_eq,eq_tag_len) ) {
				tmp += eq_tag_len;
			}
		} else if ( !memcmp(tmp,tag_eq,eq_tag_len) ) {
			in_form = !in_form;
			if ( !in_form ) {
				e = tmp;
				break;
			} else b = tmp;
			tmp += eq_tag_len;
		} else tmp++;
	}

	*begin = b;
	*end = e;
}



//%$%
static inline char *
extract_field(char *begin,char *p_name,char *e_p_name)
{
	char *nb = begin;
	char *ne;

	if ( !mark_section(&nb,&ne,p_name,e_p_name) ) return((char *)"");

	if ( nb > begin ) {
		char buffer[512];

		memset(buffer,0,512);
		char *tmp = buffer;

		while ( nb < ne ) *tmp++ = *nb++;
		*tmp = 0;
		trim(buffer);
		return(ctr_strcdup(buffer));
	} else {
		return((char *)"");
	}
}


//%$%
static inline char *
extract_large_field(char *begin,char *p_name,char *e_p_name)
{
	char *nb = begin;
	char *ne;

	if ( !mark_section(&nb,&ne,p_name,e_p_name) ) return((char *)"");

	if ( ne == NULL ) ne = nb + strlen(nb);

	if ( nb > begin ) {
		if ( *(nb - 1) != '>' ) {
			nb = skip_to_char(nb,'>');
			if ( *nb ) nb++;
		}
		size_t size = (size_t)(ne - nb) + 2;
		char *buffer = new char[size];

		memset(buffer,0,size);
		char *tmp = buffer;

		while ( nb < ne ) *tmp++ = *nb++;
		*tmp = 0;

		return(buffer);
	} else {
		return((char *)"");
	}
}

//%$%
static inline string
string_extract_field(string data,string tag)
{
	string e_tag_s = "</";
	e_tag_s += tag.substr(1);
	string fdata = extract_large_field((char *)data.c_str(),(char *)tag.c_str(),(char *)e_tag_s.c_str());
	return(fdata);
}



//%$%
static inline string
string_extract_field_with_tags(char **rest,string tag)
{
	string e_tag_s = "</";
	e_tag_s += tag.substr(1);

	char *nb = *rest;
	char *ne;

	char *p_name;
	char *e_p_name;

	p_name = (char *)tag.c_str();
	e_p_name = (char *)e_tag_s.c_str();

	*rest = NULL;

	string output = "";

	if ( !mark_section(&nb,&ne,p_name,e_p_name) ) return(output);

	if ( ne == NULL ) ne = nb + strlen(nb);

	if ( nb > *rest ) {
		nb -= tag.size();
		ne += e_tag_s.size();
		
		ne = skip_to_char(ne,'>');

		*rest = ne+1;

		size_t size = (size_t)(ne - nb) + 2;
		char *buffer = new char[size];

		memset(buffer,0,size);
		char *tmp = buffer;

		while ( nb < ne ) *tmp++ = *nb++;
		*tmp = 0;

		output = buffer;
	}

	return(output);
}

//%$%
static inline char *
extract_field_region(char *begin,char *end,char *p_name,char *e_p_name)
{
	end++;
	char c_prime = *end;
	*end = 0;

	char *name = extract_field(begin,p_name,e_p_name);

	*end = c_prime;
	return(name);
}

//%$%
static inline string
extract_xml_attribute(string attr_name,const char *start_tag)
{
	string tag_data;

	char *tmp = (char *)start_tag;
	char *end = strchr(tmp,'>');

	string result = "";
	do {
		*end = 0;
		tmp = strstr(tmp,attr_name.c_str());
		*end = '>';

		if ( tmp == NULL ) return(result);
		
		char *tst = tmp + attr_name.size();
		if ( *tst == '=' ) break;
		if ( isspace(c_abs(*tst)) ) break;

	} while ( true );


	tmp = skip_to_char(tmp,'\"');
	if ( !*tmp ) return(result);

	tmp++;
	end = strchr(tmp,'\"');
	if ( end == NULL ) return(result);

	*end = 0;
	result = ctr_strcdup(tmp);
	*end = '\"';

	return(result);
}


//%$%
static inline string
v_concat(vector<string> *sv,char c,int depth)
{
	string rslt = "";
	if ( sv->size() > 0 ) {
		for ( int i = 0; i <= depth; i++ ) {
			rslt += (*sv)[i];
			rslt += c;
		}
	}

	return(rslt);
}

//%$%
static inline string
remove_stem(string file, char c)
{
	int last_c = (int)file.find_last_of(c);
	if ( last_c == -1 ) return("");
	file.erase(last_c);
	return(file);
}
//%$%
static inline string
get_stem(string file, char c)
{
	int last_c = (int)file.find_last_of(c);
	if ( last_c >= 0 ) return(file.substr(last_c+1));
	else return("");
}

//%$%
static inline string 
extract_domain(string url_str)
{
	char *tmp = (char *)(url_str.c_str());
	tmp = strstr(tmp,"//");
	if ( tmp == NULL ) {
		return("");
	}

	tmp += 2;
	char *end = strchr(tmp,'/');
	if ( end != NULL ) *end = 0;

	string result = ctr_strcdup(tmp);
	if ( end != NULL ) *end = '/';

	return(result);
}
//%$%
static inline string
next_word(char *str)
{
	string word = "";
	str = skip_white(str);
	if ( !(*str) ) return(word);
	char *tmp = skip_identifier(str);
	char c = *tmp;
	*tmp = 0;
	word = ctr_strcdup(str);
	*tmp = c;
	return(word);
}


//%$%
static inline bool
no_space_allowed(char c)
{
	return( ( isspace(c_abs(c)) || (!isalnum(c_abs(c)) && (c != '_') && (c != '.') && (c != '[') && (c != ']')) ) ? true : false);
}
//%$%
static inline bool
no_space_allowed_star_ok(char c)
{
	return( ( isspace(c_abs(c)) || (!isalnum(c_abs(c)) && (c != '*') && (c != '_') && (c != '.') && (c != '[') && (c != ']')) ) ? true : false);
}


//%$%
extern char *strip_comments(char *buffer);
extern char *strip_hash_comments(char *buffer);

//%$%
static inline string
ito_str(unsigned long int i) {
	char buffer[24];
	sprintf(buffer,"%ld",i);
	string si = ctr_strcdup(buffer);
	return(si);
}



//%$%
static inline string
dto_str(double i) {
	char buffer[24];
	sprintf(buffer,"%lf",i);
	string si = ctr_strcdup(buffer);
	return(si);
}


//%$%
static inline string
pack_spaces(string query)
{
	string tsq = trim(ctr_strcdup(query.c_str()));

	char *tmp = (char *)tsq.c_str();
	char *dst = tmp;
	char *end = tmp + tsq.size();

	while ( tmp < end ) {
		if ( isspace(c_abs(*tmp)) ) {
			char *tst = skip_white(tmp);
			if ( !ispunct(*tst)|| (*tst == '&') ) {
				*dst++ = ' ';
			}
			tmp = skip_white(tmp);
		} else if ( *tmp == '\"' ) {
			*dst++ = *tmp++;
			while ( *tmp != '\"' ) {
				*dst++ = *tmp++;
			}
			*dst++ = *tmp++;
		} else if ( ispunct(*tmp ) ) {
			*dst++ = *tmp++;
			tmp = skip_white(tmp);
		} else *dst++ = *tmp++;
	}

	string result = ctr_strcdup(tsq.c_str());
	return(result);
}

//%$%
static inline char *
end_line_copy(char *str,string *saddr)
{
	// non safety checking.
	char buffer[1024];
	char *tmp = buffer;
	*tmp = 0;

	while ( *str ) {
		char c = *str++;
		if ( c == '\n' ) {
			*tmp++ = c;
			*tmp = 0;
			*saddr = ctr_strcdup(buffer);
			return(str);
		} else if ( c != '\r' ) {
			*tmp++ = c;
		}
	}

	return(NULL);
}

//%$%
static inline list<string> *
lines_to_list(char *data)
{
	string str;
	list<string> *ls = new list<string>();

	while ( (data = end_line_copy(data,&str)) != NULL ) {
		ls->push_back(str);
	}

	return(ls);
}



//___________________________________________________________
//===========================================================
//%$%
static inline string
token(char **str_ptr)
{
	char *tmp = skip_white(*str_ptr);
	char *end = skip_to_white(tmp);

	char c = *end;
	*end = 0;
	string rtoken = ctr_strcdup(tmp);
	*end = c;

	*str_ptr = end;
	return(rtoken);
}

//%$%
static inline string
idtoken(char **str_ptr)
{
	char *tmp = skip_white(*str_ptr);
	char *end = skip_identifier(tmp);

	char c = *end;
	*end = 0;
	string rtoken = ctr_strcdup(tmp);
	*end = c;

	*str_ptr = end;
	return(rtoken);
}

//%$%
static inline string
convert_month(string month)
{
	if ( month == "Jan" ) return("01");
	if ( month == "Feb" ) return("02");
	if ( month == "Mar" ) return("03");
	if ( month == "Apr" ) return("04");
	if ( month == "May" ) return("05");
	if ( month == "Jun" ) return("06");
	if ( month == "Jul" ) return("07");
	if ( month == "Aug" ) return("08");
	if ( month == "Sep" ) return("09");
	if ( month == "Oct" ) return("10");
	if ( month == "Nov" ) return("11");
	if ( month == "Dec" ) return("12");
	return("11");
}

//%$%
static inline bool
is_month(char *tmp)
{
	if ( isspace(*tmp) ) return(false);
	if ( *tmp == 'J' ) {
		if ( strncmp(tmp+1,"an",2) == 0 ) {
			return(true);
		}
		if ( strncmp(tmp+1,"un",2) == 0 ) {
			return(true);
		}
		if ( strncmp(tmp+1,"ul",2) == 0 ) {
			return(true);
		}
		return(false);
	} else if ( *tmp == 'A' ) {
		if ( strncmp(tmp+1,"ug",2) == 0 ) {
			return(true);
		}
		if ( strncmp(tmp+1,"pr",2) == 0 ) {
			return(true);
		}
		return(false);
	} else if ( *tmp == 'M' ) {
		if ( strncmp(tmp+1,"ay",2) == 0 ) {
			return(true);
		}
		if ( strncmp(tmp+1,"ar",2) == 0 ) {
			return(true);
		}
		return(false);
		//
	} else if ( *tmp == 'N' ) {
		return( strncmp(tmp+1,"ov",2) == 0 );
	} else if ( *tmp == 'F' ) {
		return( strncmp(tmp+1,"eb",2) == 0 );
	} else if ( *tmp == 'D' ) {
		return( strncmp(tmp+1,"ec",2) == 0 );
	} else if ( *tmp == 'S' ) {
		return( strncmp(tmp+1,"ep",2) == 0 );
	} else if ( *tmp == 'O' ) {
		return( strncmp(tmp+1,"ct",2) == 0 );
	}
	return(false);
}


//%$%
static inline void
extract_e_mail_date(string date_str,string &day, string &month, string &year, string &time_format)
{
	char *tmp = (char *)date_str.c_str();
	bool skip_set = false;

	while ( !isdigit(*tmp) && *tmp ) {
		if ( is_month(tmp) ) {
			month = token(&tmp);
			day = token(&tmp);
			year = token(&tmp);
			time_format = token(&tmp);
			skip_set = true;
			break;
		}
		tmp++;
	}

	if ( !skip_set ) {
		if ( !*tmp ) return;
		day = token(&tmp);
		month = token(&tmp);
		year = token(&tmp);
		time_format = token(&tmp);
	}

	// sometimes time and year are switched in input.

	int i = (int)year.find(':');
	if ( i > 0 ) {
		swap(year,time_format);
	}

	if ( day.size() == 1 ) day = "0" + day;
	month = convert_month(month);

}


//%$%
static inline string
e_mail_date_to_odbc_format(string date_str)
{
	string day;
	string month;
	string year;
	string time_format;

	extract_e_mail_date(date_str,day,month,year,time_format);

	string date_form = year + "-" + month + "-" + day + " " + time_format;

	return(date_form);
}


//%$%
static inline void
extract_date_integers(string date_str, int &i_day, int &i_month, int &i_year)
{
	string day;
	string month;
	string year;
	string time_format;

	extract_e_mail_date(date_str,day,month,year,time_format);

	i_day = atoi(day.c_str());
	i_month = atoi(month.c_str());
	i_year = atoi(year.c_str());

}


//%$%
static inline time_t 
extract_time_stamp(string date_str) {

	string date_part;
	string time_part;
	split_string(date_str,date_part,time_part,' ');

	string rest;
	string year;
	string month;
	string day;
	split_string(date_part,year,rest,':');
	split_string(rest,month,day,':');

	string hour;
	string min;
	string second;
	split_string(time_part,hour,min,':');
	split_string(rest,min,second,':');

	struct tm mystruct;
	
	mystruct.tm_sec = atol(second.c_str());         /* seconds */
	mystruct.tm_min = atol(min.c_str());         /* minutes */
	mystruct.tm_hour = atol(hour.c_str());        /* hours */

	mystruct.tm_mday = atol(day.c_str());        /* day of the month */
	mystruct.tm_mon = atol(month.c_str());         /* month */
	mystruct.tm_year = atol(year.c_str());        /* year */
	mystruct.tm_wday = 0;
	mystruct.tm_yday = 0;
	mystruct.tm_isdst = 0;

	time_t tt = mktime(&mystruct);
	return(tt);

}

//%$%
static inline string
token_quotes(char **str_ptr)
{
	char *tmp = skip_white(*str_ptr);
	char *end;
	
	if ( *tmp == '\"' ) {
		end = skip_quoted(tmp,'\"');
	} else end = tmp;
	end = skip_to_white(end);

	char c = *end;
	*end = 0;
	string rtoken = ctr_strcdup(tmp);
	*end = c;

	*str_ptr = end;
	return(rtoken);
}


//%$%
template<class T>
list<string> *
token_list_quotes(string s,T *divider)
{
	list<string> *l = new list<string>();

	T *tmp = (T *)(s.c_str());
	T *end = tmp + (s.size()/sizeof(T));
	T *prev = tmp;

	while ( tmp < end ) {
		if ( ( *tmp == '\"' ) || ( *tmp == '\'' ) ) {
			T mark[2];
			mark[0] = *tmp;
			mark[1] = 0;

			T quoter[2];
			quoter[0] = (T)('\\');
			quoter[1] = 0;

			mark_section_same(&tmp,&end,mark,quoter);

			if ( ( tmp != NULL ) && ( end != NULL ) ) {
				char buffer[128];
				memset(buffer,0,128);
				size_t sz = (size_t)(end) - (size_t)(tmp);
 				memcpy(buffer,prev,sz);
				l->push_back(strdup (buffer));

				tmp = prev = end + 1;
			} else break;

			//
		} else if ( memcmp(tmp,divider,sizeof(T)) == 0 ) {
			if ( tmp > (prev + 1) ) {
				char buffer[128];
				memset(buffer,0,128);
				size_t sz = (size_t)(tmp) - (size_t)(prev);
				memcpy(buffer,prev,sz);
				l->push_back(ctr_strcdup(buffer));
			}
			prev = tmp + 1;
		}
		tmp++;
	}

	if ( *prev ) l->push_back(prev);
		
	return(l);
}



//%$%
static inline void
output_string(string o_file,string formatted_data)
{
	FILE *fp = fopen(o_file.c_str(),"w");
	if ( fp != NULL ) {
		fprintf(fp,"%s",formatted_data.c_str());
		fflush(fp);
		fclose(fp);
	}
}



//%$%
static inline void
append_output_string(string o_file,string formatted_data)
{
	
	FILE *fp = fopen(o_file.c_str(),"a+");
	if ( fp != NULL ) {
		fprintf(fp,"%s",formatted_data.c_str());
		fflush(fp);
		fclose(fp);
	}
}


//%$%
static inline void
output_set(string o_file_path,set<string> *set_lines)
{
	if ( set_lines == NULL ) return;
////
	string output = "";
	set<string>::iterator sit;
	for ( sit = set_lines->begin(); sit != set_lines->end(); sit++ ) {
		string ll = *sit;
		output += ll+ "\n";
	}

	output.erase(output.find_last_of('\n'));

	output_string(o_file_path,output);
}



//%$%
static inline string
first_line(char *tmp)
{
	char *end = end_line(tmp);
	char c = *end;
	*end = 0;
	string line = ctr_strcdup(tmp);
	*end = c;
	return(line);
}

//%$%
static inline string
buffer_to_line(char *data)
{
	char *end = end_line(data);

	end--;
	char c = *end;
	*end = 0;

	string result = ctr_strcdup(data);

	*end = c;

	return(result);
}

//%$%
static inline string
padspace(string nstr, int pads)
{
	const char *tmp = nstr.c_str();
	char *end = skip_to_white((char *)tmp);

	int i = (int)(end - tmp);
	i = pads - i;
	while ( i-- ) {
		nstr = " " + nstr;
	}

	return(nstr);
}

//%$%
// hex representation

// Restore a value from a hex integer.
static inline unsigned char 
hex_to_i(char c)
{
	if ( c == '0' ) return(0);
	if ( c == ' ' ) return(0);
	if ( c == '1' ) return(1);
	if ( c == '2' ) return(2);
	if ( c == '3' ) return(3);
	if ( c == '4' ) return(4);
	if ( c == '5' ) return(5);
	if ( c == '6' ) return(6);
	if ( c == '7' ) return(7);
	if ( c == '8' ) return(8);
	if ( c == '9' ) return(9);
	if (( c == 'a' ) || ( c == 'A') ) return(10);
	if (( c == 'b' ) || ( c == 'B') )  return(11);
	if (( c == 'c' ) || ( c == 'C') )  return(12);
	if (( c == 'd' ) || ( c == 'D') )  return(13);
	if (( c == 'e' ) || ( c == 'E') )  return(14);
	if (( c == 'f' ) || ( c == 'F') )  return(15);
	return(0);
}

//%$%
static inline char
c_hex_conv(char c)
{
	short i = c;
	char base;
	if ( i < 10 ) {
		base = '0';
		base += i;
	} else {
		base = 'a';
		base += (i-10);
	}
	return(base);
}


//%$%
static inline void
c_to_hex(unsigned char c,char *dst)
{
	unsigned char c_low = c & 0x0F;
	unsigned char c_high = (c >> 4) & 0x0F;

	dst[0] = c_hex_conv(c_high);
	dst[1] = c_hex_conv(c_low);
}

//%$%
static inline string
hex_representation(char *p,int len)
{
	char *buffer = NULL;
	unsigned char *up = (unsigned char *)p;
	unsigned char *end = up + len;

	buffer = new char[2*len+1];
	memset(buffer,0,2*len+1);

	string hex_str = "";

	char *tmp = buffer;
	while ( up < end ) {
		unsigned char i = (unsigned char)(*up++);
		c_to_hex(i,tmp);
		tmp += 2;
	}
	hex_str = buffer;
	return(hex_str);
}


//%$%
static inline char *
from_hex_representation(char *str,int slen)
{
	int len = (slen >>= 1);
	int buffer_len = len + 1;

	unsigned char *bytes = new unsigned char[buffer_len];
	memset(bytes,0,buffer_len);

	unsigned char *tmp = bytes;
	unsigned char *end = bytes + len;

	while ( tmp < end ) {
		unsigned char i = hex_to_i(*str++);
		i <<= 4;
		i |= ( hex_to_i(*str++) & 0xF );
		*tmp++ = i;
	}

	return((char *)bytes);
}

//%$%
template<class T>
static inline string
as_hex(T i)
{
	int n = sizeof(T);
	T i_holder[1];

	i_holder[0] = i;
	char *bytes = (char *)(&i_holder[0]);
	string longstr = hex_representation(bytes,n);

	return(longstr);
}

//%$%
template<class T>
static inline T
from_hex(string s)
{
	T i_holder[1];
	i_holder[0] = 0;

	char *tmp = from_hex_representation((char *)s.c_str(),s.size());
	memcpy(i_holder,tmp,sizeof(T));
	delete tmp;

	T i = i_holder[0];
	return(i);

}


//%$%
template<class T>
static inline string
number_to_string(T n)
{
	char buffer[64];

	string result = ctr_strcdup(itoa(n,buffer,64));
	return(result);
}


//%$%
static inline list<string> *
to_lines(string page)
{
	char *tmp = (char *)page.c_str();

	list<string> *l_list = new list<string>();

	while ( *tmp ) {
		string line = extract_line(&tmp);
		line = trim(ctr_strcdup(line.c_str()));
		l_list->push_back(line);
	}

	return(l_list);
}


static inline list<string> *
to_no_empty_lines(string page)
{
	char *tmp = (char *)page.c_str();

	list<string> *l_list = new list<string>();

	while ( *tmp ) {
		string line = extract_line(&tmp);
		line = trim(ctr_strcdup(line.c_str()));
		if ( line.size() ) {
			l_list->push_back(line);
		}
	}

	return(l_list);
}




//%$%
static inline set<string> *
to_line_set(string page)
{
	char *tmp = (char *)page.c_str();

	set<string> *l_set = new set<string>();

	while ( *tmp ) {
		string line = extract_line(&tmp);
		line = trim(ctr_strcdup(line.c_str()));
		l_set->insert(line);
	}

	return(l_set);
}

//%$%
static inline list<string> *
to_nontrimmed_lines(string page)
{
	char *tmp = (char *)page.c_str();

	list<string> *l_list = new list<string>();

	while ( *tmp ) {
		string line = extract_line(&tmp);
		line = ctr_strcdup(line.c_str());
		l_list->push_back(line);
	}

	return(l_list);
}


//%$%
static inline string
input_entry(void)
{
	char buffer[256];
	cin.sync();
	buffer[0] = 0;
	cin.getline(buffer,128);
	string entry = ctr_strcdup(buffer);
	cin.clear();

	return(entry);
}

//%$%
static inline void
upperCase(char *tmp)
{
	while ( *tmp ) {
		if ( isalpha(c_abs(*tmp)) ) *tmp = toupper(*tmp);
		tmp++;
	}
}


//%$%
static inline void
upperCase(string &strData)
{
	char *tmp = (char *)strData.c_str();
	upperCase(tmp);
}


//%$%
static inline void
lowerCase(char *tmp)
{
	while ( *tmp ) {
		if ( isupper(c_abs(*tmp)) ) *tmp = _tolower(*tmp);
		tmp++;
	}
}

//%$%
static inline void
lowerCase(string &strData)
{
	char *tmp = (char *)strData.c_str();
	lowerCase(tmp);
}

//%$%
static inline char *
Capitalize(char *str)
{
	char *tmp = str;

	while ( *tmp ) {
		tmp = skip_white(tmp);
		if ( islower(c_abs(*tmp)) ) *tmp = _toupper(*tmp);
		tmp = skip_to_white(tmp);
	}

	return(str);
}

//%$%
static inline string
stringTrim(string strData)
{
	char *tmp = (char *)strData.c_str();

	trim_end(tmp);
	string result = tmp;

	return(result);
}


//%$%
static inline void
spaceToBar(char *str)
{
	while ( *str ) {
		if ( isspace(c_abs(*str)) ) {
			*str = '_';
		}
		str++;
	}
}

//%$%
static inline char *
replace_star(char *str, char replace)
{
	replace_character(str,'*',replace);
	return(str);
}

//%$%
static inline char *
replace_comma(char *str, char replace)
{
	replace_character(str,',',replace);
	return(str);
}

//%$%
static inline list<string> *
to_tokens(string token_str)
{
	list<string> *slist = new list<string>();

	while ( token_str.size() ) {
		string s_token;
		split_string(token_str,s_token,token_str,',');
		s_token = trim_string(s_token);
		slist->push_back(s_token);
	}

	return(slist);
}


//%$%
static inline list<string> *
to_items(string token_str,char itemDel)
{
	list<string> *slist = new list<string>();

	while ( token_str.size() ) {
		string s_token;
		split_string(token_str,s_token,token_str,itemDel);
		s_token = trim_string(s_token);
		slist->push_back(s_token);
	}

	return(slist);
}


//%$%
static inline void
to_items_vector(vector<string> &data_parts,string token_str,char itemDel)
{
	while ( token_str.size() ) {
		string s_token;
		split_string(token_str,s_token,token_str,itemDel);
		s_token = trim_string(s_token);
		data_parts.push_back(s_token);
	}
}


//%$%
static inline string
last_item(string itemstr,char c)
{
	char *tmp = (char *)itemstr.c_str();
	tmp = strrchr(tmp,c);
	string res = "";
	if ( tmp != NULL ) {
		tmp++;
		res = ctr_strcdup(tmp);
	} else res = itemstr;
	return(res);
}


//%$%
static inline int
count_tabs(string line_rep)
{
	const char *stmp = line_rep.c_str();
	int i = 0;

	while ( isspace(c_abs(*stmp)) ) {
		char c = *stmp++;
		if ( c == '\t' ) i++;
	}
	return(i);
}

//%$%
#ifndef line_item
typedef struct {
	int depth;
	string line;
} line_item;
#endif

static inline list<line_item> *
extract_indented_lines(char *data)
{
	char *tmp = data;

	list<line_item> *lines = new list<line_item>();

	while ( *tmp ) {

		string line = extract_line(&tmp);
		if ( !line.empty() ) {

			line_item line_rep;
			line_rep.depth = count_tabs(line);
			line_rep.line = trim_string(line);

			lines->push_back(line_rep);

		}
	}

	return(lines);
}

//%$%
static inline bool
isnumber(string nstr)
{
	const char *tmp = nstr.c_str();
	while ( *tmp ) {
		if ( (*tmp < '0') || (*tmp > '9') ) return(false);
		tmp++;
	}
	return(true);
}

/*
//%$%
template<class T>
static inline 
T *elt(list< T * > *li,int i)
{
	list< T * >::iterator lit = li->begin();
	while ( i > 0 ) {
		lit++;
		i--;
	}

	T &dat = *lit;
	return(dat);
}


template<typename T>
static inline 
T elt(list< T > *li,int i)
{
	list< T >::iterator lit = li->begin();
	while ( i > 0 ) {
		lit++;
		i--;
	}

	T dat = *lit;
	return(dat);
}
*/


//%$%
static inline void
remove_trailing_punctuation(string &str)
{
	char *tmp = (char *)str.c_str();
	tmp += str.size();
	while ( (*--tmp) && (tmp > str.c_str())) {
		char c = *tmp;
		if ( ispunct(c_abs(c)) ) *tmp = 0;
		else break;
	}

	str = ctr_strcdup(str.c_str());
}


//%$%
static inline bool
is_array_access(string accessor)
{
	const char *tmp = strchr(accessor.c_str(),'[');
	return ( tmp == NULL ? false : true );
}


//%$%
static inline void
string_adder(list<string> *dir_set,string d_path)
{
	remove_spaces((char *)d_path.c_str());
	if ( ( d_path[0] == '/' ) && ( d_path[1] == '/' ) ) return;
	dir_set->push_back(d_path);
}


//%$%
static inline string
chop(string str,char c)
{
	string s = str.substr(str.find(c) + 1);
	if ( s == str ) {
		return("");
	}
	return(s);
}

//%$%
static inline char *
line_factor_quotes(char *str)
{
	char *tmp = str;

	char *end = tmp;

	char c;
	while ( (c = *end) ) {
		if ( c == '\n' ) {
			*end = 0;
			break;
		}
		end++;
	}

	char end_state = c;

	char *last_q = (end - 1);

	while ( ( last_q > str ) && ( *last_q != '\"' ) ) last_q--;

	if ( last_q > str ) {
		//
		tmp = strchr(tmp,'\"');
		tmp++;

		while ( tmp < last_q ) {
			c = *tmp;
			if ( c == '\"' ) {
				*tmp = '\'';
			}
			tmp++;
		}
	}

	//
	//
	if ( end_state ) {
		*end = '\n';
		end++;
	}
	
	if ( !*end ) return(NULL);

	return(end);
}


//%$%
static inline void
factor_quotes(string jstring)
{
	char *tmp = (char *)jstring.c_str();

	while ( tmp != NULL ) {
		tmp = line_factor_quotes(tmp);
	}
}


//%$%
static inline string
clear_empty_lines(string interp_result)
{
	list<string> *lines = to_lines((char *)interp_result.c_str());


	string result = "";

	list<string>::iterator it;
	for ( it = lines->begin(); it != lines->end(); it++ ) {
		string line = *it;

		if ( empty_line((char *)line.c_str()) ) {
			continue;
		}

		trim_end((char *)line.c_str());
		line = ctr_strcdup(line.c_str());
		result += line + "\n";
	}

	return(result);
}


//%$%
static inline char *
remove_trailing_number(char *str)
{
	char *end = str + strlen(str);
	
	while ( (end > str) && isdigit(*(--end)) ) {
		*end = 0;
	}
	if ( ( *end == '-' ) || ( *end == '_' ) ) {
		*end = 0;
	}

	return(str);
}


//%$%
static inline bool
mark_section_and_middle(char **begin,char **end,const char *tag_b,const char *tag_e, char **middle, char *tag_m)
{
	int b_tag_len = (int)strlen(tag_b);
	int e_tag_len = (int)strlen(tag_e);
	int m_tag_len = (int)strlen(tag_m);

	char *tmp = *begin;
	char *e = NULL, *b = NULL, *m = NULL;

	int start_count = 0;

	while ( *tmp ) {
		if ( !strncmp(tmp,tag_b,b_tag_len) ) {
			start_count++;
			tmp += b_tag_len;
			if ( b == NULL ) b = tmp; // Mark the first one only.
		} else if ( !strncmp(tmp,tag_e,e_tag_len) ) {
			start_count--;
			if ( !start_count ) {
				e = tmp; // e is pointing at the start of the tag.
				break;
			}
			tmp += e_tag_len;
		} else if ( !strncmp(tmp,tag_m,m_tag_len) ) {
			if ( start_count == 1 ) {
				m = tmp; // e is pointing at the start of the tag.
			}
			tmp += m_tag_len;
		} else tmp++;
		if ( start_count < 0 ) return(false);
	}

	if ( b == NULL ) return(false);
	if ( strlen(b) == 0 ) return(false);
	*begin = b;
	*end = e;
	*middle = m;

	return(true);
}


template<class T> 
class accum_string : unary_function<T,void> {

public:

	accum_string( string sep, string ( * Op )(const T *) ) {
		accumlator = "";
		_Op = Op;
		_sep = sep;
		
	}

	string accumlator;

	void operator()(const T * pi) {
		accumlator += _sep + (* _Op)(pi);
	}

	string _sep;
	string ( * _Op )(const T *);

};

template<class T> 
class accum_string_ref : unary_function<T,void> {

public:

	accum_string_ref( string sep, string ( * Op )(const T &) ) {
		accumlator = "";
		_Op = Op;
		_sep = sep;
		
	}

	string accumlator;

	void operator()(const T &pi) {
		accumlator += _sep + (* _Op)(pi);
	}

	string _sep;
	string ( * _Op )(const T &);

};


template<class T>
static inline string
implode(string seperator,list<T *> olist, string ( * Op )(const T *),bool no_front_sep = true)
{
	string str = "";
	string sep = "";

	accum_string<T> accop(seperator,Op);

	accop = for_each(olist.begin(),olist.end(),accop);
	str = accop.accumlator;
	if ( ( no_front_sep ) && ( str.size() > 0 ) ) {
		str = str.substr(1);
	}
	return str;
}

template<class T>
static inline string
implode(string seperator,list<T> olist, string ( * Op )(const T &),bool no_front_sep = true)
{
	string str = "";
	string sep = "";

	accum_string_ref<T> accop(seperator,Op);

	accop = for_each(olist.begin(),olist.end(),accop);
	str = accop.accumlator;
	if ( ( no_front_sep ) && ( str.size() > 0 ) ) {
		str = str.substr(1);
	}
	return str;
}

template<class T>
static inline string
implode(string seperator,vector<T *> olist, string ( * Op )(const T *),bool no_front_sep = true)
{
	string str = "";
	string sep = "";

	accum_string<T> accop(seperator,Op);

	accop = for_each(olist.begin(),olist.end(),accop);
	str = accop.accumlator;
	if ( ( no_front_sep ) && ( str.size() > 0 ) ) {
		str = str.substr(1);
	}
	return str;
}

template<class T>
static inline string
implode(string seperator,vector<T> olist, string ( * Op )(const T &),bool no_front_sep = true)
{
	string str = "";
	string sep = "";

	accum_string_ref<T> accop(seperator,Op);

	accop = for_each(olist.begin(),olist.end(),accop);
	str = accop.accumlator;
	if ( ( no_front_sep ) && ( str.size() > 0 ) ) {
		str = str.substr(1);
	}
	return str;
}


template<class T>
static inline string
implode(string seperator,set<T> olist, string ( * Op )(const T &),bool no_front_sep = true)
{
	string str = "";
	string sep = "";

	accum_string_ref<T> accop(seperator,Op);

	accop = for_each(olist.begin(),olist.end(),accop);
	str = accop.accumlator;
	if ( ( no_front_sep ) && ( str.size() > 0 ) ) {
		str = str.substr(1);
	}
	return str;
}



template<class _I,class T> 
class map_accum_string : unary_function<T,void> {

public:

	map_accum_string( string sep, string ( * Op )(const T *) ) {
		accumlator = "";
		_Op = Op;
		_sep = sep;
		
	}

	string accumlator;

	void operator()(const pair<_I,T *> &pi) {
		accumlator += _sep + (* _Op)(pi.second);
	}

	string _sep;
	string ( * _Op )(const T *);

};


template<class _I,class T>
static inline string
map_implode(string seperator,map<_I,T *> omap, string ( * Op )(const T *),bool no_front_sep = true)
{
	string str = "";
	string sep = "";

	map_accum_string<_I,T> accop(seperator,Op);
	accop = for_each(omap.begin(),omap.end(),accop);

	str = accop.accumlator;
	if ( ( no_front_sep ) && ( str.size() > 0 ) ) {
		str = str.substr(1);
	}
	return str;
}


static inline string
timestring(void) {
	char outbuffer[200];
	time_t t;
	struct tm *tmp;
	
	t = time(NULL);
	tmp = localtime(&t);
	if ( tmp != NULL ) {
		if ( strftime(outbuffer, sizeof(outbuffer), "%F %T", tmp) == 0 ) {
			cerr << "Timer function error" << endl;
		}
	}

	string timeout = strdup(outbuffer);
	return(timeout);
}

static inline string
nullop(const string &s) {
	return(s);
}


static inline string
identifier_stem(string identifier) {

	static char *outset = (char *)"{}[]()!@#$%^&*-+=?/\\\"\',.";

	char *tmp = strdup(identifier.c_str());
	char *savetmp = tmp;

	while ( *tmp ) {
		char c = *tmp;
		if ( strchr(outset,c) != NULL ) {
			break;
		}
		tmp++;
	}

	string output = identifier;
	if ( *tmp ) {
		*tmp = 0;
		output = strdup(savetmp);
	}
	
	delete savetmp;
	return(output);
}

static inline bool
all_digits(string data) {
	const char *tmp = data.c_str();
	while ( *tmp && isdigit(*tmp) ) tmp++;
	if ( *tmp ) return(false);
	return(true);
}

static inline bool
isfloat(string data) {
	string manitessa = "";
	string decimal = "";

	split_string(data,manitessa,decimal,'.');

	if ( all_digits(manitessa) ) {
		string exponent = "";
		string decdata = decimal;
		char *tmp = (char *)strchr(exponent.c_str(),'+');
		if ( tmp != NULL ) {
			split_string(decdata,decimal,exponent,'+');
			if ( exponent.size() && ( exponent[0] == 'E' || exponent[0] == 'e' ) ) {
				exponent = exponent.erase(0);
			}
		} else {
			tmp = (char *)strchr(exponent.c_str(),'-');
			if ( tmp != NULL ) {
				split_string(decdata,decimal,exponent,'-');
				if ( exponent.size() && ( exponent[0] == 'E' || exponent[0] == 'e' ) ) {
					exponent = exponent.erase(0);
				}
			} else {
				tmp = (char *)strchr(exponent.c_str(),'E');
				if ( tmp != NULL ) {
					split_string(decdata,decimal,exponent,'E');
				}
			}
		}

		if ( all_digits(decdata) || all_digits(decimal) ) {
			return(true);
		}

	}
	return(false);
}

//%$% EXTERNAL
extern char *strip_comments(char *buffer);
extern char *strip_hash_comments(char *buffer);

//%$% FOOTER
#endif //_H_STATIC_UTILS_COPIOUS_2001_
