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
            cur.execute('SELECT id, name, weapon, collection, rarity, wear_min, wear_max FROM skins')
            for record in cur.fetchall():
                data.append({'id' : record[0],
                             'name' : record[1],
                             'weapon' : record[2],
                             'collection' : record[3],
                             'rarity' : record[4],
                             'wear_min' : record[5],
                             'wear_max' : record[6]})
    return data


def update_prices(data):
    with psycopg.connect('service=elker') as conn:
        with conn.cursor() as cur:
            for skin in data:
                collection = skin['collection']
                rarity = skin['rarity']
                weapon = skin['weapon']
                name = skin['name']
                wear_min = skin['wear_min']
                wear_max = skin['wear_max']

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

                print(sell_prices)
                print(sell_prices_stattrak)
                print(buy_prices)
                print(buy_prices_stattrak)

                cur.execute('''
UPDATE skin_prices SET 
sell_bs = %s,
sell_ww = %s,
sell_ft = %s,
sell_mw = %s,
sell_fn = %s,

buy_bs = %s,
buy_ww = %s,
buy_ft = %s,
buy_mw = %s,
buy_fn = %s,

sell_bs_volume = %s,
sell_ww_volume = %s,
sell_ft_volume = %s,
sell_mw_volume = %s,
sell_fn_volume = %s,

buy_bs_volume = %s,
buy_ww_volume = %s,
buy_ft_volume = %s,
buy_mw_volume = %s,
buy_fn_volume = %s,

sell_bs_st = %s,
sell_ww_st = %s,
sell_ft_st = %s,
sell_mw_st = %s,
sell_fn_st = %s,

buy_bs_st = %s,
buy_ww_st = %s,
buy_ft_st = %s,
buy_mw_st = %s,
buy_fn_st = %s,

sell_bs_st_volume = %s,
sell_ww_st_volume = %s,
sell_ft_st_volume = %s,
sell_mw_st_volume = %s,
sell_fn_st_volume = %s,

buy_bs_st_volume = %s,
buy_ww_st_volume = %s,
buy_ft_st_volume = %s,
buy_mw_st_volume = %s,
buy_fn_st_volume = %s
WHERE skin_id = %s''', 
                            [table[wear] for table in (sell_prices, buy_prices, sell_volumes, buy_volumes, sell_prices_stattrak, buy_prices_stattrak, sell_stattrak_volumes, buy_stattrak_volumes) for wear in WEAR_VALUES] + [skin['id']])
                print(skin['id'])

                conn.commit()

def main():
    data = load_skindata()
    update_prices(data)

if __name__ == '__main__':
    main()
