import csv
import os
from typing import List, Optional

# --- Configuration ---
INPUT_FILE = '/Users/winstondejong/Projects/la_metro_live/mappings/Train mappings - Combined B_D.csv'
OUTPUT_FILE = 'processed_output.txt'
# ---------------------

def process_row(row: List[str]) -> Optional[str]:
    return f"{{{int(row[1])%10000}, {{{row[3]}, {row[2]}}}}},     // {row[0]}\n"


def process_csv_and_write_output(input_filepath: str, output_filepath: str) -> None:
    # List to collect all the output strings
    all_outputs = []
    
    # 1. Read the input CSV file (No error checks)
    with open(input_filepath, mode='r', newline='', encoding='utf-8') as infile:
        reader = csv.reader(infile)
        
        # Skip the header row (will raise StopIteration if file is empty)
        header = next(reader)
        print(f"Skipping header: {header}")

        # Iterate over each data row
        for i, row in enumerate(reader):
            all_outputs.append(process_row(row))

    with open(output_filepath, mode='w', encoding='utf-8') as outfile:
        outfile.writelines(all_outputs)
        
    print(f"\nProcessing complete!")
    print(f"Total lines written: {len(all_outputs)}")
    print(f"Output saved to: {os.path.abspath(output_filepath)}")

process_csv_and_write_output(INPUT_FILE, OUTPUT_FILE)