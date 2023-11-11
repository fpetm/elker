#!/usr/bin/env python3
import time
import csv
import market
import psycopg

WEAR_VALUES = ('Battle-Scarred', 'Well-Worn', 'Field-Tested', 'Minimal Wear', 'Factory New')
WEAR_VALUES_SHORT = ('BS', 'WW', 'FT', 'MW', 'FN')

def load_skindata():
    data=[]
    with psycopg.connect('service=elker') as conn:
        with conn.cursor() as cur:
            cur.execute('SELECT id, name, weapon FROM skins')
            for record in cur.fetchall():
                data.append({'id' : record[0],
                             'name' : record[1],
                             'weapon' : record[2],})
    return data


def update_prices(data):
    with psycopg.connect('service=elker') as conn:
        with conn.cursor() as cur:
            for skin in data:
                weapon = skin['weapon']
                name = skin['name']

                print(skin)
                sell_prices, sell_volumes = {}, {}
                sell_prices_stattrak, sell_stattrak_volumes = {}, {}
                buy_prices, buy_volumes = {}, {}
                buy_prices_stattrak, buy_stattrak_volumes = {}, {}

                for condition in WEAR_VALUES:
                    sell_prices[condition], buy_prices[condition], sell_volumes[condition], buy_volumes[condition] = \
                            market.get_csgo_item(f'{weapon} | {name} ({condition})')

                    sell_prices_stattrak[condition], buy_prices_stattrak[condition], sell_stattrak_volumes[condition], buy_stattrak_volumes[condition] = \
                        market.get_csgo_item(f'StatTrak™ {weapon} | {name} ({condition})')

                d = tuple([[table[wear] for table in pair for wear in WEAR_VALUES] for pair in [[sell_prices, sell_prices_stattrak], [buy_prices, buy_prices_stattrak], [sell_volumes, sell_stattrak_volumes], [buy_volumes, buy_stattrak_volumes]]] + [skin['id']])
                print(d)
                cur.execute('''UPDATE skin_prices SET sell = %s, buy = %s, sell_volume = %s, buy_volume = %s WHERE skin_id = %s''', d)
                print(skin['id'])

                conn.commit()

def main():
    data = load_skindata()
    update_prices(data)

if __name__ == '__main__':
    main()
