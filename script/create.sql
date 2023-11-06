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
  skin_id INTEGER,
  CONSTRAINT skin FOREIGN KEY(skin_id) REFERENCES skins(id),

  sell_bs FLOAT,
  sell_ww FLOAT,
  sell_ft FLOAT,
  sell_mw FLOAT,
  sell_fn FLOAT,

  buy_bs FLOAT,
  buy_ww FLOAT,
  buy_ft FLOAT,
  buy_mw FLOAT,
  buy_fn FLOAT,

  sell_bs_volume INT,
  sell_ww_volume INT,
  sell_ft_volume INT,
  sell_mw_volume INT,
  sell_fn_volume INT,

  buy_bs_volume INT,
  buy_ww_volume INT,
  buy_ft_volume INT,
  buy_mw_volume INT,
  buy_fn_volume INT,

  sell_bs_st FLOAT,
  sell_ww_st FLOAT,
  sell_ft_st FLOAT,
  sell_mw_st FLOAT,
  sell_fn_st FLOAT,

  buy_bs_st FLOAT,
  buy_ww_st FLOAT,
  buy_ft_st FLOAT,
  buy_mw_st FLOAT,
  buy_fn_st FLOAT,

  sell_bs_st_volume INT,
  sell_ww_st_volume INT,
  sell_ft_st_volume INT,
  sell_mw_st_volume INT,
  sell_fn_st_volume INT,

  buy_bs_st_volume INT,
  buy_ww_st_volume INT,
  buy_ft_st_volume INT,
  buy_mw_st_volume INT,
  buy_fn_st_volume INT
);
