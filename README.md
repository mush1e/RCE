# RCE (Name TBD)

**RCE** is a secure remote code execution platform that allows users to submit, execute, and view results of code written in various programming languages. It features real-time execution output, sandboxing, and multi-user support.

## Features

### Core Functionality
- **Multi-Language Support:** Python, Go, C++, JavaScript, and more.
- **Secure Execution:** Run code in isolated Docker containers.
- **Real-time Output:** View execution logs live using WebSockets.
- **Code Snippet Management:** Save, edit, and share code snippets.

### Security Measures
- **Sandboxing:** Isolated containers with limited resources.
- **Resource Control:** CPU/memory restrictions to prevent abuse.
- **Authentication:** Twilio OTP and OAuth (Google/GitHub).
- **Input Validation:** Prevent injection attacks.

### Additional Features
- **RESTful API:** Secure API endpoints for integrations.
- **Job Queue:** Redis-based task scheduling for parallel execution.
- **Syntax Highlighting:** Web-based code editor with auto-completion.
- **Code Analytics:** Track most-used languages and execution stats.

## Tech Stack

### Backend
- **Go** (net/http for API, GORM for database handling)
- **Docker API** (for managing execution containers)
- **Redis** (for task queuing)
- **PostgreSQL** (for data persistence)

### Frontend
- **HTMX** (for dynamic frontend updates)
- **Templ** (for rendering templates)
- **Monaco/CodeMirror** (for code editing)

### Infrastructure
- **AWS EC2** (for hosting backend services)
- **Nginx** (as a reverse proxy and load balancer)

## Installation

### Prerequisites
- Go 1.22+
- Docker & Docker Compose
- PostgreSQL
- Redis
- Node.js (for frontend dependencies)

### Steps

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/yourusername/coderunhub.git
   cd coderunhub
   ```

2. **Set Up Environment Variables:**
   Create a `.env` file with the following:
   ```env
   DB_HOST=localhost
   DB_PORT=5432
   DB_USER=yourusername
   DB_PASSWORD=yourpassword
   DB_NAME=coderunhub
   REDIS_URL=redis://localhost:6379
   TWILIO_SID=your_twilio_sid
   TWILIO_AUTH_TOKEN=your_auth_token
   ```

3. **Start Services:**
   ```bash
   docker-compose up -d
   ```

4. **Run the Backend Server:**
   ```bash
   go run main.go
   ```

5. **Start the Frontend:**
   ```bash
   npm install
   npm run dev
   ```

6. **Access the Application:**
   - Open `http://localhost:3000` in your browser.

## Usage

1. Sign up and log in using Twilio OTP or Google OAuth.
2. Choose a programming language and write code in the editor.
3. Click `Run` to execute code and see output in real time.
4. Save and share code snippets.
5. Monitor your execution history from the dashboard.


## Security Considerations

- Execution in isolated Docker containers.
- Limits on execution time, memory, and CPU.
- Sanitized user input to prevent code injection.
- Rate limiting for API endpoints.

## Future Improvements

- Support for collaborative coding sessions.
- AI-based code analysis and suggestions.
- Deployment via Kubernetes for scalability.
- WebRTC-based real-time coding collaboration.

## Contributing

1. Fork the repository.
2. Create a new feature branch.
3. Commit your changes.
4. Push to the branch and open a pull request.

## License

This project is licensed under the MIT License.

## Contact

For inquiries, please contact: [your email] or open an issue on GitHub.
