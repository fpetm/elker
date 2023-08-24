import itertools
import sys

MAX = 10

head = '''#pragma once
#include <unordered_map>
#include <vector>
namespace motek {
'''

bottom = '''
}
'''
base = '''  const std::unordered_map<size_t, std::vector<std::vector<int>>> {name} = {{ 
{content} }};'''

def rows(table, A, B, k):
    n = 0
    while n < B:
        yield table[n*A+k]
        n += 1

def cols(table, A, B, k):
    n = 0
    while n < A:
        yield table[k*A+n]
        n += 1

def genvars(A, B, summa_a, summa_b):
    r = range(0,MAX+1)
    product = itertools.product(*[r for _ in range(A*B)])

    ret = []

    for p in product:
        if sum(p) != MAX: continue
        
        try:
            for k, s in enumerate(summa_a):
                if sum(rows(p, A, B, k)) != s:
                    raise StopIteration
            for k, s in enumerate(summa_b):
                if sum(cols(p, A, B, k)) != s:
                    raise StopIteration
        except StopIteration:
            continue

        ret.append(p)
    return ret

def partition(n,m, minimum = 1):
    if m == 1: return [(n,)]
    r = range(minimum,n)
    product = itertools.product(*[r for _ in range(m)])

    ret = []
    for p in product:
        if sum(p) == n:
            ret.append(p)
    return ret

def gen_key(p1,p2):
    BIT_COUNT = 4
    s = 0

    parts = [0,0]
    for i, p in enumerate([p1,p2]):
        for j,e in enumerate(p):
            parts[i] |= e << (BIT_COUNT * j)
    s = (parts[1] << 32) | parts[0]
#    sys.stderr.write(f'{s:16X} : {p1} : {p2}\n')
    return s

def gen(A, B):
    p1s = partition(MAX, A, 0)
    p2s = partition(MAX, B, 0)

    m = {}
    for p1, p2 in itertools.product(p1s, p2s):
        g = genvars(A, B, p1, p2)
        key = gen_key(p1, p2)
        m[key] = g
    return m

def main():
    s = ''
    pairs = [[1,1],[1,2],[2,1],[2,2],[3,2],[2,3],[3,3]]
    s += head

    m = {}
    for A,B in pairs:
        m.update(gen(A,B))

    c = ''
    for k,v in m.items():
        l = ''
        for j in v:
            l += '{' + ','.join([f'{e}' for e in j]) + '},'
        c += f'    {{0x{k:016X}, {{{l}}}}},\n'

    s += base.format(name = f'g_Variations', content = c)
    s += bottom
    sys.stdout.write(s)

if __name__ == '__main__':
    main()
