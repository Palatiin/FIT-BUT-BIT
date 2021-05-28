/*
 *  sps.c
 *  Matus Remen (xremen01)
 *  2020-11-24
 *  1BIT Project 2 - Work with data structures
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SELECTION_COORDS 4
#define INIT_SIZE 10
#define ASCII_SIZE 256
#define MAX_COMMANDS 1000
#define TEMP_VAR_COUNT 10
#define OPTION_ALIGN_TABLE 'a'
#define OPTION_CUT_OFF_EMPTY_COLUMNS 'c'

// var_size contains length of content in variable
typedef struct  ContentVariable{
    int var_size;
    char *var_content;
} content_var_t;

typedef struct TemporaryVariables{
    int saved_selection[MAX_SELECTION_COORDS];
    content_var_t var[TEMP_VAR_COUNT];
} temp_vars_t;

// row_n is beginning row of selection
// col_n is beginning col of selection
// _row_n is ending row of selection
// _col_n is ending col of selection
typedef struct Selection{
    int row_n;
    int col_n;
    int _row_n;
    int _col_n;
} selection_t;

// holds parameters for current command
typedef struct Command{
    char *str_param;
    int num_param[MAX_SELECTION_COORDS];
    void (*func)();
} command_t;

// stores commands from user input
typedef struct Sequence{
    int comm_count;
    command_t *seq;
} comm_seq_t;

typedef struct Cell{
    char *content;
    int size;
    int capacity;
} cell_t;

typedef struct Row{
    cell_t *cell;
    int cell_count;
} row_t;

typedef struct Table{
    row_t *row;
    int row_count;
    int col_count;
    selection_t select;
} table_t;

// initialization commands
int command_sequence_init(comm_seq_t *comm_seq);
int temp_vars_init       (temp_vars_t *temp_vars);
void command_init        (command_t *command);
int table_init           (table_t *table);
int row_init             (row_t *row);
int cell_init            (cell_t *cell);
void selection_init      (selection_t *select);
void delim_init          (int argc, char **argv, int *delim);

// help structure resize commands
int add_command(comm_seq_t *comm_seq);
int add_row    (table_t *table);
int remove_row (table_t *table);
int remove_col (row_t *row);
int add_col    (row_t *row);
int resize_cell(cell_t *cell);
int align_table(table_t *table, char option);

// final commands
void free_memory(table_t *table, comm_seq_t *comm_seq, temp_vars_t *temp_vars);
void prepare_cell_content(cell_t *cell, const int *delim, int *err_num);
void print_table(table_t *table, const int *delim, char *file_name, int *err_num);

// functions that input information
int load_file_into_table             (char *file_name, const int *delim, table_t *table);
int load_command_sequence            (int argc, char **argv, comm_seq_t *comm_seq);
int analyse_substring                (char *substring, comm_seq_t *comm_seq);
int selection_command_check          (char *substring, comm_seq_t *comm_seq);
int check_default_selection_comm     (const char *substring, comm_seq_t *comm_seq);
int table_edit_command_check         (char *substring, comm_seq_t *comm_seq);
int content_edit_command_check       (char *substring, comm_seq_t *comm_seq);
int temporary_variable_argument_check(char *substring, comm_seq_t *comm_seq);
void swap_rows                       (table_t *table, int i);
void swap_cols                       (row_t *row, int i);
int check_table_overflow             (table_t *table, comm_seq_t *comm_seq);

int processor(table_t *table, comm_seq_t *comm_seq, temp_vars_t *temp_vars);

// selection commands
void select_min        (table_t *table, int *err_num);
void select_max        (table_t *table, int *err_num);
void select_find_str   (table_t *table, command_t *params, int *err_num);
void set_selection     (table_t *table, command_t *params, int *err_num);
void load_selection    (table_t *table, temp_vars_t *temp_vars, int *err_num);
void save_selection    (table_t *table, temp_vars_t *temp_vars);

// table structure edit commands
void irow (table_t *table, int *err_num);
void arow (table_t *table, int *err_num);
void drow (table_t *table, int *err_num);
void icol (table_t *table, int *err_num);
void acol (table_t *table, int *err_num);
void dcol (table_t *table, int *err_num);
void clear(table_t *table);

// table content udpate commands
void set  (table_t *table, command_t *params, int *err_num);
void swap (table_t *table, command_t *params, int *err_num);
void sum  (table_t *table, command_t *params, int *err_num);
void avg  (table_t *table, command_t *params, int *err_num);
void count(table_t *table, command_t *params, int *err_num);
void len  (table_t *table, command_t *params, int *err_num);

// commands for work with temporary variables
void def(table_t *table, command_t *params, temp_vars_t *temp_vars, int *err_num);
void use(table_t *table, command_t *params, temp_vars_t *temp_vars, int *err_num);
void inc(command_t *params, temp_vars_t *temp_vars, int *err_num);

typedef enum {ALLOCATION_ERROR=1, FILE_ERROR, INPUT_ERROR, UNKNOWN_COMMAND_ERROR, COMMAND_STRUCTURE_ERROR, 
			  MISSING_VALUE_ERROR, SELECTION_ERROR, COMMAND_COUNT_LIMIT_OVERFLOW, EMPTY_FILE_ERROR
} ERROR_NUMS;

const char *ERROR_MESSAGE[] = {
        "Error: Allocation failed.\n",
        "Error: Unknown or missing file.\n",
        "Error: Input error\n",
        "Error: Unknown command\n",
        "Error: Incorrect command structure\n",
        "Error: Value not found\n",
        "Error: Invalid selection\n",
        "Error: Command count over limit (limit = 1000)\n",
        "Error: Empty file\n"
};

int main(int argc, char **argv){
	if (argc < 2){
        fprintf(stderr, "Missing arguments\n");
        return 1;
    }

    int err_num;

    // declaring key parts for running program
    comm_seq_t command_seq;
    command_seq.seq = NULL;

    temp_vars_t temp_vars;

    int delim[ASCII_SIZE + 1];
    delim_init(argc, argv, delim);

    table_t table;

    /* if whichever step fails, memory is deallocated and error message is pritned */

    // step 1: initializing all new data structures
    // step 2: loading command sequence from input
    // step 3: loading table content from text file
    // step 4: aligning table columns if they aren't consistent
    // step 5: running table processor, which edits it's content
    if ((err_num = temp_vars_init(&temp_vars)) ||
    (err_num = table_init(&table)) ||
    (err_num = command_sequence_init(&command_seq)) ||
    (err_num = load_command_sequence(argc, argv, &command_seq)) ||
    (err_num = load_file_into_table(argv[argc-1], delim, &table)) ||
    (err_num = align_table(&table, OPTION_ALIGN_TABLE)) ||
    (err_num = processor(&table, &command_seq, &temp_vars))){
        free_memory(&table, &command_seq, &temp_vars);
        fprintf(stderr, "%s", ERROR_MESSAGE[err_num-1]);
        return 1;
    }

    // step 6: printing table
    print_table(&table, delim, argv[argc-1], &err_num);
    if (err_num) fprintf(stderr, "%s", ERROR_MESSAGE[err_num-1]);
    // step 7: in case of success freeing whole memory that program allocated
    free_memory(&table, &command_seq, &temp_vars);
    return 0;
}

int command_sequence_init(comm_seq_t *comm_sequence){
    comm_sequence->seq = malloc(sizeof(command_t));
    comm_sequence->comm_count = 1;
    if (comm_sequence == NULL) return ALLOCATION_ERROR;

    command_init(&comm_sequence->seq[0]);
    return 0;
}

void command_init(command_t *command){
    command->func = NULL;
    command->str_param = NULL;
    memset(command->num_param, 0, sizeof(command->num_param));
}

// adds empty command element to command sequence
int add_command(comm_seq_t *comm_sequence){
    comm_sequence->seq = realloc(comm_sequence->seq, (comm_sequence->comm_count+1)*sizeof(command_t));
    if (comm_sequence->seq == NULL) return ALLOCATION_ERROR;
    comm_sequence->comm_count++;

    command_init(&comm_sequence->seq[comm_sequence->comm_count-1]);

    return 0;
}

// setting content of every temporary variable to empty
int temp_vars_init(temp_vars_t *temp_vars){
    for (int j = 0; j < 4; j++) temp_vars->saved_selection[j] = 1;
    for (int i = 0; i < TEMP_VAR_COUNT; i++){
        temp_vars->var[i].var_size = 0;
        temp_vars->var[i].var_content = NULL;
    }
    return 0;
}

// creating empty table of pre-allocated size 1x1
int table_init(table_t *table){
    table->row_count = 1;
    table->col_count = 1;
    table->row = malloc(sizeof(row_t));
    if (table->row == NULL) return ALLOCATION_ERROR;
    selection_init(&table->select);

    return row_init(&table->row[0]);
}

// creating empty row
int row_init(row_t *row){
    row->cell = malloc(sizeof(cell_t));
    if (row->cell == NULL) return ALLOCATION_ERROR;
    row->cell_count = 1;

    return cell_init(&row->cell[row->cell_count-1]);
}

// creating empty cell with pre-allocated capacity of 10 bytes - for 10 characters
int cell_init(cell_t *cell){
    cell->size = 0;
    cell->content = malloc(INIT_SIZE*sizeof(char));
    if (cell->content == NULL) return ALLOCATION_ERROR;
    cell->capacity = INIT_SIZE;
    cell->content[0] = '\0';

    return 0;
}

void selection_init(selection_t *select){
    select->row_n = 1;
    select->col_n = 1;
    select->_row_n = 1;
    select->_col_n = 1;
}

// loading delimeters if present otherwise ' ' (ascii 32) is set as default
void delim_init(int argc, char **argv, int *delim){
    for (int i = 0; i < ASCII_SIZE+1; i++) delim[i] = 0;
    if (argc > 2 && !strcmp(argv[1], "-d")){
        for (int j = 0; j < (int) strlen(argv[2]); j++){
            if (j == 0) delim[ASCII_SIZE] = (int) argv[2][j];
            delim[(int) argv[2][j]] = 1;
        }
    } else{
        delim[32] = 1;
        delim[ASCII_SIZE] = 32;
    }
}

// adding empty row at the end of the table
int add_row(table_t *table){
    table->row = realloc(table->row, (table->row_count+1)*sizeof(row_t));
    if (table->row == NULL) return ALLOCATION_ERROR;
    table->row_count++;
    if (row_init(&table->row[table->row_count-1])) return ALLOCATION_ERROR;

    for (int i = 0; i < (table->col_count - table->row[table->row_count-1].cell_count+i); i++){
        if (add_col(&table->row[table->row_count-1])) return ALLOCATION_ERROR;
    }

    return 0;
}

// removing last column
int remove_row(table_t *table){
    for (int i = 0; i < table->col_count; i++){
        free(table->row[table->row_count-1].cell[i].content);
    }
    free(table->row[table->row_count-1].cell);
    table->row_count--;
    if (table->row_count > 0) {
        table->row = realloc(table->row, table->row_count * sizeof(row_t));
        if (table->row == NULL) return ALLOCATION_ERROR;
    }
    return 0;
}

// adding behind last column in row
int add_col(row_t *row){
    row->cell = realloc(row->cell, (row->cell_count+1)*sizeof(cell_t));
    if (row->cell == NULL) return ALLOCATION_ERROR;
    row->cell_count++;

    return cell_init(&row->cell[row->cell_count-1]);
}

// removing last column in row
int remove_col(row_t *row){
    free(row->cell[row->cell_count-1].content);
    row->cell_count--;
    if (row->cell_count > 0){
        row->cell = realloc(row->cell, row->cell_count*sizeof(cell_t));
        if (row->cell == NULL) return ALLOCATION_ERROR;
    }
    return 0;
}

// extending cell capacity
int resize_cell(cell_t *cell){
    cell->content = realloc(cell->content, (cell->capacity+INIT_SIZE)*sizeof(char));
    if (cell->content == NULL) return ALLOCATION_ERROR;
    cell->capacity += INIT_SIZE;

    return 0;
}

// cutting of empty columns or adding columns if cell counts arent equal in rows
int align_table(table_t *table, char option){
    int err_num = 0;
    // adding missing columns after loading table
    if (option == OPTION_ALIGN_TABLE) {
        for (int ri = 0; ri < table->row_count - 1; ri++) {
            if (table->col_count - table->row[ri].cell_count == 0) break;
            for (int i = 0; i < (table->col_count - table->row[ri].cell_count + i); i++) {
                if (add_col(&table->row[ri])) return ALLOCATION_ERROR;
            }
        }
    }
    // cutting off empty columns from right side and empty rows from the end
    else if (option == OPTION_CUT_OFF_EMPTY_COLUMNS) {
        int empty_col;
        // empty column check
        for (int ci = table->col_count - 1; ci >= 0; ci--) {
            empty_col = 0;
            for (int ri = 0; ri < table->row_count; ri++) {
                if (table->row[ri].cell[ci].content[0] != '\0') {
                    empty_col++;
                    break;
                }
            }
            if (empty_col != 0) break;
            else {
                // removing empty column
                for (int ri = 0; ri < table->row_count; ri++) {
                    if ((err_num = remove_col(&table->row[ri]))) return err_num;
                }
            }
            table->col_count--;
        }
        // removing empty rows
        for (int ri = table->row_count-1; ri; ri--){
            if (table->row[ri].cell_count == 0) {
                if ((err_num = remove_row(table))) return err_num;
            } else break;
        }
    }

    return err_num;
}

// deallocating memory
void free_memory(table_t *table, comm_seq_t *comm_seq, temp_vars_t *temp_vars){
    // table memory
    if (table->row_count > 0) {
        for (int ri = 0; ri < table->row_count; ri++) {
            for (int ci = 0; ci < table->row[ri].cell_count; ci++) {
                free(table->row[ri].cell[ci].content);
            }
            free(table->row[ri].cell);
        }
    }
    free(table->row);
    // command sequence memory
    if (comm_seq->seq != NULL) {
        for (int ci = 0; ci < comm_seq->comm_count; ci++) free(comm_seq->seq[ci].str_param);
        free(comm_seq->seq);
    }
    // temporary variables memory
    for (int i = 0; i < TEMP_VAR_COUNT; i++){
        if (temp_vars->var[i].var_content == NULL) continue;
        free(temp_vars->var[i].var_content);
    }
}

// adds quotes to cell content if it contains any delimeter
void prepare_cell_content(cell_t *cell, const int *delim, int *err_num){
    int is_under_quotes = 0;
    if (cell->size > 2 && cell->content[0] == '"' && cell->content[cell->size-1] == '"') is_under_quotes++;
    for (int i = 0; i < cell->size; i++){
        if (is_under_quotes == 0) {
            if (delim[(int) cell->content[i]]) {
                if (cell->capacity < cell->size + 3) {
                    cell->content = realloc(cell->content, cell->capacity + 2);
                    if (cell->content == NULL) {
                        *err_num = ALLOCATION_ERROR;
                        return;
                    }
                    cell->capacity += 2;
                }
                cell->size += 2;
                char help_string[cell->size + 2];
                sprintf(help_string, "\"%s\"", cell->content);
                strcpy(cell->content, help_string);
                break;
            }
        } else{
            if (delim[(int) cell->content[i]]) break;
            if (i == cell->size-1) {
                cell->content[cell->size-1] = '\0';
                for (int j = 0; j < cell->size - 1; j++) {
                    cell->content[j] = cell->content[j+1];
                }
                cell->size -= 2;
            }
        }
    }
}

// writing table content to file
void print_table(table_t *table, const int *delim, char *file_name, int *err_num){
    FILE *file;
    file = fopen(file_name, "w");
    if ((*err_num = align_table(table, OPTION_CUT_OFF_EMPTY_COLUMNS))) return;
    // for each row
    for (int ri = 0; ri < table->row_count; ri++){
        // for each col
        for (int ci = 0; ci < table->row[ri].cell_count; ci++){
            if (ci != 0){
                fprintf(file, "%c", delim[ASCII_SIZE]);
            }
            prepare_cell_content(&table->row[ri].cell[ci], delim, err_num);
            if (*err_num){
                fclose(file);
                return;
            }
            if (table->row[ri].cell[ci].content[0] != '\0') {
                fprintf(file, "%s", table->row[ri].cell[ci].content);
            }
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

// reading and saving table content
int load_file_into_table(char *file_name, const int *delim, table_t *table){
    FILE *file;
    file = fopen(file_name, "r");
    if (file == NULL) return FILE_ERROR;
    cell_t temp_cell;
    if (cell_init(&temp_cell)) return ALLOCATION_ERROR;
    int letter, is_under_backslash = 0, is_under_quotes = 0;
    int curr_row = 0, curr_col = 0, cell_index;
    // reading text file
    while ((letter = fgetc(file)) != EOF){
        // next row
        if (letter == '\n'){
            if (curr_col+1 > table->col_count) table->col_count = curr_col+1;
            if (add_row(table)){
                free(temp_cell.content);
                fclose(file);
                return ALLOCATION_ERROR;
            }
            is_under_backslash = 0;
            is_under_quotes = 0;
            curr_row++; curr_col = 0;
        }
        // next cell
        else if (delim[letter] && is_under_backslash == 0 && is_under_quotes == 0){
            if (curr_col+1 == table->row[curr_row].cell_count){
                if (add_col(&table->row[curr_row])){
                    free(temp_cell.content);
                    fclose(file);
                    return ALLOCATION_ERROR;
                }
            }
            curr_col++;
            if (curr_col+1 > table->col_count) table->col_count = curr_col+1;
        }
        // filling cell content
        else {
            if (letter == '"' && is_under_backslash == 0){
                is_under_quotes = is_under_quotes == 1 ? 0: 1;
                continue;
            }
            if (letter == '\\' && is_under_backslash == 0){
                is_under_backslash = 1;
                continue;
            }
            else if (is_under_backslash == 1) is_under_backslash = 0;

            cell_index = table->row[curr_row].cell[curr_col].size;
            if (cell_index >= table->row[curr_row].cell[curr_col].capacity-1){
                if (resize_cell(&table->row[curr_row].cell[curr_col])){
                    free(temp_cell.content);
                    fclose(file);
                    return ALLOCATION_ERROR;
                }
            }
            table->row[curr_row].cell[curr_col].content[cell_index] = (char) letter;
            table->row[curr_row].cell[curr_col].content[cell_index+1] = '\0';
            table->row[curr_row].cell[curr_col].size++;
        }
    }

    fclose(file);
    free(temp_cell.content);
    if (remove_row(table)) return ALLOCATION_ERROR;
    if (table->row_count == 0) return EMPTY_FILE_ERROR;
    return 0;
}

// argument parsing
int load_command_sequence(int argc, char **argv, comm_seq_t *comm_seq){
    int i = 1;
    if(!strcmp(argv[1], "-d")) i = 3;
    // check if argv meets requirements - COMM_SEQ and FILENAME
    if (i+1 >= argc) return INPUT_ERROR;

    int err_num;
    char *substring;
    int substr_i = 0;
    int substr_size = INIT_SIZE*2;
    substring = malloc(substr_size*sizeof(char));
    if (substring == NULL) return ALLOCATION_ERROR;

    // collecting command and it's parameters
    int j = 0;
    while (argv[i][j] != '\0'){
        if (argv[i][j] == ';'){
            if (j == 0){
                free(substring);
                return INPUT_ERROR;
            }
            if (argv[i][j-1] != '\\') {
                substring[substr_i] = '\0';
                // sending substring for analysis
                if ((err_num = analyse_substring(substring, comm_seq))) {
                    free(substring);
                    return err_num;
                }
                substr_i = 0;
            }
        } else{
            // collecting content between semicolons
            if (substr_i+1 >= substr_size){
                substring = realloc(substring, (substr_size+INIT_SIZE)*sizeof(char));
                substr_size += INIT_SIZE;
                if (substring == NULL) return ALLOCATION_ERROR;
            }
            substring[substr_i++] = argv[i][j];
        }
        j++;
    }

    substring[substr_i] = '\0';
    err_num = analyse_substring(substring, comm_seq);

    free(substring);
    return err_num;
}

// command analysis
int analyse_substring(char *substring, comm_seq_t *comm_seq){
    int err_num;
    int has_space = 0;
    for (int i = 0; substring[i] != '\0'; i++) if (substring[i] == ' ') has_space++;
    // selection
    if (substring[0] == '['){
        if ((err_num = selection_command_check(substring, comm_seq))) return err_num;
    }
    // table structure edit
    else if (substring[0] >= 'a' && substring[0] <= 'z' && has_space == 0){
        if ((err_num = table_edit_command_check(substring, comm_seq))) return err_num;
    }
    // table content edit and temporary variables
    else if (substring[0] >= 'a' && substring[0] <= 'z' && has_space){
        if ((err_num = content_edit_command_check(substring, comm_seq))) return err_num;
    } else return UNKNOWN_COMMAND_ERROR;

    return 0;
}

// checkign whether loaded command should change selection
int selection_command_check(char *substring, comm_seq_t *comm_seq){
    int comma_count = 0;
    int is_closed = 0;
    int has_space = 0;
    int err_num;

    if (comm_seq->seq[comm_seq->comm_count-1].func != NULL) if ((err_num = add_command(comm_seq))) return err_num;

    // checking type of selection command
    for (int i = 1; substring[i] != '\0'; i++){
        if (is_closed) return COMMAND_STRUCTURE_ERROR;
        if (substring[i] == ']' && i != 1) is_closed++;
        if (substring[i] == ',') comma_count++;
        if (substring[i] == ' ') has_space++;
    }

    char help_string[11];
    int j = 0;
    int param_i = 0;
    if (is_closed == 0) return ALLOCATION_ERROR;
    // selection of cell with min/max value or using saved selection
    if (comma_count == 0 && has_space == 0){
        if (!strcmp(substring, "[min]")){
            comm_seq->seq[comm_seq->comm_count-1].func = select_min;
        } else if (!strcmp(substring, "[max]")){
            comm_seq->seq[comm_seq->comm_count-1].func = select_max;
        } else if (!strcmp(substring, "[_]")){
            comm_seq->seq[comm_seq->comm_count-1].func = load_selection;
        } else if (!strcmp(substring, "[set]")){
            comm_seq->seq[comm_seq->comm_count-1].func = save_selection;
        } else return UNKNOWN_COMMAND_ERROR;
    // find STR
    } else if (has_space == 1 && comma_count == 0){
        int i = 1;
        for (; substring[i] != '\0'; i++){
            if (substring[i] == ' '){
                help_string[j] = '\0';
                if (strcmp(help_string, "find") != 0) return UNKNOWN_COMMAND_ERROR;
                else{
                    comm_seq->seq[comm_seq->comm_count-1].str_param = malloc((strlen(substring)-i)*sizeof(char));
                    if (comm_seq->seq[comm_seq->comm_count-1].str_param == NULL) return ALLOCATION_ERROR;

                    for (i += 1; substring[i] != ']' && substring[i] != '\0'; i++){
                        comm_seq->seq[comm_seq->comm_count-1].str_param[param_i++] = substring[i];
                    }
                    comm_seq->seq[comm_seq->comm_count-1].func = select_find_str;
                }
                break;
            } else help_string[j++] = substring[i];
        }
    // selection edit - normal [R,C] / [R1,C1,R2,C2] / [_,_]
    } else if ((comma_count == 1 || comma_count == 3) && has_space == 0){
        if (!strcmp(substring, "[_,_]")){
            comm_seq->seq[comm_seq->comm_count-1].num_param[0] = comm_seq->seq[comm_seq->comm_count-1].num_param[1] = -2;
            comm_seq->seq[comm_seq->comm_count-1].func = set_selection;
            return 0;
        }
        if ((err_num = check_default_selection_comm(substring, comm_seq))) return err_num;
        comm_seq->seq[comm_seq->comm_count-1].func = set_selection;
    } else return UNKNOWN_COMMAND_ERROR;

    return 0;
}

// filling selection command's parameters
int check_default_selection_comm(const char *substring, comm_seq_t *comm_seq){
    char *end, help_string[INIT_SIZE+1];
    int j = 0, param_i = 0;
    for (int i = 1; substring[i] != '\0'; i++){
        if (substring[i] == ',' || substring[i] == ']'){
            help_string[j] = '\0';
            comm_seq->seq[comm_seq->comm_count-1].num_param[param_i++] = (int) strtol(help_string, &end, 10);
            if (*end != '\0')
                if ((*end != '_' && *end != '-') || *(end+1) != '\0') return COMMAND_STRUCTURE_ERROR;
            if (*end == '_') comm_seq->seq[comm_seq->comm_count-1].num_param[param_i-1] = -2;
            else if (*end == '-') comm_seq->seq[comm_seq->comm_count-1].num_param[param_i-1] = -1;
            j = 0;
        } else help_string[j++] = substring[i];
    }
    return 0;
}

// checking whether loaded command should edit table structure
int table_edit_command_check(char *substring, comm_seq_t *comm_seq){
    int err_num;
    if (comm_seq->seq[comm_seq->comm_count-1].func != NULL) if ((err_num = add_command(comm_seq))) return err_num;

    if      (!strcmp(substring, "irow"))  comm_seq->seq[comm_seq->comm_count-1].func = irow;
    else if (!strcmp(substring, "arow"))  comm_seq->seq[comm_seq->comm_count-1].func = arow;
    else if (!strcmp(substring, "drow"))  comm_seq->seq[comm_seq->comm_count-1].func = drow;
    else if (!strcmp(substring, "icol"))  comm_seq->seq[comm_seq->comm_count-1].func = icol;
    else if (!strcmp(substring, "acol"))  comm_seq->seq[comm_seq->comm_count-1].func = acol;
    else if (!strcmp(substring, "dcol"))  comm_seq->seq[comm_seq->comm_count-1].func = dcol;
    else if (!strcmp(substring, "clear")) comm_seq->seq[comm_seq->comm_count-1].func = clear;
    else return UNKNOWN_COMMAND_ERROR;

    return 0;
}

// checking whether loaded command should edit table content
int content_edit_command_check(char *substring, comm_seq_t *comm_seq){
    int err_num;
    if (comm_seq->seq[comm_seq->comm_count-1].func != NULL) if ((err_num = add_command(comm_seq))) return err_num;

    int help_string_size, j = 0;
    char *help_string = malloc((INIT_SIZE+1)*sizeof(char));
    if (help_string == NULL) return ALLOCATION_ERROR;
    else help_string_size = INIT_SIZE+1;

    // checking type of content edit command
    int i = 0;
    for (; substring[i] != ' '; i++){
        if (substring[i] > 'z' || substring[i] < 'a'){
            free(help_string);
            return UNKNOWN_COMMAND_ERROR;
        }
        if (i >= help_string_size-1){
            free(help_string);
            return UNKNOWN_COMMAND_ERROR;
        }
        help_string[j++] = substring[i];
    }
    int has_underscore = substring[i+1] == '_' ? 1: 0;
    help_string[j] = '\0';
    j = 0;

    if (!strcmp(help_string, "set")){
        // loading STR parameter of set command
        for (i += 1; substring[i] != '\0'; i++){
            if (substring[i] == '\\') continue;
            if (j >= help_string_size-1){
                help_string = realloc(help_string, (help_string_size+INIT_SIZE)*sizeof(char));
                if (help_string == NULL) return ALLOCATION_ERROR;
                else help_string_size += INIT_SIZE;
            }
            help_string[j++] = substring[i];
        }
        // saving STR parameter
        help_string[j] = '\0';
        comm_seq->seq[comm_seq->comm_count-1].str_param = malloc((strlen(help_string)+1)*sizeof(char));
        if (comm_seq->seq[comm_seq->comm_count-1].str_param == NULL){
            free(help_string);
            return ALLOCATION_ERROR;
        }
        else strcpy(comm_seq->seq[comm_seq->comm_count-1].str_param, help_string);
        comm_seq->seq[comm_seq->comm_count-1].func = set;
        free(help_string);
        return 0;
    }
    else if (has_underscore == 1 && !strcmp(help_string, "def")) comm_seq->seq[comm_seq->comm_count-1].func = def;
    else if (has_underscore == 1 && !strcmp(help_string, "use")) comm_seq->seq[comm_seq->comm_count-1].func = use;
    else if (has_underscore == 1 && !strcmp(help_string, "inc")) comm_seq->seq[comm_seq->comm_count-1].func = inc;
    else if (!strcmp(help_string, "swap"))  comm_seq->seq[comm_seq->comm_count-1].func = swap;
    else if (!strcmp(help_string, "sum"))   comm_seq->seq[comm_seq->comm_count-1].func = sum;
    else if (!strcmp(help_string, "avg"))   comm_seq->seq[comm_seq->comm_count-1].func = avg;
    else if (!strcmp(help_string, "count")) comm_seq->seq[comm_seq->comm_count-1].func = count;
    else if (!strcmp(help_string, "len"))   comm_seq->seq[comm_seq->comm_count-1].func = len;
    else {
        free(help_string);
        return UNKNOWN_COMMAND_ERROR;
    }

    // checking temporary variable command arguments
    if (comm_seq->seq[comm_seq->comm_count-1].func == def ||
    comm_seq->seq[comm_seq->comm_count-1].func == use ||
    comm_seq->seq[comm_seq->comm_count-1].func == inc){
        if ((err_num = temporary_variable_argument_check(substring, comm_seq))){
            free(help_string);
            return err_num;
        }
    // loading directory cell coordinates for command that have selection parameter
    } else{
        for (i += 1; substring[i] != '\0'; i++){
            if (j >= help_string_size-1){
                free(help_string);
                return COMMAND_STRUCTURE_ERROR;
            }
            help_string[j++] = substring[i];
        }
        help_string[j] = '\0';
        err_num = check_default_selection_comm(help_string, comm_seq);
    }

    free(help_string);
    return err_num;
}

// loading index of temporary variable that should be used
int temporary_variable_argument_check(char *substring, comm_seq_t *comm_seq){
    int i = 0;
    for (; substring[i] != ' '; i++);
    // i+1 is underscore, checked before the function is called
    // clearing selection saved in temporary variable
    if (substring[i+2] == '\0'){
        comm_seq->seq[comm_seq->comm_count-1].num_param[0] = -1;
        return 0;
    }
    // because there are only 10 temporary variables 0 - 9 so it there can be only one digit
    else if (substring[i+3] != '\0' || !(substring[i+2] >= '0' && substring[i+2] <= '9')) return COMMAND_STRUCTURE_ERROR;
    else if (substring[i+2] >= '0' && substring[i+2] <= '9')
        comm_seq->seq[comm_seq->comm_count-1].num_param[0] = (int) strtol(&substring[i+2], NULL, 10);

    return 0;
}

// swaping neighbour rows
void swap_rows(table_t *table, int i){
    row_t temp;
    temp = table->row[i];
    table->row[i] = table->row[i-1];
    table->row[i-1] = temp;
}

// swaping neighbour columns
void swap_cols(row_t *row, int i){
    cell_t temp;
    temp = row->cell[i];
    row->cell[i] = row->cell[i-1];
    row->cell[i-1] = temp;
}

// checking if multi-cell selection is correct and if content edit command with directory cell is in dimensions of table
int check_table_overflow(table_t *table, comm_seq_t *comm_seq){
    int tab_dimension[2] = {table->row_count, table->col_count};
    for (int i = 0; i < comm_seq->comm_count; i++){
        if (comm_seq->seq[i].func == def || comm_seq->seq[i].func == use || comm_seq->seq[i].func == inc) continue;

        if (comm_seq->seq[i].func == arow || comm_seq->seq[i].func == irow) tab_dimension[0] += 1;
        else if (comm_seq->seq[i].func == drow) tab_dimension[0] -= 1;
        else if (comm_seq->seq[i].func == acol || comm_seq->seq[i].func == icol) tab_dimension[1] += 1;
        else if (comm_seq->seq[i].func == dcol) tab_dimension[1] -= 1;
        else if (comm_seq->seq[i].func == set_selection)
            if (comm_seq->seq[i].num_param[0] > tab_dimension[0] || comm_seq->seq[i].num_param[1] > tab_dimension[1]) {
                tab_dimension[0] = comm_seq->seq[i].num_param[0];
                tab_dimension[1] = comm_seq->seq[i].num_param[1];
            }
        // checking negative selection except -1 and -2
        // -1 is placeholder for last row/colmun
        // -2 is placeholder for selection of all rows/columns
        if (comm_seq->seq[i].num_param[0] > tab_dimension[0] || comm_seq->seq[i].num_param[1] > tab_dimension[1] ||
        comm_seq->seq[i].num_param[0] < -2 || comm_seq->seq[i].num_param[1] < -2 || comm_seq->seq[i].num_param[2] < -2 ||
        comm_seq->seq[i].num_param[3] < -2){
            return SELECTION_ERROR;
        }
    }

    return 0;
}

// main command processor
int processor(table_t *table, comm_seq_t *comm_seq, temp_vars_t *temp_vars){
    int err_num = 0;
    if (comm_seq->comm_count > MAX_COMMANDS) return COMMAND_COUNT_LIMIT_OVERFLOW;
    if ((err_num = check_table_overflow(table, comm_seq))) return err_num;
    // loop through commands
    for (int i = 0; i < comm_seq->comm_count && err_num == 0; i++){
        // dividing commands according to their function parameters
        // commands irow, arow, drow, icol, acol, dcol, [min], [max]
        if(comm_seq->seq[i].func == irow || comm_seq->seq[i].func == arow || comm_seq->seq[i].func == drow ||
        comm_seq->seq[i].func == icol || comm_seq->seq[i].func == acol || comm_seq->seq[i].func == dcol ||
        comm_seq->seq[i].func == select_min || comm_seq->seq[i].func == select_max){
            comm_seq->seq[i].func(table, &err_num);
        }
        // commands set, sum, avg, count, len, swap, selection, [find STR]
        else if (comm_seq->seq[i].func == set || comm_seq->seq[i].func == sum || comm_seq->seq[i].func == avg ||
        comm_seq->seq[i].func == count || comm_seq->seq[i].func == len || comm_seq->seq[i].func == swap ||
        comm_seq->seq[i].func == set_selection || comm_seq->seq[i].func == select_find_str){
            comm_seq->seq[i].func(table, &comm_seq->seq[i], &err_num);
        }
        else if (comm_seq->seq[i].func == save_selection){
            comm_seq->seq[i].func(table, temp_vars);
        }
        else if (comm_seq->seq[i].func == load_selection){
            comm_seq->seq[i].func(table, temp_vars, &err_num);
        }
        else if (comm_seq->seq[i].func == def || comm_seq->seq[i].func == use){
            comm_seq->seq[i].func(table, &comm_seq->seq[i], temp_vars, &err_num);
        }
        else if (comm_seq->seq[i].func == inc){
            comm_seq->seq[i].func(&comm_seq->seq[i], temp_vars, &err_num);
        }
        else if (comm_seq->seq[i].func == clear){
            comm_seq->seq[i].func(table);
        } else return UNKNOWN_COMMAND_ERROR;
        if (err_num) break;
    }

    return err_num;
}

// finding cell in selection with the lowest number
void select_min(table_t *table, int *err_num){
    char *end;
    double num, min;
    int is_min_initialized = 0;
    int min_coords[] = {-1, -1};
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            // skipping empty cell
            if (table->row[ri].cell[ci].content[0] == '\0') continue;
            // check if cell content is number and saving it's coordinates if it is lowest
            num = strtod(table->row[ri].cell[ci].content, &end);
            if (*end == '\0'){
                if (is_min_initialized == 0){
                    is_min_initialized++;
                    min = num;
                    min_coords[0] = ri; min_coords[1] = ci;
                } else{
                    if (num < min){
                        min = num;
                        min_coords[0] = ri; min_coords[1] = ci;
                    }
                }
            }
        }
    }
    // saving cell coords if found at least one number
    if (min_coords[0] < 0 || min_coords[1] < 0) *err_num = MISSING_VALUE_ERROR;
    else{
        table->select.row_n = table->select._row_n = min_coords[0]+1;
        table->select.col_n = table->select._col_n = min_coords[1]+1;
    }
}

// finding cell with the biggest number
void select_max(table_t *table, int *err_num){
    char *end;
    double num, max;
    int is_max_initialized = 0;
    int min_coords[] = {-1, -1};
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            // skipping empty cell
            if (table->row[ri].cell[ci].content[0] == '\0') continue;
            // check if cell content is number and saving it's coordinates if it is biggest
            num = strtod(table->row[ri].cell[ci].content, &end);
            if (*end == '\0'){
                if (is_max_initialized == 0){
                    is_max_initialized++;
                    max = num;
                    min_coords[0] = ri; min_coords[1] = ci;
                } else{
                    if (num > max){
                        max = num;
                        min_coords[0] = ri; min_coords[1] = ci;
                    }
                }
            }
        }
    }
    // saving cell coords if found at least one number
    if (min_coords[0] < 0 || min_coords[1] < 0) *err_num = MISSING_VALUE_ERROR;
    else{
        table->select.row_n = table->select._row_n = min_coords[0]+1;
        table->select.col_n = table->select._col_n = min_coords[1]+1;
    }
}

// finding first ocurrance of STR in current selection
void select_find_str(table_t *table, command_t *params, int *err_num){
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            // searching for STR in cell
            if (strstr(table->row[ri].cell[ci].content, params->str_param) != NULL){
                table->select.row_n = table->select._row_n = ri+1;
                table->select.col_n = table->select._col_n = ci+1;
                return;
            }
        }
    }
    *err_num = MISSING_VALUE_ERROR;
}

// updating selection and resizes table if needed
void set_selection(table_t *table, command_t *params, int *err_num){
    // checking presence of underscore in command
    // -2 is placeholder for underscore

    // case when underscore is not present
    if (params->num_param[0] != -2 && params->num_param[1] != -2) {
        table->select.row_n = params->num_param[0] == -1 ? table->row_count : params->num_param[0];
        table->select.col_n = params->num_param[1] == -1 ? table->col_count : params->num_param[1];
        if (params->num_param[2] != 0 && params->num_param[3] != 0) {
            table->select._row_n = params->num_param[2] == -1 ? table->row_count : params->num_param[2];
            table->select._col_n = params->num_param[3] == -1 ? table->col_count : params->num_param[3];
        } else{
            table->select._row_n = params->num_param[0] == -1 ? table->col_count : params->num_param[0];
            table->select._col_n = params->num_param[1] == -1 ? table->col_count : params->num_param[1];
        }
    // case when underscore is present
    } else{
        if (params->num_param[0] == -2){
            table->select.row_n = 1;
            table->select._row_n = table->row_count;
        } else{
            table->select.row_n = table->select._row_n = params->num_param[0] == -1 ? table->row_count : params->num_param[0];
        }
        if (params->num_param[1] == -2){
            table->select.col_n = 1;
            table->select._col_n = table->col_count;
        } else{
            table->select.col_n = table->select._col_n = params->num_param[1] == -1 ? table->col_count : params->num_param[1];
        }
    }
    // check if start row is not bigger than end row as well as column
    if (table->select.row_n > table->select._row_n || table->select.col_n > table->select._col_n) *err_num = 7;
    if (*err_num == 0){
        // table alignment if selection exceeds table dimensions
        if (table->select._col_n > table->col_count){
            for (int i = 0; i < table->select._col_n - table->col_count; i++){
                if ((*err_num = add_col(&table->row[table->row_count-1]))) return;
            }
            table->col_count = table->select._col_n;
            if ((*err_num = align_table(table, OPTION_ALIGN_TABLE))) return;
        }
        if (table->select._row_n > table->row_count){
            int temp_row_count = table->row_count;
            for (int i = 0; i < table->select._row_n - temp_row_count; i++){
                if ((*err_num = add_row(table))) return;
            }
        }
    }
}

// setting selection from temporary variable
void load_selection(table_t *table, temp_vars_t *temp_vars, int *err_num){
    if (temp_vars->saved_selection[0] == 0){
        *err_num = SELECTION_ERROR;
        return;
    }
    table->select.row_n = temp_vars->saved_selection[0];
    table->select.col_n = temp_vars->saved_selection[1];
    table->select._row_n = temp_vars->saved_selection[2];
    table->select._col_n = temp_vars->saved_selection[3];
}

// saving curret selection to temporary variable
void save_selection(table_t *table, temp_vars_t *temp_vars){
    temp_vars->saved_selection[0] = table->select.row_n;
    temp_vars->saved_selection[1] = table->select.col_n;
    temp_vars->saved_selection[2] = table->select._row_n;
    temp_vars->saved_selection[3] = table->select._col_n;
}

// inserting row before selection
void irow(table_t *table, int *err_num){
    if ((*err_num = add_row(table))) return;
    int i = table->row_count-1;
    for (; i >= table->select.row_n; i--){
        swap_rows(table, i);
    }
}

// appending row after selection
void arow(table_t *table, int *err_num){
    // adding row
    if ((*err_num = add_row(table))) return;
    int i = table->row_count-1;
    // moving new row to the right position
    for (; i >= table->select._row_n+1; i--){
        swap_rows(table, i);
    }
}

// removing selected rows
void drow(table_t *table, int *err_num){
    // loop through selected rows
    for (int i = table->select._row_n-1; i >= table->select.row_n-1; i--){
        // moving row to the end of the table
        for (int j = i+1; j < table->row_count; j++){
            swap_rows(table, j);
        }
        // removing last row
        if ((*err_num = remove_row(table))) return;
    }
}

// inserting column before selection
void icol(table_t *table, int *err_num){
    table->col_count++;
    // for each row
    for (int ri = 0; ri < table->row_count; ri++) {
        if ((*err_num = add_col(&table->row[ri]))) return;
        // fit new column to right place
        for (int i = table->row[ri].cell_count-1; i >= table->select.col_n; i--){
            swap_cols(&table->row[ri], i);
        }
    }
}

// appending column after selection
void acol(table_t *table, int *err_num){
    table->col_count++;
    // for each row
    for (int ri = 0; ri < table->row_count; ri++) {
        if ((*err_num = add_col(&table->row[ri]))) return;
        // fit new column to right place
        for (int i = table->row[ri].cell_count-1; i >= table->select.col_n+1; i--){
            swap_cols(&table->row[ri], i);
        }
    }
}

// removing selected columns
void dcol(table_t *table, int *err_num){
    // for each row
    for (int ri = 0; ri < table->row_count; ri++) {
        // for selected columns
        for (int i = table->select._col_n-1; i >= table->select.col_n - 1; i--) {
            // moving cell to the end of row
            for (int j = i+1; j < table->row[ri].cell_count; j++) {
                swap_cols(&table->row[ri], j);
            }
            // removing last column
            if ((*err_num = remove_col(&table->row[ri]))) return;
        }
        table->col_count--;
    }
}

// clearing selected cell content
void clear(table_t *table){
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            table->row[ri].cell[ci].content[0] = '\0';
            table->row[ri].cell[ci].size = 0;
        }
    }
}

// setting STR as content of selected cells
void set(table_t *table, command_t *params, int *err_num){
    int str_param_size = (int) strlen(params->str_param);
    // for each row in selection
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        // for each cell in selection
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            // rewriting content of selected cell
            if (table->row[ri].cell[ci].capacity < str_param_size){
                table->row[ri].cell[ci].capacity = ((str_param_size+INIT_SIZE)/INIT_SIZE)*INIT_SIZE;
                table->row[ri].cell[ci].content = realloc(table->row[ri].cell[ci].content, table->row[ri].cell[ci].capacity);
                if (table->row[ri].cell[ci].content == NULL){
                    *err_num = ALLOCATION_ERROR;
                    return;
                }
            }
            table->row[ri].cell[ci].size = str_param_size;
            strcpy(table->row[ri].cell[ci].content, params->str_param);
        }
    }
}

// swaping cells over the table
void swap(table_t *table, command_t *params, int *err_num){
    // 0 was not checked before because some of commands do not require selection and default selection is [0,0] for comm
    if (params->num_param[0] <= 0 || params->num_param[1] <= 0){
        *err_num = SELECTION_ERROR;
        return;
    }
    // moving cell selected by swap command to right position
    cell_t temp;
    params->num_param[0]--; params->num_param[1]--;
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            temp = table->row[params->num_param[0]].cell[params->num_param[1]];
            table->row[params->num_param[0]].cell[params->num_param[1]] = table->row[ri].cell[ci];
            table->row[ri].cell[ci] = temp;
        }
    }
}

// counting values of selected cells and saves sum in chosen cell
void sum(table_t *table, command_t *params, int *err_num){
    // 0 was not checked before because some of commands do not require selection and default selection is [0,0] for comm
    if (params->num_param[0] <= 0 || params->num_param[1] <= 0 ||
    params->num_param[0] > table->row_count || params->num_param[1] > table->col_count){
        *err_num = SELECTION_ERROR;
        return;
    }
    double sum = 0.0;
    double num = 0.0;
    char *end;
    // for each row in selection
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        // for each cell in selection
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            num = strtod(table->row[ri].cell[ci].content, &end);
            if (*end == '\0'){
                sum += num;
            }
        }
    }
    // saving sum to cell selected by command
    char help_str[INIT_SIZE*3];
    sprintf(help_str, "%g", sum);
    int help_str_size = (int) strlen(help_str);
    params->num_param[0]--; params->num_param[1]--;
    if (table->row[params->num_param[0]].cell[params->num_param[1]].capacity < help_str_size){
        table->row[params->num_param[0]].cell[params->num_param[1]].capacity = ((help_str_size+INIT_SIZE)/INIT_SIZE)*INIT_SIZE;
        table->row[params->num_param[0]].cell[params->num_param[1]].content = realloc(table->row[params->num_param[0]].cell[params->num_param[1]].content, table->row[params->num_param[0]].cell[params->num_param[1]].capacity);
        if (table->row[params->num_param[0]].cell[params->num_param[1]].content == NULL){
            *err_num = ALLOCATION_ERROR;
            return;
        }
    }
    strcpy(table->row[params->num_param[0]].cell[params->num_param[1]].content, help_str);
}

// counting average from values in selected cells
void avg(table_t *table, command_t *params, int *err_num){
    // 0 was not checked before because some of commands do not require selection and default selection is [0,0] for comm
    if (params->num_param[0] <= 0 || params->num_param[1] <= 0 ||
    params->num_param[0] > table->row_count || params->num_param[1] > table->col_count){
        *err_num = SELECTION_ERROR;
        return;
    }
    double sum = 0.0;
    double num = 0.0;
    unsigned int count = 0;
    char *end;
    // for each row in selection
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        // for each cell in selection
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            num = strtod(table->row[ri].cell[ci].content, &end);
            if (*end == '\0'){
                sum += num;
                count++;
            }
        }
    }
    // saving average to cell selected by command
    char help_str[INIT_SIZE*3];
    sum = sum/count;
    sprintf(help_str, "%g", sum);
    int help_str_size = (int) strlen(help_str);
    params->num_param[0]--; params->num_param[1]--;
    if (table->row[params->num_param[0]].cell[params->num_param[1]].capacity < help_str_size){
        table->row[params->num_param[0]].cell[params->num_param[1]].capacity = ((help_str_size+INIT_SIZE)/INIT_SIZE)*INIT_SIZE;
        table->row[params->num_param[0]].cell[params->num_param[1]].content = realloc(table->row[params->num_param[0]].cell[params->num_param[1]].content, table->row[params->num_param[0]].cell[params->num_param[1]].capacity);
        if (table->row[params->num_param[0]].cell[params->num_param[1]].content == NULL){
            *err_num = ALLOCATION_ERROR;
            return;
        }
    }
    strcpy(table->row[params->num_param[0]].cell[params->num_param[1]].content, help_str);
}

// counting non-empty cells in selection
void count(table_t *table, command_t *params, int *err_num){
    // 0 was not checked before because some of commands do not require selection and default selection is [0,0] for comm
    if (params->num_param[0] <= 0 || params->num_param[1] <= 0 ||
    params->num_param[0] > table->row_count || params->num_param[1] > table->col_count){
        *err_num = SELECTION_ERROR;
        return;
    }
    unsigned int count = 0;
    // for each row in selection
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        // for each cell in selection
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            if (table->row[ri].cell[ci].content[0] != '\0') count++;
        }
    }
    // saving count of non-empty cells to cell selected by command
    char help_str[INIT_SIZE*3];
    sprintf(help_str, "%d", count);
    int help_str_size = (int) strlen(help_str);
    params->num_param[0]--; params->num_param[1]--;
    if (table->row[params->num_param[0]].cell[params->num_param[1]].capacity < help_str_size){
        table->row[params->num_param[0]].cell[params->num_param[1]].capacity = ((help_str_size+INIT_SIZE)/INIT_SIZE)*INIT_SIZE;
        table->row[params->num_param[0]].cell[params->num_param[1]].content = realloc(table->row[params->num_param[0]].cell[params->num_param[1]].content, table->row[params->num_param[0]].cell[params->num_param[1]].capacity);
        if (table->row[params->num_param[0]].cell[params->num_param[1]].content == NULL){
            *err_num = ALLOCATION_ERROR;
            return;
        }
    }
    strcpy(table->row[params->num_param[0]].cell[params->num_param[1]].content, help_str);
}

// counting characters in selection
void len(table_t *table, command_t *params, int *err_num){
    // 0 was not checked before because some of commands do not require selection and default selection is [0,0] for comm
    if (params->num_param[0] <= 0 || params->num_param[1] <= 0 ||
    params->num_param[0] > table->row_count || params->num_param[1] > table->col_count){
        *err_num = SELECTION_ERROR;
        return;
    }
    // counting characters in selected cells
    unsigned int char_count = 0;
    // for each row in selection
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        // for each cell in selection
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            char_count += table->row[ri].cell[ci].size;
        }
    }
    // saving count of characters to cell selected by command
    char help_str[INIT_SIZE*3];
    sprintf(help_str, "%d", char_count);
    int help_str_size = (int) strlen(help_str);
    params->num_param[0]--; params->num_param[1]--;
    if (table->row[params->num_param[0]].cell[params->num_param[1]].capacity < help_str_size){
        table->row[params->num_param[0]].cell[params->num_param[1]].capacity = ((help_str_size+INIT_SIZE)/INIT_SIZE)*INIT_SIZE;
        table->row[params->num_param[0]].cell[params->num_param[1]].content = realloc(table->row[params->num_param[0]].cell[params->num_param[1]].content, table->row[params->num_param[0]].cell[params->num_param[1]].capacity);
        if (table->row[params->num_param[0]].cell[params->num_param[1]].content == NULL){
            *err_num = ALLOCATION_ERROR;
            return;
        }
    }
    strcpy(table->row[params->num_param[0]].cell[params->num_param[1]].content, help_str);
}

// saving content of (first if multi-cell selection) selected cell into temporary variable
void def(table_t *table, command_t *params, temp_vars_t *temp_vars, int *err_num){
    int new_size = table->row[table->select.row_n-1].cell[table->select.col_n-1].size+1;
    // checking if temporary variable was allocated before
    if (temp_vars->var[params->num_param[0]].var_content == NULL){
        temp_vars->var[params->num_param[0]].var_size = new_size;
        temp_vars->var[params->num_param[0]].var_content = malloc(new_size);
        if (temp_vars->var[params->num_param[0]].var_content == NULL){
            *err_num = ALLOCATION_ERROR;
            return;
        }
    }
    // checking if temporary variable has allocated enough space
    else if (temp_vars->var[params->num_param[0]].var_size < new_size) {
        temp_vars->var[params->num_param[0]].var_content = realloc(temp_vars->var[params->num_param[0]].var_content, new_size);
        if (temp_vars->var[params->num_param[0]].var_content == NULL){
            *err_num = ALLOCATION_ERROR;
            return;
        }
    } else temp_vars->var[params->num_param[0]].var_size = new_size;

    strcpy(temp_vars->var[params->num_param[0]].var_content, table->row[table->select.row_n-1].cell[table->select.col_n-1].content);
}

// setting value from temporary variable into selected cells
void use(table_t *table, command_t *params, temp_vars_t *temp_vars, int *err_num){
    int str_param_size = temp_vars->var[params->num_param[0]].var_size;
    // for each row in selection
    for (int ri = table->select.row_n-1; ri < table->select._row_n; ri++){
        // for each cell in selection
        for (int ci = table->select.col_n-1; ci < table->select._col_n; ci++){
            // if temporary variable is empty then selected cell/-s are cleared
            if (temp_vars->var[params->num_param[0]].var_content == NULL){
                table->row[ri].cell[ci].size = 0;
                table->row[ri].cell[ci].content[0] = '\0';
            }
            // checking if selected cell/-s have enough capacity
            else if (table->row[ri].cell[ci].capacity < str_param_size){
                table->row[ri].cell[ci].capacity = ((str_param_size+INIT_SIZE)/INIT_SIZE)*INIT_SIZE;
                table->row[ri].cell[ci].content = realloc(table->row[ri].cell[ci].content, table->row[ri].cell[ci].capacity);
                if (table->row[ri].cell[ci].content == NULL){
                    *err_num = ALLOCATION_ERROR;
                    return;
                }
            }
            table->row[ri].cell[ci].size = str_param_size;
            strcpy(table->row[ri].cell[ci].content, temp_vars->var[params->num_param[0]].var_content);
        }
    }
}

// incrementing value in temporary variable
void inc(command_t *params, temp_vars_t *temp_vars, int *err_num){
    char *end;
    int num;
    if (temp_vars->var[params->num_param[0]].var_size > 1) {
        num = (int) strtol(temp_vars->var[params->num_param[0]].var_content, &end, 10);
        // if temp variable does not contain numerical value then it is set to one otherwise it is incremented
        num = *end == '\0' ? num + 1 : 1;
    } else num = 1;
    char help_str[INIT_SIZE*3];
    sprintf(help_str, "%d", num);
    int new_size = (int) strlen(help_str)+1;
    // resize of temp variable size
    if (temp_vars->var[params->num_param[0]].var_size < new_size){
        temp_vars->var[params->num_param[0]].var_content = realloc(temp_vars->var[params->num_param[0]].var_content, new_size);
        if (temp_vars->var[params->num_param[0]].var_content == NULL){
            *err_num = ALLOCATION_ERROR;
            return;
        }
    }
    temp_vars->var[params->num_param[0]].var_size = new_size;
    strcpy(temp_vars->var[params->num_param[0]].var_content, help_str);
}
