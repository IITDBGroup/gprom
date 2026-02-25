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
FAT_STYLE = "bold black on white"

console=None
options=None

def log(m):
    if options.debug:
        print(m)

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
            log(f"a table's string representation has to have at least two lines:\n\n{inputstr}")
            raise ValueError("a table's string representation has to have at least two lines")
        schema = [ x.strip() for x in lines[0].split('|')[:-1] ]
        numattr = len(schema)
        lines = lines[2:]
        t = Table(schema)
        for l in lines:
            vals = [ x.strip() for x in l.split("|")[:-1] ]
            row = tuple(vals)
            if len(vals) != numattr:
                log(f"Table has {numattr} attributes, but this row has {len(vals)} values:\n\n{vals}\n\n{inputstr}")
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
    setting: Dict[str,object]

    def union(self, other: "GProMSetting") -> "GProMSetting":
        mergedict =  {k: v for d in (self.setting, other.setting) for k, v in d.items()}
        return GProMSetting(mergedict)

    def merge_into(self, other: "GProMSetting") -> "GProMSetting":
        self.settings = self.setting.union(other.setting)

    def __getitem__(self,key):
        return self.settings[key]

    def __setitem__(self, key, value):
        self.settings[key] = value

    def __eq__(self,o):
        return self.setting == o.setting

    def __hash__(self):
        return hash(self.setting)

    def to_list(self):
        result = []
        for o in self.settings.entries():
            v = self.settings[o]
            if v:
                result += [ o, v ]
            else:
                result.append(o)

    @classmethod
    def option_kv_to_str(cls,k:str, v:object):
        if not v:
            return k
        else:
            return k + " " + str(v)

    def __str__(self):
        return ' '.join([ GProMSetting.option_kv_to_str(*kv) for kv in self.setting.items()])

@dataclass
class GProMSettings:
    settings: Dict[str, GProMSetting]

    def __getitem__(self,key):
        return self.settings[key]

    def __setitem__(self, key, value):
        self.settings[key] = value

    def __iter__(self):
        return self.settings.keys().__iter__()

    def __str__(self):
        return str(self.settings)

    def names(self):
        return self.settings.keys()

    def combinations(self,other: "GProMSettings"):
        return GProMSettings({k2+"-"+k: other.settings[k].union(self.settings[k2]) for k2 in self.settings for k in other.settings })

    def extensions(self,name,other: "GProMSettings"):
        return GProMSettings({name+"-"+k: other.settings[k].union(self.settings[name]) for k in other.settings })

    def singleton(self,name):
        return GProMSettings({name:self.settings[name]})

@dataclass
class GProMTest:
    name: tuple[str]
    extra_settings: GProMSettings
    disallowed_settings: GProMSettings

    def get_name_str(self):
        return '.'.join(self.name)

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
        files = [ f for f in files if f.endswith(".xml") ]
        root = GProMTestSuite((), None, None)
        for f in files:
            nameparts = GProMXMLTestLoader.name_parts_from_file_name(f)
            filesuite = GProMXMLTestLoader.load_xml_test_cases(dir,f)
            GProMXMLTestLoader.register_test_suit(root, nameparts, filesuite)
        return root

    @classmethod
    def name_parts_from_file_name(cls,f):
        filename = os.path.basename(f)
        name, extension = os.path.splitext(filename)
        return tuple(name.split("."))

    @classmethod
    def register_test_suit(cls, root: GProMTestSuite, nameparts: tuple[str], thetest: GProMTestSuite):
        cur = root
        for n in [ tuple(nameparts[0:i]) for i in range(1, len(nameparts) - 1) ]:
            if n not in cur.tests:
                cur.tests[n] = GProMTestSuite(n,None,None)
            cur = cur.tests[n]
        cur.tests[nameparts] = thetest

    @classmethod
    def load_xml_test_cases(cls,dir: str, f: str) -> GProMTestSuite:
        log(f"PROCESS TESTCASE FILE: {dir}/{f}")
        testcases = {}
        propdict = java_xml_properties_to_dict(dir + "/" + f)
        queries = sorted(list(set([ q.split('.')[0] for q in propdict if q[0] == 'q' ])),key = lambda x: int(x[1:]))
        # TODO read gprom settings
        suitenameparts = GProMXMLTestLoader.name_parts_from_file_name(f)

        for q in queries:
            qkey = q + '.query'
            rkey = q + '.result'
            skey = q + '.issorted'
            dkey = q + '.disabled'
            testcasename = tuple(list(suitenameparts) + [q])
            query = propdict[qkey]
            result = propdict[rkey]
            issorted = propdict[skey] if skey in propdict else False
            disabled = dkey in propdict
            log(f"PARSE TEST CASE {q} [{testcasename} sorted:{issorted} disabled:{disabled} from file <{f}>:\n{query}\n\n{result}")
            if not disabled:
                t = OrderedTable.from_str(result) if issorted else Table.from_str(result)
                testcases[q] = GProMTestCase(testcasename, None, None, query, t, issorted)

        return GProMTestSuite(suitenameparts, None, None, testcases)

def java_xml_properties_to_dict(file:str):
    xml = ET.parse(file)
    d = { x.get('key'):x.text for x in xml.getroot().findall('entry') }
    log(f"Read properties file {file} with keys:\n{'\n'.join(d.keys())}")
    return d


@dataclass
class GProMTestRunner:
    root: GProMTestSuite
    gprompath: str
    conf: GProMSettings
    failonerror: bool = False
    testcases: list[str] = None
    results: Dict[str,Dict[str, bool]] = field(default_factory=dict)
    errors: Dict[str,Dict[str, str]] = field(default_factory=dict)

    FAT_STYLE = "bold black on white"

    def run_test(self, test: GProMTestCase, conf: GProMSettings, name: str): #TODO deal with forbidden settings
        log(f"Test case query:\n{test.query}\nwith expected result:\n{test.expected}")
        exp = test.expected
        setting = conf[name]
        if name not in self.results:
            self.results[name] = {}
            self.errors[name] = {}
        try:
            if test.issorted:
                actual = GProMRunner.gprom_exec_to_table(self.gprompath, test.query, setting)
            else:
                actual = GProMRunner.gprom_exec_to_ordered_table(self.gprompath, test.query, setting)
            self.results[name][test.name] = (exp == actual)

            return exp == actual
        except Exception as e:
            self.results[name][test.name] = False
            if self.failonerror:
                raise e
            else:
                self.errors[name][test.name] = str(e)

    def run(self, conf: GProMSettings = None):
        if not conf:
            conf = self.conf
        log(f"Start running tests: {self.testcases}")
        self.results = {}
        self.errors = {}
        self.run_suite(self.root, conf, '')
        self.print_results(self.root)

    def should_run_test(self, t: GProMTest):
        if not self.testcases:
            return True
        for allowed in self.testcases:
            if len(t.name) >= len(allowed) and t.name[:len(allowed)] == allowed:
                return True
        for allowed in self.testcases:
            if len(allowed) > len(t.name) and allowed[:len(t.name)] == t.name:
                return True
        return False

    def run_suite(self, t: GProMTestSuite, parentconf: GProMSettings, setname: str):
        if not self.should_run_test(t):
            return
        log(f"RUN TEST SUITE: [{t.name}]")
        conf = parentconf.extensions(setname, t.extra_settings) if t.extra_settings else parentconf.singleton(setname)
        success = True
        allconfsuccess = True
        for name in conf:
            if name not in self.results:
                self.results[name] = {}
                self.errors[name] = {}
            for child in t.tests.values():
                if self.should_run_test(child):
                    if isinstance(child,GProMTestCase):
                        log(f"run test case {child.name} in suite {t.name}, setting <{name}>")
                        self.run_test(child, conf, name)
                    else:
                        self.run_suite(child, conf, name)
                    success = success and self.results[name][child.name]
            self.results[name][t.name] = success
            allconfsuccess = allconfsuccess and self.results[name][t.name]
        self.results[setname][t.name] = allconfsuccess

    def determine_settings(self, t: GProMTestSuite):
        return [ "" ]

    def print_results(self, t: GProMTestSuite):
        if not self.should_run_test(t):
            return
        console = rich.get_console()
        indentlen = len(t.name) * 4
        testindentlen = indentlen + 4
        blankindent = indentlen * " "
        blackindent = f"[white on black]{blankindent}[/]"
        testblackindent = testindentlen * " "
        testblackindent = f"[white on black]{blankindent}[/]"
        settings = self.determine_settings(t)

        for set in settings:
            suitestr = f"[white on black]SUITE: {t.get_name_str()} SETTING: <{set}> [/]"
            console.print(f"{blackindent}[b white on black]START [/] {suitestr}")
            for child in t.tests.values():
                if isinstance(child,GProMTestCase):
                    mes = f"[black on green]OK[/]   [green]{child.get_name_str()}[/]" if self.results[set][child.name] else f"[white on red]FAIL[/]"
                    console.print(f"{blankindent}{mes}")
                else:
                    self.print_results(child)
            runtests = [ x for x in t.tests.values() if self.should_run_test(x) ]
            numtests = len(runtests)
            numsuccess = reduce(lambda x,y: x + y, [ self.results[set][c] for c in t.tests if c in self.results[set] ], 0)
            allpass = numsuccess == numtests
            mes = f"[black on green]OK {numsuccess}/{numtests} PASSED[/]" if self.results[set][t.name] else f"[white on red]FAIL {numsuccess}/{numtests} PASSED[/]"
            console.print(f"{blackindent}{suitestr} {mes}")

class GProMRunner():

    @classmethod
    def construct_gprom_cmd_as_list(cls, gprom: str, query: str, args: GProMSetting):
        cmdlist = gprom + args.to_list() + ["-query", query]
        return cmdlist

    @classmethod
    def gprom_exec_to_string(cls, gprom: str, query: str, args: GProMSetting):
        cmdlist = GProMRunner.construct_gprom_cmd_as_list(gprom, query, args)
        log(f"run gprom with args:\n\t{' '.join(cmdlist)}")
        process = subprocess.run(cmdlist,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
        return (process.returncode, process.stdout.strip(), process.stderr.strip())

    @classmethod
    def gprom_exec_to_table(cls, gprom: str, query: str, args: GProMSetting):
        res = GProMRunner.gprom_exec_to_string(gprom, query, args)
        return Table.from_str(res)

    @classmethod
    def gprom_exec_to_ordered_table(cls, gprom: str, query: str, args: GProMSetting):
        res = GProMRunner.gprom_exec_to_string(gprom, query, args)
        return OrderedTable.from_str(res)

def gprom_debug_settings():
    return GProMSetting({
        "-Loperator_verbose": None,
        "-Loperator_verbose_props": 2,
        "-loglevel": 3
    })

def default_gprom_settings_from_options(opions):
    common = GProMSetting({"-loglevel": 0})
    if options.backend == 'sqlite':
        settings = common.union(GProMSetting({
            "-backend": "sqlite",
            "-db": options.db
        }))
    elif options.backend == 'duckdb':
        settings = common.union(GProMSetting({
            "-backend": "duckdb",
            "-db": options.db
        }))
    else:
        settings = common.union(GProMSetting({
            "-backend": options.db,
            "-db": options.db
        }))
    return GProMSettings({"": settings})

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
    ap.add_argument("-p", "--port", type=int, default=5432,
                    help="database port")
    ap.add_argument("-d", "--db", type=str, default="./examples/test.db",
                    help="database name")
    ap.add_argument("-P", "--password", type=str, default="test",
                    help="database password")

    args = ap.parse_args()
    return args

def parse_test_cases_selection():
    if options.tests:
        s = options.tests
        options.tests = [ tuple(t.strip().split(".")) for t in s.split(",") ]
        log(f"user selected test cases: {options.tests}")

def main():
    global options
    global console
    console = rich.get_console()
    options = parse_args()
    parse_test_cases_selection()
    conf = default_gprom_settings_from_options(options)
    rootsuite = GProMXMLTestLoader.load_xmls_from_dir('./testcases')
    runner = GProMTestRunner(root=rootsuite,
                    gprompath=options.gprom,
                    conf=conf,
                    failonerror=options.stoponerror,
                    testcases=options.tests)
    runner.run(conf)

if __name__ == '__main__':
    main()
