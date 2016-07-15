
#ifndef MY_CURL_H
#define MY_CURL_H

#include <unitypes.h>

/*
 * Public objects
 */

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

/*
 * Public functions
 */

struct mycurl_result *mycurl_do(
	char *url,
	struct mycurl_item *header_items,
	struct mycurl_item *query_items,
	char *post_data,
	long post_size,
	char *post_content_type,
	bool ascii_result
);

void mycurl_cleanup_if_needed();

#endif // MY_CURL_H