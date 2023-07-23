import requests, time, json

key = 'D6LwF248v8SFi5QLk2C7IiY8UlM'
appid = '730'

currencies = {
    'USD' : 1,
    'GBP' : 2,
    'EUR' : 3,
    'CHF' : 4,
    'RUB' : 5,
}


r = requests.get('https://api.steamapis.com/market/items/730?api_key='+key)
j = r.json()

prices = {}

for s in j['data']:
    prices[s['market_hash_name']] = s['prices']['safe']

#print(prices.keys())

def get_csgo_item(name, currency = 'EUR'):
#    if 'Hydra' in name:
#        if name in prices: print(prices[name])
#    if name in prices:
#        return prices[name]

#    print(name + ' not in prices')
    time.sleep(0.5)
    url = 'https://api.steamapis.com/market/item/' + str(appid) + '/' + name + '?api_key=' + key
    market_item = requests.get(url)

    if 'error' in market_item.json(): return -1

    #print(json.dumps(market_item.json(), indent = 2))

    return market_item.json()['median_avg_prices_15days'][0][1]

