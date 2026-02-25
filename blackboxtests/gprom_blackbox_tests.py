#!/usr/bin/env python
from dataclasses import dataclass, field
import subprocess
import os
import rich
from collections import Counter
from functools import reduce
from enum import Enum
import xml.etree.ElementTree as ET
import argparse
from typing import Dict, Union

GPROM_BIN = "../src/command_line/gprom"
DEBUG_ARGS = [ "-Loperator_verbose_props", "2", "-loglevel" ]
FAT_STYLE = "bold black on white"

console=None
options=None

def log(m):
    if options.debug:
        console.print(m)

def logfat(m, other=""):
    if options.debug:
        console.print(80 * " ", style=FAT_STYLE, justify="center")
        console.print(m, style=FAT_STYLE, justify="center")
        console.print(80 * " ", style=FAT_STYLE, justify="center")
        print(other)


class DatabaseBackends(Enum):
    POSTGRES = 1
    SQLITE = 2
    DUCKDB = 3
    MSSQL = 4
    ORACLE = 5

@dataclass
class Table:
    schema: list[str]
    rows: Counter[tuple[str]] = field(default_factory=Counter)

    def __post_init__(self):
        if type(self.rows)  == list:
            self.rows = Counter(self.rows)

    def add_row(self, row: tuple[str]):
        self.rows.update([row])

    def num_rows(self):
        return sum(self.rows.values())

    def __eq__(self,o):
        if self.schema != o.schema:
            return False
        return self.rows == o.rows

    @classmethod
    def from_str(cls, inputstr: str):
        lines = inputstr.split('\n')
        lines = [ x for x in lines if x.strip() != "" ]
        if len(lines) < 2:
            raise ValueError("a table's string representation has to have at least two lines")
        schema = [ x.strip() for x in lines[0].split('|')[:-1] ]
        numattr = len(schema)
        lines = lines[2:]
        t = Table(schema)
        for l in lines:
            vals = [ x.strip() for x in l.split("|")[:-1] ]
            row = tuple(vals)
            if len(vals) != numattr:
                raise ValueError(f"Table has {numattr} attributes, but this row has {len(vals)} values:\n\n{vals}")
            t.add_row(row)
        return t

    @classmethod
    def row_to_string(cls, r, attrvallen):
        return ' |'.join([ ' ' + x.ljust(attrvallen[i]) for i, x in enumerate(r)]) + " |\n"

    def __str__(self):
        result = ""
        attrvallen = [ max([ len(x[i]) for x in self.rows ]) for i in range(0,len(self.schema)) ]
        attrvallen = [ max(attrvallen[i], len(self.schema[i])) for i in range(0,len(self.schema)) ]

        result += Table.row_to_string(self.schema, attrvallen)
        dividerlen = sum(attrvallen) + 3 * len(attrvallen)

        result += "-" * dividerlen + "\n"

        for r in self.rows:
            result += Table.row_to_string(r, attrvallen)

        result = result[:-1]

        return result

@dataclass
class OrderedTable(Table):
    rows: list[tuple[str]] = field(default_factory=list)

    def add_row(self, row: tuple[str]):
        self.rows.append(row)

    def __eq__(self,o):
        if self.schema != o.schema:
            return False
        return self.rows == o.rows

    @classmethod
    def from_str(cls, inputstr: str):
        lines = inputstr.split('\n')
        lines = [ x for x in lines if x.strip() != "" ]
        if len(lines) < 2:
            raise ValueError("a table's string representation has to have at least two lines")
        schema = [ x.strip() for x in lines[0].split('|')[:-1] ]
        numattr = len(schema)
        lines = lines[2:]
        t = OrderedTable(schema)
        for l in lines:
            vals = [ x.strip() for x in l.split("|")[:-1] ]
            row = tuple(vals)
            if len(vals) != numattr:
                raise ValueError(f"Table has {numattr} attributes, but this row has {len(vals)} values:\n\n{vals}")
            t.add_row(row)
        return t

@dataclass
class GProMSetting:
    setting: frozenset[str]

    def union(self, other: "GProMSetting") -> "GProMSetting":
        return GProMSetting(self.setting.union(other.setting))

    def merge_into(self, other: "GProMSetting") -> "GProMSetting":
        self.settings = self.setting.union(other.setting)

    def __eq__(self,o):
        return self.setting == o.setting

    def __hash__(self):
        return hash(self.setting)

    def __str__(self):
        return ' '.join(self.setting)

@dataclass
class GProMSettings:
    settings: Dict[str, GProMSetting]

    def __str__(self):
        return str(self.settings)

@dataclass
class GProMTest:
    name: tuple[str]
    extra_settings: GProMSettings
    disallowed_settings: GProMSettings

@dataclass
class GProMTestCase(GProMTest):
    query: str
    expected: Union[Table,OrderedTable]
    issorted: bool

@dataclass
class GProMTestSuite(GProMTest):
    tests: Dict[tuple[str],GProMTest] = field(default_factory=dict)

class GProMXMLTestLoader:

    @classmethod
    def split_into_components(namestr: str):
        return list(namestr.spit(".")[0:-2])

    @classmethod
    def load_xmls_from_dir(cls,dir: str) -> GProMTestSuite:
        files = os.listdir(dir)
        root = GProMTestSuite("")
        for f in files:
            nameparts = GProMXMLTestLoader.file_name_parts(f)
            filesuite = GProMXMLTestLoader.load_xml_test_cases(f)
            GProMXMLTestLoader.register_test_suit(root, nameparts, filesuite)
        return root

    @classmethod
    def name_parts_from_file_name(cls,f):
        filename = os.path.basename(f)
        name, extension = os.path.splitext(filename)
        return tuple(name.split("."))

    @classmethod
    def register_test_suit(cls, root: GProMTestSuite, nameparts: list[str], thetest: GProMTestSuite):
        cur = root
        for n in [ tuple(nameparts[0:i]) for i in range(1, len(nameparts) - 1) ]:
            if n not in cur.tests:
                cur.tests[n] = GProMTestSuite(n)
            cur = cur.tests[n]
        cur.tests[n] = thetest

    @classmethod
    def load_xml_test_cases(cls,f: str) -> GProMTestSuite:
        testcases = {}
        propdict = java_xml_properties_to_dict(f)
        queries = sorted(list(set([ q.split('.')[0] for q in propdict ])),key = lambda x: int(x[1:]))
        suitenameparts = GProMXMLTestLoader.name_parts_from_file_name(f)

        for q in queries:
            qkey = q + '.query'
            rkey = q + '.result'
            skey = q + '.issorted'
            testcasename = tuple(list(suitenameparts) + [q])
            query = propdict[qkey]
            result = propdict[rkey]
            issorted = propdict[skey] if skey in propdict else False
            t = OrderedTable.from_str(result) if issorted else Table.from_str(result)
            testcases[q] = GProMTestCase(testcasename, None, None, query, t, issorted)

        return GProMTestSuite(suitenameparts, None, None, testcases)

def java_xml_properties_to_dict(file:str):
    xml = ET.parse(file)
    return { x.get('key'):x.text for x in xml.getroot().findall('entry') }


@dataclass
class GProMTestRunner:
    root: GProMTestSuite
    gprompath: str
    conf: GProMSettings
    failonerror: bool = False
    results: Dict[str,Dict[str, bool]] = field(default_factory=dict)
    errors: Dict[str,Dict[str, str]] = field(default_factory=dict)

    FAT_STYLE = "bold black on white"

    def run_test(self, conf: GProMSetting, test: GProMTestCase): #TODO deal with extra and forbidden settings
        exp = test.expected
        try:
            if test.issorted:
                actual = GProMRunner.gprom_exec_to_table(self.gprompath, test.query, conf)
            else:
                actual = GProMRunner.gprom_exec_to_ordered_table(self.gprompath, test.query, self.conf)
            self.results[str(conf)][test.name] = exp == actual

            return exp == actual
        except Exception as e:
            self.results[str(conf)][test.name] = False
            if self.failonerror:
                raise e
            else:
                self.errors[test.name] = str(e)

    def run(self, conf: GProMSettings = None, stop_on_error: bool = False):
        if not conf:
            conf = self.conf
        self.results = {}
        self.run_suite(self.root)

    def run_suite(self, t: GProMTestSuite):
        success = True
        for child in t.tests:
            if child is GProMTestCase:
                self.run_test(child)
            else:
                self.run_suite(child)
            success = success and self.result[child.name]
        self.result[t.name] = success

    def print_results(self, t: GProMTestSuite):
        console = rich.get_console()
        indentlen = (len(self.root.name) - 1) * 4
        blankindent = indentlen * " "
        blackindent = f"[white on black]{blankindent}[/]"

        for child in t.tests:
            if child is GProMTestCase:
                mes = f"[black on green]OK[/]   [green]{t.fullname()}[/]" if self.results[child.name] else f"[white on red]FAIL[/] [red]{t.fullname()}[/]"
                console.print(f"{blankindent}{mes}")
            else:
                console.print(f"{blackindent}[b white on black]START TESTS: {t.name[-1]}[/]")
                self.print_results(child)
                numtests = len(t.tests)
                numsuccess = reduce(lambda x,y: x + y, [ self.results[c] for c in t.tests  ])
                mes = f"[black on green]{numsuccess}/{numtests}[/]" if self.results[t] else f"[white on red]{numsuccess}/{numtests}[/]"
                console.print(f"{blackindent}{mes}[white on black] {t.name[-1]}[/]")

class GProMRunner():

    @classmethod
    def construct_gprom_cmd_as_list(cls, gprom, query, args):
        cmdlist = [ options.gprom ] + args + ["-query", query]
        return cmdlist

    @classmethod
    def gprom_exec_to_string(cls, gprom, query, args):
        cmdlist = GProMRunner.construct_gprom_cmd_as_list(query, args)
        process = subprocess.run(cmdlist,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
        return (process.returncode, process.stdout.strip(), process.stderr.strip())

    @classmethod
    def gprom_exec_to_table(cls, gprom, query, args):
        res = GProMRunner.gprom_exec_to_string(gprom, query, args)
        return Table.from_str(res)

    @classmethod
    def gprom_exec_to_ordered_table(cls, gprom, query, args):
        res = GProMRunner.gprom_exec_to_string(gprom, query, args)
        return OrderedTable.from_str(res)


def parse_args():
    ap = argparse.ArgumentParser(description='Running semantic optimization experiment')
    ap.add_argument('-t', '--tests', type=str, default=None,
                    help=f"run only these tests, this can be a single string or a list separated by comma (default is to run all)")
    ap.add_argument('--gprom', type=str, default=GPROM_BIN,
                    help="use this gprom binary")
    ap.add_argument('-s', '--stoponerror', action='store_true',
                    help="if provided, then stop after the first error")
    ap.add_argument("-D", "--debug", action='store_true',
                    help="debug the process by logging more information.")
    ap.add_argument("-l", "--loglevel", type=int, default=3,
                    help="log level to use when debugging.")
    ap.add_argument("-b", "--backend", type=str, default="sqlite",
                    help="backend database type to use")
    ap.add_argument("-H", "--host", type=str, default="127.0.0.1",
                    help="database host")
    ap.add_argument("-u", "--user", type=str, default="postgres",
                    help="database user")
    ap.add_argument("-p", "--port", type=int, default=5450,
                    help="database port")
    ap.add_argument("-d", "--db", type=str, default="gpromtest",
                    help="database name")
    ap.add_argument("-P", "--password", type=str, default="test",
                    help="database password")

    args = ap.parse_args()
    return args


def main():
    global options
    global console
    console = rich.get_console()
    options = parse_args()
    conf = GProMSettings()
    rootsuite = GProMXMLTestLoader.load_xmls_from_dir('./testcases')
    runner = GProMTestRunner(root=rootsuite,
                    gprompath='../src/command_line/gprom',
                    conf=conf)
    runner.run()

if __name__ == '__main__':
    main()
