#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { AND, OR, NAND, NOR, XOR, NOT, PASS, DECODER, MULTIPLEXER } kind_t;

char* operative[9] =
{ "AND", "OR", "NAND", "NOR", "XOR", "NOT", "PASS", "DECODER", "MULTIPLEXER" };

typedef struct gate {
    kind_t kind;
    int size;       
    int *parameters;   
                   
} gate_t;


typedef struct variable {
    char *name;     
    int data;      
    int type;       
} var_t;

typedef struct Circuit {
    int in;     
    int out;    
    int num_variable;   
    int num_Ofgates;  
    int *inputs;    
    int *outputs;   
    gate_t **gates;
    var_t **variables;  
} circuit_t;

// Declare Function
circuit_t *new_circ(char *filename);
circuit_t *create_circ(void);
var_t *new_var(char *name, circuit_t *circuit);
int add_var(char *name, circuit_t *circuit);
int find_variable(char *var, circuit_t *circuit);
gate_t *new_gate(kind_t kind, int x, int size, circuit_t *circuit);
void add_gate(gate_t *gate, circuit_t *circuit);
int solve_gate(gate_t *gate, circuit_t *circuit);
void solve_circ(circuit_t *circuit);
int to_decimal(int start, int end, gate_t *gate, circuit_t *circuit);
int incrm(circuit_t *circuit);
void undef(circuit_t *circuit);
int check_outputs(circuit_t *circuit);
void print_inputs(circuit_t *circuit);
void print_outputs(circuit_t *circuit);
void free_gate(gate_t *gate);
void free_var(var_t *var);
void free_circ(circuit_t *circuit);


circuit_t *new_circ(char *filename) {
    FILE *input_file = fopen(filename, "r");
    if (input_file == NULL) {
        printf("Error opening file\n");
        return NULL;
    }

    char str[17];
    int out, out_ind, a, x, sz;
    kind_t kind;
    circuit_t *circuit;
    gate_t *gate;

    circuit = create_circ();
    if (circuit == NULL) {
        printf("Error creating circuit\n");
        fclose(input_file);
        return NULL;
    }

    if (fscanf(input_file, "%16s %d", str, &circuit->in) != 2) {
        printf("Error reading input variables\n");
        fclose(input_file);
        free_circ(circuit);
        return NULL;
    }

    circuit->inputs = (int *)malloc(circuit->in * sizeof(int));
    if (circuit->inputs == NULL) {
        printf("Error allocating input list\n");
        fclose(input_file);
        free_circ(circuit);
        return NULL;
    }

    for (int i = 0; i < circuit->in; i++) {
        if (fscanf(input_file, "%16s", str) != 1) {
            printf("Error reading input variable\n");
            fclose(input_file);
            free(circuit->inputs);
            free_circ(circuit);
            return NULL;
        }
        x = add_var(str, circuit);
        circuit->inputs[i] = x;
    }

    if (fscanf(input_file, "%16s %d", str, &circuit->out) != 2) {
        printf("Error reading output variables\n");
        fclose(input_file);
        free(circuit->inputs);
        free_circ(circuit);
        return NULL;
    }

    circuit->outputs = (int *)malloc(circuit->out * sizeof(int));
    if (circuit->outputs == NULL) {
        printf("Error allocating output list\n");
        fclose(input_file);
        free(circuit->inputs);
        free_circ(circuit);
        return NULL;
    }

    for (int i = 0; i < circuit->out; i++) {
        if (fscanf(input_file, "%16s", str) != 1) {
            printf("Error reading output variable\n");
            fclose(input_file);
            free(circuit->inputs);
            free(circuit->outputs);
            free_circ(circuit);
            return NULL;
        }
        x = add_var(str, circuit);
        circuit->outputs[i] = x;
        circuit->variables[circuit->outputs[i]]->type = 1;
    }

    while (fscanf(input_file, "%16s", str) == 1) {
        out_ind = 2;
        out = 1;

        if (!strcmp(str, "NOT")) {
            kind = NOT;
            x = 2;
            out_ind = 1;
            out = 1;
        } else if (!strcmp(str, "PASS")) {
            kind = PASS;
            x = 2;
            out_ind = 1;
            out = 1;
        } else if (!strcmp(str, "AND")) {
            kind = AND;
            x = 3;
        } else if (!strcmp(str, "OR")) {
            kind = OR;
            x = 3;
        } else if (!strcmp(str, "NAND")) {
            kind = NAND;
            x = 3;
        } else if (!strcmp(str, "NOR")) {
            kind = NOR;
            x = 3;
        } else if (!strcmp(str, "XOR")) {
            kind = XOR;
            x = 3;
        } else if (!strcmp(str, "DECODER")) {
            kind = DECODER;
            if (fscanf(input_file, "%d", &sz) != 1) {
                printf("Error reading DECODER directive\n");
                fclose(input_file);
                free(circuit->inputs);
                free(circuit->outputs);
                free_circ(circuit);
                return NULL;
            }
            x = sz + (1 << sz);
            out_ind = sz;
            out = 1 << sz;
        } else if (!strcmp(str, "MULTIPLEXER")) {
            kind = MULTIPLEXER;
            if (fscanf(input_file, "%d", &sz) != 1) {
                printf("Error reading MULTIPLEXER directive\n");
                fclose(input_file);
                free(circuit->inputs);
                free(circuit->outputs);
                free_circ(circuit);
                return NULL;
            }
            x = sz + (1 << sz) + 1;
            out_ind = sz + (1 << sz);
            out = 1;
        }

        gate = new_gate(kind, x, sz, circuit);

        for (int i = 0; i < x; i++) {
            if (fscanf(input_file, "%16s", str) != 1) {
                printf("Error reading gate parameters\n");
                fclose(input_file);
                free(circuit->inputs);
                free(circuit->outputs);
                free_circ(circuit);
                return NULL;
            }
            a = add_var(str, circuit);
            gate->parameters[i] = a;
            if (i >= out_ind && i <= (out + out_ind - 1)) {
                circuit->variables[a]->type = 1;
            }
        }

        add_gate(gate, circuit);
    }

    fclose(input_file);
    return circuit;
}


circuit_t *create_circ(void) {
    circuit_t *circuit = (circuit_t *)calloc(1, sizeof(circuit_t));

    if (circuit == NULL) {
        printf("Error allocating circuit\n");
        exit(EXIT_FAILURE);
    }

    // Initialize variables for 0, 1, and _
    int zero_variable = add_var("0", circuit);
    int one_variable = add_var("1", circuit);
    int placeholder_variable = add_var("_", circuit);

    if (zero_variable == -1 || one_variable == -1 || placeholder_variable == -1) {
        printf("Error adding initial variables\n");
        free(circuit);
        exit(EXIT_FAILURE);
    }

    // Assign data values to 0 and 1 variables
    circuit->variables[zero_variable]->data = 0;
    circuit->variables[one_variable]->data = 1;

    return circuit;
}

// new variable coming into play
var_t *new_var(char *name, circuit_t *circuit) {
    var_t *var = (var_t *)malloc(sizeof(var_t));

    if (var == NULL) {
        printf("Error allocating variable\n");
        free_circ(circuit); // Clean up the entire circuit if variable allocation fails
        exit(EXIT_FAILURE);
    }

    size_t name_length = strlen(name) + 1;
    var->name = (char *)malloc(name_length);

    if (var->name == NULL) {
        printf("Error allocating variable name\n");
        free(var); // Clean up allocated variable if name allocation fails
        free_circ(circuit); // Clean up the entire circuit
        exit(EXIT_FAILURE);
    }

    strncpy(var->name, name, name_length);

    var->data = -1;
    var->type = 0;

    return var;
}


// variable gettig added to the circuit list and then returning to index 
int add_var(char *name, circuit_t *circuit) {
    int existing_var_index = find_variable(name, circuit);

    if (existing_var_index != -1) {
        return existing_var_index; // Variable already exists, return its index
    }

    int new_var_index = circuit->num_variable;

    // Allocate memory for a new list of variables
    var_t **updated_vars = (var_t **)realloc(circuit->variables, (new_var_index + 1) * sizeof(var_t *));
    
    if (updated_vars == NULL) {
        printf("Error reallocating variable list\n");
        free_circ(circuit);
        exit(EXIT_FAILURE);
    }

    circuit->variables = updated_vars;
    circuit->variables[new_var_index] = new_var(name, circuit);
    circuit->num_variable++;

    return new_var_index;
}



int find_variable(char *var_name, circuit_t *circuit) {
    int index = -1;
    int i = 0;

    while (i < circuit->num_variable && index == -1) {
        if (!strcmp(var_name, circuit->variables[i]->name)) {
            index = i;
        }
        i++;
    }

    return index;
}


gate_t *new_gate(kind_t kind, int x, int size, circuit_t *circuit) {
    gate_t *gate_instance = (gate_t *)malloc(sizeof(gate_t));

    if (gate_instance == NULL) {
        printf("Allocation error: Gate\n");
        free_circ(circuit);
        exit(EXIT_FAILURE);
    }

    gate_instance->kind = kind;
    gate_instance->parameters = (int *)malloc(x * sizeof(int));

    if (gate_instance->parameters == NULL) {
        printf("Allocation error: Gate parameters\n");
        free(gate_instance);
        free_circ(circuit);
        exit(EXIT_FAILURE);
    }

    gate_instance->size = size;

    return gate_instance;
}




void add_gate(gate_t *new_gate, circuit_t *circuit) {
    int gate_count = circuit->num_Ofgates;
    gate_t **gate_list;

    // Allocate or reallocate memory for gate list
    if (gate_count == 0) {
        gate_list = (gate_t **)malloc(sizeof(gate_t *));
    } else {
        gate_list = (gate_t **)realloc(circuit->gates, (gate_count + 1) * sizeof(gate_t *));
    }

    if (gate_list == NULL) {
        printf("Error: Unable to allocate memory for gate list\n");
        free_circ(circuit);
        exit(EXIT_FAILURE);
    }

    circuit->gates = gate_list;
    circuit->gates[gate_count] = new_gate;
    circuit->num_Ofgates++;
}



int solve_gate(gate_t *gate, circuit_t *circuit) {
    int input1, input2, result;
    int output = -1;

    input1 = circuit->variables[gate->parameters[0]]->data;
    input2 = (gate->kind != NOT) ? circuit->variables[gate->parameters[1]]->data : 0;

    switch (gate->kind) {
        case AND:
            result = (input1 != -1 && input2 != -1) ? (input1 & input2) : -1;
            circuit->variables[gate->parameters[2]]->data = result;
            break;

        case OR:
            result = (input1 != -1 && input2 != -1) ? (input1 | input2) : -1;
            circuit->variables[gate->parameters[2]]->data = result;
            break;

        case NAND:
            result = (input1 != -1 && input2 != -1) ? (~(input1 & input2) & 1) : -1;
            circuit->variables[gate->parameters[2]]->data = result;
            break;

        case NOR:
            result = (input1 != -1 && input2 != -1) ? (~(input1 | input2) & 1) : -1;
            circuit->variables[gate->parameters[2]]->data = result;
            break;

        case XOR:
            result = (input1 != -1 && input2 != -1) ? (input1 ^ input2) : -1;
            circuit->variables[gate->parameters[2]]->data = result;
            break;

        case NOT:
            result = (input1 != -1) ? (~input1 & 1) : -1;
            circuit->variables[gate->parameters[1]]->data = result;
            break;

        case PASS:
            result = input1;
            circuit->variables[gate->parameters[1]]->data = result;
            break;

        case DECODER:

        int index = to_decimal(0, gate->size - 1, gate, circuit);

    if(index == -1) {
        for (int i = 0; i < (1 << gate->size); i++) {
            circuit->variables[gate->parameters[gate->size + i]]->data = -1;
        }
    } else {
        for (int i = 0; i < (1 << gate->size); i++) {
            circuit->variables[gate->parameters[gate->size + i]]->data = (i == index) ? 1 : 0;
        }
        output = 1;
    }
    break;

case MULTIPLEXER:
    index = to_decimal(1 << gate->size, (1 << gate->size) + gate->size - 1, gate, circuit);

    if (index == -1) {
        circuit->variables[gate->parameters[(1 << gate->size) + gate->size]]->data = -1;
    } else {
        output = circuit->variables[gate->parameters[index]]->data;
        circuit->variables[gate->parameters[(1 << gate->size) + gate->size]]->data = output;
    }
    break;

    }

    return result;
}


void solve_circ(circuit_t *circuit)
{
    int x;

    // Set all input values to zero bit
    for (int i = 0; i < circuit->in; i++) {
        circuit->variables[circuit->inputs[i]]->data = 0;
    }

    while (!x) {
        print_inputs(circuit);
        undef(circuit);

        while (!check_outputs(circuit)) {
            for (int i = 0; i < circuit->num_Ofgates; i++) {
                solve_gate(circuit->gates[i], circuit);
            }
        }
        printf("|");
        print_outputs(circuit);
        printf("\n");
        x = incrm(circuit);
    }
}


int to_decimal(int start, int end, gate_t *gate, circuit_t *circuit) {
    int decimalValue = 0;

    for (int i = end, position = 0; i >= start && decimalValue != -1; i--, position++) {
        if (circuit->variables[gate->parameters[i]]->data == -1) {
            decimalValue = -1;
        } else {
            decimalValue += circuit->variables[gate->parameters[i]]->data << position;
        }
    }
    return decimalValue;
}



int incrm(circuit_t *circuit) {
    int carry = 1;

    for (int i = circuit->in - 1; i >= 0 && carry == 1; i--) {
        if (circuit->variables[circuit->inputs[i]]->data == 0) {
            circuit->variables[circuit->inputs[i]]->data = 1;
            carry = 0;
        } else if (circuit->variables[circuit->inputs[i]]->data == 1) {
            circuit->variables[circuit->inputs[i]]->data = 0;
            carry = 1;
        } else {
            // In case of undefined data, set to 1 and exit
            circuit->variables[circuit->inputs[i]]->data = 1;
            carry = 0;
        }
    }
    return carry;
}


void undef(circuit_t *circuit) {
    // Unset data for flagged variables
    for (int i = 0; i < circuit->num_variable; i++) {
        if (circuit->variables[i]->type && i >= 3) {
            circuit->variables[i]->data = -1;
        }
    }
}


int check_outputs(circuit_t *circuit) {
    for (int i = 0; i < circuit->out; i++) {
        if (circuit->variables[circuit->outputs[i]]->data != 0 &&
            circuit->variables[circuit->outputs[i]]->data != 1) {
            return 0;
        }
    }
    return 1;
}

// Print inputs
void print_inputs(circuit_t *circuit)
{
    for (int i = 0; i < circuit->in; i++) {
        printf("%d ", circuit->variables[circuit->inputs[i]]->data);
    }
}

// Print outputs
void print_outputs(circuit_t *circuit)
{
    for (int i = 0; i < circuit->out; i++) {
        printf(" %d", circuit->variables[circuit->outputs[i]]->data);
    }
}

// Free allocated gate memory
void free_gate(gate_t *gate) {
    if (gate != NULL) {
        if (gate->parameters != NULL) {
            free(gate->parameters);
            gate->parameters = NULL; // Set to NULL after freeing to prevent dangling pointer
        }
        free(gate);
        gate = NULL; // Set to NULL after freeing to prevent dangling pointer
    }
}

// Free allocated variable memory
void free_var(var_t *var) {
    if (var != NULL) {
        if(var->name != NULL) {
            free(var->name);
            var->name = NULL; // Set to NULL after freeing to prevent dangling pointer
        }
        free(var);
        var = NULL; // Set to NULL after freeing to prevent dangling pointer
    }
}
// Free allocated circuit memory
void free_circ(circuit_t *circuit) {
    if (circuit != NULL) {
        if (circuit->inputs != NULL) {
            free(circuit->inputs);
            circuit->inputs = NULL;
        }
        if (circuit->outputs != NULL) {
            free(circuit->outputs);
            circuit->outputs = NULL;
        }
        if (circuit->gates != NULL) {
            for (int i = 0; i < circuit->num_Ofgates; i++) {
                free_gate(circuit->gates[i]);
                circuit->gates[i] = NULL;
            }
            free(circuit->gates);
            circuit->gates = NULL;
        }
        if (circuit->variables != NULL) {
            for (int i = 0; i < circuit->num_variable; i++) {
                free_var(circuit->variables[i]);
                circuit->variables[i] = NULL;
            }
            free(circuit->variables);
            circuit->variables = NULL;
        }
        free(circuit);
        circuit = NULL;
    }
}


// Main driver
int main(int argc, char **argv)
{
    circuit_t *circuit;

    // If not given single argument, error
    if(argc != 2) {
		printf("Error\n");
		return EXIT_FAILURE;
	}

    // Create circuit given file
    circuit = new_circ(argv[1]);
    solve_circ(circuit);
    free_circ(circuit);
    return EXIT_SUCCESS;
}