import pathlib
import subprocess
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]


def executable_path() -> pathlib.Path:
    candidates = [
        ROOT / "build" / "Debug" / "riscv-sim.exe",
        ROOT / "build" / "riscv-sim.exe",
        ROOT / "build" / "riscv-sim",
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise FileNotFoundError("No se encontró el ejecutable. Ejecuta CMake primero.")


def run_sim(program: str, commands: list[str]) -> str:
    exe = executable_path()
    completed = subprocess.run(
        [str(exe), str(ROOT / "programs" / program)],
        input="\n".join(commands) + "\n",
        text=True,
        capture_output=True,
        cwd=ROOT,
        check=False,
    )
    if completed.returncode != 0:
        raise AssertionError(
            f"El simulador terminó con código {completed.returncode}:\n"
            f"{completed.stdout}\n{completed.stderr}"
        )
    return completed.stdout


def assert_contains(output: str, expected: str) -> None:
    if expected not in output:
        raise AssertionError(f"No se encontró {expected!r} en:\n{output}")


def test_arithmetic() -> None:
    output = run_sim("arithmetic.bin", ["run", "regs x5 x6 x7", "exit"])
    assert_contains(output, "x5 = 0x00000007")
    assert_contains(output, "x6 = 0x00000005")
    assert_contains(output, "x7 = 0x0000000C")


def test_abi_names() -> None:
    output = run_sim("arithmetic.bin", ["run", "regs tp t0 t1 t2", "exit"])
    assert_contains(output, "x4 = 0x00000000")
    assert_contains(output, "x5 = 0x00000007")
    assert_contains(output, "x6 = 0x00000005")
    assert_contains(output, "x7 = 0x0000000C")


def test_memory() -> None:
    output = run_sim("memory.bin", ["run", "regs x5 x6 x7", "mem 0x40 0x43", "exit"])
    assert_contains(output, "x5 = 0x00000040")
    assert_contains(output, "x6 = 0x0000007B")
    assert_contains(output, "x7 = 0x0000007B")
    assert_contains(output, "Memoria (0x00000040-0x00000043): 7B 00 00 00")


def test_infinite_loop() -> None:
    output = run_sim("infinite_loop.bin", ["run", "exit"])
    assert_contains(output, "Ciclo infinito detectado")


def main() -> int:
    tests = [test_arithmetic, test_abi_names, test_memory, test_infinite_loop]
    for test in tests:
        test()
        print(f"{test.__name__}: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
