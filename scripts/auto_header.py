#!/usr/bin/python3
"""Arduino Pendant - Auto Header.

Generates an header file containing explicity environmental variables.

Copyright (c) 2017 Alex Dale
See LICENSE for information.
"""

import argparse
import logging
from pprint import pprint
import os
import re
import sys
from typing import List, Dict

# Number expressions.
DEC_REGEX = re.compile(r"^[1-9]\d*[uU]?[lL]{0,2}$")
OCT_REGEX = re.compile(r"0\d+[uU]?[lL]{0,2}")
HEX_REGEX = re.compile(r"^0x[\dA-Fa-f]+[uU]?[lL]{0,2}$")
NUM_REGEXES = frozenset([DEC_REGEX, OCT_REGEX, HEX_REGEX])

TEST_COMMENT = """This is not a very long string."""


def get_program_options(args: List[str]) -> argparse.Namespace:
    """Get Program Options."""
    parser = argparse.ArgumentParser(
        epilog="Copyright (c) 2017 Alex Dale - See LICENSE")

    parser.add_argument(
        "env_files",
        help="List of files containing key=value pair variables.",
        nargs="+",
        metavar="ENV_FILES")

    parser.add_argument(
        "-o", "--output",
        help="Output header file. (should be a .h file)",
        type=str,
        default=None)

    parser.add_argument(
        "--debug",
        help="Run server in debug mode.",
        action="store_true")

    return parser.parse_args(args=args)


def init_logging(options: argparse.Namespace):
    """Initialize Program Logger."""
    logging_config = {
        "level": logging.DEBUG if options.debug else logging.INFO,
        "format": "%(asctime)s %(name)-15s [%(levelname)-5s] %(message)s",
        "datefmt": "%Y-%m-%d %H:%M:%S"
    }
    logging.basicConfig(**logging_config)


def open_output_file(options: argparse.Namespace):
    """Open Output File."""
    if options.output is None:
        logging.debug("Piping output to stdout.")

        def close_output():
            pass
        return sys.stdout, close_output

    filepath = os.path.abspath(options.output)

    logging.debug("Piping output to {}.".format(filepath))
    # ofile = open(filepath, "w")
    ofile = None

    def close_output():
        # ofile.close()
        pass

    return ofile, close_output


def filter_env_files(options: argparse.Namespace) -> List[str]:
    """Filter Env. Files.

    Produce a list of absolute file pathnames for all of the specified
    env files.  This will exclude any invalid pathnames and produce a
    warning.
    """
    env_files = []
    for filename in options.env_files:
        if os.path.isfile(filename):
            env_files.append(os.path.abspath(filename))
        else:
            logging.warning("File does not exists: {}.".format(filename))
    return env_files


def convert_variable_name(oldname: str) -> str:
    """Convert Variable Name.

    Converts a variable name to be a valid C macro name.
    """
    return oldname.strip().upper().replace(" ", "_")


def remove_quotes(quoted_values: str) -> str:
    """Remove Quotes."""
    if (len(quoted_values) >= 2
            and quoted_values[0] == '"'
            and quoted_values[-1] == '"'):
        return quoted_values[1:-1]
    return quoted_values


def convert_variable_value(oldvalue: str) -> str:
    """Convert Variable Value."""
    tempvalue = remove_quotes(oldvalue.strip())
    if len(tempvalue) == 0:
        return None
    if any(map(lambda exp: exp.match(tempvalue) is not None, NUM_REGEXES)):
        return tempvalue
    tempvalue = tempvalue.replace('"', '\\"')
    return '"{}"'.format(tempvalue)


def process_file_vars(filename: str) -> Dict[str, str]:
    """Process File Variables."""
    env_vars = {}
    with open(filename) as fin:
        for lineno, line in enumerate(fin.readlines()):
            line = line.strip()
            if len(line) == 0 or line[0] == '#':
                continue
            if '=' not in line:
                logging.warn(
                    ("Parse error in {}, line {}: missing assignment '=' "
                     "'operator.").format(filename, lineno+1))
                continue
            if line.count('=') > 1:
                logging.warn(
                    ("Parse error in {}, line {}: too many assignment '=' "
                     "operators.").format(filename, lineno+1))
                continue
            varname, varvalue = line.split('=')
            if len(varname) == 0:
                logging.warn(
                    ("Parser error in {}, line{}: variable name cannot be "
                     "blank.").format(filename, lineno+1))
                continue
            if varname[0].isdigit():
                logging.warn(
                    ("Parser error in {}, line{}: variable name cannot start "
                     "with a digit.").format(filename, lineno+1))
            varname = convert_variable_name(varname)
            varvalue = convert_variable_value(varvalue)
            env_vars[varname] = varvalue
    return env_vars


def generate_comment(content: str, title_mode: bool=False) -> str:
    """Generate Comment.

    Generates the C-style comment using the string provided.
    """
    # Tokenize the input, and remove any irrelevant white space.
    words = list(filter(
        len,
        (content.strip()
         .replace("\n", " ")
         .replace("\t", " ")
         .replace("*/", "*\\/")
         .split())))
    bar = " ".join(["*"]*35)

    if (sum(map(len, words)) + len(words)) < 70:
        if title_mode:
            return "/*\n * {bar}\n * {content}\n * {bar}\n */".format(
                bar=bar,
                content=" ".join(words))
        return "/* {content} */".format(content=" ".join(words))

    if title_mode:
        base = "/*\n * {bar}\n * {{lines}}\n * {bar}\n */".format(
            bar=bar)
    else:
        base = "/*\n * {lines}\n */"

    lines = []
    line = []
    line_len = 0
    for word in words:
        if line_len + len(word) >= 70:
            lines.append(" ".join(line))
            line.clear()
            line_len = 0
        line.append(word)
        line_len += len(word) + 1
    if len(line) > 0:
        lines.append(" ".join(line))

    return base.format(lines="\n * ".join(lines))


def generate_header_content(filename: str, env_vars: Dict[str, str]) -> str:
    """Generate Header Content."""
    pass


def main(args):
    """Auto Header - Main Function."""
    options = get_program_options(args)
    init_logging(options)
    ofile, closer = open_output_file(options)
    env_files = filter_env_files(options)

    env_vars = {}

    for filename in env_files:
        env_vars.update(process_file_vars(filename))

    # TODO: Print the variables in C preprocessor style macros.
    pprint(env_vars)
    print(generate_comment(TEST_COMMENT))

    closer()


if __name__ == "__main__":
    main(sys.argv[1:])
