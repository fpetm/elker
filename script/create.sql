DROP TABLE IF EXISTS skin_prices;
DROP TABLE IF EXISTS skins;
DROP TABLE IF EXISTS collections;
DROP TYPE IF EXISTS weapontype;
DROP TYPE IF EXISTS skinrarity;

CREATE TYPE weapontype AS ENUM (
  'CZ75-Auto', 'Desert Eagle', 'Dual Berettas', 'Five-SeveN', 'Glock-18', 'P2000', 'P250', 'R8 Revolver', 'Tec-9', 'USP-S', 'MAG-7', 'Nova', 'Sawed-Off', 'XM1014', 'M249', 'Negev', 'MAC-10', 'MP5-SD', 'MP7', 'MP9', 'P90', 'PP-Bizon', 'UMP-45', 'AK-47', 'AUG', 'FAMAS', 'Galil AR', 'M4A1-S', 'M4A4', 'SG 553', 'AWP', 'SSG 08', 'G3SG1', 'SCAR-20'
);

CREATE TYPE skinrarity AS ENUM (
  'Consumer Grade', 'Industrial Grade', 'Mil-Spec Grade', 'Restricted', 'Classified', 'Covert');

CREATE TABLE collections (
  id SERIAL PRIMARY KEY,
  name VARCHAR(100) UNIQUE NOT NULL
);
CREATE TABLE skins (
  id SERIAL PRIMARY KEY,
  name VARCHAR(100) NOT NULL,
  rarity skinrarity NOT NULL,
  weapon weapontype NOT NULL,
  wear_min FLOAT, wear_max FLOAT,
  collection_id SERIAL REFERENCES collections(id)
);

CREATE TABLE skin_prices (
  skin_id INTEGER,
  CONSTRAINT skin FOREIGN KEY(skin_id) REFERENCES skins(id),

  sell FLOAT[10],
  buy FLOAT[10],

  sell_volume INTEGER[10],
  buy_volume INTEGER[10]
);
