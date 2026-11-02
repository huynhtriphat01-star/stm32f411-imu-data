"""Offline check of actual logger CRC functions and binary payload size."""
from pathlib import Path
import ast, struct
root=Path(__file__).resolve().parents[1]
for filename in ['logger.py','collect.py']:
    tree=ast.parse((root/filename).read_text(encoding='utf-8'))
    function=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='crc8')
    namespace={}
    exec(compile(ast.Module(body=[function],type_ignores=[]),filename,'exec'),namespace)
    if namespace['crc8'](b'123456789') != 0xF4:
        raise AssertionError(filename+': CRC vector failed')
assert struct.calcsize('<HIhhhhhh') == 18
print('PASS: both actual CRC functions match 0xF4; 2+18+1 = 21 bytes')
