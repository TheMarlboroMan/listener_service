# listener_service

A reusable listener service in C++ which can be injected with arbitrary logic.

## Dependencies

This application depends on the libraries located at:

	https://github.com/TheMarlboroMan/log
	https://github.com/TheMarlboroMan/tools 

Be sure to build them first!. If you find yourself having problems, maybe you need earlier versions (actually, I would need to update this).

## Building

Build these libraries:

- https://github.com/TheMarlboroMan/log
- https://github.com/TheMarlboroMan/tools 

Take good note of their paths and pass them along to make like:

make all EXT_INCLUDES="-I ../log -I ../tools" EXT_LINK="-L ../log -L ../tools"

# Building with SSL

If you are bold enough, you may want to build with SSL in which case you must
use the WITH_SSL flag and define the variable WISH_SSL when building, like so:

make all EXT_INCLUDES="-I ../log -I ../tools" EXT_LINK="-L ../log -L ../tools" WITH_SSL=1

This will build an extra object and change the behaviour the enable_ssl and 
has_ssl methods on the server class.

Also, if you are using a current OpenSSL version, make sure to add the WITH_SSL_CURRENT
flag to your make, so it tries to use TLS_method().

Finally, if you are building with openssl and want to try the example, make sure
to include the certificate file in the same directory as the example file
with the "cert.pem" filename.

Apropos the source: the open_ssl wrapper object will always exist in code, but
will have different definitions if compiled with or without SSL, so worry not
about that.

#Generating the certificate and key files:

openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365

If you'd like no passphrase, add "-nodes"

If you are just testing, add -subj '/CN=localhost'

#Implementing:

TODO TODO TODO TODO

About SSL capabilities.

Handling exceptions on write...
