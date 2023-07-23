import market, json, csv

data = {}

with open('skins.json', encoding='utf8') as f:
    data = json.load(f)

with open('skins.csv', 'w', encoding='utf8', newline='') as f:
    writer = csv.writer(f, delimiter=',')
    writer.writerow(['NAME', 'COLLECTION', 'RARITY', 'WEAPON', 'PRICE_BS', 'PRICE_WW', 'PRICE_FT', 'PRICE_MW', 'PRICE_FN', 'PRICE_BS_ST', 'PRICE_WW_ST', 'PRICE_FT_ST', 'PRICE_MW_ST', 'PRICE_BS_ST'])
    for skin in data:
        print(skin)
        COLLECTION = skin['collection']
        RARITY = skin['quality']
        WEAPON = skin['weapon']
        NAME = skin['skin']

        PRICE = {}
        PRICEST = {}

        for wear in ['Battle-Scarred', 'Well-Worn', 'Field-Tested', 'Minimal Wear', 'Factory New']:
            PRICE[wear] = -1
            PRICEST[wear] = -1
            if (COLLECTION == 'Revolution Case' or COLLECTION == 'Recoil Case') or True:
                PRICE[wear] = market.get_csgo_item(WEAPON + ' | ' + NAME + ' (' + wear + ')')
                PRICEST[wear] = market.get_csgo_item('StatTrak™ ' + WEAPON + ' | ' + NAME + ' (' + wear + ')')

        writer.writerow([NAME, COLLECTION, RARITY, WEAPON,
                         PRICE  ['Battle-Scarred'], PRICE  ['Well-Worn'], PRICE  ['Field-Tested'], PRICE  ['Minimal Wear'], PRICE  ['Factory New'],
                         PRICEST['Battle-Scarred'], PRICEST['Well-Worn'], PRICEST['Field-Tested'], PRICEST['Minimal Wear'], PRICEST['Factory New'],
                         ])





#print(data[55])
