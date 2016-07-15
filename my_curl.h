
#ifndef MY_CURL_H
#define MY_CURL_H

#include <curl/curl.h>
#include <stdlib.h>
#include <unitypes.h>



//! Should the curl environment be initialized with SSL on?
enum mycurl_init_ssl {
	MYCURL_SSL_YES,
	MYCURL_SSL_NO
};

struct mycurl_item {
	char *item_name;
	char *item_value;
	struct mycurl_item *next;
};

struct mycurl_result {
	int code;
	uint8_t *body_utf8;
	char *body_ascii;
};

//! This is true if the global init is complete
static bool mycurl_active = 0;

//! This is a pointer to the active easy curl handle
static CURL *mycurl_handle;

//! The currently-active curl SSL mode
static enum mycurl_init_ssl mycurl_ssl_mode;

struct mycurl_result *mycurl_do(
	char * url,
	struct mycurl_item *header_items,
	struct mycurl_item *query_items,
	bool ascii_result
);

void mycurl_cleanup_if_needed();





#endif // MY_CURL_H