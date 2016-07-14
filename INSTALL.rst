Building and Installation Instructions
======================================

This document describes what we need in order to get `ec2seed` running from scratch,
including...

* What dependencies need to be provided.

* How to build the software.

* (Optionally) How to run tests.

* How to configure access to the AWS KMS.

* How to pass configuration to `ec2seed`.

* How to run `ec2seed`.

Dependencies
------------

This software has three dependencies that must be present before it will run:

* **Linux**.  Even though this is written in C, it uses Linux-specific functionality, and
so will not run (or easily build) on other OSes.

* **CURL** is used for all of our HTTP and HTTPS calls.  You will need a CURL that has
been built with SSL support.  The exact SSL library in use (OpenSSL, LibreSSL, or NSS)
doesn't matter, just so long as SSL support is there.

* **JSON-C** is used for parsing JSON from AWS.

When building, we need the headers for all the above dependencies.

If you are building directly from Git, you will need the `autotools` (including
`automake`) in order to build `configure` (and other related files).

Building
--------

In brief, the following sequence of commands should be enough to build this software:

::

	./configure
	make
	sudo make install

If there is no configure script in your distribution, that means that this came directly
from Git, so you will need to run `autoreconf -i -s` (or, if that doesn't work, then just
`autoreconf -i`) to generate the `configure` script (along with several other support
files).

To see all of the arguments that you can pass to `configure`, run `configure -h`.  There
aren't any options specific to this software, but there is one option that may be of
interest to packagers:

--prefix=some/path
	Use the specified path as the root for installing files.  Defaults to `/`.

Testing
-------

If you would like to run the test suite, run this command after building (in other words,
after running `make`):

::

	make check

You can also say `make test`, to support tools that like `make test` (such as
Travis-CI) instead of `make check` (which GNU Automake prefers).

The `ec2seed` command has two options which can be provided to aid testing:

--skip-aws
	Do not make any calls to AWS.  The entropy that would have been gathered from AWS
	is replaced with fake entropy.  This option automatically enables `--skip-metadata`.

--skip-metadata
	Do not make any calls to the metadata service.  The entropy that would have been
	gathered from the metadata service is replaced with fake entropy.  Also, the
	the environment variables `AWS_KEY_ID`, `AWS_SECRET_KEY`, and `AWS_REGION` are now
	required, unless `--skip-aws` is also specified.

.. warning::
	These options will seed fake entropy into a your operating system.  For that reason,
	if you are using these options, you **must** test in a dedicated VM, to avoid
	compromising OS randomness.

AWS Access
----------

In order to work, `ec2seed` needs to have access to the KMS `GenerateRandom` operation.
The best way to do this is to create an IAM Role (or use an existing Role) that provides
access.

To create a new role from scratch, follow these steps:

1. Create a new IAM Policy.  Use this policy document:

::

	{
	    "Version": "2012-10-17",
	    "Statement": [
	        {
	            "Effect": "Allow",
	            "Action": [
	                "kms:GenerateRandom"
	            ],
	            "Resource": [
	                "*"
	            ]
	        }
	    ]
	}

2. Create a new IAM Role (or open an existing Role) and attach the newly-created policy
to the role.

3. When you launch your instances, you will get an opportunity to attach an IAM Role to
the instance.  Select the role you created above.

When `ec2seed` runs, it queries the EC2 Metadata service, and will see the role.
`ec2seed` will then use those role credentials to call KMS.

Configuration
-------------

By default, `ec2seed` will get all of its information from the EC2 Metadata service, and
will find the IAM Role that you attached to the instance when you launched (created) it.
However, if you do not want to use an IAM Role, or you are trying to use this software
in an instance that does not have an IAM Role attached to it, you can provide credentials
using the following environment variables:

AWS_KEY_ID

This is the AWS key ID to use when accessing KMS.

AWS_SECRET_KEY

This the AWS secret key associated with `AWS_KEY_ID`.

.. warning::
	Passing secret data through environment variables can be unsafe!

AWS_REGION

This is the ID of the AWS region whose KMS you want to use.
If you do not specify a region, then `ec2seed` will use the region that it looked up
using the EC2 Metadata service.

.. caution::
	Although `ec2seed` can use any region (because it do not use any KMS keys),
	calling outside of your region will incur bandwidth charges.

Running
-------

`ec2seed` runs as a one-shot:  It is started, it gets random data, it seeds the OS,
and then it exits (preferably with exit code zero).

`ec2seed` works best when it is run as soon as the network comes on line, but before
OpenSSH (or any other network service) is started.  By running as soon as the network is
online, but before anything else starts, `ec2seed` is able to ensure that network
services have enough entropy available when they start.  This is particularly important
for OpenSSH, because it generates the system's host keys when it is first started.