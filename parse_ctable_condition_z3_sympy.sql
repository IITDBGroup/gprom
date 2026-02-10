-- PostgreSQL 函数：使用 Z3 解析 ctable 条件表达式
-- 需要安装:
--   1. PL/Python 扩展: CREATE EXTENSION IF NOT EXISTS plpython3u;
--   2. Z3 Python 绑定: pip3 install z3-solver 或 apt-get install python3-z3
--
-- 支持的操作符：>, <, >=, <=, &&, ||
-- 返回范围字符串，例如："(10,20)", "(-∞,10)", "(20,+∞)", "false" 等

CREATE OR REPLACE FUNCTION parse_ctable_condition_z3_sympy(cond_str TEXT, var_name TEXT)
RETURNS TEXT AS $$
import re

# 尝试导入 Z3
try:
    from z3 import Real, Solver, Optimize, unsat, sat, is_rational_value, And, Or
    Z3_AVAILABLE = True
except ImportError:
    Z3_AVAILABLE = False

def parse_single_condition(cond_str):
    """
    解析单个条件，提取操作符和数值
    返回: (操作符, 数值) 或 None
    支持集合语法: X={v1,v2,...}
    """
    # 首先检查集合语法: X={v1,v2,...}
    set_pattern = r'=\{([\d\s,]+)\}'
    set_match = re.search(set_pattern, cond_str)
    if set_match:
        values_str = set_match.group(1)
        # 提取所有数值
        values = []
        for val_str in re.findall(r'-?\d+(?:\.\d+)?', values_str):
            try:
                val = float(val_str)
                if val.is_integer():
                    val = int(val)
                values.append(val)
            except ValueError:
                continue
        if values:
            return '=', values  # 返回操作符'='和值列表
    
    # 然后检查比较操作符（包括!=和<>）
    patterns = [
        (r'!=(-?\d+(?:\.\d+)?)', '!='),
        (r'<>(-?\d+(?:\.\d+)?)', '!='),  # <> 等同于 !=
        (r'>=(-?\d+(?:\.\d+)?)', '>='),
        (r'<=(-?\d+(?:\.\d+)?)', '<='),
        (r'>(-?\d+(?:\.\d+)?)', '>'),
        (r'<(-?\d+(?:\.\d+)?)', '<'),
    ]
    
    for pattern, op in patterns:
        match = re.search(pattern, cond_str)
        if match:
            value_str = match.group(1)
            try:
                value = float(value_str)
                if value.is_integer():
                    value = int(value)
                return op, value
            except ValueError:
                continue
    return None

def parse_single_condition_to_z3(cond_str, var):
    """
    将单个条件字符串转换为 Z3 约束
    例如: "X>10" -> var > 10
    支持集合语法: "X={v1,v2}" -> (var == v1) Or (var == v2)
    返回: Z3 约束表达式 或 None
    """
    result = parse_single_condition(cond_str)
    if result is None:
        return None
    
    op, value = result
    
    # 处理集合语法: X={v1,v2,...}
    if op == '=' and isinstance(value, list):
        if len(value) == 0:
            return None
        # 创建多个相等约束的OR组合
        constraints = [var == v for v in value]
        if len(constraints) == 1:
            return constraints[0]
        return Or(*constraints)
    
    # 处理比较操作符
    if op == '>':
        return var > value
    elif op == '<':
        return var < value
    elif op == '>=':
        return var >= value
    elif op == '<=':
        return var <= value
    elif op == '!=':
        return var != value  # 不等于约束
    return None

def format_range(min_val, max_val, min_op, max_op):
    """
    格式化范围输出
    """
    left_bracket = '[' if min_op == '>=' else '('
    right_bracket = ']' if max_op == '<=' else ')'
    
    if min_val is None and max_val is None:
        return None
    elif min_val is None:
        if isinstance(max_val, float) and max_val.is_integer():
            max_val = int(max_val)
        return f'(-∞,{max_val}{right_bracket}'
    elif max_val is None:
        if isinstance(min_val, float) and min_val.is_integer():
            min_val = int(min_val)
        return f'{left_bracket}{min_val},+∞)'
    else:
        if isinstance(min_val, float) and min_val.is_integer():
            min_val = int(min_val)
        if isinstance(max_val, float) and max_val.is_integer():
            max_val = int(max_val)
        return f'{left_bracket}{min_val},{max_val}{right_bracket}'

def parse_with_z3(cond_str, var_name):
    """
    使用 Z3 解析条件表达式（统一使用 Z3 进行约束验证和可满足性检查）
    """
    if not Z3_AVAILABLE:
        raise ImportError("Z3 not available")
    
    # 检查特殊值
    if cond_str.upper().strip() == 'TRUE':
        return var_name
    
    # 创建 Z3 变量
    var = Real(var_name)
    
    # 处理 AND 条件：使用 Z3 验证约束，从字符串提取边界
    if '&&' in cond_str:
        parts = cond_str.split('&&')
        constraints = []
        min_bound = None
        max_bound = None
        min_op = None
        max_op = None
        set_values = None  # 用于存储集合值
        exclude_values = []  # 用于存储!=操作符排除的值
        
        # 构建变量匹配模式：只处理包含目标变量的约束
        var_pattern = r'(^|[^a-zA-Z0-9])' + re.escape(var_name) + r'([^a-zA-Z0-9]|$)'
        
        # 将每个部分转换为 Z3 约束，同时提取边界值
        for part in parts:
            part = part.strip()
            
            # 关键修复：只处理包含目标变量的约束
            if not re.search(var_pattern, part):
                continue  # 跳过不包含目标变量的约束
            
            # 首先解析条件（提取集合值或边界值）
            result = parse_single_condition(part)
            if result:
                op, value = result
                # 处理集合语法: X={v1,v2,...}
                if op == '=' and isinstance(value, list):
                    # 如果有多个集合条件，取交集
                    if set_values is None:
                        set_values = set(value)
                    else:
                        set_values = set_values.intersection(set(value))
                elif op == '!=':
                    # 记录排除的值
                    exclude_values.append(value)
                elif op in ['>', '>=']:
                    if min_bound is None or value > min_bound:
                        min_bound = value
                        min_op = op
                elif op in ['<', '<=']:
                    if max_bound is None or value < max_bound:
                        max_bound = value
                        max_op = op
            
            # 创建Z3约束（用于可满足性检查）
            constraint = parse_single_condition_to_z3(part, var)
            if constraint is not None:
                constraints.append(constraint)
        
        if not constraints:
            return var_name
        
        # 使用 Z3 Solver 检查可满足性
        combined_constraint = And(*constraints) if len(constraints) > 1 else constraints[0]
        solver = Solver()
        solver.add(combined_constraint)
        if solver.check() == unsat:
            return 'false'
        
        # 如果存在集合值，优先返回集合（过滤满足其他约束的值）
        if set_values is not None:
            # 过滤满足其他约束的值
            filtered_values = []
            for val in set_values:
                # 检查值是否满足其他约束
                satisfies = True
                if min_bound is not None:
                    if min_op == '>' and val <= min_bound:
                        satisfies = False
                    elif min_op == '>=' and val < min_bound:
                        satisfies = False
                if max_bound is not None:
                    if max_op == '<' and val >= max_bound:
                        satisfies = False
                    elif max_op == '<=' and val > max_bound:
                        satisfies = False
                if satisfies:
                    filtered_values.append(val)
            
            if len(filtered_values) == 0:
                return 'false'  # 没有值满足所有约束
            elif len(filtered_values) == 1:
                return str(filtered_values[0])  # 单个值
            else:
                # 多个值，返回集合格式
                sorted_values = sorted(filtered_values)
                return '{' + ','.join(str(v) for v in sorted_values) + '}'
        
        # 处理!=操作符：如果有排除值，需要形成并集
        if exclude_values:
            # 构建区间列表，排除指定值
            ranges = []
            
            # 简化处理：对于每个排除值，创建两个区间（排除值之前和之后）
            # 然后与现有的min_bound和max_bound合并
            
            # 收集所有边界点（包括排除值）
            boundary_points = []
            if min_bound is not None:
                boundary_points.append(('min', min_bound, min_op))
            for excl_val in exclude_values:
                boundary_points.append(('excl', excl_val, None))
            if max_bound is not None:
                boundary_points.append(('max', max_bound, max_op))
            
            # 如果没有边界点，只有排除值，返回两个区间
            if not boundary_points:
                for excl_val in exclude_values:
                    ranges.append(f'(-∞,{excl_val})')
                    ranges.append(f'({excl_val},+∞)')
            else:
                # 排序边界点
                boundary_points.sort(key=lambda x: x[1])
                
                # 构建区间
                prev_val = None
                prev_type = None
                for i, (btype, val, op) in enumerate(boundary_points):
                    if btype == 'excl':
                        # 排除值：添加之前和之后的区间
                        if prev_val is not None:
                            # 添加 (prev_val, excl_val) 区间
                            left_bracket = '[' if prev_type == 'min' and min_op == '>=' else '('
                            ranges.append(f'{left_bracket}{prev_val},{val})')
                        else:
                            # 第一个边界是排除值，添加 (-∞, excl_val)
                            ranges.append(f'(-∞,{val})')
                        
                        # 检查下一个边界
                        if i + 1 < len(boundary_points):
                            next_val = boundary_points[i + 1][1]
                            ranges.append(f'({val},{next_val})')
                        else:
                            # 最后一个边界，添加 (excl_val, +∞)
                            ranges.append(f'({val},+∞)')
                    
                    prev_val = val
                    prev_type = btype
            
            if ranges:
                # 去重并排序
                unique_ranges = sorted(set(ranges))
                if len(unique_ranges) == 1:
                    return unique_ranges[0]
                else:
                    return ' U '.join(unique_ranges)
            else:
                # 所有值都被排除，返回false
                return 'false'
        
        # 格式化输出（边界值从字符串提取，已验证可满足性）
        result = format_range(min_bound, max_bound, min_op, max_op)
        return result if result else var_name
    
    # 处理 OR 条件：分别处理每个分支
    elif '||' in cond_str:
        parts = cond_str.split('||')
        ranges = []
        
        for part in parts:
            part = part.strip()
            constraint = parse_single_condition_to_z3(part, var)
            if constraint is None:
                continue
            
            # 使用 Z3 Solver 检查可满足性
            solver = Solver()
            solver.add(constraint)
            if solver.check() == unsat:
                continue  # 跳过不可满足的分支
            
            # 从字符串中提取边界值
            result = parse_single_condition(part)
            if result:
                op, value = result
                min_val = None
                max_val = None
                min_op = None
                max_op = None
                
                if op in ['>', '>=']:
                    min_val = value
                    min_op = op
                elif op in ['<', '<=']:
                    max_val = value
                    max_op = op
                
                # 格式化单个分支的范围
                branch_range = format_range(min_val, max_val, min_op, max_op)
                if branch_range:
                    ranges.append(branch_range)
        
        if not ranges:
            return var_name
        
        # 组合成并集格式
        if len(ranges) == 1:
            return ranges[0]
        else:
            # 排序范围（小的在前）
            def get_sort_key(r):
                match = re.search(r'(-?\d+(?:\.\d+)?)', r)
                if match:
                    return float(match.group(1))
                return 0
            
            ranges_sorted = sorted(ranges, key=get_sort_key)
            return ' U '.join(ranges_sorted)
    
    # 单个条件：使用 Z3 验证，从字符串提取边界
    else:
        constraint = parse_single_condition_to_z3(cond_str, var)
        if constraint is None:
            return var_name
        
        # 使用 Z3 Solver 检查可满足性
        solver = Solver()
        solver.add(constraint)
        if solver.check() == unsat:
            return 'false'
        
        # 从字符串中提取边界值
        result = parse_single_condition(cond_str)
        if result:
            op, value = result
            min_val = None
            max_val = None
            min_op = None
            max_op = None
            
            if op in ['>', '>=']:
                min_val = value
                min_op = op
            elif op in ['<', '<=']:
                max_val = value
                max_op = op
            
            # 格式化输出
            result = format_range(min_val, max_val, min_op, max_op)
            return result if result else var_name
        
        return var_name

def parse_condition(cond_str, var_name):
    """
    主解析函数
    优先使用 Z3，如果不可用则回退
    """
    # 检查特殊值
    if cond_str.upper().strip() == 'TRUE':
        return var_name
    
    # 优先使用 Z3
    if Z3_AVAILABLE:
        try:
            return parse_with_z3(cond_str, var_name)
        except Exception as e:
            # Z3 解析失败，回退
            pass
    
    # 回退：返回原值
    return var_name

return parse_condition(cond_str, var_name)
$$ LANGUAGE plpython3u IMMUTABLE;

-- 测试函数
-- SELECT parse_ctable_condition_z3_sympy('X>10&&X<20', 'X');  -- 应该返回: "(10,20)"
-- SELECT parse_ctable_condition_z3_sympy('Y>20 || Y<2', 'Y'); -- 应该返回: "(-∞,2) U (20,+∞)"
-- SELECT parse_ctable_condition_z3_sympy('Z>=15&&Z<=25', 'Z'); -- 应该返回: "[15,25]"
-- SELECT parse_ctable_condition_z3_sympy('X<10&&X>20', 'X');   -- 应该返回: "false"
