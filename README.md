# HTTP Server - Remote Code Execution Platform - README

## Overview
This project is an HTTP server implemented in C++ providing basic CRUD operations using SQLite for storing Python scripts. 
The server is intended to evolve into a submission platform for Python assignments, analogous to platforms like LeetCode. 
Currently, it listens on port 8080 and responds to incoming HTTP requests.

## Features (To Be Implemented)
- **HTTP Server**: The server accepts incoming HTTP requests and responds with static HTML content.
- **SQLite Database**: CRUD operations are supported for storing Python scripts and managing user data.
- **Submission Platform**: The server will evolve into a platform for submitting Python assignments for the Data Structures and Algorithms course at Seneca College.
- **User Authentication**: Implement user authentication to ensure that only registered users can access the submission platform. This could involve username/password authentication or integration with OAuth providers like Google or GitHub.
- **Role-Based Access Control (RBAC)**: Define different roles (e.g., student, instructor, admin) and assign permissions accordingly. For example, only instructors might be able to create and manage assignments, while students can only submit solutions.
- **Assignment Management**: Allow instructors to create, update, and delete assignments. Each assignment could include a description, due date, and possibly associated resources (e.g., PDF documents, sample code).
- **Submission Tracking**: Keep track of submissions for each assignment, including submission timestamp, status (submitted, in review, graded), and any feedback provided by instructors.
- **Automated Testing**: Implement automated testing of submitted Python scripts to ensure they meet certain criteria (e.g., correctness, efficiency). This could involve running test cases against the submitted code and providing feedback to students.
- **Plagiarism Detection**: Integrate with plagiarism detection tools or implement your own algorithms to detect plagiarism in submitted assignments. This helps maintain academic integrity by identifying cases of cheating.
- **Feedback and Grading**: Allow instructors to provide feedback and grades for submitted assignments. This could include a grading rubric with predefined criteria and weights for different aspects of the assignment.



## Usage
1. **Building the Server**: Use the provided Makefile to build the server.
2. **Starting the Server**: Run the compiled executable to start the HTTP server.
3. **Accessing the Platform**: Access the server through a web browser or HTTP client at `http://localhost:8080`.


## Building Instructions
- If using the provided Makefile, simply run `make` to compile the server executable.


## Contributing

Contributions to this project are welcome! If you find any issues or have suggestions for improvements, feel free to open an issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

