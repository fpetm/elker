import json
import os
import time

import requests
import dotenv

dotenv.load_dotenv()

KEY = os.getenv('STEAMAPIS_API_KEY')
APPID = '730'

def f(x):
    return float(x) if x else 0.0
def i(x):
    return int(x) if x else 0

def get_csgo_item(name):
    url = 'https://api.steamapis.com/market/item/' + str(APPID) + '/' + name + '?api_key=' + KEY
    market_item = requests.get(url, timeout = 10)

    j = {}
    try:
        j = market_item.json()
    except json.decoder.JSONDecodeError:
        print(f'Failed request retrying: {market_item.text}')

        market_item = requests.get(url, timeout = 10)
        try:
            j = market_item.json()
        except json.decoder.JSONDecodeError:
            print(f'Failed request retrying twice: {market_item.text}')
            market_item = requests.get(url, timeout = 10)
            try:
                j = market_item.json()
            except json.decoder.JSONDecodeError:
                print(f'Failed request finally: {market_item.text}')

    if 'error' in j:
        if j['error'] == 'No matching item found with these parameters':
            return 0.0, 0.0, 0, 0
        print(json.dumps(market_item.json(), indent = 2))
        return 0.0, 0.0, 0, 0

    time.sleep(0.1)
    ret = (f(market_item.json()['histogram']['lowest_sell_order']),
           f(market_item.json()['histogram']['highest_buy_order']),
           i(market_item.json()['histogram']['sell_order_summary']['quantity']),
           i(market_item.json()['histogram']['buy_order_summary']['quantity']),)
    print(ret)
    return ret
