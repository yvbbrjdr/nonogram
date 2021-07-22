#!/usr/bin/env python3

import sys

import requests

URL = 'https://onlinenonograms.com/actions.php'

def get_row(solution, i):
    return solution[i]

def get_col(solution, i):
    return ''.join([row[i] for row in solution])

def output_puzzle(line):
    puzzle = list(map(lambda x: len(x), filter(lambda x: x, line.split('9'))))
    for i in range(len(puzzle) - 1):
        print(f'{puzzle[i]}', end=' ')
    if puzzle:
        print(f'{puzzle[-1]}', end='')
    print()

def main(argc, argv):
    if argc != 4:
        print(f'usage: {argv[0]} id rows cols', file=sys.stderr)
        sys.exit(1)

    id = int(argv[1])
    rows = int(argv[2])
    cols = int(argv[3])
    size = rows * cols

    resp = requests.post(URL, data={'m': 'getmask', 'id': id})
    if resp.status_code != 200:
        print(f'error: {resp.status_code}', file=sys.stderr)
        sys.exit(1)
    
    mask = resp.json()['mask']
    if mask is None:
        print('error: no mask found', file=sys.stderr)
        sys.exit(1)
    
    if len(mask) != size:
        print('error: invalid mask size', file=sys.stderr)
        sys.exit(1)
    
    print(f'{rows} {cols}')
    
    solution = [mask[i:i+cols] for i in range(0, size, cols)]
    for i in range(rows):
        output_puzzle(get_row(solution, i))
    for i in range(cols):
        output_puzzle(get_col(solution, i))

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
