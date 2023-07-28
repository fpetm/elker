import market, json, csv

data = []

SKINDATA_PATH = './resources/skindata.csv'
SKINS_PATH = './resources/skins.csv'

with open(SKINDATA_PATH, 'r', encoding='utf8') as f:
    reader = csv.reader(f, delimiter = ',', quotechar = '"')
    for row in reader:
        if row[0] == 'name': continue
        name = row[0]
        weapon_type = row[1]
        rarity = row[2]
        collection = row[3]
        wear_min = float(row[4])
        wear_max = float(row[5])
        data.append({'name':name, 'weapon' : weapon_type, 'rarity' : rarity, 'collection' : collection, 'wear_min' : wear_min, 'wear_max' : wear_max})

with open(SKINS_PATH, 'w', encoding='utf8', newline='') as f:
    writer = csv.writer(f, delimiter=',')
    writer.writerow(['NAME', 'COLLECTION', 'RARITY', 'WEAPON', 'WEAR_MIN', 'WEAR_MAX', 
                     'PRICE_BS_SELL',       'PRICE_WW_SELL',    'PRICE_FT_SELL',    'PRICE_MW_SELL',    'PRICE_FN_SELL',
                     'PRICE_BS_ST_SELL',    'PRICE_WW_ST_SELL', 'PRICE_FT_ST_SELL', 'PRICE_MW_ST_SELL', 'PRICE_FN_ST_SELL',
                     'PRICE_BS_BUY',        'PRICE_WW_BUY',     'PRICE_FT_BUY',     'PRICE_MW_BUY',     'PRICE_FN_BUY',
                     'PRICE_BS_ST_BUY',     'PRICE_WW_ST_BUY',  'PRICE_FT_ST_BUY',  'PRICE_MW_ST_BUY',  'PRICE_FN_ST_BUY',
                     ])
    for skin in data:
        COLLECTION = skin['collection']
        RARITY = skin['rarity']
        WEAPON = skin['weapon']
        NAME = skin['name']
        WEAR_MIN = skin['wear_min']
        WEAR_MAX = skin['wear_max']

        print(skin)
        SPRICE = {}
        SPRICEST = {}
        BPRICE = {}
        BPRICEST = {}


        for wear in ['Battle-Scarred', 'Well-Worn', 'Field-Tested', 'Minimal Wear', 'Factory New']:
            SPRICE[wear], BPRICE[wear] = market.get_csgo_item(WEAPON + ' | ' + NAME + ' (' + wear + ')')
            SPRICEST[wear], BPRICEST[wear] = market.get_csgo_item('StatTrak™ ' + WEAPON + ' | ' + NAME + ' (' + wear + ')')

        print(SPRICE)
        print(SPRICEST)
        print(BPRICE)
        print(BPRICEST)

        writer.writerow([NAME, COLLECTION, RARITY, WEAPON, WEAR_MIN, WEAR_MAX,
                         SPRICE  ['Battle-Scarred'], SPRICE  ['Well-Worn'], SPRICE  ['Field-Tested'], SPRICE  ['Minimal Wear'], SPRICE  ['Factory New'],
                         SPRICEST['Battle-Scarred'], SPRICEST['Well-Worn'], SPRICEST['Field-Tested'], SPRICEST['Minimal Wear'], SPRICEST['Factory New'],
                         BPRICE  ['Battle-Scarred'], BPRICE  ['Well-Worn'], BPRICE  ['Field-Tested'], BPRICE  ['Minimal Wear'], BPRICE  ['Factory New'],
                         BPRICEST['Battle-Scarred'], BPRICEST['Well-Worn'], BPRICEST['Field-Tested'], BPRICEST['Minimal Wear'], BPRICEST['Factory New'],
                         ])





#print(data[55])
