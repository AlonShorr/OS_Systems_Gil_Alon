import subprocess
import itertools
import random
import os

# === Configuration ===
bank_exec = "./bank"  # Ensure this is compiled and executable
max_tests = 30
max_comb_size = 10

# === Gather ATM1.txt to ATM30.txt from current folder ===
atm_files = [f"ATM{i}.txt" for i in range(1, 31) if os.path.isfile(f"ATM{i}.txt")]

if len(atm_files) < 30:
    print(f"⚠️ Warning: Only found {len(atm_files)} ATM files. Make sure ATM1.txt to ATM30.txt exist.")

# === Generate 30 unique combinations of 1–10 files ===
combinations_list = []
used = set()
random.seed(42)

while len(combinations_list) < max_tests:
    count = random.randint(1, max_comb_size)
    combo = tuple(sorted(random.sample(atm_files, count)))
    if combo not in used:
        combinations_list.append(combo)
        used.add(combo)

# === Run tests ===
results = []
for idx, combo in enumerate(combinations_list, 1):
    cmd = ["valgrind", "--leak-check=full", "--error-exitcode=1", bank_exec] + list(combo)
    print(f"Running test {idx} with {len(combo)} ATM files... ", end="")
    try:
        result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=500)
        if result.returncode == 0:
            print("✅ Passed")
            results.append("Passed")
        else:
            print("❌ Failed (valgrind)")
            results.append("Failed")
    except subprocess.TimeoutExpired:
        print("⏱️ Timeout")
        results.append("Timeout")
    except Exception as e:
        print(f"❌ Error ({e})")
        results.append(f"Error: {e}")

# === Final Summary ===
print("\n===== Test Summary =====")
print(f"Total Runs: {len(results)}")
print(f"✅ Passed: {results.count('Passed')}")
print(f"❌ Failed: {results.count('Failed')}")
print(f"⏱️ Timeouts: {results.count('Timeout')}")
print(f"⚠️ Errors: {sum(1 for r in results if str(r).startswith('Error'))}")
