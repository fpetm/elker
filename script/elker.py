import argparse
import updateprices
import getskins

def extract(path = './resources/skindata.csv'):
    print('Extracting skins')
    getskins.extract_skins(path)

def update(skindata = './resources/skindata.csv', skinprices = './resources/skins.csv'):
    print('Updating skins')
    data = updateprices.load_skindata(skindata)
    updateprices.update_prices(data, skinprices)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('action', action = 'store', choices = ['extract', 'update', 'full'])
    arguments = parser.parse_args()

    if arguments.action == 'extract':
        extract()
    elif arguments.action == 'update':
        update()
    elif arguments.action == 'full':
        extract()
        update()

if __name__ == '__main__':
    main()
