/*
 *  sheet.c
 *  Matus Remen <xremen01>
 *  2020-11-06
 *  1BIT Project 1 - Work with text
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TE_COUNT 8
#define MAX_COMMANDS 102
#define CELL_SIZE 101
#define ROW_LENGTH 10242

/*
 *  breakpoint[100] represents count of command
 *  breakpoint[101] holds position of last checked index while searching in breakpoints
 */
typedef struct Parameter{
    int breakpoint[MAX_COMMANDS];
    char strParam[CELL_SIZE];
} Parameter;

int editTable(int *d, void (**command)(), Parameter *param);
int structureProcessor(void (**command)(), Parameter *param, int *d, char *row, int colCount);
int editContent(int *d, void (**command)(), Parameter *param);
void rowProcessor(int selection, void (**command)(), Parameter *param, int *d, char *row);
int findBreakpointInRowC(int currRow, Parameter *param);
int findBreakpoint(int currCol, Parameter *param);
void resetParamIndexes(Parameter *param);
int chkRowLimits(const char *row, const int *d);

void chkDelim(int argc, char **argv, int *i, int *d);
int chkCommArgs(int argc, char **argv, int i, int commandArgc, int dashArg, Parameter *param);
int chkParamPairs(Parameter *param);
int chkForTableEditC(int argc, char **argv, int *d, void (**command)(), Parameter *param);
int chkContentEditC(int argc, char **argv, int *d, void (**command)(), Parameter *param);
int countFinalColCount(Parameter *param, int colCount);

bool isEmpty(void (**command)());
void insertionSort(int *bPoints);

void irow(int *d, int finalColCount);
void drow();
void drows();
void arow(int *d, int finalColCount);
void icol(int *d);
void acol(int *d);
void dcol(const int *d, char *row, int *rowIndex, Parameter *param);
void dcols(int *d, char *row, int *rowIndex, Parameter *param);

void alwaysTrue();
void rows(const char *row, int currRow, Parameter *param, int *condition);
void beginswith(const char *row, const int *d, Parameter *param, int *condition);
void contains(const char *row, const int *d, Parameter *param, int *condition);

void printr(const int *d, const char *row);
void cset(const int *d, Parameter *param, char *row, int *rowIndex);
void toLower(const int *d, char *row, int *rowIndex);
void toUpper(const int *d, char *row, int *rowIndex);
void roundC(const int *d, char *row, int *rowIndex);
void integer(const int *d, char *row, int *rowIndex);
void copy(const int *d, const char *row, Parameter *param);
void swap(const int *d, const char *row, Parameter *param);
void move(const int *d, const char *row, Parameter *param);

const char *COMMAND_LIST[] = {
        "irow", "drow", "drows", "icol", "dcol", "dcols", "acol", "arow",
        "rows", "beginswith", "contains",
        "cset", "tolower", "toupper", "round", "int", "copy", "swap", "move",
        "csum", "cavg", "cmin", "cmax", "ccount",
        "cseq", "rseq", "rsum", "ravg", "rmin", "rmax", "rcount"
};

const char *ERROR_LIST[] = {
        "Error: Invalid function params\n",
        "Error: Unknown command\n",
        "Error: Zero arguments\n",
        "Error: Missing params\n",
        "Error: Mixed commands\n",
        "Error: Param order N <= M\n",
        "Error: Table limits passed (check cell size/row length)\n",
        "Error: Empty file\n",
		"Error: Duplicated command\n"
};

int main(int argc, char **argv){
    /*
     *  initialization of default delim, can be edited in function  chkForTableEditC
     *  d - array of possible delimeters, ascii 32 (' ') is default
     *  [256] - holds index of first delimeter
     *  [257] - holds 1 if -d is present
     */
    int d[257];
    memset(d, 0, sizeof(d));
    d[32] = 1;
    d[256] = 32;
    /*
     *  array of funciton pointers
     *  each command is in array only once, then each command's argumets are stored in second array
     *  program consists of 2 cases, if table structure is edited, *command indexes are reserved by precedence:
     *  *command[8] =  [0]   [1]    [2]   [3]   [4]    [5]   [6]   [7]
     *               {irow, drow, drows, icol, dcol, dcols, acol, arow}
     *  if table content is edited then:
     *  command[0] == [rows/beginswith/contains] and command[1] is for one of the rest of commands
     */
    void (*command[TE_COUNT])();
    memset(command, 0, sizeof(command));
    /*
     *  each command until acol holds array of arguments that refers to size of the array of arguments
     *  indexes of param are binded to *command indexes
     */
    Parameter param[8];
    memset(param, 0, sizeof(param));

    /*
     *  checks all program arguments
     *  looks for -d and command presence and stores parameters for functions
     *
     *  var 'mode' represents 0 for table editing or 1 for content editing
     */
    int errNum = chkForTableEditC(argc, argv, d, command, param);
    int mode = 0;
    if (errNum == 2) {
        if (!isEmpty(command)) {
            fprintf(stderr, "%s", ERROR_LIST[4]);
            return 1;
        } else{
            // if none of table edit commands was found
            mode++;
            errNum = chkContentEditC(argc, argv, d, command, param);
        }
    }
    // error check
    if (errNum){
        fprintf(stderr, "%s", ERROR_LIST[errNum-1]);
        return 1;
    }
    // main table processor
    if (!mode){
        insertionSort(param[0].breakpoint);
        insertionSort(param[1].breakpoint);
        insertionSort(param[3].breakpoint);
        insertionSort(param[4].breakpoint);
        errNum = editTable(d, command, param);
    } else{
        if (command[0] == NULL) command[0] = alwaysTrue;
        errNum = editContent(d, command, param);
    }
    // error check
    if (errNum){
        fprintf(stderr, "%s", ERROR_LIST[errNum-1]);
        return 1;
    }

    return 0;
}

// first mode - table editing
int editTable(int *d, void (**command)(), Parameter *param){
    /*
     *   [0]   [1]    [2]   [3]   [4]    [5]   [6]   [7]
     *  {irow, drow, drows, icol, dcol, dcols, acol, arow}
     */
    char row[ROW_LENGTH];
    int currRow = 0, colCount = 1, finalColCount;
    int errNum = 0;
    int cIndex;
    bool isDeleted;

    // loop through table rows
    while (fgets(row, ROW_LENGTH, stdin) != NULL){
        isDeleted = false;
        // counts number of columns from first row
        if (!currRow){
            for (int i = 0; row[i] != '\0' && i < ROW_LENGTH-2; i++) if (d[(int) row[i]]) colCount++;
            finalColCount = countFinalColCount(param, colCount);
        }
        // validation of row
        if (chkRowLimits(row, d)) return 7;
        currRow++;

        // check if current row should be deleted
        cIndex = findBreakpointInRowC(currRow, param);
        while (cIndex != -1){
            if (cIndex == 0) command[cIndex](d, finalColCount);
            else {
                isDeleted = true;
                break;
            }
            cIndex = findBreakpointInRowC(currRow, param);
        }

        // there is row edited and printed if it is not deleted
        if (!isDeleted){
            errNum = structureProcessor(command, param, d, row, colCount);
            // acol at the end of the line
            if (command[6] != NULL) for (int i = 0; i < param[6].breakpoint[0]; i++) acol(d);
            printf("\n");
            resetParamIndexes(param);
        }
        if (errNum) return errNum;
    }
    // arow - after last row
    if (command[7] != NULL) {
        for (int i = 0; i < param[7].breakpoint[0]; i++) arow(d, finalColCount);
    }
	if (currRow == 0) return 8;
    return 0;
}

// row structure editor
int structureProcessor(void (**command)(), Parameter *param, int *d, char *row, int colCount){
    int currCol = 1, ri = 0, cIndex;
    // checks whether an action should be done on first column - fixes column miscounting
    cIndex = findBreakpoint(currCol, param);
    while (cIndex != -1){
        if (cIndex == 3) command[cIndex](d);
        else {
            command[cIndex](d, row, &ri, param);
            currCol++;
        }
        cIndex = findBreakpoint(currCol, param);
    }
    // loop through row
    for (; ri < ROW_LENGTH-2 && row[ri] != '\n' && row[ri] != '\0'; ri++){
        if (d[(int) row[ri]]) {
            // when counter of current column is incremented, program checks whether current row should be edited
            currCol++;
            cIndex = findBreakpoint(currCol, param);
            // loop through breakpoints
            while (cIndex != -1){
                if (cIndex == 3) command[cIndex](d);
                else {
                    command[cIndex](d, row, &ri, param);
                    currCol++;
                }
                cIndex = findBreakpoint(currCol, param);
            }
            if (currCol != 1 && d[(int) row[ri]]) printf("%c", d[256]);
            else if (row[ri] != '\0') printf("%c", row[ri]);
        } else if (row[ri] != '\0') printf("%c", row[ri]);
    }
    if (currCol < colCount) for(int i = 0; i < colCount-currCol; i++) printf("%c", d[256]);
    return 0;
}

// second mode - content editing
int editContent(int *d, void (**command)(), Parameter *param){
    // two rows are stored at one time, second one is as a scanner for the last row
    int currRow = 0, colCount = 1;
    int selection;
    char row[ROW_LENGTH];
    char nextRow[ROW_LENGTH];
    // check for empty file
    if (fgets(nextRow, ROW_LENGTH, stdin) == NULL) return 9;
    strcpy(row, nextRow);
    for (int i = 0; row[i] != '\0' && row[i] != '\n'; i++) if (d[(int) row[i]]) colCount++;
    while (fgets(nextRow, ROW_LENGTH, stdin) != NULL){
        if (chkRowLimits(row, d)) return 7;
        currRow++;
        selection = 0;
        // checking selection
        if (command[0] == rows){
            command[0](nextRow, currRow, &param[0], &selection);
        } else if (command[0] == alwaysTrue){
            selection = 1;
        } else{
            command[0](row, d, &param[0], &selection);
        }
        // editing row
        rowProcessor(selection, command, param, d, row);
        strcpy(row, nextRow);
    }
    // editing last row
    if (chkRowLimits(nextRow, d)) return 7;
    selection = 0;
    nextRow[0] = '\0';
    currRow++;
    if (command[0] == rows){
        command[0](nextRow, currRow, &param[0], &selection);
    } else if (command[0] == alwaysTrue){
        selection = 1;
    } else{
        command[0](row, d, &param[0], &selection);
    }
    rowProcessor(selection, command, param, d, row);
    return 0;
}

// row content editor
void rowProcessor(int selection, void (**command)(), Parameter *param, int *d, char *row){
    int currCol = 1, ri = 0;
    if (selection){
        // separator between commmands that work in single column and that work on whole line
        if (param[1].breakpoint[1] == 0) {
            // checks whether action should be done in first column - fixes error in miscounting columns
            if (currCol == param[1].breakpoint[0]) {
                if (command[1] == cset) command[1](d, param, row, &ri);
                else command[1](d, row, &ri);
            }
            for (; ri < ROW_LENGTH - 2 && row[ri] != '\n' && row[ri] != '\0'; ri++) {
                if (d[(int) row[ri]]) {
                    // when counter of current column is incremented, program checks whether current row should be edited
                    currCol++;
                    if (currCol == param[1].breakpoint[0]) {
                        // applying command on cell
                        if (command[1] == cset) command[1](d, param, row, &ri);
                        else command[1](d, row, &ri);
                        if (d[(int) row[ri]]){
                            currCol++;
                            printf("%c", d[256]);
                        }
                    } else {
                        if (d[(int) row[ri]]) printf("%c", d[256]);
                        else printf("%c", row[ri]);
                    }
                } else printf("%c", row[ri]);
            }
        } else command[1](d, row, &param[1]);
    } else printr(d, row);
    printf("\n");
}

// checks whether current row should be deleted or a row should be inserted before it
int findBreakpointInRowC(int currRow, Parameter *param){
    // returns index of command where currRow is a breakpoint
    // first loop goes through command type
    for (int i = 0; i < 3; i++){
        // loop through command[i] params, seaching current row
        for (int j = param[i].breakpoint[101]; j < param[i].breakpoint[100]; j++){
            // irow & dcol
            if (i != 2 && currRow == param[i].breakpoint[j]) {
                param[i].breakpoint[101] = j+1;
                return i;
            }
            // drows
            else{
                if (currRow >= param[i].breakpoint[j] && currRow <= param[i].breakpoint[j+1]){
                    return i;
                } else j++;
            }
        }
    }
    // if current row was not found between breakpoints -1 is returned
    return -1;
}

// checks whether current column should be edited
int findBreakpoint(int currCol, Parameter *param){
    // returns index of command where current column is as breakpoint
    // first loop goes through command type
    param[5].breakpoint[101] = 0;
    for (int i = 3; i < TE_COUNT-2; i++){
        // loop through command[i] params, searching current column
        for (int j = param[i].breakpoint[101]; j < param[i].breakpoint[100]; j++){
            // dcols
            if (i == 5){
                if (currCol >= param[i].breakpoint[j] && currCol <= param[i].breakpoint[j+1]){
                    param[i].breakpoint[101] = j;
                    return i;
                }
                else j++;
            }
            // icol & acol
            if (currCol == param[i].breakpoint[j]) {
                param[i].breakpoint[101] = j+1;
                return i;
            }
        }
    }
    // if current column was not found between breakpoints -1 is returned
    return -1;
}

// resets start indexes for searching in breakpoints, repeats after row is printed
void resetParamIndexes(Parameter *param){
    for (int i = 2; i < TE_COUNT-2; i++) param[i].breakpoint[101] = 0;
}

// checks whether row is not longer than 10 KiB and cells aren't longer than 100 B
int chkRowLimits(const char *row, const int *d){
    int cellSize = 0;
    int i = 0;
    for (; i < ROW_LENGTH && row[i] != '\0' && row[i] != '\n'; i++){
        if (d[(int) row[i]]){
            cellSize = 0;
        } else cellSize++;
        // cell limit check
        if (cellSize > CELL_SIZE-3) return 7;
    }
    // row limit check
    if (i > ROW_LENGTH-2) return 7;
    return 0;
}

// processes -d command which sets delimeters
void chkDelim(int argc, char **argv, int *i, int *d){
    // turns on delim indexes
    if ((*i+1 < argc) && !strcmp(argv[*i], "-d")){
        d[32] = 0;
        *i += 1;
        for (int j = 0; argv[*i][j] != '\0'; j++){
            // d[256] stores index of main delimeter
            if (j == 0) d[256] = (int)argv[*i][j];
            d[(int)argv[*i][j]] = 1;
        }
        *i += 1;
    }
}

// checks correction of parameters for commands
int chkCommArgs(int argc, char **argv, int i, int commandArgc, int dashArg, Parameter *param){
    // checks correctness of parameters
    for (int j = i; j < i + commandArgc; j++){
        // checking argument presence
        if (j >= argc) return 4;

        // checks if number is correct
        char *end;
        int num = (int) strtol(argv[j], &end, 10);
        if (*end != '\0' && *end != '-') return 1;

        if (num <= 0 && *end != '-') return 1;

        // checks if function can have dash
        if (*end == '-' && !dashArg){
            return 1;
        }

        // fills array of breakpoints
        if (*end == '-') num = -1;
        param->breakpoint[param->breakpoint[100]] = num;
        param->breakpoint[100] += 1;
    }
    return 0;
}

// checks if N <= M
int chkParamPairs(Parameter *param){
    for (int i = 0; i < param->breakpoint[100]; i+=2){
        // check if '-' are used correctly and if '-' not present then check N <= M
        if (param->breakpoint[0] == -1 && param->breakpoint[1] != -1) return 6;
        if (param->breakpoint[1] == -1) return 0;
        if (param->breakpoint[i] > param->breakpoint[i+1]) return 6;
    }
    return 0;
}

// checks arguments for Table Edit commands
int chkForTableEditC(int argc, char **argv, int *d, void (**command)(), Parameter *param){
    /*
     *  goes through argv, collects and stores data about commands that will be executed
     *  returns index of error message if table edit commands are not present or got incorrect params
     */
    int i = 1, errNum = 0;

    while (i < argc){
        chkDelim(argc, argv, &i, d);
        if (i > argc-1) break;
        // irow R
        if (!strcmp(argv[i], COMMAND_LIST[0])){
            if (command[0] == NULL) command[0] = irow;
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, &param[0]);
            i += 2;
        }
        // drow R
        else if (!strcmp(argv[i], COMMAND_LIST[1])){
            if (command[1] == NULL) command[1] = drow;
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, &param[1]);
            i += 2;
        }
        // drows N M
        else if (!strcmp(argv[i], COMMAND_LIST[2])){
            if (command[2] == NULL) command[2] = drows;
            errNum = chkCommArgs(argc, argv, i+1, 2, 1, &param[2]);
            i += 3;
        }
        // icol C
        else if (!strcmp(argv[i], COMMAND_LIST[3])){
            if (command[3] == NULL) command[3] = icol;
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, &param[3]);
            i += 2;
        }
        // dcol C
        else if (!strcmp(argv[i], COMMAND_LIST[4])){
            if (command[4] == NULL) command[4] = dcol;
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, &param[4]);
            i += 2;
        }
        // dcols N M
        else if (!strcmp(argv[i], COMMAND_LIST[5])){
            if (command[5] == NULL) command[5] = dcols;
            errNum = chkCommArgs(argc, argv, i+1, 2, 1, &param[5]);
            i += 3;
        }
        // acol
        else if (!strcmp(argv[i], COMMAND_LIST[6])){
            if (command[6] == NULL) command[6] = acol;
            param[6].breakpoint[0] += 1;
            i++;
        }
        // arow
        else if (!strcmp(argv[i], COMMAND_LIST[7])){
            if (command[8] == NULL) command[7] = arow;
            param[7].breakpoint[0] += 1;
            i++;
        }
        else{
            errNum = 2;
        }
        // checks error presence
        if (errNum) return errNum;
    }
    errNum = chkParamPairs(&param[2]);
    if (errNum == 0) errNum = chkParamPairs(&param[5]);
    return errNum;
}

// checks arguments for Content Edit commands
int chkContentEditC(int argc, char **argv, int *d, void (**command)(), Parameter *param){
    int i = 1;
    int errNum = 0;

    while (i < argc){
        chkDelim(argc, argv, &i, d);
        if (i > argc-1) break;
        // rows N M
        if (!strcmp(argv[i], COMMAND_LIST[8])){
            errNum = chkCommArgs(argc, argv, i+1, 2, 1, param);
            if (command[0] == NULL) {
                if (errNum != 0) return errNum;
                command[0] = rows;
                i += 3;
            }
            else errNum = 9;
        }
            // beginswith C STR
        else if (!strcmp(argv[i], COMMAND_LIST[9])){
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, param);
            if (i+2 < argc) strcpy(param->strParam, argv[i+2]);
            if (command[0] == NULL) {
                if (errNum != 0) return errNum;
                command[0] = beginswith;
                if (i+2 < argc) {
					int len = (int) strlen(argv[i+2]);
					if (len> CELL_SIZE-2) return 7;
					else if (len == 0) return 4;
                    strcpy(param[0].strParam, argv[i+2]);
                    param[0].strParam[len] = '\0';
                } else return 4;
                i += 3;
            }
            else errNum = 9;
        }
            // contains C STR
        else if (!strcmp(argv[i], COMMAND_LIST[10])){
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, param);
            if (command[0] == NULL) {
                if (errNum != 0) return errNum;
                command[0] = contains;
				if (i+2 < argc) {
					int len = (int) strlen(argv[i+2]);
					if (len > CELL_SIZE-2) return 7;
                    strcpy(param[0].strParam, argv[i+2]);
                    param[0].strParam[len] = '\0';
                } else return 4;
                i += 3;
            }
            else errNum = 9;
        }
        // cset C STR
        else if (!strcmp(argv[i], COMMAND_LIST[11])) {
            if (command[1] == NULL) command[1] = cset;
            else return 9;
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, &param[1]);
            if (errNum != 0) return errNum;
            if (i+2 < argc) {
				int len = (int) strlen(argv[i+2]);
				if (len> CELL_SIZE-2) return 7;
                strcpy(param[1].strParam, argv[i+2]);
                param[1].strParam[len] = '\0';
            } else return 4;
            i += 3;
        }
        // toLower C
        else if (!strcmp(argv[i], COMMAND_LIST[12])) {
            if (command[1] == NULL) command[1] = toLower;
            else return 9;
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, &param[1]);
            i += 2;
        }
        // toUpper C
        else if (!strcmp(argv[i], COMMAND_LIST[13])) {
            if (command[1] == NULL) command[1] = toUpper;
            else return 9;
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, &param[1]);
            i += 2;
        }
        // round C
        else if (!strcmp(argv[i], COMMAND_LIST[14])) {
            if (command[1] == NULL) command[1] = roundC;
            else return 9;
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, &param[1]);
            i += 2;
        }
        // integer C
        else if (!strcmp(argv[i], COMMAND_LIST[15])) {
            if (command[1] == NULL) command[1] = integer;
            else return 9;
            errNum = chkCommArgs(argc, argv, i+1, 1, 0, &param[1]);
            i += 2;
        }
        // copy N M
        else if (!strcmp(argv[i], COMMAND_LIST[16])) {
            if (command[1] == NULL) command[1] = copy;
            else return 9;
            errNum = chkCommArgs(argc, argv, i+1, 2, 0, &param[1]);
            i += 3;
        }
        // swap N M
        else if (!strcmp(argv[i], COMMAND_LIST[17])) {
            if (command[1] == NULL) command[1] = swap;
            else return 9;
            errNum = chkCommArgs(argc, argv, i+1, 2, 0, &param[1]);
            i += 3;
        }
        // move N M
        else if (!strcmp(argv[i], COMMAND_LIST[18])) {
            if (command[1] == NULL) command[1] = move;
            else return 9;
            errNum = chkCommArgs(argc, argv, i+1, 2, 0, &param[1]);
            i += 3;
        }
        // unknown command
        else return 2;
        if (errNum) return errNum;
    }

    if (command[0] == rows) errNum = chkParamPairs(param);

    return errNum;
}

// returns final count of columns for one row
int countFinalColCount(Parameter *param, int colCount){
    /*
     *  [0]    [1]    [2]   [3]   [4]    [5]   [6]   [7]
     *  {irow, drow, drows, icol, dcol, dcols, acol, arow}
     */
    int dcols_c = 0;
    for (int i = 0; i < 100 && param[5].breakpoint[i] != 0; i+=2){
        dcols_c += (param[5].breakpoint[i+1] - param[5].breakpoint[i] + 1);
    }

    return colCount + param[3].breakpoint[100] - param[4].breakpoint[100] - dcols_c + param[6].breakpoint[0];
}

// checks whether any commands are present
bool isEmpty(void (**command)()){
    for (int i = 0; i < 8; i++) if (command[i] != NULL) return false;
    return true;
}

// sorts breakpoints
void insertionSort(int *bPoints){
    int i, current, j;
    for (i = 1; i < bPoints[100]; i++)
    {
        current = bPoints[i];
        j = i - 1;

        // moves moves current num while it is on it's position
        while (j >= 0 && bPoints[j] > current)
        {
            bPoints[j+1] = bPoints[j];
            j--;
        }
        bPoints[j+1] = current;
    }
}

// prints final amount of columns
void irow(int *d, int finalColCount){
    for (int i = 0; i < finalColCount-1; i++) printf("%c", d[256]);
    printf("\n");
}

// these two functions are present just to control whether they are present as commands
void drow(){}
void drows(){}

// prints final amount of columns
void arow(int *d, int finalColCount){
    for (int i = 0; i < finalColCount-1; i++) printf("%c", d[256]);
    printf("\n");
}

// inserts column
void icol(int *d){
    printf("%c", d[256]);
}

// appends column after whole row is printed
void acol(int *d){
    printf("%c", d[256]);
}

// deletes column
void dcol(const int *d, char *row, int *rowIndex, Parameter *param){
    int i;
    for (i = *rowIndex+1; i < ROW_LENGTH; i++){
        if (row[i] == '\n' || row[i] == '\0' || d[(int) row[i]]) break;
    }
    if (row[i] == '\n') row[i] = '\0';
    int dcolsStartIndex = param[5].breakpoint[101];
    if (*rowIndex == 0 || param[5].breakpoint[dcolsStartIndex] == 1) *rowIndex = i+1;
    else *rowIndex = i;
}

// deletes column, is called repeatedly
void dcols(int *d, char *row, int *rowIndex, Parameter *param){
    dcol(d, row, rowIndex, param);
}

// selection commands
void alwaysTrue(){}
void rows(const char *row, int currRow, Parameter *param, int *condition){
    if (param->breakpoint[0] == -1 && param->breakpoint[1] == -1){
        if (*row == '\0') *condition = 1;
    } else if (param->breakpoint[1] == -1){
        if (currRow >= param->breakpoint[0]) *condition = 1;
    } else {
        if (currRow >= param->breakpoint[0] && currRow <= param->breakpoint[1]) *condition = 1;
    }
}
void beginswith(const char *row, const int *d, Parameter *param, int *condition){
    int currCol = 1, strParamLen = (int) strlen(param->strParam);
    *condition = 0;
    int col = param->breakpoint[0];
    for (int i = 0; row[i] != '\0' && row[i] != '\n'; i++){
        if (col == currCol){
            for (int j = 0; j < strParamLen; j++){
                if (row[i+j] != param->strParam[j]){
                    *condition = 0;
                    return;
                }
            }
            *condition = 1;
            return;
        }
        if (d[(int) row[i]]) currCol++;
    }
}
void contains(const char *row, const int *d, Parameter *param, int *condition){
    int currCol = 1, strParamLen = (int) strlen(param->strParam);
    *condition = 0;
    int col = param->breakpoint[0];
    for (int i = 0; row[i] != '\0' && row[i] != '\n'; i++){
        if (col == currCol){
            for (int j = 0; j < strParamLen; j++){
                if (row[i+j] != param->strParam[j]) break;
                if (j == strParamLen-1){
                    *condition = 1;
                    return;
                }
            }
        }
        if (d[(int) row[i]]) currCol++;
    }
}

// prints whole row if no other action is needed to be performed here
void printr(const int *d, const char *row){
    for (int i = 0; row[i] != '\n' && row[i] != '\0'; i++) {
        if (d[(int) row[i]]) printf("%c", d[256]);
        else printf("%c", row[i]);
    }
}

// writes string on selected column
void cset(const int *d, Parameter *param, char *row, int *rowIndex){
    int i = *rowIndex;
    if (param[1].breakpoint[0] == 1){
        printf("%s", param[1].strParam);
    } else{
        printf("%c%s", d[256], param[1].strParam);
        i++;
    }
    for (; !d[(int) row[i]] && row[i] != '\n' && row[i] != '\0'; i++);
    if (row[i] == '\n') row[i] = '\0';
    *rowIndex = i;
}

// makes every letter in cell lowercase
// other characters are ignored
void toLower(const int *d, char *row, int *rowIndex){
    int i = *rowIndex;
    int diff = 'a' - 'A';
    if (d[(int) row[i]]){
        i++;
        printf("%c", d[256]);
    }
    for (; !d[(int) row[i]] && row[i] != '\n' && row[i] != '\0'; i++){
        if (row[i] >= 'A' && row[i] <= 'Z'){
            printf("%c", row[i]+diff);
        } else{
            printf("%c", row[i]);
        }
    }
    if (row[i] == '\n') row[i] = '\0';
    *rowIndex = i;
}

// makes every letter in cell uppercase
// other characters are ignored
void toUpper(const int *d, char *row, int *rowIndex){
    int i = *rowIndex;
    int diff = 'a' - 'A';
    if (d[(int) row[i]]){
        i++;
        printf("%c", d[256]);
    }
    for (; !d[(int) row[i]] && row[i] != '\n' && row[i] != '\0'; i++){
        if (row[i] >= 'a' && row[i] <= 'z'){
            printf("%c", row[i]-diff);
        } else{
            printf("%c", row[i]);
        }
    }
    if (row[i] == '\n') row[i] = '\0';
    *rowIndex = i;
}

// rounds number in cell
// command is executed if cell contains only numbers otherwise the cell is not edited
void roundC(const int *d, char *row, int *rowIndex){
    int i = *rowIndex, j = 0;
    char *end;
    char buffer[CELL_SIZE];
    if (d[(int) row[i]]){
        i++;
        printf("%c", d[256]);
    }
    // fills buffer which holds cell content
    for (; !d[(int) row[i]] && row[i] != '\n' && row[i] != '\0'; i++){
        if (j < CELL_SIZE-1) buffer[j++] = row[i];
    }
    buffer[j] = '\0';
    if (row[i] == '\n') row[i] = '\0';

    // rounding
    long double fnum = strtold(buffer, &end);
    if (*end == '\0' && buffer[0] != '\0'){
        fnum = fnum < 0.0 ? fnum-0.5:fnum+0.5;
        sprintf(buffer, "%d", (int)fnum);
    }
    printf("%s", buffer);
    *rowIndex = i;
}

// cuts off decimal point and everything behind it in cell
// command is executed if cell contains only numbers otherwise the cell is not edited
void integer(const int *d, char *row, int *rowIndex){
    int i = *rowIndex, j = 0;
    char *end;
    char buffer[CELL_SIZE];
    if (d[(int) row[i]]){
        i++;
        printf("%c", d[256]);
    }
    // fills buffer which holds cell content
    for (; !d[(int) row[i]] && row[i] != '\n' && row[i] != '\0'; i++){
        if(j < CELL_SIZE-1) buffer[j++] = row[i];
    }
    buffer[j] = '\0';
    if (row[i] == '\n') row[i] = '\0';

    double fnum = strtod(buffer, &end);
    if (*end == '\0' && buffer[0] != '\0'){
        // cuts off everything from decimal point
        sprintf(buffer, "%d", (int) fnum);
    }
    printf("%s", buffer);
    *rowIndex = i;
}

// those commands work with whole row
// copies content from cell N to cell M
void copy(const int *d, const char *row, Parameter *param){
    int currCol = 1;
    char buffer[CELL_SIZE];
    int n = param->breakpoint[0], m = param->breakpoint[1];
    // no action needed if same cell is copied to the same cell
    if (n == m){
        printr(d, row);
        return;
    }
    int j = 0, i;
    // fills buffer
    for (i = 0; row[i] != '\n' && row[i] != '\0'; i++){
        if (currCol == n && !d[(int) row[i]]){
            if (j < CELL_SIZE-1) buffer[j++] = row[i];
        }
        if (d[(int) row[i]]) currCol++;
    }
    if (currCol < n || currCol < m) {
        printr(d, row);
        return;
    }
    buffer[j] = '\0';
    currCol = 1;
    // first column check
    if (m == 1){
        printf("%s", buffer);
        for (i = 0; !d[(int) row[i]] && row[i] != '\n' && row[i] != '\0'; i++);
    } else i = 0;
    // row editing
    for (; row[i] != '\n' && row[i] != '\0'; i++) {
        if (d[(int) row[i]]) {
            currCol++;
            printf("%c", d[256]);
            if (currCol == m) {
                printf("%s", buffer);
                for (; !d[(int) row[i+1]] && row[i+1] != '\n' && row[i+1] != '\0'; i++);
            }
        }else printf("%c", row[i]);
    }
}

// swaps cells N and M
void swap(const int *d, const char *row, Parameter *param){
    int currCol = 1;
    char bufferN[CELL_SIZE];
    char bufferM[CELL_SIZE];
    int n = param->breakpoint[0], m = param->breakpoint[1];
    // no action needed if swaping same cell with same cell
    if (n == m){
        printr(d, row);
        return;
    }
    int j = 0, k = 0, i;
    // fills buffers
    for (i = 0; row[i] != '\n' && row[i] != '\0'; i++){
        // buffer N
        if (currCol == n && !d[(int) row[i]]) if (j < CELL_SIZE-1) bufferN[j++] = row[i];
        // buffer M
        if (currCol == m && !d[(int) row[i]]) if (k < CELL_SIZE-1) bufferM[k++] = row[i];
        if (d[(int) row[i]]) currCol++;
    }
    if (currCol < n || currCol < m) {
        printr(d, row);
        return;
    }
    bufferN[j] = '\0'; bufferM[k] = '\0';
    currCol = 1;

    // first column check
    if (m == 1){
        printf("%s", bufferN);
        // moving index to next column or end of the line
        for (i = 0; !d[(int) row[i]] && row[i] != '\n' && row[i] != '\0'; i++);
    } else if (n == 1){
        printf("%s", bufferM);
        for (i = 0; !d[(int) row[i]] && row[i] != '\n' && row[i] != '\0'; i++);
    } else i = 0;

    // row editing
    for (; row[i] != '\n' && row[i] != '\0'; i++) {
        if (d[(int) row[i]]) {
            currCol++;
            printf("%c", d[256]);
            // swapping
            if (currCol == m) {
                printf("%s", bufferN);
                for (; !d[(int) row[i+1]] && row[i+1] != '\n' && row[i+1] != '\0'; i++);
            } else if (currCol == n) {
                printf("%s", bufferM);
                for (; !d[(int) row[i+1]] && row[i+1] != '\n' && row[i+1] != '\0'; i++);
            }
        }else printf("%c", row[i]);
    }
}

// moves cell N before cell M
void move(const int *d, const char *row, Parameter *param){
    int currCol = 1, colMax;
    char buffer[CELL_SIZE];
    int n = param->breakpoint[0], m = param->breakpoint[1];
    int j = 0, i;
    // no action needed if a cell is moved before itself or before a cell after it
    if (n == m || n == m-1){
        printr(d, row);
        return;
    }
    // fills buffer & counts columns
    for (i = 0; row[i] != '\n' && row[i] != '\0'; i++){
        if (currCol == n && !d[(int) row[i]]){
            if (j < CELL_SIZE-1) buffer[j++] = row[i];
        }
        if (d[(int) row[i]]) currCol++;
    }
    if (currCol < n || currCol < m) {
        printr(d, row);
        return;
    }
    buffer[j] = '\0';
    colMax = currCol, currCol = 1;
    i = 0;
    // first column check
    if (m == 1) printf("%s%c", buffer, d[256]);
    // row editing
    for (; row[i] != '\n' && row[i] != '\0'; i++) {
        if (d[(int) row[i]]) {
            currCol++;
            if (currCol == n+1) continue;
            if (currCol == m) {
                printf("%c%s%c", d[256], buffer, d[256]);
                for (; !d[(int) row[i]] && row[i] != '\n' && row[i] != '\0'; i++);
            } else if (n != colMax || currCol != n) printf("%c", d[256]);
        }else if (currCol != n) printf("%c", row[i]);
    }
}
