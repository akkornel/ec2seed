bin_PROGRAMS = ec2seed
ec2seed_SOURCES = ec2seed.c my_curl.c
ec2seed_CFLAGS = $(JSON_CFLAGS) $(CURL_LIBS)
ec2seed_LDADD = $(JSON_LIBS) $(CURL_LIBS) $(LIBS)
test: check
