# Tiny WebServer

A lightweight C++ Web Server implementation.

## Features

- Multi-threading support with thread pool
- I/O multiplexing using epoll
- MySQL connection pool
- Logging system
- Timer functionality
- HTTP protocol support
- Signal handling
- Graceful shutdown support

## Project Structure

```
tiny-webserver/
├── src/                    # Source code
│   ├── core/              # Core server implementation
│   ├── config/            # Configuration management
│   └── utils/             # Utility classes
├── include/               # Header files
├── third_party/          # Third-party dependencies
├── build/                # Build directory
├── tests/                # Test code
└── docs/                 # Documentation
```

## Dependencies

- C++11 or later
- CMake 3.10 or later
- MySQL development libraries

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Configuration

The server can be configured through command line arguments:

```bash
./tiny_webserver [port] [user] [password] [database_name] [log_write] [opt_linger] [trigmode] [sql_num] [thread_num] [close_log] [actor_model]
```

## License

This project is licensed under the terms specified in the LICENSE file.