CREATE TYPE weapontype AS ENUM (
  'CZ75-Auto', 'Desert Eagle', 'Dual Berettas', 'Five-SeveN', 'Glock-18', 'P2000', 'P250', 'R8 Revolver', 'Tec-9', 'USP-S', 'MAG-7', 'Nova', 'Sawed-Off', 'XM1014', 'M249', 'Negev', 'MAC-10', 'MP5-SD', 'MP7', 'MP9', 'P90', 'PP-Bizon', 'UMP-45', 'AK-47', 'AUG', 'FAMAS', 'Galil AR', 'M4A1-S', 'M4A4', 'SG 553', 'AWP', 'SSG 08', 'G3SG1', 'SCAR-20'
);

CREATE TYPE skinrarity AS ENUM (
  'Consumer Grade', 'Industrial Grade', 'Mil-Spec Grade', 'Restricted', 'Classified', 'Covert');

CREATE TABLE collections (
  name VARCHAR(100) UNIQUE NOT NULL
);

CREATE TABLE skins (
  id SERIAL PRIMARY KEY,
  name VARCHAR(100) NOT NULL,
  rarity skinrarity NOT NULL,
  weapon weapontype NOT NULL,
  wear_max FLOAT, wear_min FLOAT,
  collection VARCHAR(100) REFERENCES collections(name)
);

CREATE TABLE skin_prices (
  id SERIAL REFERENCES skins(id),

  sell_bs MONEY,
  sell_ww MONEY,
  sell_ft MONEY,
  sell_mw MONEY,
  sell_fn MONEY,

  buy_bs MONEY,
  buy_ww MONEY,
  buy_ft MONEY,
  buy_mw MONEY,
  buy_fn MONEY
);
