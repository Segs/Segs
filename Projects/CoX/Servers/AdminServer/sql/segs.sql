CREATE DATABASE segs WITH TEMPLATE = template0 ENCODING = 'UTF8';


ALTER DATABASE segs OWNER TO segsadmin;

connect segs

CREATE TABLE accounts (
    id integer NOT NULL,
    username character varying(32),
    access_level smallint,
    creation_date timestamp(6) without time zone DEFAULT now() NOT NULL,
    passw bytea
);

ALTER TABLE public.accounts OWNER TO segsadmin;

CREATE SEQUENCE accounts_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;
ALTER TABLE public.accounts_id_seq OWNER TO segsadmin;
ALTER SEQUENCE accounts_id_seq OWNED BY accounts.id;
ALTER TABLE accounts ALTER COLUMN id SET DEFAULT nextval('accounts_id_seq'::regclass);

CREATE TABLE bans (
    id integer NOT NULL,
    account_id integer,
    offending_ip inet,
    started timestamp without time zone DEFAULT now(),
    reason text
);
ALTER TABLE public.bans OWNER TO segsadmin;

CREATE SEQUENCE bans_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;
ALTER TABLE public.bans_id_seq OWNER TO segsadmin;
ALTER SEQUENCE bans_id_seq OWNED BY bans.id;

CREATE TABLE game_servers (
    id integer NOT NULL,
    addr inet,
    port smallint,
    name character varying(100),
    token bigint
);
ALTER TABLE public.game_servers OWNER TO segsadmin;
CREATE SEQUENCE game_servers_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;
ALTER TABLE public.game_servers_id_seq OWNER TO segsadmin;

ALTER SEQUENCE game_servers_id_seq OWNED BY game_servers.id;
ALTER TABLE bans ALTER COLUMN id SET DEFAULT nextval('bans_id_seq'::regclass);

ALTER TABLE game_servers ALTER COLUMN id SET DEFAULT nextval('game_servers_id_seq'::regclass);

ALTER TABLE ONLY accounts
    ADD CONSTRAINT accounts_pkey PRIMARY KEY (id);

ALTER TABLE ONLY accounts
    ADD CONSTRAINT accounts_username_key UNIQUE (username);

ALTER TABLE ONLY bans
    ADD CONSTRAINT bans_pkey PRIMARY KEY (id);

ALTER TABLE ONLY game_servers
    ADD CONSTRAINT game_servers_pkey PRIMARY KEY (id);

ALTER TABLE ONLY bans
    ADD CONSTRAINT bans_account_id_fkey FOREIGN KEY (account_id) REFERENCES accounts(id) ON UPDATE CASCADE ON DELETE CASCADE;

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;

