# Tiny WebServer

A lightweight C++ Web Server implementation with a modern frontend built using Next.js and TailwindCSS.

## Features

### Backend Features

- **Multi-threading**: Efficient thread pool implementation.
- **I/O Multiplexing**: Uses `epoll` for high-performance I/O.
- **MySQL Connection Pool**: Manages database connections efficiently.
- **Logging System**: Asynchronous logging with support for different log levels.
- **Timer Functionality**: Handles inactive connections using a timer.
- **HTTP Protocol Support**: Implements HTTP request parsing and response generation.
- **Signal Handling**: Graceful handling of system signals like `SIGINT` and `SIGTERM`.
- **Graceful Shutdown**: Ensures proper resource cleanup during shutdown.

### Frontend Features

- **Next.js Framework**: Server-side rendering and static site generation.
- **TailwindCSS**: Utility-first CSS framework for styling.
- **TypeScript**: Strongly typed frontend codebase.
- **Reusable Components**: Includes reusable components like `FormInput`.
- **Authentication Pages**: Pre-built login and registration pages.

## Project Structure

```bash
tiny-webserver/
├── backend/                # Backend server implementation
│   ├── src/                # Source code
│   │   ├── core/           # Core server logic
│   │   ├── config/         # Configuration management
│   │   ├── utils/          # Utility classes (logging, threading, etc.)
│   │   └── third_party/    # Third-party dependencies (e.g., MySQL connection pool)
│   └── CMakeLists.txt      # CMake build configuration
├── frontend/               # Frontend implementation
│   ├── components/         # Reusable React components
│   ├── pages/              # Next.js pages (e.g., login, register)
│   ├── styles/             # Global CSS styles
│   ├── tsconfig.json       # TypeScript configuration
│   └── package.json        # Frontend dependencies and scripts
├── docs/                   # Documentation
├── tests/                  # Test code
├── .vscode/                # VSCode configuration files
├── LICENSE                 # License file
└── README.md               # Project documentation
```

## Dependencies

### Backend Dependencies

- **C++14 or later**
- **CMake 3.10 or later**
- **MySQL development libraries**
- **JSONCPP**: For JSON parsing and generation.

### Frontend Dependencies

- **Node.js**: For running the Next.js application.
- **TailwindCSS**: For styling.
- **TypeScript**: For type-safe development.

## Building and Running

### Backend Building

1. Install dependencies:
   - MySQL development libraries
   - JSONCPP library
2. Build the backend:

   ```bash
   mkdir backend/build && cd backend/build
   cmake ..
   make
   ```

3. Run the server:

   ```bash
   ./tiny_webserver [port] [user] [password] [database_name] [log_write] [opt_linger] [trigmode] [sql_num] [thread_num] [close_log] [actor_model]
   ```

### Frontend Building

1. Navigate to the `frontend` directory:

   ```bash
   cd frontend
   ```

2. Install dependencies:

   ```bash
   npm install
   ```

3. Run the development server:

   ```bash
   npm run dev
   ```

4. Access the frontend at `http://localhost:3000`.

## Configuration

### Backend Configuration

The backend server can be configured using command-line arguments or by modifying the configuration file. Key parameters include:

- `port`: Server port (default: 9000)
- `user`: MySQL username
- `password`: MySQL password
- `database_name`: MySQL database name
- `log_write`: Log write mode (0: synchronous, 1: asynchronous)
- `trigmode`: Trigger mode (0: LT+LT, 1: LT+ET, 2: ET+LT, 3: ET+ET)
- `sql_num`: Number of MySQL connections
- `thread_num`: Number of threads in the thread pool

### Frontend Configuration

The frontend is configured using the following files:

- `tsconfig.json`: TypeScript configuration.
- `tailwind.config.js`: TailwindCSS configuration.
- `next.config.js`: Next.js configuration.

## Authentication API

The backend provides the following API endpoints for authentication:

- **POST /api/login**: User login.
- **POST /api/register**: User registration.

## License

This project is licensed under the terms specified in the [LICENSE](LICENSE) file.
