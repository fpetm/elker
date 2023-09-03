import sys
import vpk

def extract(vpk_dir_path, inpath, outpath):
    pak = vpk.open(vpk_dir_path)
    with open(outpath, 'wb') as outfile:
        with pak.get_file(inpath) as infile:
            outfile.write(infile.read())

def main():
    if len(sys.argv[1]) < 3:
        print(f'usage: {sys.argv[0]} [ vpk_dir file path ] [ items_game output path ]')
    extract(sys.argv[1], 'scripts/items/items_game.txt', sys.argv[2])


if __name__ == '__main__':
    main()

