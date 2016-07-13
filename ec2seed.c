#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <linux/random.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

/* 
 * Program static configuration.
 */
 
const char *RANDOM_PATH = "/dev/urandom";

const int RANDOM_COUNT = 1024;

/*
 * Functions that we'll be calling later.
 */

int entropy_available(const int);

/*
 * Main code!
 */

int main (
) {
	// Open the random device for writing
	// Let's do this now, just in case things fail.
	int random_fd;
	random_fd = open(RANDOM_PATH, O_WRONLY);
	if (random_fd == -1) {
		perror("Error opening random file");
		return 1;
	}

	// Capture the amount of entropy available before seeding
	int entropy_before = entropy_available(random_fd);
	if (entropy_before == -1) {
		perror("Error getting current entropy\n");
	}
	
	// Create a pool of fake entropy
	void *fake_entropy = malloc(RANDOM_COUNT);
	if (fake_entropy == NULL) {
		perror("Unable to allocate memory for fake entropy");
		return 1;
	}
	memset(fake_entropy, RANDOM_COUNT, 255);

	// Create our entropy struct
	/* Fun fact: The buffer is declared to be an array of 32-bit units, but
	 * in code it gets treated as an array of bytes.
	 * To be safe, make sure that our amount of entropy is divisible by 32.
	 */
	struct rand_pool_info *entropy;
	assert((RANDOM_COUNT) % sizeof(__u32) == 0);
	entropy = malloc(sizeof(struct rand_pool_info) + RANDOM_COUNT);
	if (entropy == NULL) {
		perror("Unable to allocate memory for the entropy pool");
		return 1;
	}
	
	// Set up our entropy struct fields
	entropy->entropy_count = RANDOM_COUNT * 8;
	entropy->buf_suze = RANDOM_COUNT;
	
	// Fill the remaining space with our entropy
	memcpy(rand_pool_info->buf, fake_entropy, RANDOM_COUNT);
	
	// Add the entropy to the kernel
	int add_result;
	add_result = ioctl(random_fd, RNDADDENTROPY, entropy);
	if (add_result == -1) {
		perror("Error adding entropy to kernel");
		return 1;
	}

	// Capture the amount of entropy after seeing
	int entropy_after = entropy_available(random_fd);
	if (entropy_after == -1) {
		perror("Error getting current entropy\n");
	}

	// All done!
	printf("Random entropy seeding %d->%d\n", entropy_before, entropy_after);
	close(random_fd);
	return 0;
}

/*
 * Support Functions
 */

/* entropy_available: 
   fd: The file descriptor pointing to a random device.
*/
int entropy_available (
	const int fd
) {
	int num_bytes, ioctl_result;
	
	ioctl_result = ioctl(fd, RNDGETENTCNT, &num_bytes);
	if (ioctl_result == -1) {
		return ioctl_result;
	}
	
	return num_bytes;
}