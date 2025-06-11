import os

data_1_path = 'data.txt'
data_2_path = 'data2.txt'
lock_path = 'lockfile.lock'

with open(data_1_path, 'w') as f1:
    with open(data_2_path, 'w') as f2:
        for _ in range(50):
            for i in range(1000):
                f1.write(f"{str(i)}")
                f2.write(f"{str(i*2)}")
            f1.write("\n")
            f2.write("\n")

# Create a 1-byte lock file
with open(lock_path, 'w') as f:
    f.write("\0")

print(f"Generated test files:\n - {data_1_path}\n - {data_2_path}\n - {lock_path}")
