import requests, time
from requests_tor import RequestsTor

currencies = {
    'USD' : 1,
    'GBP' : 2,
    'EUR' : 3,
    'CHF' : 4,
    'RUB' : 5,
}

s = requests.Session()
#s = RequestsTor()
retries = requests.adapters.Retry(total = 10, backoff_factor = 0.1, status_forcelist=[500, 502, 503, 504, 429])
s.mount('http://', requests.adapters.HTTPAdapter(max_retries=retries))

rc = 0

def get_item(appid, name, currency = 'EUR'):
    global rc
    time.sleep(3)
    url = 'http://steamcommunity.com/market/priceoverview'
    market_item = s.get(url,params={
        'appid': str(appid),
        'market_hash_name': name,
        'currency': currencies[currency]
    })

    rc += 1

#    print(rc)
#    if rc % 10 == 0:
#        s.new_id()
#        print(s.get('https://api.ipify.org/').text)
#        print('new id')
#        s.test()

    print(market_item, market_item.text)
    return market_item.json()

def get_csgo_item(item, currency = 'EUR'):
    return get_item('730', item, currency)
