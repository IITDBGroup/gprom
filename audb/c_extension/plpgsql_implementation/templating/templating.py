import os


types = [
    {"RANGE_TYPE": "int2range", "BASE_TYPE": "int2", "RANGE_TYPE_ABBREV": "i2r", "LOWER_EXPR": "((LEAST(p1, p2, p3, p4)))::int", "UPPER_EXPR": "((GREATEST(p1, p2, p3, p4)) + 1)::int"},
    {"RANGE_TYPE": "int4range", "BASE_TYPE": "int4", "RANGE_TYPE_ABBREV": "i4r", "LOWER_EXPR": "((LEAST(p1, p2, p3, p4)))::int", "UPPER_EXPR": "((GREATEST(p1, p2, p3, p4)) + 1)::int"},
    {"RANGE_TYPE": "numrange", "BASE_TYPE": "numeric", "RANGE_TYPE_ABBREV": "numer", "LOWER_EXPR": "((LEAST(p1, p2, p3, p4)))::int", "UPPER_EXPR": "((GREATEST(p1, p2, p3, p4)) + 1)::int"}
]

def test():
    base_dir = "./testTemplating"
    out_dir = "./generated"
    os.makedirs(out_dir, exist_ok=True)

    for d in os.listdir("./testTemplating"):
        if "." in d:
            continue

        path = os.path.join(base_dir, d)
        if path == "./testTemplating/arithmeticOperators":  ########## remove when ready to test on other
            for file in os.listdir(path):
                file_path = os.path.join(path, file)
                if not os.path.isfile(file_path):
                    continue
                
                with open(file_path, 'r') as f:
                    template = f.read()        
                    
                for T in types:
                    content = template
                    content = content.replace("{{RANGE_TYPE}}",T["RANGE_TYPE"])
                    content = content.replace("{{RANGE_TYPE_ABBREV}}",T["RANGE_TYPE_ABBREV"])
                    content = content.replace("{{BASE_TYPE}}",T["BASE_TYPE"])
                    content = content.replace("{{LOWER_EXPR}}",T["LOWER_EXPR"])
                    content = content.replace("{{UPPER_EXPR}}",T["UPPER_EXPR"])
                
                    os.makedirs(os.path.join(out_dir, T["RANGE_TYPE_ABBREV"]), exist_ok=True)
                    out_file = os.path.join(out_dir, T["RANGE_TYPE_ABBREV"], f"{file.replace("{t}", T["RANGE_TYPE_ABBREV"])}")
                    with open(out_file, 'w') as outF:
                        outF.write(content)



if __name__ == "__main__":
    test()