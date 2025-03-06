import subprocess


def run_compiler(input_code):
    process = subprocess.run(
        ["./compiler"],
        input=input_code,
        text=True,
        capture_output=True
    )
    return process.stdout.strip()


def test_print():
    code = "main() { print(10); }"
    output = run_compiler(code)
    assert output == "10", f"Expected '10', got '{output}'"


def test_declaration_and_assignment():
    code = "main() { declare x: int; x = 10; print(x); }"
    output = run_compiler(code)
    assert output == "10", f"Expected '10', got '{output}'"


def test_arithmetic():
    code = "main() { declare x: int; x = 5 + 3 * 2; print(x); }"
    output = run_compiler(code)
    assert output == "11", f"Expected '11', got '{output}'"


def test_if_true():
    code = "main() { declare x: int; x = 8; if (x == 8) { print(1); } else { print(0); } }"
    output = run_compiler(code)
    assert output == "1", f"Expected '1', got '{output}'"


def test_if_false():
    code = "main() { declare x: int; x = 7; if (x == 8) { print(1); } else { print(0); } }"
    output = run_compiler(code)
    assert output == "0", f"Expected '0', got '{output}'"


def test_division():
    code = "main() { declare x: int; x = 10 / 2; print(x); }"
    output = run_compiler(code)
    assert output == "5", f"Expected '5', got '{output}'"


if __name__ == "__main__":
    tests = [
        test_print,
        test_declaration_and_assignment,
        test_arithmetic,
        test_if_true,
        test_if_false,
        test_division
    ]
    for i, test in enumerate(tests, 1):
        try:
            test()
            print(f"Test {i} passed!")
        except AssertionError as e:
            print(f"Test {i} failed: {e}")
