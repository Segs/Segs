-- CREATE DATABASE segs_game WITH TEMPLATE = template0 ENCODING = 'UTF8';
-- ALTER DATABASE segs_game OWNER TO segsadmin;

CREATE TABLE accounts (
    id integer NOT NULL,
    account_id bigint NOT NULL,
    max_slots smallint DEFAULT 8 NOT NULL
);
ALTER TABLE public.accounts OWNER TO segsadmin;
COMMENT ON COLUMN accounts.account_id IS 'references account database';
CREATE SEQUENCE accounts_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;
ALTER TABLE public.accounts_id_seq OWNER TO segsadmin;
ALTER SEQUENCE accounts_id_seq OWNED BY accounts.id;

CREATE TABLE characters (
    id integer NOT NULL,
    char_level smallint DEFAULT 0 NOT NULL,
    slot_index smallint DEFAULT 0 NOT NULL,
    account_id integer NOT NULL,
    char_name character varying(32) NOT NULL,
    archetype character varying(32) NOT NULL,
    origin character varying(32) NOT NULL,
    bodytype integer NOT NULL,
    current_map character varying(128) NOT NULL,
    last_costume_id integer DEFAULT 0
);
ALTER TABLE public.characters OWNER TO segsadmin;
CREATE SEQUENCE characters_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;
ALTER TABLE public.characters_id_seq OWNER TO segsadmin;
ALTER SEQUENCE characters_id_seq OWNED BY characters.id;

CREATE TABLE costume (
    id integer NOT NULL,
    character_id integer NOT NULL,
    costume_index smallint NOT NULL,
    skin_color bigint
);
ALTER TABLE public.costume OWNER TO segsadmin;

CREATE SEQUENCE costume_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;

ALTER TABLE public.costume_id_seq OWNER TO segsadmin;
ALTER SEQUENCE costume_id_seq OWNED BY costume.id;

CREATE TABLE costume_part (
    id integer NOT NULL,
    costume_id integer NOT NULL,
    part_type integer DEFAULT 0 NOT NULL,
    name_0 character varying(256),
    name_1 character varying(256),
    name_2 character varying(256),
    name_3 character varying(256),
    color_0 bigint NOT NULL,
    color_1 bigint NOT NULL,
    color_2 integer DEFAULT 0 NOT NULL,
    color_3 integer DEFAULT 0 NOT NULL
);

ALTER TABLE public.costume_part OWNER TO segsadmin;

CREATE SEQUENCE costume_part_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;

ALTER TABLE public.costume_part_id_seq OWNER TO segsadmin;
ALTER SEQUENCE costume_part_id_seq OWNED BY costume_part.id;

ALTER TABLE accounts ALTER COLUMN id SET DEFAULT nextval('accounts_id_seq'::regclass);
ALTER TABLE characters ALTER COLUMN id SET DEFAULT nextval('characters_id_seq'::regclass);
ALTER TABLE costume ALTER COLUMN id SET DEFAULT nextval('costume_id_seq'::regclass);
ALTER TABLE costume_part ALTER COLUMN id SET DEFAULT nextval('costume_part_id_seq'::regclass);

ALTER TABLE ONLY accounts
    ADD CONSTRAINT account_account_id_key UNIQUE (account_id);

ALTER TABLE ONLY accounts
    ADD CONSTRAINT account_pkey PRIMARY KEY (id);

ALTER TABLE ONLY characters
    ADD CONSTRAINT character_account_id_key UNIQUE (account_id, slot_index);

ALTER TABLE ONLY characters
    ADD CONSTRAINT character_pkey PRIMARY KEY (id);

ALTER TABLE ONLY costume
    ADD CONSTRAINT costume_character_id_key UNIQUE (character_id, costume_index);

ALTER TABLE ONLY costume_part
    ADD CONSTRAINT costume_part_costume_id_key UNIQUE (costume_id, part_type);

ALTER TABLE ONLY costume_part
    ADD CONSTRAINT costume_part_pkey PRIMARY KEY (id);

ALTER TABLE ONLY costume
    ADD CONSTRAINT costume_pkey PRIMARY KEY (id);

CREATE INDEX fki_ ON characters USING btree (account_id);

ALTER TABLE ONLY characters
    ADD CONSTRAINT character_account_id_fkey FOREIGN KEY (account_id) REFERENCES accounts(id) ON UPDATE CASCADE ON DELETE CASCADE;

ALTER TABLE ONLY costume
    ADD CONSTRAINT costume_character_id_fkey FOREIGN KEY (character_id) REFERENCES characters(id) ON UPDATE CASCADE ON DELETE CASCADE;

ALTER TABLE ONLY costume_part
    ADD CONSTRAINT costume_part_character_id_fkey FOREIGN KEY (costume_id) REFERENCES costume(id) ON UPDATE CASCADE ON DELETE CASCADE;

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;




