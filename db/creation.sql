CREATE TABLE questions (
    question_id INTEGER PRIMARY KEY,
    question_title TEXT,
    question_text TEXT,
    solution_script TEXT,
    date_posted DATE,
    admin_id INTEGER,
    FOREIGN KEY (admin_id) REFERENCES users(user_id)
);

CREATE TABLE submissions (
    submission_id INTEGER PRIMARY KEY,
    question_id INTEGER,
    user_id INTEGER,
    submission_script TEXT,
    date_submitted DATE,
    FOREIGN KEY (question_id) REFERENCES questions(question_id),
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

CREATE TABLE users (
    user_id INTEGER PRIMARY KEY,
    username TEXT,
    password TEXT,
    is_admin INTEGER
);
