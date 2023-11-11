#!/usr/bin/env python3
import re
import time
import csv
import psycopg

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
            raise SyntaxError(f'unexpected character {string[index]}')
    return tokens

def parse(tokens):
    def recurs(index = 0):
        objects = {}
        while index < len(tokens):
            first = tokens[index]

            if first == "}":
                return objects, index+1

            second = tokens[index+1]

            index += 2

            if second == "{":
                lower, index = recurs(index)
                if first in objects:
                    for k in lower.items():
                        objects[first][k[0]] = k[1]
                else:
                    objects[first] = lower
            else:
                objects[first] = second

        return objects, index
    return recurs()[0]

def load(path):
    success = None
    text = ''
    with open(path, encoding='utf8') as file:
        tic = time.perf_counter()
        for line in file.readlines():
            if line.lstrip().startswith('//') or \
               line.rstrip().endswith('[$PS3]') or \
               line.rstrip().endswith('[$X360]') or \
               line.rstrip().endswith('[$OSX]') or \
               line.rstrip().endswith('[$X360||$PS3]') or \
               line.rstrip().endswith('[$PS3||$X360]') or \
               line.rstrip().endswith('[!$WIN32&&!$OSX]'):
                continue

            text += re.sub(r'[^:]\/\/.*\n', '', line)
        text = text.replace('\\"', "'"). \
                replace('[$WIN32]',''). \
                replace('[$WIN32||$X360]',''). \
                replace('[$WIN32||$PS3]',''). \
                replace('[$WIN32||$OSX]',''). \
                replace('[$WIN32||$X360||$OSX]',''). \
                replace('[!$PS3]',''). \
                replace('[!$X360]',''). \
                replace('[$WIN32||$OSX||$PS3]', ''). \
                replace('[!$X360&&!$PS3]','')
        toc = time.perf_counter()

        print(f'Loaded {path} into memory in {toc - tic:0.4f} seconds!')

    tic = time.perf_counter()
    tokens = lex(text)
    toc = time.perf_counter()
    print(f'Lexed {path} in {toc - tic:0.4f} seconds!')

    tic = time.perf_counter()
    success = parse(tokens)
    toc = time.perf_counter()
    print(f'Parsed {path} in {toc - tic:0.4f} seconds!')
    return success

def extract_skins(SKINDATA_PATH = './resources/skindata.csv',
                  ITEM_GAME_PATH = './resources/items_game.txt',
                  CSGO_ENGLISH_PATH = './resources/csgo_english.txt'):
    def translate(dictionary, name):
        for entry, value in dictionary.items():
            if name.upper() == entry.upper():
                return value
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
        if not 'item_name' in prefab[1]:
            continue
        item_name = translate(names, prefab[1]['item_name'][1:])
        prefabs[name] = {'item_name' : item_name}

    items = {}
    for item in items_ig.items():
        if 'baseitem' in item[1]:
            name = item[1]['name']
            prefab = item[1]['prefab']
            if not prefab in prefabs:
                continue
            item_name = prefabs[prefab]['item_name']
            items[name] = {'item_name' : item_name}

    paint_kits = []
    for paint_kit in paint_kits_ig.items():
        identifier = paint_kit[0]
        name = paint_kit[1]['name']
        description_string, description_tag = None, None

        if 'description_string' in paint_kit[1]:
            description_string = translate(names, paint_kit[1]['description_string'][1:])
        if 'description_tag' in paint_kit[1]:
            description_tag = translate(names, paint_kit[1]['description_tag'][1:])

        wear_remap_min = 0.06
        wear_remap_max = 0.8
        if 'wear_remap_min' in paint_kit[1]:
            wear_remap_min = float(paint_kit[1]['wear_remap_min'])
        if 'wear_remap_max' in paint_kit[1]:
            wear_remap_max = float(paint_kit[1]['wear_remap_max'])

        paint_kits.append({'id' : identifier,
                           'name' : name,
                           'wear_min': wear_remap_min,
                           'wear_max': wear_remap_max,
                           'description_string' : description_string,
                           'description_tag' : description_tag})

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
        else:
            continue

        name = item_set[0]
        description = translate(names, item_set[1]['name'][1:])
        set_description = translate(names, item_set[1]['set_description'][1:])
        weapons = []

        for weapon in item_set[1]['items']:
            if not '[' in weapon:
                continue
            skin_name = re.findall(r'\[.*\]', weapon)[0][1:-1]
            weapon_name = re.sub(r'\[.*\]', '', weapon)
            weapons.append({'skin' : skin_name, 'weapon' : weapon_name})

        item_sets.append({'name' : name,
                          'description' : description,
                          'set_description' : set_description,
                          'weapons' : weapons})

    skin_rarities = {}
    for client_loot_list in client_loot_lists_ig.items():
        for rarity in rarities:
            if client_loot_list[0].endswith(rarity['name']):
                for weapon in client_loot_list[1]:
                    skin_rarities[weapon] = rarity

    skins = []
    for item_set in item_sets:
        for paint_kit in paint_kits:
            for item in item_set['weapons']:
                if paint_kit['name'] == item['skin']:
                    collection = item_set
                    skins.append({'paint_kit' : paint_kit,
                                  'weapon' : item['weapon'],
                                  'collection' : collection})

    collections = {}
    for skin in skins:
        collections[skin['collection']['description']] = skin['collection']

    with psycopg.connect('service=elker') as conn:
        with conn.cursor() as cur:
            cur.execute('DELETE FROM skin_prices')
            cur.execute('DELETE FROM skins')
            cur.execute('DELETE FROM collections')
            cur.executemany('INSERT INTO collections (name) VALUES (%(name)s)', [{'name': v['description']} for k,v in collections.items()])

            skins_data = []
            for skin in skins:
                name = skin['paint_kit']['name']
                tag = skin['paint_kit']['description_tag']
                #description = skin['paint_kit']['description_string']
                weapon_type = items[skin['weapon']]['item_name']
                weapon = skin['weapon']
            
                if not f'[{name}]{weapon}' in skin_rarities:
                    continue
                rarity = skin_rarities[f'[{name}]{weapon}']['description']
                collection = skin['collection']['description']
                wear_min = skin['paint_kit']['wear_min']
                wear_max = skin['paint_kit']['wear_max']
                skins_data.append({'name' : tag, 'rarity' : rarity, 'weapon' : weapon_type, 'wear_min' : wear_min, 'wear_max' : wear_max, 'collection' : collection})
            cur.executemany('''
INSERT INTO skins (name, rarity, weapon, wear_min, wear_max, collection_id)
VALUES (%(name)s, %(rarity)s, %(weapon)s, %(wear_min)s, %(wear_max)s, (SELECT id FROM collections WHERE name = %(collection)s))''', skins_data)
        conn.commit()
        with conn.cursor() as cur:
            cur.execute('SELECT id FROM skins')
            ids = cur.fetchall()
            cur.executemany('INSERT INTO skin_prices (skin_id) VALUES ( %(id)s )', [{'id':str(e[0])} for e in ids])
        conn.commit()
        

def main():
    extract_skins()

if __name__ == '__main__':
    main()
