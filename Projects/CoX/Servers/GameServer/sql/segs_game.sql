-- CREATE DATABASE segs_game
--   WITH OWNER = postgres
--       ENCODING = 'UTF8'
--       TABLESPACE = pg_default;
-- DROP SEQUENCE account_id_seq;
-- DROP TABLE accounts;
CREATE TABLE accounts
(
  id serial NOT NULL,
  account_id int8 NOT NULL, -- references account database
  max_slots int2 NOT NULL DEFAULT 8,
  CONSTRAINT account_pkey PRIMARY KEY (id),
  CONSTRAINT account_account_id_key UNIQUE (account_id)
) 
WITHOUT OIDS;
ALTER TABLE accounts OWNER TO segsadmin;
COMMENT ON COLUMN accounts.account_id IS 'references account database';


-- DROP SEQUENCE character_id_seq;
-- DROP TABLE characters;

CREATE TABLE characters
(
  id serial NOT NULL,
  char_level int2 NOT NULL DEFAULT 0,
  slot_index int2 NOT NULL DEFAULT 0,
  account_id int4 NOT NULL,
  char_name varchar(32) NOT NULL,
  archetype varchar(32) NOT NULL,
  origin varchar(32) NOT NULL,
  bodytype int4 NOT NULL,
  face_bits int4 NOT NULL,
  current_map varchar(128) NOT NULL,
  villain bool NOT NULL,
  unkn1 float4 NOT NULL,
  unkn2 float4 NOT NULL,
  unkn3 int4 NOT NULL DEFAULT 1,
  unkn4 int4 NOT NULL DEFAULT 1,
  last_costume_id int4,
  CONSTRAINT character_pkey PRIMARY KEY (id),
  CONSTRAINT character_account_id_fkey FOREIGN KEY (account_id)
      REFERENCES accounts (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT character_account_id_key UNIQUE (account_id, slot_index)
) 
WITHOUT OIDS;
ALTER TABLE characters OWNER TO segsadmin;


-- Index: fki_

-- DROP INDEX fki_;

CREATE INDEX fki_
  ON characters
  USING btree
  (account_id);

-- DROP SEQUENCE costume_id_seq;
-- DROP TABLE costume;

CREATE TABLE costume
(
  id serial NOT NULL,
  floats_14 float4[] NOT NULL,
  arr7 int4[] NOT NULL,
  character_id int4 NOT NULL,
  costume_index int2 NOT NULL,
  CONSTRAINT costume_pkey PRIMARY KEY (id),
  CONSTRAINT costume_character_id_fkey FOREIGN KEY (character_id)
      REFERENCES characters (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT costume_character_id_key UNIQUE (character_id, costume_index)
) 
WITHOUT OIDS;
ALTER TABLE costume OWNER TO segsadmin;

-- DROP SEQUENCE costume_part_id_seq;
-- DROP TABLE costume_part;

CREATE TABLE costume_part
(
  id serial NOT NULL,
  costume_id int4 NOT NULL,
  part_type int4 NOT NULL DEFAULT 0,
  name_0 varchar(256),
  name_0_idx integer NOT NULL,
  name_1 varchar(256),
  name_1_idx integer NOT NULL,
  name_2 varchar(256),
  name_2_idx integer NOT NULL,
  name_3 varchar(256),
  name_3_idx integer NOT NULL,
  color_0 int4 NOT NULL,
  color_0_idx integer NOT NULL,
  color_1 int4 NOT NULL,
  color_1_idx integer NOT NULL,
  color_2 int4 NOT NULL,
  color_2 int4 NOT NULL,
  color_3_idx integer NOT NULL,
  color_3_idx integer NOT NULL,
  CONSTRAINT costume_part_pkey PRIMARY KEY (id),
  CONSTRAINT costume_part_character_id_fkey FOREIGN KEY (costume_id)
      REFERENCES costume (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT costume_part_costume_id_key UNIQUE (costume_id, part_type)
) 
WITHOUT OIDS;
ALTER TABLE costume_part OWNER TO segsadmin;
ALTER TABLE characters ADD CONSTRAINT characters_last_costume_id_fkey FOREIGN KEY (last_costume_id) REFERENCES costume (id) ON UPDATE SET NULL ON DELETE SET NULL;
