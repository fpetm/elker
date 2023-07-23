import market, json, csv

data = {}

with open('skins.json', encoding='utf8') as f:
    data = json.load(f)

with open('skins_newo.csv', 'w', encoding='utf8', newline='') as f:
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
            if (COLLECTION == 'Revolution Case' or COLLECTION == 'Recoil Case') and True:
                r = market.get_csgo_item(WEAPON + ' | ' + NAME + ' (' + wear + ')')
                rst = market.get_csgo_item('StatTrak™ ' + WEAPON + ' | ' + NAME + ' (' + wear + ')')
            else:
                r = {'success' : False}
                rst = {'success' : False}

            print(r,rst)

            if not r['success']:
                print('error failed to get {}'.format(WEAPON + ' | ' + NAME + ' (' + wear + ')'))
                PRICE[wear] = -1
            else:
                if 'lowest_price' in r: PRICE[wear] = float(r['lowest_price'][:-1].replace(',','.').replace('--','00'))
                elif 'median_price' in r: PRICE[wear] = float(r['median_price'][:-1].replace(',','.').replace('--','00'))
                else: PRICE[wear] = -1

            if not rst['success']:
                print('error failed to get {}'.format('StatTrak™ ' + WEAPON + ' | ' + NAME + ' (' + wear + ')'))
                PRICEST[wear] = -1
            else:
                if 'lowest_price' in r: PRICEST[wear] = float(rst['lowest_price'][:-1].replace(',','.').replace('--','00'))
                elif 'median_price' in r: PRICEST[wear] = float(rst['median_price'][:-1].replace(',','.').replace('--','00'))
                else: PRICEST[wear] = -1

        writer.writerow([NAME, COLLECTION, RARITY, WEAPON,
                         PRICE  ['Battle-Scarred'], PRICE  ['Well-Worn'], PRICE  ['Field-Tested'], PRICE  ['Minimal Wear'], PRICE  ['Factory New'],
                         PRICEST['Battle-Scarred'], PRICEST['Well-Worn'], PRICEST['Field-Tested'], PRICEST['Minimal Wear'], PRICEST['Factory New'],
                         ])





#print(data[55])
