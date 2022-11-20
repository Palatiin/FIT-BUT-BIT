"""
IPP Project - task 2 - Interpret

Classes: Stack, InputQueue, Frame, InsArguments, Value, Interpret, Instruction
Functions: check_args, prg_exit

Author: Matus Remen (xremen01@stud.fit.vutbr.cz)
Date: 15.04.2022
"""

import os
import re
import sys
import argparse
import xml.etree.ElementTree as ET


DEBUG = False


class Stack:
    """Stack data structure, used for data and frames."""

    def __init__(self):
        self.top_idx = -1
        self.content = []

    def __repr__(self):
        return f"{self.content} <- TOP [{self.top_idx}]"

    def push(self, item):
        self.content.append(item)
        self.top_idx += 1

    def pop(self):
        """Return and remove item from top of the stack.

        Return:
            Item on the top on success, else None.
        """
        if self.top_idx < 0:
            return None
        item = self.content[self.top_idx]
        del self.content[self.top_idx]
        self.top_idx -= 1
        return item

    def top(self):
        return self.content[self.top_idx] if self.top_idx > -1 else None


class InputQueue:
    """Queue data structure, manages user input from files"""

    def __init__(self, filename):
        self.data = []
        with open(filename, "r") as f:
            for line in f:
                self.data.append(line.rstrip("\n"))

    def next(self):
        """Return next item in queue on success, else None."""
        if not self.data:
            return None
        item = self.data[0]
        del self.data[0]
        return item


class Frame:
    """Dictionary structure which manages variables in current data frame"""

    def __init__(self, label=False):
        self.vars = {}
        self.is_label = label

    def __repr__(self):
        return f"{self.vars}"

    def def_var(self, var, val=None):
        """Define variable.
        
        Args:
            var: name of variable
            val: specifying value is usability extension of this class for labels
        """
        if var in self.vars:
            prg_exit(52, f"Frame.def_var: Variable '{var}' redefinition")
        self.vars.update({var: val})

    def set_value(self, var, value):
        """Set variable's value.

        Args:
            var: name of variable
            value: new value of variable
        """
        if var not in self.vars:
            prg_exit(54, f"Frame.set_value: Variable '{var}' not found")
        self.vars[var] = value

    def get_value(self, var, allow_none=False):
        """Get value of variable.

        Args:
            var: name of variable
            allow_none: bool, specific behaviour for instruction 'TYPE'
        Return:
            Value object on success, else exits the program.
        """
        if var not in self.vars:
            rc = 54 if not self.is_label else 52
            prg_exit(rc, f"Frame.get_value: Variable '{var}' not found")
        if not allow_none and self.vars[var] is None:
            prg_exit(56, f"Frame.get_value: Variable '{var}' not defined")
        return self.vars[var]


class InsArguments:
    """Class, that manages Instruction Arguments."""

    def __init__(self, ins):
        """Initialize object - parse and validate order of instruction arguments.
        
        Args:
            ins: Instruction tag from XML.
        """
        self.args = {"arg1": None, "arg2": None, "arg3": None}

        for arg in ins:
            if len(arg.keys()) != 1 or "type" not in arg.keys():
                prg_exit(32, "InsArguments.__init__: Argument tag attributes")
            if arg.tag.lower() not in self.args.keys():
                prg_exit(32, "InsArguments.__init__: Invalid argument tag")
            if self.args[arg.tag]:
                prg_exit(32, "InsArguments.__init__: Redefinition of argument tag (duplicity)")
            self.args[arg.tag.lower()] = Value(arg.get("type"), arg.text)

        self.arg1 = self.args["arg1"]
        self.arg2 = self.args["arg2"]
        self.arg3 = self.args["arg3"]
        self.validate()

    def __repr__(self):
        return f"{self.args}"

    def validate(self):
        """Check instruction arguments tag order"""
        if self.arg3 and (self.arg2 is None or self.arg1 is None):
            prg_exit(32, "InsArguments.validate: Argument values not in order (3-21)")
        if self.arg2 and self.arg1 is None:
            prg_exit(32, "InsArguments.validate: Argument values not in order (2-1)")


class Value:
    """Class for storing variable values. Type-value pairs."""

    def __init__(self, value_type, value):
        self.type = value_type
        self.value = value

    def __repr__(self):
        return f"{self.type}@{self.value}"


class Interpret:
    """Main class of this program, manages and stores data about interpreted code."""

    frame_stack = Stack()
    frame = {"GF": Frame(), "LF": None, "TF": None}
    labels = Frame(label=True)
    call_stack = Stack()
    data_stack = Stack()

    instructions = {}
    curr_order = 1
    executed_ins_count = 0

    prg_input = sys.stdin

    def __init__(self, src, prg_input):
        self.src = src
        Interpret.prg_input = InputQueue(prg_input) if prg_input != sys.stdin else prg_input

    @classmethod
    def stats(cls):
        """Return information about current state of interpret."""
        return str(
            "Interpret state:\n"
            f"\tCurrent instruction position: {cls.curr_order}\n"
            f"\tExecuted instructions: {cls.executed_ins_count}\n"
            f"\tFrames:\n\t\t{cls.frame}\n"
            f"\tFrame stack:\n\t\t{cls.frame_stack}\n"
            f"\tLabels:\n\t\t{cls.labels}\n"
            f"\tCall stack:\n\t\t{cls.call_stack}\n"
            f"\tData stack:\n\t\t{cls.data_stack}\n"
        )

    def process(self):
        """Process XML tree of IPPcode22 programming language instructions.

        Steps:
            1. build xml ElementTree
            2. parse instructions and their arguments from ElementTree
            3. validate order and evaluate 'LABEL' instructions
            4. execute parsed instructions
        """
        # build and validate xml ElementTree
        try:
            tree = ET.ElementTree(file=self.src)
        except FileNotFoundError:
            prg_exit(11, "Interpret.process: Failed to open file")
        except Exception as e:
            prg_exit(31, "Interpret.process: Failed to initialize ElementTree")

        root = tree.getroot()
        if root.tag != "program":
            prg_exit(32, "Interpret.process: Invalid root")
        if "language" not in root.attrib:
            prg_exit(31, "Interpret.process: XML missing attribute 'language'")
        if root.attrib["language"].upper() != "IPPCODE22":
            prg_exit(32, f'Interpret.process: Language \'{root.attrib["language"]}\' not supported')

        # parse instructions and their arguments from ElementTree
        for instruction in root:
            if instruction.tag != "instruction":
                prg_exit(32, f"Interpret.process: Expected instruction tag, got '{instruction.tag}'")
            self._parse_instruction(instruction)

        # validate order and evaluate 'LABEL' instructions
        Interpret._check_instructions()

        # execute parsed instructions
        max_key = max(Interpret.instructions.keys())
        while Interpret.curr_order < max_key + 1:
            instruction = Interpret.instructions.get(Interpret.curr_order, None)
            if instruction:
                instruction.execute()
                Interpret.executed_ins_count += 1
            Interpret.curr_order += 1

    @staticmethod
    def _parse_instruction(ins):
        """Parse instruction and it's arguments from xml ElementTree."""
        if len(ins.attrib) != 2 or "order" not in ins.attrib or "opcode" not in ins.attrib:
            prg_exit(32, "Interpret.parse_instruction: Instruction tag attributes (count/order/opcode)")
        try:
            order = int(ins.attrib["order"])
        except ValueError:
            prg_exit(32, "Interpret._parse_instruction: ValueError - string order")
        opcode = ins.attrib["opcode"].upper()

        args = InsArguments(ins)
        if order in Interpret.instructions:
            prg_exit(32, "Interpret.parse_instruction: Duplicit instruction order number")
        Interpret.instructions[order] = Instruction(opcode, args)

    @classmethod
    def _check_instructions(cls):
        """Validate order and evaluate 'LABEL' instructions."""
        if not all([key > 0 for key in cls.instructions.keys()]):
            prg_exit(32, "Interpret.check_ins_order: Invalid order numbers")

        for i in Interpret.instructions:
            Interpret.curr_order = i
            if Interpret.instructions[i].opcode == "LABEL":
                Interpret.instructions[i].execute()
        Interpret.curr_order = 1


class Instruction:
    """Class that manages Instruction data and executes operation codes."""

    def __init__(self, opcode: str, args: InsArguments):
        self.opcode = opcode
        self.args = args

    def execute(self):
        function = getattr(self, self.opcode, lambda: prg_exit(32, "Instruction.execute: Unknown opcode"))
        function()

    # Helper methods
    @staticmethod
    def check_var(var):
        """Check and validate variable frame and name."""
        if var.type != "var":
            prg_exit(53, "Instruction.check_var: Arg type is not var")

        var_frame, at, var_value = var.value.partition("@")
        if not var_frame or not at:
            prg_exit(32, "Instruction.check_var: Missing frame")
        if var_frame not in Interpret.frame.keys():
            prg_exit(32, f'Instruction.check_var: Invalid frame "{var_frame}"')
        if Interpret.frame[var_frame] is None:
            prg_exit(55, "Instruction.check_var: Undefined frame")
        if not re.match(r"[!$%&*_\-?a-zA-Z]+[!$%&*_\-?a-zA-Z\d]*$", var_value):
            prg_exit(32, "Instruction.check_var: Variable name")
        return var_frame, var_value

    def check_symb(self, symb, allow_none=False):
        """Check, validate and return type and value of symbol."""
        if symb.type == "var":
            var_frame, var_value = self.check_var(symb)
            var_value = Interpret.frame[var_frame].get_value(var_value, allow_none)
            if allow_none and var_value is None:    # specific for instruction 'TYPE'
                return "", None
            return var_value.type, var_value.value

        value = None
        if symb.type == "int":
            value = re.match(r"-?\d+$", symb.value)
        elif symb.type == "string":
            value = re.match(r"(\\\d{3}|\S)*$", symb.value)
        elif symb.type == "bool":
            value = re.match(r"(true|false)$", symb.value)
        elif symb.type == "nil":
            value = re.match(r"nil$", symb.value)
        else:
            prg_exit(53, f'Instruction.check_symb: Unknown symbol type "{symb.type}"')

        if not value:
            prg_exit(32, "Instruction.check_symb: Missing value (regex did not match)")
        return symb.type, value.group(0)

    @staticmethod
    def check_label(label):
        """Check and validate 'label' argument."""
        if label.type != "label":
            prg_exit(53, "Instruction.check_label: Arg type is not label")
        value = re.match(r"\S+$", label.value)
        if not value:
            prg_exit(32, "Instruction.check_label: Label name (regex did not match)")

        return value.group(0)

    def check_arithmetic(self, args):
        """Check operands for arithmetic operations and return their values."""
        symb1_ft, symb1_value = self.check_symb(args.arg2)
        symb2_ft, symb2_value = self.check_symb(args.arg3)

        if symb1_ft != "int":
            prg_exit(53, f'Instruction.check_arithmetic: First operand is not int "{symb1_ft}"')
        if symb2_ft != "int":
            prg_exit(53, f'Instruction.check_arithmetic: Second operand is not int "{symb2_ft}"')

        return int(symb1_value), int(symb2_value)

    def check_relational(self, args):
        """Check operands for relational operations and return their values."""
        symb1_ft, symb1_value = self.check_symb(args.arg2)
        symb2_ft, symb2_value = self.check_symb(args.arg3)

        if symb1_ft != symb2_ft:
            prg_exit(53, "Instruction.check_relational: Operands do not have the same data type")

        if symb1_ft == "string":
            return symb2_value, symb1_value
        return symb1_value, symb2_value

    def check_bool(self, args):
        """Check operands for logical operations and return their values."""
        symb1_ft, symb1_value = self.check_symb(args.arg2)
        symb2_ft, symb2_value = self.check_symb(args.arg3)

        if symb1_ft != symb2_ft:
            prg_exit(53, "Instruction.check_bool: Operands do not have the same data type")
        if symb1_ft != "bool":
            prg_exit(53, "Instruction.check_bool: Operand(s) is not bool")

        symb1_value = True if symb1_value == "true" else False
        symb2_value = True if symb2_value == "true" else False
        return symb1_value, symb2_value

    # Instruction implementations
    def CREATEFRAME(self):
        if self.args.arg1 is not None:
            prg_exit(32, "Instruction.CREATEFRAME: Got arguments (required 0)")
        Interpret.frame["TF"] = Frame()

    def PUSHFRAME(self):
        if self.args.arg1 is not None:
            prg_exit(32, "Instruction.PUSHFRAME: Got arguments (required 0)")
        if Interpret.frame["TF"] is None:
            prg_exit(55, "Instruction.PUSHFRAME: Temporary Frame is undefined")
        Interpret.frame_stack.push(Interpret.frame["TF"])
        Interpret.frame["TF"] = None
        Interpret.frame["LF"] = Interpret.frame_stack.top()

    def POPFRAME(self):
        if self.args.arg1 is not None:
            prg_exit(32, "Instruction.POPFRAME: Got arguments (required 0)")
        frame = Interpret.frame_stack.pop()
        if not frame:
            prg_exit(55, "Instruction.POPFRAME: Empty frame stack")
        Interpret.frame["TF"] = frame
        Interpret.frame["LF"] = Interpret.frame_stack.top()

    def MOVE(self):
        if self.args.arg3 is not None or self.args.arg2 is None:
            prg_exit(32, "Instruction.MOVE: Missing/Too many arguments")
        var_frame, var_value = self.check_var(self.args.arg1)
        symb_ft, symb_value = self.check_symb(self.args.arg2)
        Interpret.frame[var_frame].set_value(var_value, Value(symb_ft, symb_value))

    def DEFVAR(self):
        if self.args.arg2 is not None or self.args.arg1 is None:
            prg_exit(32, "Instruction.DEFVAR: Missing/Too many arguments")
        var_frame, var_value = self.check_var(self.args.arg1)
        Interpret.frame[var_frame].def_var(var_value)

    def LABEL(self):
        if self.args.arg2 is not None or self.args.arg1 is None:
            prg_exit(32, "Instruction.LABEL: Missing/Too many arguments")
        value = self.check_label(self.args.arg1)
        Interpret.labels.def_var(value, Interpret.curr_order)
        self.opcode = "_NOP"

    def _NOP(self):
        """Dummy method. Replaces instruction LABEL, when it's evaluated by Interpret."""
        pass

    def CALL(self):
        if self.args.arg2 is not None or self.args.arg1 is None:
            prg_exit(32, "Instruction.CALL: Missing/Too many arguments")
        label = self.check_label(self.args.arg1)
        Interpret.call_stack.push(Interpret.curr_order)
        Interpret.curr_order = Interpret.labels.get_value(label)

    def RETURN(self):
        if self.args.arg1 is not None:
            prg_exit(32, "Instruction.RETURN: Got arguments (required 0)")
        if Interpret.call_stack.top() is None:
            prg_exit(56, "Instruction.RETURN: Empty call stack")
        Interpret.curr_order = Interpret.call_stack.pop()

    def PUSHS(self):
        if self.args.arg2 is not None or self.args.arg1 is None:
            prg_exit(32, "Instruction.PUSHS: Missing/Too many arguments")

        symb_ft, symb_value = self.check_symb(self.args.arg1)
        Interpret.data_stack.push(Value(symb_ft, symb_value))

    def POPS(self):
        if self.args.arg2 is not None or self.args.arg1 is None:
            prg_exit(32, "Instruction.POPS: Missing/Too many arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        value = Interpret.data_stack.pop()
        if value is None:
            prg_exit(56, "Instruction.POPS: Empty data stack")
        Interpret.frame[var_frame].set_value(var_value, value)

    def ADD(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.ADD: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_value, symb2_value = self.check_arithmetic(self.args)

        value = symb1_value + symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("int", value))

    def SUB(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.SUB: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_value, symb2_value = self.check_arithmetic(self.args)

        value = symb1_value - symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("int", value))

    def MUL(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.MUL: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_value, symb2_value = self.check_arithmetic(self.args)

        value = symb1_value * symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("int", value))

    def IDIV(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.IDIV: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_value, symb2_value = self.check_arithmetic(self.args)
        if symb2_value == 0:
            prg_exit(57, "Instruction.IDIV: Division by zero")

        value = symb1_value // symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("int", value))

    def LT(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.LT: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_value, symb2_value = self.check_relational(self.args)

        value = symb1_value < symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("bool", str(value).lower()))

    def GT(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.GT: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_value, symb2_value = self.check_relational(self.args)

        value = symb1_value > symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("bool", str(value).lower()))

    def EQ(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.EQ: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_value, symb2_value = self.check_relational(self.args)

        value = symb1_value == symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("bool", str(value).lower()))

    def AND(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.AND: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_value, symb2_value = self.check_bool(self.args)

        value = symb1_value and symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("bool", str(value).lower()))

    def OR(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.OR: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_value, symb2_value = self.check_bool(self.args)

        value = symb1_value or symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("bool", str(value).lower()))

    def NOT(self):
        if self.args.arg3 is not None or self.args.arg2 is None:
            prg_exit(32, "Instruction.NOT: Missing/Too many arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb_ft, symb_value = self.check_symb(self.args.arg2)
        if symb_ft != "bool":
            prg_exit(53, "Instruction.NOT: Operand is not bool")

        value = not symb_value
        Interpret.frame[var_frame].set_value(var_value, Value("bool", str(value).lower()))

    def INT2CHAR(self):
        if self.args.arg3 is not None or self.args.arg2 is None:
            prg_exit(32, "Instruction.INT2CHAR: Missing/Too many arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb_ft, symb_value = self.check_symb(self.args.arg2)
        if symb_ft != "int":
            prg_exit(53, "Instruction.INT2CHAR: Operand is not int")

        try:
            value = chr(int(symb_value))
            Interpret.frame[var_frame].set_value(var_value, Value("string", value))
        except ValueError as val_e:
            prg_exit(53, f"Instruction.INT2CHR: {str(val_e)}")
        except Exception as e:
            prg_exit(58, f"Instruction.INT2CHR: {str(e)}")

    def STRI2INT(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.STR2INT: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_ft, symb1_value = self.check_symb(self.args.arg2)
        symb2_ft, symb2_value = self.check_symb(self.args.arg3)
        if symb1_ft != "string":
            prg_exit(53, "Instruction.STRI2INT: First symbol is not string")
        if symb2_ft != "int":
            prg_exit(53, "Instruction.STRI2INT: Second symbol is not int")

        try:
            value = ord(symb1_value[int(symb2_value)])
            Interpret.frame[var_frame].set_value(var_value, Value("int", value))
        except IndexError as idx_e:
            prg_exit(58, f"Instruction.STRI2INT: {str(idx_e)}")
        except ValueError as val_e:
            prg_exit(53, f"Instruction.STRI2INT: {str(val_e)}")
        except Exception as e:
            prg_exit(58, f"Instruction.STRI2INT: {str(e)}")

    def READ(self):
        if self.args.arg3 is not None or self.args.arg2 is None:
            prg_exit(32, "Instruction.READ: Missing/Too many arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        dtype = self.args.arg2
        if dtype.type != "type" or dtype.value not in ("int", "string", "bool"):
            prg_exit(53, "Instruction.READ: Argument 'type' invalid")
        dtype = dtype.value
        try:
            inp = input() if Interpret.prg_input == sys.stdin else Interpret.prg_input.next()
            if dtype == "int":
                value = int(inp)
            elif dtype == "bool":
                value = "true" if inp.lower() == "true" else "false"
            else:
                value = inp
        except Exception:
            value = None

        if value is None:
            dtype = "nil"
            value = "nil"
        Interpret.frame[var_frame].set_value(var_value, Value(dtype, value))

    def WRITE(self):
        if self.args.arg2 is not None or self.args.arg1 is None:
            prg_exit(32, "Instruction.WRITE: Missing/Too many arguments")

        symb_ft, symb_value = self.check_symb(self.args.arg1)
        if symb_ft == "nil":
            symb_value = ""
        elif symb_ft == "string":
            esc_sequences = list(set(re.findall(r"\\(\d{3})", symb_value)))
            for seq in esc_sequences:
                symb_value = symb_value.replace(f"\\{seq}", chr(int(seq)))

        print(symb_value, end="")

    def CONCAT(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.CONCAT: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_ft, symb1_value = self.check_symb(self.args.arg2)
        symb2_ft, symb2_value = self.check_symb(self.args.arg3)
        if symb1_ft != "string" or symb2_ft != "string":
            prg_exit(53, "Instruction.CONCAT: Invalid operand types")

        value = symb1_value + symb2_value
        Interpret.frame[var_frame].set_value(var_value, Value("string", value))

    def STRLEN(self):
        if self.args.arg3 is not None or self.args.arg2 is None:
            prg_exit(32, "Instruction.STRLEN: Missing/Too many arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb_ft, symb_value = self.check_symb(self.args.arg2)
        if symb_ft != "string":
            prg_exit(53, "Instruction.STRLEN: Symbol is not 'string'")

        value = len(symb_value)
        Interpret.frame[var_frame].set_value(var_value, Value("int", value))

    def GETCHAR(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.GETCHAR: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb1_ft, symb1_value = self.check_symb(self.args.arg2)
        symb2_ft, symb2_value = self.check_symb(self.args.arg3)
        if symb1_ft != "string" or symb2_ft != "int":
            prg_exit(53, "Instruction.GETCHAR: Invalid operand types")

        try:
            value = symb1_value[int(symb2_value)]
            Interpret.frame[var_frame].set_value(var_value, Value("string", value))
        except IndexError as idx_e:
            prg_exit(58, f"Instruction.GETCHAR: {str(idx_e)}")
        except ValueError as val_e:
            prg_exit(53, f"Instruction.GETCHAR: {str(val_e)}")
        except Exception as e:
            prg_exit(58, f"Instruction.GETCHAR: {str(e)}")

    def SETCHAR(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.SETCHAR: Missing arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        var_type, value = self.check_symb(self.args.arg1)
        symb1_ft, symb1_value = self.check_symb(self.args.arg2)
        symb2_ft, symb2_value = self.check_symb(self.args.arg3)
        if var_type != "string" or symb1_ft != "int" or symb2_ft != "string":
            prg_exit(53, "Instruction.SETCHAR: Invalid operand types")

        try:
            char = symb2_value[0]
            index = int(symb1_value)
            ex_detect = value[index]    # detect IndexError
            value = value[:index] + char + value[index + 1:]
            Interpret.frame[var_frame].set_value(var_value, Value("string", value))
        except IndexError as idx_e:
            prg_exit(58, f"Instruction.SETCHAR: {str(idx_e)}")
        except ValueError as val_e:
            prg_exit(53, f"Instruction.SETCHAR: {str(val_e)}")
        except Exception as e:
            prg_exit(58, f"Instruction.SETCHAR: {str(e)}")

    def TYPE(self):
        if self.args.arg3 is not None or self.args.arg2 is None:
            prg_exit(32, "Instruction.TYPE: Missing/Too many arguments")

        var_frame, var_value = self.check_var(self.args.arg1)
        symb_ft, _ = self.check_symb(self.args.arg2, allow_none=True)
        Interpret.frame[var_frame].set_value(var_value, Value("string", symb_ft))

    def JUMP(self):
        if self.args.arg2 is not None or self.args.arg1 is None:
            prg_exit(32, "Instruction.JUMP: Missing/Too many arguments")

        label = self.check_label(self.args.arg1)
        Interpret.curr_order = Interpret.labels.get_value(label)

    def JUMPIFEQ(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.JUMPIFEQ: Missing arguments")

        label = self.check_label(self.args.arg1)
        symb1_ft, symb1_value = self.check_symb(self.args.arg2)
        symb2_ft, symb2_value = self.check_symb(self.args.arg3)
        if symb1_ft != symb2_ft:
            if not (symb1_ft == "nil" and symb2_ft != "nil" or symb1_ft != "nil" and symb2_ft == "nil"):
                prg_exit(53, "Instruction.JUMPIFEQ: 2x nil or incompatible symbol types")
        if str(symb1_value) == str(symb2_value):
            Interpret.curr_order = Interpret.labels.get_value(label)

    def JUMPIFNEQ(self):
        if self.args.arg3 is None:
            prg_exit(32, "Instruction.JUMPIFNEQ: Missing arguments")

        label = self.check_label(self.args.arg1)
        symb1_ft, symb1_value = self.check_symb(self.args.arg2)
        symb2_ft, symb2_value = self.check_symb(self.args.arg3)
        if symb1_ft != symb2_ft:
            if not (symb1_ft == "nil" and symb2_ft != "nil" or symb1_ft != "nil" and symb2_ft == "nil"):
                prg_exit(53, "Instruction.JUMPIFNEQ: 2x nil or incompatible symbol types")
        if symb1_value != symb2_value:
            Interpret.curr_order = Interpret.labels.get_value(label)

    def EXIT(self):
        if self.args.arg2 is not None or self.args.arg1 is None:
            prg_exit(32, "Instruction.EXIT: Missing/Too many arguments")

        symb_ft, symb_value = self.check_symb(self.args.arg1)
        if symb_ft != "int":
            prg_exit(53, "Instruction.EXIT: Symbol type is not 'int'")
        if 0 <= int(symb_value) <= 49:
            prg_exit(int(symb_value))
        else:
            prg_exit(57, f"Instruction.EXIT: Symbol value is not between 0 and 49 ({int(symb_value)})")

    def DPRINT(self):
        if self.args.arg2 is not None or self.args.arg1 is None:
            prg_exit(32, "Instruction.DPRINT: Missing/Too many arguments")

        _, symb_value = self.check_symb(self.args.arg1)
        print(symb_value, end="", file=sys.stderr)

    def BREAK(self):
        if self.args.arg1 is not None:
            prg_exit(32, "Instruction.BREAK: Too many arguments")
        print(Interpret.stats(), end="", file=sys.stderr)


def check_args(args):
    """Check program arguments - file existence and accessibility."""
    if args.source == sys.stdin and args.input == sys.stdin:
        prg_exit(10, "check_args: Missing source/input file")
    if args.source != sys.stdin and (not os.path.isfile(args.source) or not os.access(args.source, os.R_OK)):
        prg_exit(11, "check_args: File source does not exist or program does not have access rights")
    if args.input != sys.stdin and (not os.path.isfile(args.input) or not os.access(args.input, os.R_OK)):
        prg_exit(11, "check_args: File input does not exist or program does not have access rights")


def prg_exit(err_code, msg: str = ""):
    """Built-in 'exit()' function with optional debug messages on stderr."""
    if DEBUG:
        print(msg, file=sys.stderr)
    exit(err_code)


if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser(description="Interpret jazyka IPPcode22")
    arg_parser.add_argument(
        "--source",
        dest="source",
        type=str,
        default=sys.stdin,
        help="vstupní soubor s XML reprezentací zdrojového kódu IPPcode22",
    )
    arg_parser.add_argument(
        "--input",
        dest="input",
        type=str,
        default=sys.stdin,
        help="soubor se vstupy pro samotnou interpretaci zadaného zdrojového kódu",
    )
    args = arg_parser.parse_args()

    check_args(args)

    interpret = Interpret(args.source, args.input)
    interpret.process()
