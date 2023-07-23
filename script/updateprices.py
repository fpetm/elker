import market, json, csv

data = []

with open('skindata.csv', 'r', encoding='utf8') as f:
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

with open('skins.csv', 'w', encoding='utf8', newline='') as f:
    writer = csv.writer(f, delimiter=',')
    writer.writerow(['NAME', 'COLLECTION', 'RARITY', 'WEAPON', 'WEAR_MIN', 'WEAR_MAX', 'PRICE_BS', 'PRICE_WW', 'PRICE_FT', 'PRICE_MW', 'PRICE_FN', 'PRICE_BS_ST', 'PRICE_WW_ST', 'PRICE_FT_ST', 'PRICE_MW_ST', 'PRICE_BS_ST'])
    for skin in data:
        COLLECTION = skin['collection']
        RARITY = skin['rarity']
        WEAPON = skin['weapon']
        NAME = skin['name']
        WEAR_MIN = skin['wear_min']
        WEAR_MAX = skin['wear_max']

#        if not COLLECTION in ('The Horizon Collection', 'The Danger Zone Collection', 'The Prisma Collection', 'The CS20 Collection', 'The Shattered Web Collection', 'The Prisma 2 Collection', 'The Fracture Collection', 'The Broken Fang Collection', 'The Snakebite Collection', 'The Operation Riptide Collection', 'The Dreams & Nightmares Collection', 'The Recoil Collection', 'The Revolution Collection'):
#            continue
        print(skin)
        PRICE = {}
        PRICEST = {}


        for wear in ['Battle-Scarred', 'Well-Worn', 'Field-Tested', 'Minimal Wear', 'Factory New']:
            #PRICE[wear] = -1
            #PRICEST[wear] = -1
            PRICE[wear] = market.get_csgo_item(WEAPON + ' | ' + NAME + ' (' + wear + ')')
            PRICEST[wear] = market.get_csgo_item('StatTrak™ ' + WEAPON + ' | ' + NAME + ' (' + wear + ')')

        writer.writerow([NAME, COLLECTION, RARITY, WEAPON, WEAR_MIN, WEAR_MAX,
                         PRICE  ['Battle-Scarred'], PRICE  ['Well-Worn'], PRICE  ['Field-Tested'], PRICE  ['Minimal Wear'], PRICE  ['Factory New'],
                         PRICEST['Battle-Scarred'], PRICEST['Well-Worn'], PRICEST['Field-Tested'], PRICEST['Minimal Wear'], PRICEST['Factory New'],
                         ])





#print(data[55])
