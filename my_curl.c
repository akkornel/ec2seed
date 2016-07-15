#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unitypes.h>

#include "my_curl.h"

/*
 * Internal constants
 */

//! Initially allocate 1K for web pages
const size_t mycurl_initial_allocation = 1*1024;

/*
 * Private globals
 */

//! This is true if the global init is complete
static bool mycurl_active = 0;

//! This is a pointer to the active easy curl handle
static CURL *mycurl_handle;

//! The currently-active curl SSL mode
static enum mycurl_init_ssl mycurl_ssl_mode;

/*
 * Private structures
 */

//! This represents a UTF-8 web page.
/*!
 *  content: A pointer to the start of the content.
 *  content_end: A pointer to the NUL byte that ends the content.
 *  length_bytes: The length of the content, in bytes (NOT characters), including the
 *   terminating NUL byte.
 *  bytes_available: The amount of free memory remaining in the current allocation.
 */
struct mycurl_web_page {
	uint8_t *content;
	uint8_t *content_end;
	size_t length_bytes;
	size_t bytes_available;
};

/*
 * Private functions
 */

CURL * mycurl_init_if_needed(CURL *, enum mycurl_init_ssl);

struct mycurl_web_page *mycurl_web_page_init();

void mycurl_web_page_free(struct mycurl_web_page *);

static size_t mycurl_receive(void *, size_t, size_t, void *);

/*
 * Initialization & Cleanup Code
 */

//! Initialize curl, if we need to
/*!
 * \param curl_handle A pointer to an active curl handle.  Optional.
 * \param ssl_required Indicates if SSL will be needed in this request.
 * \returns The curl handle to use going forward.
 */
CURL *mycurl_init_if_needed(
	enum mycurl_init_ssl ssl_required
) {
	// If things are where we want them, don't do anything
	if (   (mycurl_active == 1)
		&& (mycurl_ssl_mode == ssl_required)
		&& (mycurl_handle != NULL)
	) {
		return mycurl_handle;
	}

	/* At this point, we need to do some work.  Here's what we'll do:
	 * 1. If curl is active, then clean it up.
	 * 2. Activate curl with the correct SSL state.
	 * 3. Get an easy curl handle, and return it.
	 */

	 // If curl is active, clear everything
 	mycurl_cleanup_if_needed();

	// Initialize the global curl environment, only enabling SSL if needed
	long curl_flags = CURL_GLOBAL_NOTHING;
	if (MYCURL_SSL_YES == ssl_required) {
		curl_flags ||= CURL_GLOBAL_SSL;
	}
	CURLcode curl_result = curl_global_init(curl_flags);
	if (curl_result != CURLE_OK) {
		return NULL;
	}
	mycurl_active = 1;

	// Initialize the easy curl handler
	mycurl_handle = curl_easy_init();
	if (curl_handle == NULL) {
		mycurl_cleanup_if_needed();
		return NULL;
	}
	
	// Update global settings and return
	mycurl_ssl_mode = ssl_required;
	return curl_handle;
}

//! Clean up curl, if needed
/*! 
 * This function is meant to clean up any existing state that might be lying around.
 * It is used by the automatic init code if we are changing SSL requirements.
 * At the very least, it must be called by the client before they exit.
 */
void mycurl_cleanup_if_needed() {
	if (mycurl_active == 0) {
		return;
	}
	// If we don't have a curl handle to free, then we might be leaking memory.
	if (mycurl_handle != NULL) {
		curl_easy_cleanup(curl_handle);
		mycurl_handle = NULL;
	}
	curl_global_cleanup();
	mycurl_active = 0;
	mycurl_ssl_mode = MYCURL_SSL_NO;
}

/*
 * Request-creating code
 */

//! Make a web request of some sort
/*!
 * \param ssl_mode The SSL mode to use.
 *
 * \param host_path The host, port (optional), and path.  NO protocol, '?', or query!

 * \param header_items Either NULL, or a pointer to the start of a list of header items.
 * You should be prepared for them to be reordered.
 *
 * \param query_items Either NULL, or a pointer to the start of a list of query
 * components.  You should be prepared to them to be reordered.
 *
 * \param post_data A pointer to any data to POST, or NULL if nothing to post.
 *
 * \param post_size The length of post_data, in bytes, or 0L if post_data is NULL.
 *
 * \param post_content_type The contents of the 
 *
 * \param ascii_result Set to true if the server's response should be converted to ASCII.
 * Be aware that this may not be possible.
 */
struct mycurl_result *mycurl_do(
	enum mycurl_init_ssl ssl_mode,
	char *host_path,
	struct mycurl_item *header_items,
	struct mycurl_item *query_items,
	char *post_data,
	long post_size,
	char *post_content_type,
	bool ascii_result
) {
	// Get our curl handle
	CURL *curl_handle = mycurl_init_if_needed(ssl_mode);
	if (curl_handle == NULL) {
		mycurl_cleanup_if_needed();
		return NULL;
	}

	// Set our protocol based on the SSL mode, and disable path manipulation
	if (ssl_mode == MYCURL_SSL_YES) {
		curl_easy_setopt(curl_handle, CURLOPT_DEFAULT_PROTOCOL, "https");
	} else {
		curl_easy_setopt(curl_handle, CURLOPT_DEFAULT_PROTOCOL, "http");
	}
	curl_easy_setopt(curl_handle, CURLOPT_PATH_AS_IS, 1L);

	// Allow redirects, and allow POSTing to redirects
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);

	// Set a vanity user-agent
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "karl-curl/1.0");

	// Make sure headers go to the server, not to proxies
	curl_easy_setopt(curl_handle, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);

	// If we have POST data, then add it now
	if (post_data != NULL) {
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post_data);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, post_size);
		#warning POST Content-Type header not yet added!
	}

	// Initialize our response object
	struct mycurl_web_page *web_page = mycurl_web_page_init();
	curl_set_easyopt(curl_handle, CURLOPT_WRITEFUNCTION, mycurl_receive);
	curl_set_easyopt(curl_handle, CURLOPT_WRITEDATA, (void *)&(handle.response_body));
	
	// Assemble our query items into the query string

	// Add our headers to the request
	
	// Make the request!	
}

//! Allocate and initialize a new struct mycurl_web_page
/*!
 * \returns A pointer to the initialized struct.  It's up to the client to free this when
 * it is no longer needed.
 */
struct mycurl_web_page* mycurl_web_page_init() {
	// Allocate space for our content handle
	struct mycurl_web_page *web_page;
	web_page = malloc(sizeof(struct mycurl_web_page));
	if (web_page == NULL) {
		return NULL;
	}

	// Allocate our initial content space
	size_t actual_initial_allocation = sizeof(uint8_t) * mycurl_initial_allocation;
	web_page.content = malloc(actual_initial_allocation);
	if (web_page.content == NULL) {
		return NULL;
	}

	// Set up the rest of the web page attributes
	web_page.content[0] = '\0';
	web_page.content_end = web_page->content;
	web_page.length_bytes = actual_initial_allocation;
	web_page.bytes_available = actual_initial_allocation - 1;

	// Ready to go!
	return web_page;
}


/*
 * Response-handling Code
 */

//! curl callback to read received data
/*!
 * \param contents A pointer to the contents received from the web server
 * \param item_size The size of each block of content.
 * \param item_count The number of content blocks received.
 * \param userp A pointer to a struct mycurl_web_page.
 * \returns The number of bytes processed.
 */
static size_t mycurl_receive(
	void *contents,
	size_t item_size,
	size_t item_count,
	void *userp
) {

	// Get our memory needs, and cast userp into what we need
	size_t memory_needed = item_size * item_count;
	struct mycurl_web_page *web_page = (struct mycurl_web_page *)userp;
	
	// Allocate more memory if needed
	if (memory_needed > web_page->bytes_available) {
		size_t new_size = web_page->length_bytes
						+ web_page->bytes_available
						+ memory_needed;
		uint8_t *new_content = realloc(web_page->content, new_size);
		
		if (new_content == NULL) {
			return 0;
		}
		elsif (new_content != web_page->content) {
			// A new allocation means updating the start AND end pointers
			web_page->content = new_content;
			web_page->content_end = &(web_page->content[web_page->length_bytes]);
		}
	}
	
	// Copy the data into place, starting at content_end.
	// Also add in a new NUL byte, since we just overwrote the old one.
	memcpy(web_page->content_end, contents, memory_needed);
	web_page->content_end[memory_needed+1] = '\0';
	
	// Update our length, and update the content_end pointer
	web_page->length_bytes += memory_needed;
	web_page->content_end = &(web_page->content[web_page->length_bytes]);
	
	// curl wants to know that we processed all that we got
	return memory_needed;
}