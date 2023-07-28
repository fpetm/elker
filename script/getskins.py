import json, re, time, csv

QUOTE = '"'
J_WHITESPACE = [' ', '\t', '\b', '\n', '\r']
J_SYNTAX = ['{', '}']


def lex(string):
    tokens = []

    index = 0
    while index < len(string):
        j_str = ''
        if string[index] == QUOTE:
            index += 1

            while index < len(string):
                if string[index] == QUOTE:
                    index += 1
                    break
                else:
                    j_str += string[index]
                    index += 1

            if j_str is not None:
                tokens.append(j_str)
                continue
            
        if string[index] in J_WHITESPACE:
            index += 1
        elif string[index] in J_SYNTAX:
            tokens.append(string[index])
            index += 1
        else:
            raise Exception(f'unexpected character {string[index]}')
    return tokens

def parse(tokens):
    def recurs(index = 0):
        o = {}
        while index < len(tokens):
            first = tokens[index]
        
            if first == "}":
                return o, index+1

            second = tokens[index+1]

            index += 2

            if second == "{":
                lower,index = recurs(index)
                if first in o:
                    for k in lower:
                        o[first][k] = lower[k]
                else:
                    o[first] = lower
            else:
                o[first] = second

        return o, index
    return recurs()[0]

def load(path):
    r = None
    
    with open(path, encoding='utf8') as f:
        tic = time.perf_counter()
        text = ''
        for line in f.readlines():
            if line.lstrip().startswith('//') or line.rstrip().endswith('[$PS3]') or line.rstrip().endswith('[$X360]') or line.rstrip().endswith('[$OSX]') or line.rstrip().endswith('[$X360||$PS3]') or line.rstrip().endswith('[$PS3||$X360]') or line.rstrip().endswith('[!$WIN32&&!$OSX]'):
                continue
            text += re.sub('[^:]\/\/.*\n', '', line)
        text = text.replace('\\"', "'").replace('[$WIN32]','').replace('[$WIN32||$X360]','').replace('[$WIN32||$PS3]','').replace('[$WIN32||$OSX]','').replace('[$WIN32||$X360||$OSX]','').replace('[!$PS3]','').replace('[!$X360]','').replace('[$WIN32||$OSX||$PS3]', '').replace('[!$X360&&!$PS3]','')
        toc = time.perf_counter()

        print(f'Loaded {path} into memory in {toc - tic:0.4f} seconds!')

        tic = time.perf_counter()
        tokens = lex(text)
        toc = time.perf_counter()
        print(f'Lexed {path} in {toc - tic:0.4f} seconds!')

        tic = time.perf_counter()
        r = parse(tokens)
        toc = time.perf_counter()
        print(f'Parsed {path} in {toc - tic:0.4f} seconds!')
    return r

class Item:
    def __init__(self, name, weapontype, rarity):
        pass

class Collection:
    def __init__(self, name, description, item_list):
        self.name = name
        self.description = description
        self.items = item_list

class PaintKit:
    def __init__(self, name, tag, description, wear_min, wear_max):
        self.name = name
        self.tag = tag
        self.description = description
        self.wear_min = wear_min
        self.wear_max = wear_max

ITEM_GAME_PATH = './resources/items_game.txt'
CSGO_ENGLISH_PATH = './resources/csgo_english.txt'
SKINDATA_PATH = './resources/skindata.csv'

def main():
    def translate(dictionary, name):
        if name in dictionary:
            return dictionary[name]
        else:
            return None
    items_game = load(ITEM_GAME_PATH)
    csgo_english = load(CSGO_ENGLISH_PATH)

    names = csgo_english['lang']['Tokens']
    
    items_ig = items_game['items_game']['items']
    paint_kits_ig = items_game['items_game']['paint_kits']
    paint_kits_rarity_ig = items_game['items_game']['paint_kits_rarity']
    rarities_ig = items_game['items_game']['rarities']
    item_sets_ig = items_game['items_game']['item_sets']
    prefabs_ig = items_game['items_game']['prefabs']
    client_loot_lists_ig = items_game['items_game']['client_loot_lists']

    rarities = []
    rarity_d = {}
    for rarity in rarities_ig.items():
        name = rarity[0]
        value = rarity[1]['value']
        description = translate(names, rarity[1]['loc_key_weapon'])
        rarities.append({'name' : name, 'value' : value, 'description' : description})
        rarity_d[name] = description

    prefabs = {}
    for prefab in prefabs_ig.items():
        name = prefab[0]
        if not 'item_name' in prefab[1]: continue 
        item_name = translate(names, prefab[1]['item_name'][1:])
        prefabs[name] = {'item_name' : item_name}

    items = {}
    for item in items_ig.items():
        if 'baseitem' in item[1]:
            name = item[1]['name']
            prefab = item[1]['prefab']
            if not prefab in prefabs: continue
            item_name = prefabs[prefab]['item_name']
            items[name] = {'item_name' : item_name}

    paint_kits = []
    for paint_kit in paint_kits_ig.items():
        ID = paint_kit[0]
        name = paint_kit[1]['name']
        description_string, description_tag = None, None 
        if 'description_string' in paint_kit[1]: description_string = translate(names, paint_kit[1]['description_string'][1:])
        if 'description_tag' in paint_kit[1]: description_tag = translate(names, paint_kit[1]['description_tag'][1:])

        # THIS IS AN UGLY FIX BUT I DON'T CARE
        if name == 'hy_ddpat_orange': 
            wear_remap_min = 0.06
            wear_remap_max = 0.80

        wear_remap_min = 0.0
        wear_remap_max = 1.0
        if 'wear_remap_min' in paint_kit[1]:
            wear_remap_min = float(paint_kit[1]['wear_remap_min'])
        if 'wear_remap_max' in paint_kit[1]:
            wear_remap_max = float(paint_kit[1]['wear_remap_max'])

        paint_kits.append({'id' : ID, 'name' : name, 'wear_min': wear_remap_min, 'wear_max': wear_remap_max, 'description_string' : description_string, 'description_tag' : description_tag})

    for paint_kit_rarity in paint_kits_rarity_ig.items():
        name, rarity = paint_kit_rarity
        for paint_kit in paint_kits:
            if paint_kit['name'] == name:
                paint_kit['rarity'] = rarity

    item_sets = []
    for item_set in item_sets_ig.items():
        if 'is_collection' in item_set[1]:
            if not item_set[1]['is_collection'] == '1':
                continue
        else: continue

        name = item_set[0]
        description = translate(names, item_set[1]['name'][1:])
        set_description = translate(names, item_set[1]['set_description'][1:])
        weapons = []

        for weapon in item_set[1]['items']:
            if not '[' in weapon: continue
            skin_name = re.findall('\[.*\]', weapon)[0][1:-1]
            weapon_name = re.sub('\[.*\]', '', weapon)
            weapons.append({'skin' : skin_name, 'weapon' : weapon_name})

        item_sets.append({'name' : name, 'description' : description, 'set_description' : set_description, 'weapons' : weapons})

    skin_rarities = {}
    for c in client_loot_lists_ig.items():
        for rarity in rarities:
            if c[0].endswith(rarity['name']):
                for weapon in c[1]:
                    skin_rarities[weapon] = rarity

    skins = []
    for item_set in item_sets:
        for paint_kit in paint_kits:
            for w in item_set['weapons']:
                if paint_kit['name'] == w['skin']:
                    collection = item_set
                    skins.append({'paint_kit' : paint_kit, 'weapon' : w['weapon'], 'collection' : collection})

    with open(SKINDATA_PATH, 'w', newline = '', encoding = 'utf8') as f:
        writer = csv.writer(f, delimiter = ',', quotechar = '"', quoting = csv.QUOTE_MINIMAL)
        writer.writerow(['name', 'weapon_type', 'rarity', 'collection', 'wear_min', 'wear_max'])
        for skin in skins:
            name = skin['paint_kit']['name']
            tag = skin['paint_kit']['description_tag']
            #description = skin['paint_kit']['description_string']
            weapon_type = items[skin['weapon']]['item_name']
            weapon = skin['weapon']

            if not f'[{name}]{weapon}' in skin_rarities: continue
            rarity = skin_rarities[f'[{name}]{weapon}']['description']
            collection = skin['collection']['description']
            wear_min = skin['paint_kit']['wear_min']
            wear_max = skin['paint_kit']['wear_max']
            
            writer.writerow([tag, weapon_type, rarity, collection, wear_min, wear_max])

    
       

if __name__ == '__main__':
    main()