import importlib.util
import json
import sys 

def load_user_submission(file_path):
    spec = importlib.util.spec_from_file_location("user_submission", file_path)
    user_submission = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(user_submission)
    return user_submission

def run_test_cases(user_func, test_cases):
    results = []
    for args, expected in test_cases:
        try:
            output = user_func(*args)
            result = (output == expected)
        except Exception as e:
            result = False
            output = str(e)
        results.append({"args": args, "expected": expected, "output": output, "result": result})
    return results

if __name__ == "__main__":
    script_path = sys.argv[1]
    func_name = sys.argv[2]
    user_submission = load_user_submission(script_path)
    user_func = getattr(user_submission, func_name, None)
    if user_func is None:
        print(json.dumps({"error": f"Function {func_name} not found in submission."}))
        sys.exit(1)

    test_cases = [
        # Your test cases for the current problem
        # Format: (input_args, expected_output)
    ]

    results = run_test_cases(user_func, test_cases)
    print(json.dumps(results, indent=2))
