#!/usr/bin/env python3
import time
import csv
import market

WEAR_VALUES = ('Battle-Scarred', 'Well-Worn', 'Field-Tested', 'Minimal Wear', 'Factory New')
WEAR_VALUES_SHORT = ('BS', 'WW', 'FT', 'MW', 'FN')

def load_skindata(skindata_path = './resources/skindata.csv'):
    data = []
    with open(skindata_path, 'r', encoding='utf8') as file:
        reader = csv.reader(file, delimiter = ',', quotechar = '"')
        for row in reader:
            if row[0] == 'name':
                continue
            name = row[0]
            weapon_type = row[1]
            rarity = row[2]
            collection = row[3]
            wear_min = float(row[4])
            wear_max = float(row[5])
            data.append({'name' : name,
                         'weapon' : weapon_type,
                         'rarity' : rarity,
                         'collection' : collection,
                         'wear_min' : wear_min,
                         'wear_max' : wear_max})
    return data

def update_prices(data, skins_path = './resources/skins.csv'):
    with open(skins_path, 'w', encoding='utf8', newline='') as file:
        writer = csv.writer(file, delimiter=',')
        toprow = ['NAME', 'COLLECTION', 'RARITY', 'WEAPON', 'WEAR_MIN', 'WEAR_MAX']
        for postfix in ['SELL', 'ST_SELL', 'BUY', 'ST_BUY']:
            for condition in WEAR_VALUES_SHORT:
                toprow.append(f'PRICE_{condition}_{postfix}')
        writer.writerow(toprow)

        for skin in data:
            collection = skin['collection']
            rarity = skin['rarity']
            weapon = skin['weapon']
            name = skin['name']
            wear_min = skin['wear_min']
            wear_max = skin['wear_max']

            print(skin)
            sell_prices = {}
            sell_prices_stattrak = {}
            buy_prices = {}
            buy_prices_stattrak = {}

            for condition in WEAR_VALUES:
                sell_prices[condition], buy_prices[condition] = \
                        market.get_csgo_item(f'{weapon} | {name} ({condition})')
                sell_prices_stattrak[condition], buy_prices_stattrak[condition] = \
                    market.get_csgo_item(f'StatTrak™ {weapon} | {name} ({condition})')

            print(sell_prices)
            print(sell_prices_stattrak)
            print(buy_prices)
            print(buy_prices_stattrak)

            row = [name, collection, rarity, weapon, wear_min, wear_max]
            for table in (sell_prices, sell_prices_stattrak, buy_prices, buy_prices_stattrak):
                for wear in WEAR_VALUES:
                    row.append(table[wear])

            writer.writerow(row)


def main():
    data = load_skindata()
    update_prices(data)

if __name__ == '__main__':
    main()
