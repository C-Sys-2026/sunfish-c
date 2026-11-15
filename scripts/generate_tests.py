from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
TESTS_DIR = ROOT / "tests"
OUTPUT = TESTS_DIR / "test_main.c"
TEST_PATTERN = re.compile(r"^\s*void\s+(test_[A-Za-z0-9_]+)\s*\(\s*void\s*\)\s*\{", re.MULTILINE)


def enabled_source(source: str) -> str:
    lines = []
    disabled_depth = 0

    for line in source.splitlines(keepends=True):
        directive = line.strip()
        if directive == "#if 0":
            disabled_depth += 1
            continue
        if directive == "#endif" and disabled_depth:
            disabled_depth -= 1
            continue
        if not disabled_depth:
            lines.append(line)

    return "".join(lines)


def discover_tests():
    discovered = []

    for path in sorted(TESTS_DIR.glob("test_*.c")):
        if path.name == OUTPUT.name:
            continue

        source = enabled_source(path.read_text(encoding="utf-8"))
        tests = [match.group(1) for match in TEST_PATTERN.finditer(source)]
        group = path.stem.removeprefix("test_")

        for name in tests:
            discovered.append((group, name))

    return discovered


def generate_content(tests):
    declarations = "\n".join(f"void {name}(void);" for _, name in tests)
    wrappers = "\n".join(f"WRAP({name})" for _, name in tests)
    cases = "\n".join(
        f'    CASE("/{group}/{name.removeprefix("test_")}", {name}),'
        for group, name in tests
    )

    return f'''#include "../third_party/munit/munit.h"

{declarations}

#define WRAP(fn) static MunitResult wrap_##fn(const MunitParameter* p, void* d) {{ (void)p; (void)d; fn(); return MUNIT_OK; }}
#define CASE(name, fn) {{ name, wrap_##fn, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }}

{wrappers}

static MunitTest tests[] = {{
{cases}
    {{ NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }}
}};

static const MunitSuite suite = {{
    "/sunfish-c", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
}};

int main(int argc, char* argv[]) {{
    return munit_suite_main(&suite, NULL, argc, argv);
}}
'''


def main():
    content = generate_content(discover_tests())
    current = OUTPUT.read_text(encoding="utf-8") if OUTPUT.exists() else None

    if current != content:
        OUTPUT.write_text(content, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
