# listener_service
Work in progress: trying to make a reusable listener service in C++ which can be injected with arbitrary logic.

## Dependencies

tools::log
tools::arg_manager
tools::string_utils
tools::text_reader

## TODO:

Write a good "build" guide.
The dependency on text_reader is flaky. string_utils should not depend on them.
