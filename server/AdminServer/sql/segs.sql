CREATE TABLE accounts (
    id serial NOT NULL,
    username character varying(32),
    access_level smallint,
    creation_date timestamp(6) without time zone DEFAULT now() NOT NULL,
    passw bytea
);
ALTER TABLE public.accounts OWNER TO segsadmin;

CREATE TABLE bans (
    id serial NOT NULL,
    account_id integer,
    offending_ip inet,
    started timestamp without time zone DEFAULT now(),
    reason text
);

ALTER TABLE public.bans OWNER TO segsadmin;

CREATE TABLE game_servers (
    id serial NOT NULL,
    addr inet,
    port smallint,
    name character varying(100),
    token bigint
);

ALTER TABLE public.game_servers OWNER TO segsadmin;
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
