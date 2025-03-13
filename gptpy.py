import os
import heapq

total_seeks = 0
total_transfers = 0

class Block:
    def __init__(self, data=None):
        self.data = data if data else []

def read_block(input_file, block_size):
    global total_transfers
    block = Block()
    try:
        with open(input_file, 'r') as f:
            for _ in range(block_size):
                value = f.readline()
                if not value:
                    break
                block.data.append(int(value.strip()))
    except FileNotFoundError:
        print(f"Error: {input_file} not found.")

    if block.data:
        total_transfers += 2
    return block

def write_block(block, filename):
    global total_transfers
    with open(filename, 'w') as f:
        for val in block.data:
            f.write(f"{val}\n")
    if block.data:
        total_transfers += 2

def create_initial_runs(input_file, block_size, memory_blocks):
    global total_seeks
    run_files = []
    run_count = 0

    while True:
        block = read_block(input_file, block_size * memory_blocks)
        if not block.data:
            break

        block.data.sort()

        run_file = f"run_{run_count}.txt"
        write_block(block, run_file)
        run_files.append(run_file)

        total_seeks += 1
        run_count += 1

    return run_files

def merge_runs(run_files, block_size, memory_blocks):
    global total_seeks, total_transfers
    merge_factor = memory_blocks - 1
    new_run_files = []
    pass_num = 0

    current_runs = run_files

    while len(current_runs) >= memory_blocks:
        run_count = 0

        for i in range(0, len(current_runs), merge_factor):
            subset = current_runs[i:i + merge_factor]

            merged_file = f"pass_{pass_num}_run_{run_count}.txt"
            new_run_files.append(merged_file)

            min_heap = []
            file_pointers = []

            for j, run_file in enumerate(subset):
                try:
                    f = open(run_file, 'r')
                    file_pointers.append(f)
                    value = f.readline()
                    if value:
                        heapq.heappush(min_heap, (int(value.strip()), j))
                        total_transfers += 2
                        total_seeks += 1
                except FileNotFoundError:
                    print(f"Error: {run_file} not found.")

            with open(merged_file, 'w') as output:
                while min_heap:
                    value, idx = heapq.heappop(min_heap)
                    output.write(f"{value}\n")

                    next_value = file_pointers[idx].readline()
                    if next_value:
                        heapq.heappush(min_heap, (int(next_value.strip()), idx))
                        total_transfers += 2
                        total_seeks += 1

            for f in file_pointers:
                f.close()

            run_count += 1

        current_runs = new_run_files
        new_run_files = []
        pass_num += 1

        print(f"Pass {pass_num}: Total Seeks = {total_seeks}, Total Transfers = {total_transfers}")

    new_run_files.extend(current_runs)
    return new_run_files

def main(input_file, n, k, b, m):
    print(f"Running external merge sort on {input_file} with parameters:")
    print(f"n = {n}, k = {k}, b = {b}, m = {m}")

    block_size = b // k

    run_files = create_initial_runs(input_file, block_size, m)
    print(f"Created {len(run_files)} initial runs.")

    final_runs = merge_runs(run_files, block_size, m)

    for run_file in final_runs:
        print(f"Merging completed. Final output written to: {run_file}")

    print(f"Total Disk Seeks: {total_seeks}, Total Disk Transfers: {total_transfers}")

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 6:
        print("Usage: python program.py input-file n k b m")
        sys.exit(1)

    input_file = sys.argv[1]
    n = int(sys.argv[2])  # Total number of keys
    k = int(sys.argv[3])  # Size of each key in bytes
    b = int(sys.argv[4])  # Disk block size in bytes
    m = int(sys.argv[5])  # Available memory in blocks

    main(input_file, n, k, b, m)
