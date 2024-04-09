import json
import random 

def generate_int(min_val, max_val):
    return random.randing(min_val, max_val)

def generate_string(min_length, max_length, characters):
    length = random.randint(min_length, max_length)
    return ''.join(random.choice(characters) for _ in range(length))

def generate_list(element_type, min_size, max_size, min_value, max_value, characters=''):
    size = generate_int(min_size, max_size)
    if element_type == "int":
        return [generate_int(min_value, max_value) for _ in range(size)]
    elif element_type == "string":
        return [generate_string(min_value, max_value, characters) for _ in range(size)]  # min_value/max_value used as min_length/max_length here
    return []

def generate_test_case(input_params):
    test_case = {}
    for param, config in input_params.items():
        if config['type'] == "int":
            test_case[param] = generate_int(config['min'], config['max'])
        elif config['type'] == "list":
            test_case[param] = generate_list(config['element_type'], config['min_size'], config['max_size'], config['min_value'], config['max_value'], config.get('characters', ''))
        elif config['type'] == "string":
            test_case[param] = generate_string(config['min_length'], config['max_length'], config['characters'])
    return test_case

def generate_test_cases_for_problem(problem_config):
    cases = []
    for _ in range(problem_config["number_of_cases"]):
        cases.append(generate_test_case(problem_config["input_parameters"]))
    return cases

def main(config_file):
    with open(config_file, 'r') as f:
        config = json.load(f)

    all_test_cases = {}
    for problem in config["problems"]:
        test_cases = generate_test_cases_for_problem(problem)
        all_test_cases[problem["name"]] = test_cases

    # Output the generated test cases
    print(json.dumps(all_test_cases, indent=2))

if __name__ == "__main__":
    config_file = "test_case_config.json"
    main(config_file)