import subprocess
import os

def run_compiler(input_code):
    ir_process = subprocess.run(
        ["./compiler", "--emit-ir"],
        input=input_code,
        text=True,
        capture_output=True
    )
    if ir_process.returncode != 0:
        raise RuntimeError(f"IR generation failed: {ir_process.stderr}")

    with open("temp.ll", "w") as f:
        f.write(ir_process.stdout)

    # with open("temp.ll", 'r') as f:
    #     content = f.read()
    #     print("Gen Code:", content)

    compile_process = subprocess.run(
        ["clang", "-o", "temp_program", "temp.ll"],
        capture_output=True
    )
    if compile_process.returncode != 0:
        raise RuntimeError(f"Compilation failed: {compile_process.stderr}")

    run_process = subprocess.run(
        ["./temp_program"],
        capture_output=True,
        text=True
    )

    os.remove("temp.ll")
    os.remove("temp_program")

    return run_process.stdout.strip()

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


# def test_print_ast():
#     code = "main() { declare x: int; x = 7; if (x == 8) { print(1); } else { print(0); } }"
#     process = subprocess.run(
#         ["./compiler", "print_ast_to=ast.txt"],
#         input=code,
#         text=True,
#         capture_output=True
#     )
#     assert process.stdout.strip(
#     ) == "0", f"Expected '0', got '{process.stdout.strip()}'"


if __name__ == "__main__":
    tests = [
        test_print,
        test_declaration_and_assignment,
        test_arithmetic,
        test_if_true,
        test_if_false,
        test_division,
        # test_print_ast
    ]

    for i, test in enumerate(tests, 1):
        try:
            test()
            print(f"Test {i} passed!")
        except Exception as e:
            print(f"Test {i} failed: {str(e)}")
