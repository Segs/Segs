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
  char_level smallint NOT NULL DEFAULT 0,
  slot_index smallint NOT NULL DEFAULT 0,
  account_id integer NOT NULL,
  char_name character varying(32) NOT NULL,
  archetype character varying(32) NOT NULL,
  origin character varying(32) NOT NULL,
  bodytype integer NOT NULL,
  current_map character varying(128) NOT NULL,
  last_costume_id integer DEFAULT 0,
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
  character_id integer NOT NULL,
  costume_index smallint NOT NULL,
  skin_color bigint,
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
  costume_id integer NOT NULL,
  part_type integer NOT NULL DEFAULT 0,
  name_0 character varying(256),
  name_1 character varying(256),
  name_2 character varying(256),
  name_3 character varying(256),
  color_0 bigint NOT NULL,
  color_1 bigint NOT NULL,
  color_2 integer NOT NULL DEFAULT 0,
  color_3 integer NOT NULL DEFAULT 0,
  CONSTRAINT costume_part_pkey PRIMARY KEY (id),
  CONSTRAINT costume_part_character_id_fkey FOREIGN KEY (costume_id)
      REFERENCES costume (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE,
  CONSTRAINT costume_part_costume_id_key UNIQUE (costume_id, part_type)
)
WITHOUT OIDS;
ALTER TABLE costume_part OWNER TO segsadmin;
ALTER TABLE characters ADD CONSTRAINT characters_last_costume_id_fkey FOREIGN KEY (last_costume_id) REFERENCES costume (id) ON UPDATE SET NULL ON DELETE SET NULL;
