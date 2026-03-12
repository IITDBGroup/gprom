#!/usr/bin/env python
import argparse
import os
import subprocess
import traceback
import xml.etree.ElementTree as ET
from collections import Counter
from dataclasses import dataclass, field
from enum import Enum
from functools import reduce
from pathlib import Path
from typing import Dict, Union
from deepdiff import DeepDiff
from deepdiff.helper import COLORED_VIEW
import rich
from pg8000.native import Connection
from tqdm import tqdm
import difflib

FAT_STYLE = "bold black on white"
DEFAULT_SETTING_NAME = "default"
DIFF_METHODS = {
    'table-level-multiplity',
    'string-colordiff'
}

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

def forcelogfat(m):
    console.print(80 * " ", style=FAT_STYLE, justify="center")
    console.print(m, style=FAT_STYLE, justify="center")
    console.print(80 * " ", style=FAT_STYLE, justify="center")

def wrap_line(m):
    return "\n" + "=" * 80 + "\n" + m + "\n"  + "=" * 80

class DatabaseBackends(Enum):
    POSTGRES = 1
    SQLITE = 2
    DUCKDB = 3
    MSSQL = 4
    ORACLE = 5

def color_diff_strings(s1, s2):
    """Highlights character-level differences using ANSI codes."""
    # GREEN = '\x1b[38;5;16;48;5;2m'
    # RED = '\x1b[38;5;16;48;5;1m'
    # END = '\x1b[0m'
    GREEN = '[bold black on green]'
    RED = '[bold white on red]'
    END = '[/]'

    output = []
    for op, a0, a1, b0, b1 in difflib.SequenceMatcher(None, s1, s2).get_opcodes():
        if op == "equal": output.append(s1[a0:a1])
        elif op == "insert": output.append(GREEN + s2[b0:b1] + END)
        elif op == "delete": output.append(RED + s1[a0:a1] + END)
        elif op == "replace":
            output.append(RED + s1[a0:a1] + END)
            output.append(GREEN + s2[b0:b1] + END)
    return "".join(output)

@dataclass
class Table:
    schema: list[str]
    rows: Counter[tuple[str]] = field(default_factory=Counter)

    RESULT_TID_ATTR="_result_tid"
    DUP_ATTR="_setprov_dup_count"

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

    def mydiff(self, o: "Table"):
        if self.schema != o.schema:
            return f"schemas differ: expected\n\n{self.schema}\n, but got:\n{o.schema}"
        allrows = set(self.rows.keys()).union(o.rows.keys())
        result = ""
        for r in allrows:
            if self.rows[r] != o.rows[r]:
                result += f"row[{Table.row_to_string(r)}]\t multiplicity differs: expected {self.rows[r]}, but was {o.rows[r]}\n"

        return result

    # def diff(self, o: "Table"):
    #     if self.schema != o.schema:
    #         return f"schemas differ: expected\n\n{self.schema}\n, but got:\n{o.schema}"
    #     diff = DeepDiff(self.rows, o.rows, verbose_level=2, view=COLORED_VIEW,  ignore_order=True)
    #     return diff.pretty()

    def colordiff(self, o: "Table"):
        selfstr = str(self)
        ostr = str(o)
        return color_diff_strings(selfstr, ostr)

    def diff(self, o: "Table"):
        if options.diffalgo == 'table-level-multiplity':
            return self.mydiff(o)
        if options.diffalgo == 'string-colordiff':
            return self.colordiff(o)

    @classmethod
    def from_str(cls, inputstr: str):
        try:
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
        except Exception as e:
            raise TableParseException(str)

    @classmethod
    def row_to_string(cls, r, attrvallen=None, newline=False):
        if not attrvallen:
            attrvallen = [ len(v) for v in r ]
        return ' |'.join([ ' ' + x.ljust(attrvallen[i]) for i, x in enumerate(r)]) + " |" + ("\n" if newline else "")

    def __str__(self):
        result = ""
        attrvallen = [ max([ len(x[i]) for x in self.rows ] + [0]) for i in range(0,len(self.schema)) ]
        attrvallen = [ max(attrvallen[i], len(self.schema[i]) ) for i in range(0,len(self.schema)) ]

        result += Table.row_to_string(self.schema, attrvallen, True)
        dividerlen = sum(attrvallen) + 3 * len(attrvallen)

        result += "-" * dividerlen + "\n"

        for r in self.rows:
            rstr = Table.row_to_string(r, attrvallen, True)
            for i in range(0,self.rows[r]):
                result += rstr

        result = result[:-1]

        return result

    def get_attr_pos(self,a):
        return self.schema.index(a)

    def get_composable_attrs_pos(self):
        return (self.get_attr_pos(Table.RESULT_TID_ATTR), self.get_attr_pos(Table.DUP_ATTR))

    @classmethod
    def is_prov_attr(cls, a):
        return a.startswith("prov_")

    @classmethod
    def is_normal_attr(cls, a):
        return a not in [ Table.DUP_ATTR, Table.RESULT_TID_ATTR ] and not Table.is_prov_attr(a)

    def row_project_normal(self,row):
        return tuple([ row[i] for i in [ i for (i,x) in enumerate(self.schema) if Table.is_normal_attr(x) ] ])

    def row_project_provenance(self,row):
        return tuple([ row[i] for i in [ i for (i,x) in enumerate(self.schema) if Table.is_prov_attr(x) ] ])

    def row_get_attr(self,row,a):
        pos = self.schema.index(a)
        return row[pos]

    def row_set_attr(self,row,a,val):
        pos = self.schema.index(a)
        row[pos] = val

    def normalize_prov_composable(self):
        """
        replace _result_tid attribute with a hash based on the (non-provenance attributes), and set provenance duplicate counter attribute based on sorting on a hash of the provenance attributes
        """
        group_by_result = {}
        normalizedcnter = Counter()

        # if this input does not have the compositional provenance attributes,
        # then do not attempt to normalize
        if not Table.RESULT_TID_ATTR in self.schema or not Table.DUP_ATTR in self.schema:
            return self

        for row in self.rows.elements():
            key = self.row_project_normal(row)
            if key not in group_by_result:
                group_by_result[key] = []
            group_by_result[key].append(row)
        log(f"grouped result: {group_by_result}")

        for grp in group_by_result:
            resulttid = hash(tuple(grp))
            rows = group_by_result[grp]
            log(f"unsorted rows {rows} for group {grp}")
            rows = sorted(rows,key=lambda x: self.row_project_provenance(x))
            log(f"sorted rows {rows} for group {grp}")
            for i,r in enumerate(rows):
                r = list(r)
                log(f"original row {r}")
                self.row_set_attr(r, Table.RESULT_TID_ATTR, str(resulttid))
                self.row_set_attr(r, Table.DUP_ATTR, str(i))
                r = tuple(r)
                log(f"updated row {r}")
                normalizedcnter.update([r])

        result = Table(self.schema, normalizedcnter)
        log(f"Updated table:\n{result}")

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

    def mydiff(self, o: "OrderedTable"):
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

    def colordiff(self, o: "OrderedTable"):
        selfstr = str(self)
        ostr = str(o)
        return color_diff_strings(selfstr, ostr)

    def diff(self, o: "OrderedTable"):
        if options.diffalgo == 'table-level-multiplity':
            return self.mydiff(o)
        if options.diffalgo == 'string-colordiff':
            return self.colordiff(o)

    @classmethod
    def from_str(cls, inputstr: str):
        try:
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
        except Exception as e:
            raise TableParseException(str)


    def __str__(self):
        result = ""
        attrvallen = [ max([ len(x[i]) for x in self.rows ] + [0]) for i in range(0,len(self.schema)) ]
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
        self.setting = self.union(other)

    def __getitem__(self,key):
        return self.setting[key]

    def __setitem__(self, key, value):
        self.setting[key] = value

    def __eq__(self,o):
        return self.setting == o.setting

    def __hash__(self):
        return hash(self.setting)

    def __contains__(self, key):
        return key in self.setting

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

    def should_run_test(self, allowedtests):
        if not allowedtests:
            return True
        for allowed in allowedtests:
            if len(self.name) >= len(allowed) and self.name[:len(allowed)] == allowed:
                return True
        for allowed in allowedtests:
            if len(allowed) > len(self.name) and allowed[:len(self.name)] == self.name:
                return True
        return False

    def should_run_setting(self, setting: str, allowedset, strict=False):
        if not allowedset:
            return True
        for allowed in allowedset:
            if setting == allowed or setting.startswith(allowed):
                return True
            if not strict and allowed.startswith(setting):
                return True
        return False

    def get_name_str(self):
        return '.'.join(self.name)

    def count_testcases(self, allowed, settings, parentset):
        return 0

@dataclass
class GProMTestCase(GProMTest):
    query: str
    expected: Union[Table,OrderedTable]
    issorted: bool

    def count_testcases(self, allowed, settings, parentset):
        if self.should_run_test(allowed) and self.should_run_setting(parentset,  settings, True):
            return 1
        else:
            return 0

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

    def count_testcases(self, allowed, settings, parentset):
        if not self.should_run_test(allowed):
            return 0
        cnt = 0
        newsets = [ parentset + "." + x for x in self.extra_settings ] if self.extra_settings else [ parentset ]
        newsets = [ n for n in newsets if self.should_run_setting(n, settings) ]
        for set in newsets:
            for t in self.tests.values():
                cnt += t.count_testcases(allowed, settings, set)
        return cnt

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
            log(f"will merge suites {cur.tests[nameparts]} and {thetest}")
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
        curquery = ""
        try:
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
                curquery = q
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
        except Exception as e:
            forcelogfat(f"Error loading tests from file <{dir}/{f}> processing <{curquery}>")
            traceback.print_exc()
            raise e

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
    debugconf: GProMSetting
    failonerror: bool = False
    testcases: list[str] = None
    settings: list[str] = None
    results: Dict[str,Dict[str, bool]] = field(default_factory=dict)
    errors: Dict[str,Dict[str, str]] = field(default_factory=dict)
    queries: Dict[str,Dict[str,str]] = field(default_factory=dict)
    diffs: Dict[str,Dict[str,str]] = field(default_factory=dict)
    testsettings: Dict[str,list[str]] = field(default_factory=dict)
    actualresults: Dict[str,list[str]] = field(default_factory=dict)
    totalnumtests: int = 0
    progressbar: tqdm = None

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
        if name not in self.actualresults:
            self.actualresults[name] = {}

    def run_test(self, test: GProMTestCase, conf: GProMSettings, name: str): #TODO deal with forbidden settings
        log(f"Test case query:\n{test.query}\nwith expected result:\n{test.expected}")
        exp = test.expected
        setting = conf[name]
        is_prov_composable = '-prov_use_composable' in setting
        self.ensure_dicts(name)
        self.queries[name][test.name] = test.query
        try:
            if test.issorted:
                actual = GProMRunner.gprom_exec_to_ordered_table(self.gprompath, test.query, setting)
            else:
                actual = GProMRunner.gprom_exec_to_table(self.gprompath, test.query, setting)
            log(f"actual result was {'different' if not (exp == actual) else 'correct'}:\n{actual}")
            self.actualresults[name][test.name] = str(actual)
            if is_prov_composable:
                actual = actual.normalize_prov_composable()
                exp = exp.normalize_prov_composable()
            correct = (exp == actual)
            self.results[name][test.name] = correct

            if not correct:
                self.diffs[name][test.name] = exp.diff(actual)
                # write query results to a file?
                if options.log_query_results:
                    with open(options.log_query_results,'a') as f:
                        f.write(80 * "-" + f"\n{test.name}\n" + 80 * "-" + "\n")
                        f.write(self.actualresults[name][test.name])
                        f.write("\n")
            self.progressbar.update()
            return correct
        except GProMRunException as e:
            self.results[name][test.name] = False
            if options.errordetails:
                self.errors[name][test.name] = "ORIGNAL RUN:\n" + str(e)
            else:
                self.errors[name][test.name] = e.shortversion()
            if options.debug:
                traceback.print_exc()
            if options.errordetails:
                try:
                    mergedconf = setting.union(self.debugconf)
                    (rc,stdout,stderr) = GProMRunner.gprom_exec_to_string(self.gprompath, test.query, mergedconf)
                    if rc == -1:
                        self.errors[name][test.name] += "TIMED OUT"
                    else:
                        self.errors[name][test.name] += wrap_line("DEBUGRUN") + f"\n\nSTDOUT:\n{stdout}\n\nSTDERR:\n{stderr}\n\nRETURN CODE: {rc}"
                except Exception as e2:
                    self.errors[name][test.name] += "\n\nADDITIONAL EXCEPTION DURING DEBUG RUN\n\n" + str(e2)
            return False
        except TimeoutError as e:
            self.results[name][test.name] = False
            self.errors[name][test.name] = "TIMED OUT"
            return False
        except Exception as e:
            self.results[name][test.name] = False
            log(f"got exception: {e}")
            if options.debug:
                traceback.print_exc()
            # if asked for details rerun gprom in debug settings
            self.results[name][test.name] = False
            if options.errordetails:
                try:
                    mergedconf = setting.union(self.debugconf)
                    (rc,stdout,stderr) = GProMRunner.gprom_exec_to_string(self.gprompath, test.query, mergedconf)
                    self.actualresults[name][test.name] = f"STDOUT:\n{stdout}\n\nSTDERR:\n{stderr}\n\nRETURN CODE: {rc}"
                    self.errors[name][test.name] = f"STDOUT:\n{stdout}\n\nSTDERR:\n{stderr}\n\nRETURN CODE: {rc}"
                except Exception as e2:
                    self.errors[name][test.name] = str(e2)
            if self.failonerror:
                self.progressbar.update()
                raise e
            else:
                if test.name not in self.errors[name]:
                    self.errors[name][test.name] = str(e)
            self.progressbar.update()
            return False

    def run(self, conf: GProMSettings = None):
        if not conf:
            conf = self.conf
        log(f"Start running tests: {self.testcases}")
        self.results = {}
        self.errors = {}
        self.totalnumtests = self.root.count_testcases(self.testcases, self.testsettings, DEFAULT_SETTING_NAME)
        self.progressbar = tqdm(total=self.totalnumtests, desc="Testcases")
        self.run_suite(self.root, conf, DEFAULT_SETTING_NAME)
        self.progressbar.close()
        self.print_results(self.root, DEFAULT_SETTING_NAME)

    def should_run_test(self, t: GProMTest):
        return t.should_run_test(self.testcases)

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
            return 0,0
        console = rich.get_console()
        indentlen = len(t.name) * 4
        testindentlen = indentlen + 4
        blankindent = indentlen * " "
        blackindent = f"[white on black]{blankindent}[/]"
        testblackindent = testindentlen * " "
        settings = self.determine_settings(t, parentset)
        redbar = "[white on red]" + 80 * " " + "[/]\n"

        numbase = 0
        basepassed = 0

        for set in settings:
            suitestr = f"[white on black]SUITE: <{t.get_name_str()}> SETTING: <{set}> [/]"
            console.print(f"{blackindent}[b white on black]START [/] {suitestr}")
            for child in t.tests.values():
                if isinstance(child,GProMTestCase) and child.name in self.results[set]:
                    numbase += 1
                    if self.results[set][child.name]:
                        mes = f"[black on green]OK[/]   [green]{child.get_name_str()}[/]"
                        basepassed += 1
                        console.print(f"{testblackindent}{mes}")
                    else:
                        mes = f"[white on red]FAIL[/] {child.get_name_str()}"
                        if options.diff and child.name in self.diffs[set]:
                            mes += "  [white on red]QUERY ANSWER DIFFERS:[/]\n" + redbar + self.queries[set][child.name] + "\n" + redbar + f"\n{self.actualresults[set][child.name]}\n" + redbar + f"\n{self.diffs[set][child.name]}\n" + redbar
                        if child.name in self.errors[set]:
                            if options.errordetails:
                                mes += "  [white on red]EXCEPTION:[/]\n" + redbar + self.queries[set][child.name] + "\n" + redbar + "\n"
                                console.print(f"{testblackindent}{mes}")
                                print(self.errors[set][child.name])
                                console.print(redbar)
                            else:
                                shorterror = self.errors[set][child.name][:60].replace("\n", " ")
                                mes += f"  [white on red]EXCEPTION:[/] {shorterror}"
                                console.print(f"{testblackindent}{mes}")
                        else:
                            console.print(f"{testblackindent}{mes}")
                else:
                    newtest, newpassed = self.print_results(child, set)
                    numbase += newtest
                    basepassed += newpassed
            runtests = [ x for x in t.tests.values() if self.should_run_test(x) ]
            numtests = len(runtests)
            numsuccess = reduce(lambda x,y: x + y, [ self.results[set][c.name] for c in t.tests.values() if c.name in self.results[set] ], 0)
            #allpass = numsuccess == runtests
            mes = f"[black on green] OK {numsuccess}/{numtests} CHILDREN PASSED {basepassed}/{numbase} INDIVIDUAL TESTS PASSED [/]" if self.results[set][t.name] else f"[white on red] FAIL {numsuccess}/{numtests} PASSED {basepassed}/{numbase} INDIVIDUAL TESTS PASSED [/]"
            console.print(f"{blackindent}{suitestr} {mes}")
        return (numbase, basepassed)

class GProMRunException(Exception):

    def __init__(self, rc, stdout, stderr, cmd):
        super().__init__(stdout + stderr)
        self.rc = rc
        self.stdout = stdout
        self.stderr = stderr
        self.cmd = cmd

    def wrap_in_lines(self, s):
        return "\n" + "=" * 80 + "\n" + s + "\n"  + "=" * 80

    def shortversion(self):
        return f"RC: [{self.rc}] {self.stdout}"

    def __str__(self):
        title = self.wrap_in_lines(f"\nGProM Run Exception [return code <{self.rc}>]\n")
        cmd = self.wrap_in_lines(self.cmd)
        stdout = self.wrap_in_lines("STDOUT")
        stderr = self.wrap_in_lines("STDERR")
        return title + cmd + stdout + "\n" + self.stdout + stderr + "\n" + self.stderr

class TableParseException(Exception):

    def __init__(self, message):
        super().__init__(message)
        self.message = message

    def __str__(self):
        return "\n" + "=" * 80 + "\nTable Parse Exception\n" + "=" * 80 + "\n\n" + self.message + "\n" + "=" * 80


class GProMRunner():

    @classmethod
    def construct_gprom_cmd_as_list(cls, gprom: str, query: str, args: GProMSetting):
        query = query.replace("\n", " ")
        cmdlist = [ gprom ] + args.to_list() + ["-query", query]
        return cmdlist

    POSTGRES_KILL_BACKEND_QUERY_TEMPLATE = """
SELECT pg_terminate_backend(pg_stat_activity.pid)
FROM pg_stat_activity
WHERE pid <> pg_backend_pid();
    """

    @classmethod
    def cleanup_database_connection(cls, gprom: str, conf: GProMSetting):
        if options.backend == 'postgres':
            conninfo = {
                "host": conf["-host"],
                "database": conf["-db"],
                "port": conf["-port"],
                "password": conf["-passwd"],
                "user": conf["-user"],
            }
            con = Connection(**conninfo)
            con.run(GProMRunner.POSTGRES_KILL_BACKEND_QUERY_TEMPLATE)
            con.close()

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
            GProMRunner.cleanup_database_connection(gprom, args)
            return (-1, "", f"TIMED OUT AFTER {options.timeout} SECONDS")

        return (process.returncode, process.stdout.strip(), process.stderr.strip())

    @classmethod
    def gprom_exec_to_table(cls, gprom: str, query: str, args: GProMSetting):
        rc, res, stderr = GProMRunner.gprom_exec_to_string(gprom, query, args)
        log(f"running get us RC: {rc} with STDOUT:\n{res}")
        if rc:
            argstr = ' '.join(GProMRunner.construct_gprom_cmd_as_list(gprom, query, args))
            raise GProMRunException(rc,res,stderr,argstr)
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
    debugsettings = settings.union(GProMSetting({
            "-loglevel": str(options.loglevel),
            "-aggressive_model_checking": "TRUE",
            "-Loperator_verbose": "TRUE",
            "-Loperator_verbose_props": "2",
    }))
    return (GProMSettings({DEFAULT_SETTING_NAME: settings}),debugsettings)

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
    ap.add_argument('--diffalgo', type=str, default='table-level-multiplity',
                    help=f"how to compute and show differences between tables (table-level-multiplitiy [DEFAULT],string-colordiff)")
    ap.add_argument('-e', '--errordetails', action='store_true',
                    help="if provided, then show detailed error messages for tests where gprom errored out")
    ap.add_argument('-S', '--stoponerror', action='store_true',
                    help="if provided, then stop after the first error")
    ap.add_argument("-D", "--debug", action='store_true',
                    help="debug the process by logging more information.")
    # ap.add_argument("--gpromdebug", action='store_true',
    #                 help="request gprom to print more logging information.")
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
    ap.add_argument("--log_query_results", type=str, default=None,
                    help="if true, then write actual query results to this file.")


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
    conf,debugconf = default_gprom_settings_from_options(options)
    rootsuite = GProMXMLTestLoader.load_xmls_from_dir(get_relative_path('testcases'))
    runner = GProMTestRunner(root=rootsuite,
                    gprompath=options.gprom,
                    conf=conf,
                    debugconf=debugconf,
                    failonerror=options.stoponerror,
                    testcases=options.tests,
                    settings=options.settings)
    runner.run(conf)

if __name__ == '__main__':
    main()
