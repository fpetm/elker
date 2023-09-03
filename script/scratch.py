import requests
import re

session = requests.Session()

def init():
    url = f'https://steamcommunity.com/market'
    r = session.get(url)
    print(r)
    pattern = re.compile(r'(?:g_sessionID = ")(.{24})(?:";)')
    result = pattern.search(r.text)
    return result.group(1)

def get(weapon):
    data = {'query' : '',
            'start' : 10,
            'count' : 10,
            'country' : 'US',
            'language' : 'english',
            'currency' : 1}
    r1 = session.get('https://steamcommunity.com/market/listings/730/AK-47%20%7C%20Ice%20Coaled%20%28Factory%20New%29')
    print(r1)
    url = f'https://steamcommunity.com/market/listings/730/{weapon}/render/#?query=&start=10&count=10&country=HU&language=english&currency=1'
    r = session.get(url, data = data)
    print(r)
    print(r.text)
    return r.json()

def main():
    sessionid = init()

    print(get('AK-47 | Slate (Field-Tested)'))

if __name__ == '__main__':
    main()
