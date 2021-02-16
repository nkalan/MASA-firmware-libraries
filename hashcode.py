#!/usr/bin/env python3
"""
CLI to develop firmware code hash. 

Recurses through a Core/ directory, aggregating text from files ending in .c
and .h, hasing aggregate text and writing to Inc/codehash.h.
"""
from argparse import ArgumentParser
from datetime import datetime
from hashlib import md5  # replace the __hash__, which is bad
from pathlib import Path

parser = ArgumentParser(description='Hash firmware project and save in codehash.h.')
parser.add_argument('corepath', default=None, nargs='?', help="""
    path to \'Core\' firmware directory. Defaults two directories up.
    """)
args = parser.parse_args()

if __name__ == '__main__':
    path2script = Path(__file__).resolve().parent
    corepath = path2script.parent
    if args.corepath is not None:
        corepath = Path(args.corepath).resolve()
    if corepath.stem != 'Core':
        raise RuntimeError(f'not a Core/ path: \'{corepath}\'')
    incpath = [v for v in corepath.iterdir() if 'inc' in str(v).lower()][0]

    # Recurse Through Files
    print(f'Aggregating code text in \'{corepath}\'')
    str2hash = ''
    for child in sorted(corepath.glob('**/*.c')):
        with open(str(child), 'r') as f:
            str2hash = str2hash + f.read()
    for child in sorted(corepath.glob('**/*.h')):
        if child.name == 'codehash.h':  # don't hash the hash file (never match)
            continue
        with open(str(child), 'r') as f:
            str2hash = str2hash + f.read()

    # Determine Hash & Output
    m = md5()
    m.update(bytes(str2hash, 'utf-8'))
    codehash = m.hexdigest()
    fnhash = incpath / 'codehash.h'
    writestr = f"""/** codehash.h
 *
 * Last Updated: {datetime.now().isoformat()}
 *
 * Firmware hash for this firmware project
 */
#define CODEHASH {codehash}"""
    with open(str(fnhash), 'w') as f:
        f.write(writestr)
    print(f'hash: {codehash}')
    
