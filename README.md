# Digital-Logic-TruthTable

## Overview

The `truthtable` program processes a text file describing a digital circuit and produces a truth table that outlines the circuit's behavior for all possible input combinations. This tool is helpful for understanding and verifying digital logic designs.

## Features

- **Input Parsing:** Reads and interprets a circuit description from a file.
- **Truth Table Generation:** Computes and prints the truth table for the circuit.
- **Support for Various Gates:** Handles common logic gates including AND, OR, NOT, NAND, NOR, XOR, DECODER, and MULTIPLEXER.

## Usage

To run the `truthtable` program, execute it with the path to the circuit description file as an argument:

```bash
./truthtable path_to_circuit_file.txt
```

### Example

Given a file `my_circuit.txt` with the following content:

```
INPUT 3 a b c
OUTPUT 1 d
AND a b x
AND c x d
```

Running the program:

```bash
./truthtable my_circuit.txt
```

Produces the output:

```
0 0 0 | 0
0 0 1 | 0
0 1 0 | 0
0 1 1 | 0
1 0 0 | 0
1 0 1 | 0
1 1 0 | 0
1 1 1 | 1
```

## Input Format

The input file should specify the circuit using a simple textual language with the following structure:

- `INPUT n i1 i2 ... in`: Declares `n` input variables.
- `OUTPUT n o1 o2 ... on`: Declares `n` output variables.
- Logic gates are specified with their respective directives followed by their input and output variables.

### Supported Gates

- `NOT i o`: Inverts the input `i` to produce `o`.
- `AND i1 i2 o`: Outputs the AND of `i1` and `i2` to `o`.
- `OR i1 i2 o`: Outputs the OR of `i1` and `i2` to `o`.
- `NAND i1 i2 o`: Outputs the NAND of `i1` and `i2` to `o`.
- `NOR i1 i2 o`: Outputs the NOR of `i1` and `i2` to `o`.
- `XOR i1 i2 o`: Outputs the XOR of `i1` and `i2` to `o`.
- `DECODER n i1 ... in o0 ... o2^n-1`: Represents an `n` to `2^n` decoder.
- `MULTIPLEXER n i0 ... i2^n-1 s1 ... sn o`: Represents a `2^n` to `1` multiplexer.

## Output Format

The output truth table lists all combinations of input values and their corresponding output values. Input columns are separated by spaces, and a vertical bar (`|`) separates input and output columns.

## Compilation

To compile the `truthtable` program, use the provided `Makefile`:

```bash
cd src
make
```

The compiled executable will be located in the `build` directory.

## Implementation Details

- The program assigns numerical identifiers to each variable for efficient processing.
- Gates are represented using a general structure that accommodates different types of gates.
- The truth table is generated and printed one row at a time to handle large tables efficiently.
