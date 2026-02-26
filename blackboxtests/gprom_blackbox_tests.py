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
from pathlib import Path

FAT_STYLE = "bold black on white"
DEFAULT_SETTING_NAME = "default"

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
        if isinstance(self.rows,list):
            self.rows = Counter(self.rows)

    def add_row(self, row: tuple[str]):
        self.rows.update([row])

    def num_rows(self):
        return sum(self.rows.values())

    def __eq__(self,o):
        if self.schema != o.schema:
            return False
        return self.rows == o.rows

    def diff(self, o: "Table"):
        if self.schema != o.schema:
            return f"schemas differ: expected\n\n{self.schema}\n, but got:\n{o.schema}"
        allrows = set(self.rows.keys()).union(o.rows.keys())
        result = ""
        for r in allrows:
            if self.rows[r] != o.rows[r]:
                result += f"row[{Table.row_to_string(r)}]\t multiplicity differs: expected {self.rows[r]}, but was {o.rows[r]}\n"

        return result

    @classmethod
    def from_str(cls, inputstr: str):
        lines = inputstr.split('\n')
        lines = [ x for x in lines if x.strip() != "" ]
        if len(lines) < 2:
            log(f"a table's string representation has to have at least two lines:\n\n{inputstr}")
            raise ValueError("a table's string representation has to have at least two lines")
        schema = [ x.strip().lower() for x in lines[0].split('|')[:-1] ]
        numattr = len(schema)
        lines = lines[2:]
        t = Table(schema)
        for line in lines:
            vals = [ x.strip() for x in line.split("|")[:-1] ]
            row = tuple(vals)
            if len(vals) != numattr:
                log(f"Table has {numattr} attributes, but this row has {len(vals)} values:\n\n{vals}\n\n{inputstr}")
                raise ValueError(f"Table has {numattr} attributes, but this row has {len(vals)} values:\n\n{vals}")
            t.add_row(row)
        return t

    @classmethod
    def row_to_string(cls, r, attrvallen=None, newline=False):
        if not attrvallen:
            attrvallen = [ len(v) for v in r ]
        return ' |'.join([ ' ' + x.ljust(attrvallen[i]) for i, x in enumerate(r)]) + " |" + ("\n" if newline else "")

    def __str__(self):
        result = ""
        attrvallen = [ max([ len(x[i]) for x in self.rows ]) for i in range(0,len(self.schema)) ]
        attrvallen = [ max(attrvallen[i], len(self.schema[i])) for i in range(0,len(self.schema)) ]

        result += Table.row_to_string(self.schema, attrvallen, True)
        dividerlen = sum(attrvallen) + 3 * len(attrvallen)

        result += "-" * dividerlen + "\n"

        for r in self.rows:
            rstr = Table.row_to_string(r, attrvallen, True)
            for i in range(0,self.rows[r]):
                result += rstr

        result = result[:-1]

        return result

@dataclass
class OrderedTable():
    schema: list[str]
    rows: list[tuple[str]] = field(default_factory=list)

    def add_row(self, row: tuple[str]):
        self.rows.update([row])

    def num_rows(self):
        return sum(self.rows.values())

    def __eq__(self,o):
        if self.schema != o.schema:
            return False
        return self.rows == o.rows

    def append(self, row: tuple[str]):
        self.rows.append(row)

    def diff(self, o: "OrderedTable"):
        if self.schema != o.schema:
            return f"schemas differ: expected {self.schema}, but got {o.schema}"
        numself = self.num_rows()
        numo = o.num_rows()
        minrows = min(numself, numo)
        result = ""
        for i, r in enumerate(self.rows[:minrows]):
            if self.rows[i] != o.rows[i]:
                result += f"row[{i}] differs: expected {Table.row_to_string(self.rows[i])}, but was {Table.row_to_string(o.rows[i])}\n"
        if numself > numo:
            for i,r in enumerate(self.rows[minrows:]):
                result += f"row[{i + minrows}] only in expected output: {Table.row_to_string(self.rows[i + minrows])}"
        if numo > numself:
            for i,r in enumerate(o.rows[minrows:]):
                result += f"row[{i + minrows}] only in actuals output: {Table.row_to_string(o.rows[i + minrows])}"

        return result


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
        for line in lines:
            vals = [ x.strip() for x in line.split("|")[:-1] ]
            row = tuple(vals)
            if len(vals) != numattr:
                raise ValueError(f"Table has {numattr} attributes, but this row has {len(vals)} values:\n\n{vals}")
            t.append(row)
        return t

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

    def items(self):
        return self.setting.items()

    def to_list(self):
        result = []
        for o,v in self.setting.items():
            if v is not None:
                result += [ o, v ]
            else:
                result.append(o)
        return result

    @classmethod
    def option_kv_to_str(cls,k:str, v:object):
        if v is None:
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

    @classmethod
    def merge(cls, s1: "GProMSettings", s2: "GProMSettings"):
        if s1 is None:
            return s2
        if s2 is None:
            return s1
        mergeddict = { **s1.settings, **s2.settings }
        return GProMSettings(mergeddict)

    def combinations(self,other: "GProMSettings"):
        """
        Build all combinations of settings from self with other. For conflicting values for options, other wins.
        """
        return GProMSettings({k2+"."+k: self.settings[k2].union(other.settings[k]) for k2 in self.settings for k in other.settings })

    def extensions(self,name,other: "GProMSettings"):
        return GProMSettings({name+"."+k: self.settings[name].union(other.settings[k]) for k in other.settings })

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

    def to_str_with_indent(self,indent):
        tabs = "\t" * indent
        childtabs = "\t" * (indent + 1)
        result = tabs + self.get_name_str() + "\n"
        for t in self.tests:
            test = self.tests[t]
            if isinstance(test, GProMTestSuite):
                result += test.to_str_with_indent(indent + 1)
            else:
                result += childtabs + test.get_name_str() + "\n"
        result += "\n"
        return result

    def __str__(self):
        result = self.to_str_with_indent(0)
        return result

class GProMXMLTestLoader:

    EXTRA_SETTINGS_KEY = 'extra_settings'
    DISALLOWED_SETTINGS_KEY = 'unsupported_settings'

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
        log(f"LOADED TEST SUITE:\n{str(root)}")
        return root

    @classmethod
    def name_parts_from_file_name(cls,f):
        filename = os.path.basename(f)
        name, extension = os.path.splitext(filename)
        return tuple(name.split("."))

    @classmethod
    def register_test_suit(cls, root: GProMTestSuite, nameparts: tuple[str], thetest: GProMTestSuite):
        cur = root
        log(f"Register {thetest.name} under {nameparts} for {root.name}")
        for n in [ tuple(nameparts[0:i]) for i in range(1, len(nameparts)) ]:
            log(f"prefix {n} for {cur.name}")
            if n not in cur.tests:
                log(f"Register {n} does not exist in {cur.name}, create blank version")
                cur.tests[n] = GProMTestSuite(n,None,None)
            cur = cur.tests[n]
        # already created one before, merge in
        log(f"Register {thetest.name} under {nameparts} for {cur.name}")
        if nameparts in cur.tests:
            log(f"will merge suites {cur.tests[nameparts]} abd {thetest}")
            merge = GProMXMLTestLoader.merge_suites(cur.tests[nameparts], thetest)
            log(f"will merged suites {merge}")
            cur.tests[nameparts] = merge
        else:
            cur.tests[nameparts] = thetest

    @classmethod
    def merge_suites(cls, t1: GProMTestSuite, t2: GProMTestSuite):
        t1.extra_settings = GProMSettings.merge(t1.extra_settings, t2.extra_settings)
        t1.disallowed_settings = GProMSettings.merge(t1.disallowed_settings, t2.disallowed_settings)
        t1.tests = { **t1.tests, **t2.tests }
        return t1

    @classmethod
    def get_settings(cls, propdict: Dict[str,str], prefix: str):
        setdict = {}
        realprefix = prefix + "."
        keys = [ k for k in propdict if k.startswith(realprefix) ]
        settings = [ k.removeprefix(realprefix) for k in keys ]

        if len(settings) == 0:
            return None

        for s in settings:
            key = realprefix + s
            optdict = {}
            opts = [ x.strip() for x in propdict[key].split(",") ]
            for o in opts:
                # option with parameter or without an extra parameter
                if ":" in o:
                    k,v = o.split(":")
                    optdict[k] = v
                else:
                    optdict[o] = None
            setdict[s] = GProMSetting(optdict)

        return GProMSettings(setdict)

    @classmethod
    def load_xml_test_cases(cls,dir: str, f: str) -> GProMTestSuite:
        log(f"PROCESS TESTCASE FILE: {dir}/{f}")
        testcases = {}
        propdict = java_xml_properties_to_dict(dir + "/" + f)
        queries = sorted(list(set([ q.split('.')[0] for q in propdict if q[0] == 'q' ])),key = lambda x: int(x[1:]))
        # TODO read gprom settings
        suitenameparts = GProMXMLTestLoader.name_parts_from_file_name(f)

        extrasettings = GProMXMLTestLoader.get_settings(
            propdict,
            GProMXMLTestLoader.EXTRA_SETTINGS_KEY
        )

        disallow = GProMXMLTestLoader.get_settings(
            propdict,
            GProMXMLTestLoader.DISALLOWED_SETTINGS_KEY
        )

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
            #log(f"PARSE TEST CASE {q} [{testcasename} sorted:{issorted} disabled:{disabled} from file <{f}>:\n{query}\n\n{result}")
            if not disabled:
                t = OrderedTable.from_str(result) if issorted else Table.from_str(result)
                testcases[q] = GProMTestCase(testcasename, None, None, query, t, issorted)

        return GProMTestSuite(suitenameparts, extrasettings, disallow, testcases)

def java_xml_properties_to_dict(file:str):
    xml = ET.parse(file)
    d = { x.get('key'):x.text for x in xml.getroot().findall('entry') }
    #log(f"Read properties file {file} with keys:\n{'\n'.join(d.keys())}")
    return d


@dataclass
class GProMTestRunner:
    root: GProMTestSuite
    gprompath: str
    conf: GProMSettings
    failonerror: bool = False
    testcases: list[str] = None
    settings: list[str] = None
    results: Dict[str,Dict[str, bool]] = field(default_factory=dict)
    errors: Dict[str,Dict[str, str]] = field(default_factory=dict)
    queries: Dict[str,Dict[str,str]] = field(default_factory=dict)
    diffs: Dict[str,Dict[str,str]] = field(default_factory=dict)
    testsettings: Dict[str,list[str]] = field(default_factory=dict)

    FAT_STYLE = "bold black on white"

    def ensure_dicts(self,name):
        if name not in self.results:
            self.results[name] = {}
        if name not in self.errors:
            self.errors[name] = {}
        if name not in self.queries:
            self.queries[name] = {}
        if name not in self.diffs:
            self.diffs[name] = {}

    def run_test(self, test: GProMTestCase, conf: GProMSettings, name: str): #TODO deal with forbidden settings
        log(f"Test case query:\n{test.query}\nwith expected result:\n{test.expected}")
        exp = test.expected
        setting = conf[name]
        self.ensure_dicts(name)
        self.queries[name][test.name] = test.query
        try:
            if test.issorted:
                actual = GProMRunner.gprom_exec_to_ordered_table(self.gprompath, test.query, setting)
            else:
                actual = GProMRunner.gprom_exec_to_table(self.gprompath, test.query, setting)
            log(f"actual result was {'different' if not (exp == actual) else 'correct'}:\n{actual}")
            correct = (exp == actual)
            self.results[name][test.name] = correct
            if not correct:
                self.diffs[name][test.name] = exp.diff(actual)

            return correct
        except Exception as e:
            log(f"got exception: {e}")
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
        self.run_suite(self.root, conf, DEFAULT_SETTING_NAME)
        self.print_results(self.root, DEFAULT_SETTING_NAME)

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

    def should_run_setting(self, setting: str, strict=False):
        if not self.settings:
            return True
        for allowed in self.settings:
            if setting == allowed or setting.startswith(allowed):
                log(f"explicitly allowed setting {setting} because of {allowed}")
                return True
            if not strict and allowed.startswith(setting):
                log(f"allowed subsetting of {setting} because of {allowed}")
                return True
        return False

    def run_suite(self, t: GProMTestSuite, parentconf: GProMSettings, setname: str):
        if not self.should_run_test(t):
            return
        log(f"RUN TEST SUITE: [{t.name}]: EXTRA: {t.extra_settings} DISALLOW: {t.disallowed_settings}")
        conf = parentconf.extensions(setname, t.extra_settings) if t.extra_settings else parentconf.singleton(setname)
        log(f"APPLICABLE SETTINGS: {reduce(lambda x,y: x + '\n' + y, [ str(k) + ":" + str(v) for k,v in conf.settings.items() ])}")
        success = True
        allconfsuccess = True

        if t.name not in self.testsettings:
            self.testsettings[t.name] = []

        for name in [ n for n in conf if self.should_run_setting(n) ] :
            log(f"do settings <{name}> for <{t.name}> from {conf}")
            self.ensure_dicts(name)
            self.testsettings[t.name].append(name)
            log(f"still do settings <{name}> for <{t.name}> from {conf}")

            for child in t.tests.values():
                log(f"child {child.name} still do settings <{name}> for <{t.name}> from {conf}")
                if self.should_run_test(child):
                    if isinstance(child,GProMTestCase):
                        if self.should_run_setting(name, True):
                            log(f"run test case {child.name} in suite {t.name}, setting <{name}> <{conf}>")
                            self.run_test(child, conf, name)
                    else:
                        log(f"call child suite {child.name} in suite {t.name}, setting <{name}> <{conf}>")
                        self.run_suite(child, conf, name)
                    success = success and self.results[name][child.name]
            self.results[name][t.name] = success
            allconfsuccess = allconfsuccess and self.results[name][t.name]
        self.results[setname][t.name] = allconfsuccess

    def determine_settings(self, t: GProMTestSuite, parentset: str):
        ss = [ s for s in self.testsettings[t.name] if s.startswith(parentset) ]
        return ss

    def print_results(self, t: GProMTestSuite, parentset: str):
        if not self.should_run_test(t):
            return
        console = rich.get_console()
        indentlen = len(t.name) * 4
        testindentlen = indentlen + 4
        blankindent = indentlen * " "
        blackindent = f"[white on black]{blankindent}[/]"
        testblackindent = testindentlen * " "
        settings = self.determine_settings(t, parentset)
        redbar = "[white on red]" + 80 * " " + "[/]\n"

        for set in settings:
            suitestr = f"[white on black]SUITE: <{t.get_name_str()}> SETTING: <{set}> [/]"
            console.print(f"{blackindent}[b white on black]START [/] {suitestr}")
            for child in t.tests.values():
                if isinstance(child,GProMTestCase) and child.name in self.results[set]:
                    if self.results[set][child.name]:
                        mes = f"[black on green]OK[/]   [green]{child.get_name_str()}[/]"
                    else:
                        mes = f"[white on red]FAIL[/] {child.get_name_str()}"
                        if options.diff and child.name in self.diffs[set]:
                            mes += "  [white on red]QUERY ANSWER DIFFERS:[/]\n" + redbar + self.queries[set][child.name] + "\n" + redbar + f"\n{self.diffs[set][child.name]}\n" + redbar
                        if child.name in self.errors[set]:
                            if options.errordetails:
                                mes += "  [white on red]EXCEPTION:[/]\n" + redbar + self.queries[set][child.name] + "\n" + redbar + f"\n{self.errors[set][child.name]}\n" + redbar
                            else:
                                shorterror = self.errors[set][child.name][:60].replace("\n", " ")
                                mes += f"  [white on red]EXCEPTION:[/] {shorterror}"
                    console.print(f"{testblackindent}{mes}")
                else:
                    self.print_results(child, set)
            runtests = [ x for x in t.tests.values() if self.should_run_test(x) ]
            numtests = len(runtests)
            numsuccess = reduce(lambda x,y: x + y, [ self.results[set][c.name] for c in t.tests.values() if c.name in self.results[set] ], 0)
            #allpass = numsuccess == runtests
            mes = f"[black on green] OK {numsuccess}/{numtests} PASSED [/]" if self.results[set][t.name] else f"[white on red] FAIL {numsuccess}/{numtests} PASSED [/]"
            console.print(f"{blackindent}{suitestr} {mes}")

class GProMRunner():

    @classmethod
    def construct_gprom_cmd_as_list(cls, gprom: str, query: str, args: GProMSetting):
        query = query.replace("\n", " ")
        cmdlist = [ gprom ] + args.to_list() + ["-query", query]
        return cmdlist

    @classmethod
    def gprom_exec_to_string(cls, gprom: str, query: str, args: GProMSetting):
        log(f"will run {query} with args {args.items()}")
        cmdlist = GProMRunner.construct_gprom_cmd_as_list(gprom, query, args)
        log(f"run gprom with args:\n\t{' '.join(cmdlist)}")
        try:
            process = subprocess.run(cmdlist,
                                     timeout=options.timeout,
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE,
                                     universal_newlines=True)
        except subprocess.TimeoutExpired:
            return (-1, "", f"TIMED OUT AFTER {options.timeout} SECONDS")
        return (process.returncode, process.stdout.strip(), process.stderr.strip())

    @classmethod
    def gprom_exec_to_table(cls, gprom: str, query: str, args: GProMSetting):
        rc, res, stderr = GProMRunner.gprom_exec_to_string(gprom, query, args)
        log(f"running get us RC: {rc} with STDOUT:\n{res}")
        if rc:
            raise Exception(f"failed running [{rc}]:\nSTDOUT:\n{res}\nSTDERR:\n{stderr}")
        return Table.from_str(res)

    @classmethod
    def gprom_exec_to_ordered_table(cls, gprom: str, query: str, args: GProMSetting):
        rc, res, stderr = GProMRunner.gprom_exec_to_string(gprom, query, args)
        log(f"running get us RC: {rc} with STDOUT:\n{res}")
        if rc:
            raise Exception(f"failed running [{rc}]:\nSTDOUT:\n{res}\nSTDERR:\n{stderr}")
        return OrderedTable.from_str(res)

def gprom_debug_settings():
    return GProMSetting({
        "-Loperator_verbose": None,
        "-Loperator_verbose_props": "2",
        "-loglevel": "3"
    })

def default_gprom_settings_from_options(opions):
    common = GProMSetting({"-loglevel": "0"})
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
            "-backend": options.backend,
            "-db": options.db,
            "-port": str(options.port),
            "-host": options.host,
            "-passwd": options.password,
            "-user": options.user
        }))
    if options.debug:
        settings = settings.union(GProMSetting({
            "-loglevel": "4",
            "-aggressive_model_checking": "TRUE",
            "-Loperator_verbose": "TRUE",
            "-Loperator_verbose_props": "2",
        }))
    return GProMSettings({DEFAULT_SETTING_NAME: settings})

def parse_args():
    ap = argparse.ArgumentParser(description='Running semantic optimization experiment')
    ap.add_argument('-t', '--tests', type=str, default=None,
                    help="run only these tests, this can be a single string or a list separated by comma (default is to run all). Note that test names are hierarchical where levels are separated by \".\". For example, specifying \"prov\", will run tests \"prov\", \"prov.agg\", \"prov.agg.recursive\", ...")
    ap.add_argument('-s', '--settings', type=str, default=None,
                    help="only run tests for these settings, this can be a single string or a list separated by comma (default is to run all settings). Note that settings are hierarchical where elements are separated by \".\", e.g., providing \"default.heuopt\" will run all settings that start with \"default.heuopt\" such as \"default.heuopt.dl\".")
    ap.add_argument('-T', '--timeout', type=int, default=10,
                    help="timeout test cases after this many seconds")
    ap.add_argument('--gprom', type=str, default=get_relative_path("../src/command_line/gprom"),
                    help="use this gprom binary")
    ap.add_argument('--diff', action='store_true',
                    help="if provided, then show difference in outputs for failed tests")
    ap.add_argument('-e', '--errordetails', action='store_true',
                    help="if provided, then show detailed error messages for tests where gprom errored out")
    ap.add_argument('-S', '--stoponerror', action='store_true',
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
    ap.add_argument("-d", "--db", type=str, default=get_relative_path("test-sqlite.db"),
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

def parse_settings_selection():
    if options.settings:
        s = options.settings
        options.settings = [ x.strip() for x in s.split(",") ]
        log(f"user selecected settings: {options.settings}")

def get_relative_path(p):
    return str(Path(__file__).resolve().parent) + "/" + p

def main():
    global options
    global console
    console = rich.get_console()
    options = parse_args()
    parse_test_cases_selection()
    parse_settings_selection()
    conf = default_gprom_settings_from_options(options)
    rootsuite = GProMXMLTestLoader.load_xmls_from_dir(get_relative_path('testcases'))
    runner = GProMTestRunner(root=rootsuite,
                    gprompath=options.gprom,
                    conf=conf,
                    failonerror=options.stoponerror,
                    testcases=options.tests,
                    settings=options.settings)
    runner.run(conf)

if __name__ == '__main__':
    main()
