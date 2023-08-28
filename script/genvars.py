import itertools
import sys
import operator

MAX = 10

def rows(table, column_count, row_count, k):
    index = 0
    while index < row_count:
        yield table[index*column_count+k]
        index += 1

def cols(table, column_count, k):
    index = 0
    while index < column_count:
        yield table[k*column_count+index]
        index += 1

def genvars(column_count, row_count, summa_a, summa_b):
    products = itertools.product(*[range(0,MAX+1) for _ in range(column_count*row_count)])

    ret = []

    for product in products:
        if sum(product) != MAX:
            continue
        try:
            for k, summa in enumerate(summa_a):
                if sum(rows(product, column_count, row_count, k)) != summa:
                    raise StopIteration
            for k, summa in enumerate(summa_b):
                if sum(cols(product, column_count, k)) != summa:
                    raise StopIteration
        except StopIteration:
            continue

        ret.append(product)
    return ret

def partition(n,m, minimum = 1):
    if m == 1:
        return [(n,)]
    r = range(minimum,n)
    product = itertools.product(*[r for _ in range(m)])

    ret = []
    for p in product:
        if sum(p) == n:
            ret.append(p)
    return ret

def gen_part(p):
    BIT_COUNT = 4
    z = 0
    for i,e in enumerate(p):
        z |= e << (BIT_COUNT * i)
    return z

def gen_key(p1,p2):
    s = 0

    parts = [0,0]
    for i, p in enumerate([p1,p2]):
        parts[i] = gen_part(p)
    s = (parts[1] << 32) | parts[0]
#    sys.stderr.write(f'{s:16X} : {p1} : {p2}\n')
    return s

def print_compositions():
    s = ''
    s += 'const std::array<std::vector<uint64_t>, 10> g_Compositions = {\n'
    for i in range(1, 11):
        sys.stderr.write(f'running {i}\n')
        sys.stderr.flush()
        part = partition(10, i)
        s += '  {'
        for p in part:
            s += f'0x{gen_part(p):X}, '
        s += '},\n'
    s += '}'
    sys.stdout.write(s)

def print_compositions_accumulated():
    s = ''
    s += 'const std::array<std::vector<uint64_t>, 10> g_CompositionsAccumulated = {\n'
    for i in range(1, 11):
        sys.stderr.write(f'running {i}\n')
        sys.stderr.flush()
        part = partition(10, i)
        s += '  {'
        for p in part:
            accu = itertools.accumulate(p, operator.add)
            s += f'0x{gen_part(accu):X}, '
        s += '},\n'
    s += '}'
    sys.stdout.write(s)

def print_compositions_flattened():
    s = ''
    s += 'const std::array<std::vector<uint64_t>, 10> g_CompositionsFlat = {\n'
    for i in range(1, 11):
        sys.stderr.write(f'running {i}\n')
        sys.stderr.flush()
        part = partition(10, i)
        s += '  {'
        for p in part:
            accu = list(itertools.accumulate(p, operator.add))
            g = []
            k = 0
            for index in range(10):
                if index == accu[k]:
                    k+=1
                g.append(k)
            s += f'0x{gen_part(g):X}, '
        s += '},\n'
    s += '}'
    sys.stdout.write(s)

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
    pairs = [[1,1],[1,2],[2,1],[2,2],[3,2],[2,3],[1,3],[3,1]]
    base = 'const std::unordered_map<size_t, std::vector<uint64_t>> {name} = {{\n{content} }};'

    m = {}
    for A,B in pairs:
        m.update(gen(A,B))

    c = ''
    for k,v in m.items():
        l = ''
        for j in v:
#            l += '{' + ','.join([f'{e}' for e in j]) + '},'
            l += f'0x{gen_part(j):0{len(j)}X} ,'
        c += f'    {{0x{k:016X}, {{{l}}}}},\n'

    s += base.format(name = 'g_Variations', content = c)
    s += '\n};'
    sys.stdout.write(s)

if __name__ == '__main__':
    main()
