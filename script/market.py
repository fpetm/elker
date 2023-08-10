import json
import os

import requests
import dotenv

dotenv.load_dotenv()

KEY = os.getenv('STEAMAPIS_API_KEY')
APPID = '730'

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

    #print(json.dumps(market_item.json(), indent = 2))

    if 'error' in j:
        if j['error'] == 'No matching item found with these parameters':
            return -1, -1
        print(json.dumps(market_item.json(), indent = 2))
        return -1, -1


    return (market_item.json()['histogram']['lowest_sell_order'],
            market_item.json()['histogram']['highest_buy_order'])
