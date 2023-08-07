import requests, re, csv, time
from bs4 import BeautifulSoup

URL = 'https://csgostash.com/skin/'

def load_skindata_csv(path):
    skins = {}
    with open(path, mode = 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if row[0] == 'name': continue
            weapon = row[1]
            skin = row[0]
            name = weapon + ' | ' + skin

            min_wear = float(row[4])
            max_wear = float(row[5])

            skins[name] = (min_wear, max_wear)
    return skins

def load_skins_csgostash_csv(path):
    skins = {}
    with open(path, mode = 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if row[0] == 'Name': continue
            name = row[0]
            min_wear = float(row[3])
            max_wear = float(row[4])
            skins[name] = (min_wear, max_wear)
    return skins

def fetch_item(i):
    r = requests.get(URL + str(i))
    if not r:
        print(f'failed to fetch item {i}')
        return None

    wear_min = -1
    wear_max = -1
    name = -1

    soup = BeautifulSoup(r.text, 'lxml')
    for p in soup.select('div.marker-wrapper.wear-min-value'):
        wear_min = p.attrs['data-wearmin']
        break
    for p in soup.select('div.marker-wrapper.wear-max-value'):
        wear_max = p.attrs['data-wearmax']
        break

    title = soup.find('title').string

    namem = re.match('(.*) \\| ([^-]*)(?:- CS\\:GO Stash)', title)
    if namem:
        weapon = namem.group(1)
        skin = namem.group(2)[:-1]
        full = weapon + ' | ' + skin
        return (full, weapon, skin, wear_min, wear_max)
    else:
        return None

def dump_csgostash(path):
    skins_csgostash = []
    skins = {}
    for i in range(1,2048):
        print(f'Downloading {i}')
        item = fetch_item(i)
        if item:
            skins[item[0]] = (item[1], item[2])
            skins_csgostash.append(fetch_item(i))
        time.sleep(1)
    with open('resources/csgostash.csv', mode='w', encoding='utf-8') as f:
        writer = csv.writer(f)
        writer.writerow(('Name', 'Weapon', 'Skin', 'Wear min', 'Wear max'))
        for weapon in skins_csgostash:
            writer.writerow(weapon)
    return skins

def main():
    skins_csgostash = dump_csgostash('resources/csgostash.csv')
    skins_og = load_skindata_csv('resources/skindata.csv')
#    skins_csgostash = load_skins_csgostash_csv('resources/csgostash.csv')

    for skin in skins_og:
        if not skin in skins_csgostash:
            print(f'{skin} not in csgostash table')
            continue
        if not (skins_og[skin][0] == skins_csgostash[skin][0] and skins_og[skin][1] == skins_csgostash[skin][1]):
            print(f'{skin}: mismatched wear values ! min: {skins_og[skin][0]} : {skins_csgostash[skin][0]} .. max : {skins_og[skin][1]} : {skins_csgostash[skin][1]} ')

main()
