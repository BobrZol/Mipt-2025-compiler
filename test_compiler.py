import subprocess
import os

def run_compiler(input_code):
    exe_process = subprocess.run(
        ["./compiler", "--emit-executable", "-o", "temp_program"],
        input=input_code,
        text=True,
        capture_output=True
    )
    if exe_process.stderr:
        raise RuntimeError(f"Compilation error: {exe_process.stderr}")
    if exe_process.returncode != 0:
        raise RuntimeError(f"Executable generation failed: {exe_process.stderr}")

    run_process = subprocess.run(
        ["./temp_program"],
        capture_output=True,
        text=True
    )

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

def test_duplicate_declaration():
    code = "main() { declare x: int; declare x: int; print(x); }"
    try:
        run_compiler(code)
        assert False, "Expected error for duplicate declaration"
    except RuntimeError as e:
        assert "already declared" in str(e), "Expected duplicate declaration error"

def test_undeclared_variable():
    code = "main() { print(x); }"
    try:
        run_compiler(code)
        assert False, "Expected error for undeclared variable"
    except RuntimeError as e:
        assert "not declared" in str(e), "Expected undeclared variable error"

def test_shadowing():
    code = "main() { declare x: int; x = 10; if (x == 10) { declare x: int; x = 20; print(x); } print(x); }"
    output = run_compiler(code)
    assert output == "20\n10", f"Expected '20\n10' for shadowing, got '{output}'"

def test_nested_shadowing():
    code = """
    main() {
        declare x: int;
        x = 5;
        if (x == 5) {
            declare x: int;
            x = 10;
            if (x == 10) {
                declare x: int;
                x = 15;
                print(x);
            }
            print(x);
        }
        print(x);
    }
    """
    output = run_compiler(code)
    assert output == "15\n10\n5", f"Expected '15\n10\n5', got '{output}'"

def test_undeclared_variable_assignment():
    code = "main() { z = 10; }"
    try:
        run_compiler(code)
        assert False, "Expected error for undeclared variable assignment"
    except RuntimeError as e:
        assert "not declared" in str(e), "Expected undeclared variable error"

def test_variable_in_nested_scope():
    code = """
    main() {
        declare b: int;
        b = 20;
        if (b == 20) {
            print(b);
        }
    }
    """
    output = run_compiler(code)
    assert output == "20", f"Expected '20', got '{output}'"

def test_if_without_else():
    code = """
    main() {
        declare d: int;
        d = 7;
        if (d == 7) {
            print(1);
        }
        print(2);
    }
    """
    output = run_compiler(code)
    assert output == "1\n2", f"Expected '1\n2', got '{output}'"

def test_nested_if():
    code = """
    main() {
        declare f: int;
        f = 10;
        if (f == 10) {
            if (f == 10) {
                print(1);
            } else {
                print(0);
            }
        } else {
            print(2);
        }
    }
    """
    output = run_compiler(code)
    assert output == "1", f"Expected '1', got '{output}'"

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
        test_duplicate_declaration,
        test_undeclared_variable,
        test_shadowing,
        test_nested_shadowing,
        test_undeclared_variable_assignment,
        test_variable_in_nested_scope,
        test_if_without_else,
        test_nested_if
        # test_print_ast
    ]

    for i, test in enumerate(tests, 1):
        try:
            test()
            print(f"Test {i} passed!")
        except Exception as e:
            print(f"Test {i} failed: {str(e)}")
