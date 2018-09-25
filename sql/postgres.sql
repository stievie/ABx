--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: account_account_keys; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.account_account_keys (
    account_uuid character(36) NOT NULL,
    account_key_uuid character(36) NOT NULL
);


ALTER TABLE public.account_account_keys OWNER TO sa;

--
-- Name: account_bans; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.account_bans (
    uuid character(36) NOT NULL,
    ban_uuid character(36) NOT NULL,
    account_uuid character(36) NOT NULL
);


ALTER TABLE public.account_bans OWNER TO sa;

--
-- Name: account_keys; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.account_keys (
    uuid character(36) NOT NULL,
    used bigint DEFAULT 0::bigint NOT NULL,
    total bigint DEFAULT 1::bigint NOT NULL,
    description character varying(255) DEFAULT NULL::character varying,
    status smallint DEFAULT 0::smallint NOT NULL,
    key_type bigint DEFAULT 0::bigint NOT NULL,
    email character varying(60) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.account_keys OWNER TO sa;

--
-- Name: accounts; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.accounts (
    uuid character(36) DEFAULT ''::bpchar NOT NULL,
    name character varying(32) NOT NULL,
    password character varying(255) NOT NULL,
    email character varying(255) DEFAULT ''::character varying NOT NULL,
    type bigint DEFAULT 1::bigint NOT NULL,
    status bigint DEFAULT 0::bigint NOT NULL,
    creation bigint DEFAULT 0::bigint NOT NULL,
    char_slots bigint DEFAULT 6::bigint NOT NULL,
    current_character_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    current_server_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    online_status bigint DEFAULT 0::bigint NOT NULL,
    titles bytea,
    guild_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL
);


ALTER TABLE public.accounts OWNER TO sa;

--
-- Name: account_view; Type: VIEW; Schema: public; Owner: sa
--

CREATE VIEW public.account_view AS
 SELECT accounts.uuid,
    accounts.name,
    accounts.password,
    accounts.email,
    accounts.type,
    accounts.creation,
    accounts.titles,
    account_keys.uuid AS account_key,
    account_keys.used,
    account_keys.total,
    (account_keys.total - account_keys.used) AS keys_left,
    account_keys.status,
    account_keys.description
   FROM ((public.accounts
     LEFT JOIN public.account_account_keys ON ((account_account_keys.account_uuid = accounts.uuid)))
     LEFT JOIN public.account_keys ON ((account_account_keys.account_key_uuid = account_keys.uuid)));


ALTER TABLE public.account_view OWNER TO sa;

--
-- Name: bans; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.bans (
    uuid character(36) NOT NULL,
    expires bigint NOT NULL,
    added bigint NOT NULL,
    reason bigint DEFAULT 0::bigint NOT NULL,
    active bigint DEFAULT 0::bigint NOT NULL,
    admin_uuid character(36) NOT NULL,
    comment character varying(255) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.bans OWNER TO sa;

--
-- Name: friend_list; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.friend_list (
    account_uuid character(36) NOT NULL,
    friend_uuid character(36) NOT NULL,
    friend_name character varying(20) DEFAULT ''::character varying NOT NULL,
    relation bigint NOT NULL
);


ALTER TABLE public.friend_list OWNER TO sa;

--
-- Name: game_attributes; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_attributes (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    profession_uuid character(36) NOT NULL,
    name character varying(255) NOT NULL,
    is_primary bigint DEFAULT 0::bigint NOT NULL
);


ALTER TABLE public.game_attributes OWNER TO sa;

--
-- Name: game_effects; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_effects (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    name character varying(63) DEFAULT ''::character varying NOT NULL,
    category bigint DEFAULT 0::bigint NOT NULL,
    script character varying(260) NOT NULL,
    icon character varying(260) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.game_effects OWNER TO sa;

--
-- Name: game_items; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_items (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    name character varying(64) DEFAULT ''::character varying NOT NULL,
    schript_file character varying(260) DEFAULT ''::character varying NOT NULL,
    server_icon_file character varying(260) DEFAULT ''::character varying NOT NULL,
    server_model_file character varying(260) DEFAULT ''::character varying NOT NULL,
    client_model_file character varying(260) DEFAULT ''::character varying NOT NULL,
    client_icon_file character varying(260) DEFAULT ''::character varying NOT NULL,
    type bigint DEFAULT 0::bigint NOT NULL
);


ALTER TABLE public.game_items OWNER TO sa;

--
-- Name: game_maps; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_maps (
    uuid character(36) NOT NULL,
    name character varying(32) NOT NULL,
    type bigint NOT NULL,
    directory character varying(255) NOT NULL,
    script_file character varying(255) NOT NULL,
    landing bigint DEFAULT 0::bigint,
    party_size bigint DEFAULT 0::bigint NOT NULL,
    map_coord_x bigint DEFAULT 0::bigint NOT NULL,
    map_coord_y bigint DEFAULT 0::bigint NOT NULL
);


ALTER TABLE public.game_maps OWNER TO sa;

--
-- Name: game_professions; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_professions (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    name character varying(32) NOT NULL,
    abbr character varying(2) NOT NULL,
    model_index_female bigint DEFAULT 0 NOT NULL,
    model_index_male bigint DEFAULT 0 NOT NULL
);


ALTER TABLE public.game_professions OWNER TO sa;

--
-- Name: COLUMN game_professions.model_index_female; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_professions.model_index_female IS 'Index in game_items table';


--
-- Name: COLUMN game_professions.model_index_male; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_professions.model_index_male IS 'Index in game_items table';


--
-- Name: game_skills; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_skills (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    name character varying(64) NOT NULL,
    attribute_uuid character(36) NOT NULL,
    type bigint NOT NULL,
    is_elite bigint DEFAULT 0::bigint NOT NULL,
    description character varying(255) DEFAULT ''::character varying NOT NULL,
    short_description character varying(255) DEFAULT ''::character varying NOT NULL,
    icon character varying(260) DEFAULT ''::character varying NOT NULL,
    script character varying(260) DEFAULT ''::character varying NOT NULL,
    is_locked bigint DEFAULT 0::bigint NOT NULL
);


ALTER TABLE public.game_skills OWNER TO sa;

--
-- Name: guild_members; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.guild_members (
    account_uuid character(36) DEFAULT NULL::bpchar,
    guild_uuid character(36) DEFAULT NULL::bpchar,
    role bigint DEFAULT 1::bigint NOT NULL,
    invite_name character varying(20) DEFAULT ''::character varying NOT NULL,
    invited bigint DEFAULT 0::bigint NOT NULL,
    joined bigint DEFAULT 0::bigint NOT NULL,
    expires bigint DEFAULT 0::bigint NOT NULL
);


ALTER TABLE public.guild_members OWNER TO sa;

--
-- Name: guilds; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.guilds (
    uuid character(36) NOT NULL,
    name character varying(32) NOT NULL,
    tag character varying(4) NOT NULL,
    creator_account_uuid character(36) NOT NULL,
    creation bigint NOT NULL
);


ALTER TABLE public.guilds OWNER TO sa;

--
-- Name: ip_bans; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.ip_bans (
    uuid character(36) NOT NULL,
    ban_uuid character(36) NOT NULL,
    ip bigint NOT NULL,
    mask bigint NOT NULL
);


ALTER TABLE public.ip_bans OWNER TO sa;

--
-- Name: mails; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.mails (
    uuid character(36) NOT NULL,
    from_account_uuid character(36) NOT NULL,
    to_account_uuid character(36) NOT NULL,
    from_name character varying(20) DEFAULT ''::bpchar NOT NULL,
    to_name character varying(20) DEFAULT ''::bpchar NOT NULL,
    subject character varying(60) DEFAULT ''::character varying NOT NULL,
    message character varying(255) DEFAULT ''::character varying NOT NULL,
    created bigint DEFAULT 0::bigint NOT NULL,
    is_read bigint DEFAULT 0::bigint NOT NULL
);


ALTER TABLE public.mails OWNER TO sa;

--
-- Name: players; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.players (
    uuid character(36) DEFAULT ''::bpchar NOT NULL,
    profession character varying(2) NOT NULL,
    profession2 character varying(2) DEFAULT ''::bpchar NOT NULL,
    profession_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    profession2_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    name character varying(20) NOT NULL,
    pvp bigint DEFAULT 0::bigint NOT NULL,
    account_uuid character(36) NOT NULL,
    level bigint DEFAULT 1::bigint NOT NULL,
    experience bigint DEFAULT 100::bigint NOT NULL,
    skillpoints bigint DEFAULT 0::bigint NOT NULL,
    sex bigint DEFAULT 0::bigint NOT NULL,
    lastlogin bigint DEFAULT 0::bigint NOT NULL,
    lastlogout bigint DEFAULT 0::bigint NOT NULL,
    onlinetime bigint DEFAULT 0::bigint NOT NULL,
    deleted bigint DEFAULT 0::bigint NOT NULL,
    creation bigint DEFAULT 0::bigint NOT NULL,
    current_map_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    model_index bigint DEFAULT 0::bigint NOT NULL,
    model_data bytea,
    effects bytea,
    skills character varying(36) DEFAULT ''::character varying NOT NULL,
    equipment bytea,
    titles bytea
);


ALTER TABLE public.players OWNER TO sa;

--
-- Name: players_online_view; Type: VIEW; Schema: public; Owner: sa
--

CREATE VIEW public.players_online_view AS
 SELECT players.uuid,
    players.profession,
    players.profession2,
    players.profession_uuid,
    players.profession2_uuid,
    players.name,
    players.pvp,
    players.account_uuid,
    players.level,
    players.experience,
    players.skillpoints,
    players.sex,
    players.lastlogin,
    players.lastlogout,
    players.onlinetime,
    players.deleted,
    players.creation,
    players.current_map_uuid,
    players.effects,
    players.skills,
    players.equipment,
    players.titles,
    accounts.online_status,
    accounts.name AS account_name,
    game_maps.name AS game_name
   FROM ((public.players
     LEFT JOIN public.accounts ON ((players.account_uuid = accounts.uuid)))
     LEFT JOIN public.game_maps ON ((players.current_map_uuid = game_maps.uuid)))
  WHERE ((accounts.online_status = 3) AND (accounts.current_character_uuid = players.uuid));


ALTER TABLE public.players_online_view OWNER TO sa;

--
-- Name: professions_view; Type: VIEW; Schema: public; Owner: sa
--

CREATE VIEW public.professions_view AS
 SELECT game_attributes.uuid,
    game_attributes.idx,
    game_attributes.profession_uuid,
    game_attributes.name,
    game_attributes.is_primary,
    game_professions.name AS profession_name,
    game_professions.abbr AS profession_abbr
   FROM (public.game_attributes
     LEFT JOIN public.game_professions ON ((game_attributes.profession_uuid = game_professions.uuid)));


ALTER TABLE public.professions_view OWNER TO sa;

--
-- Name: reserved_names; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.reserved_names (
    uuid character(36) NOT NULL,
    name character varying(40) NOT NULL,
    is_reserved smallint DEFAULT (0)::smallint NOT NULL,
    reserved_for_account_uuid character(36) DEFAULT ''::bpchar,
    expires bigint DEFAULT 0 NOT NULL
);


ALTER TABLE public.reserved_names OWNER TO sa;

--
-- Name: TABLE reserved_names; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON TABLE public.reserved_names IS 'Names that can not be used to create characters';


--
-- Name: COLUMN reserved_names.reserved_for_account_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.reserved_names.reserved_for_account_uuid IS 'When a user deletes a character the name will be reserved for some time';


--
-- Name: COLUMN reserved_names.expires; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.reserved_names.expires IS 'Timestamp when this reservation expires';


--
-- Name: services; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.services (
    uuid character(36) NOT NULL,
    name character varying(64) DEFAULT ''::character varying NOT NULL,
    type bigint DEFAULT 0::bigint NOT NULL,
    location character varying(10) DEFAULT ''::bpchar NOT NULL,
    host character varying(64) DEFAULT ''::character varying NOT NULL,
    port bigint DEFAULT 0::bigint NOT NULL,
    status bigint DEFAULT 0::bigint NOT NULL,
    start_time bigint DEFAULT 0::bigint NOT NULL,
    stop_time bigint DEFAULT 0::bigint NOT NULL,
    run_time bigint DEFAULT 0::bigint NOT NULL,
    file character varying(260) DEFAULT ''::character varying NOT NULL,
    path character varying(260) DEFAULT ''::character varying NOT NULL,
    arguments character varying(260) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.services OWNER TO sa;

--
-- Name: storage; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.storage (
    uuid character(36) NOT NULL,
    owner_uuid character(36) NOT NULL,
    item_uuid character(36) NOT NULL,
    location bigint NOT NULL,
    count bigint DEFAULT 1::bigint NOT NULL,
    "position" bigint NOT NULL
);


ALTER TABLE public.storage OWNER TO sa;

--
-- Name: versions; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.versions (
    uuid character(36) NOT NULL,
    name character varying(20) NOT NULL,
    value bigint NOT NULL,
    internal bigint DEFAULT 0::bigint NOT NULL
);


ALTER TABLE public.versions OWNER TO sa;

--
-- Name: account_bans_ban_uuid_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.account_bans
    ADD CONSTRAINT account_bans_ban_uuid_key UNIQUE (ban_uuid);


--
-- Name: account_bans_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.account_bans
    ADD CONSTRAINT account_bans_pkey PRIMARY KEY (uuid);


--
-- Name: account_keys_uuid; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.account_keys
    ADD CONSTRAINT account_keys_uuid PRIMARY KEY (uuid);


--
-- Name: accounts_name_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.accounts
    ADD CONSTRAINT accounts_name_key UNIQUE (name);


--
-- Name: accounts_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.accounts
    ADD CONSTRAINT accounts_pkey PRIMARY KEY (uuid);


--
-- Name: bans_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.bans
    ADD CONSTRAINT bans_pkey PRIMARY KEY (uuid);


--
-- Name: friend_list_account_uuid_friend_uuid_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.friend_list
    ADD CONSTRAINT friend_list_account_uuid_friend_uuid_key UNIQUE (account_uuid, friend_uuid);


--
-- Name: game_attributes_idx_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_attributes
    ADD CONSTRAINT game_attributes_idx_key UNIQUE (idx);


--
-- Name: game_attributes_uuid_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_attributes
    ADD CONSTRAINT game_attributes_uuid_key UNIQUE (uuid);


--
-- Name: game_effects_idx_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_effects
    ADD CONSTRAINT game_effects_idx_key UNIQUE (idx);


--
-- Name: game_effects_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_effects
    ADD CONSTRAINT game_effects_pkey PRIMARY KEY (uuid);


--
-- Name: game_items_idx_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_items
    ADD CONSTRAINT game_items_idx_key UNIQUE (idx);


--
-- Name: game_items_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_items
    ADD CONSTRAINT game_items_pkey PRIMARY KEY (uuid);


--
-- Name: game_maps_name_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_maps
    ADD CONSTRAINT game_maps_name_key UNIQUE (name);


--
-- Name: game_maps_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_maps
    ADD CONSTRAINT game_maps_pkey PRIMARY KEY (uuid);


--
-- Name: game_professions_abbr_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_professions
    ADD CONSTRAINT game_professions_abbr_key UNIQUE (abbr);


--
-- Name: game_professions_idx_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_professions
    ADD CONSTRAINT game_professions_idx_key UNIQUE (idx);


--
-- Name: game_professions_name_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_professions
    ADD CONSTRAINT game_professions_name_key UNIQUE (name);


--
-- Name: game_professions_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_professions
    ADD CONSTRAINT game_professions_pkey PRIMARY KEY (uuid);


--
-- Name: game_skills_idx_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_skills
    ADD CONSTRAINT game_skills_idx_key UNIQUE (idx);


--
-- Name: game_skills_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_skills
    ADD CONSTRAINT game_skills_pkey PRIMARY KEY (uuid);


--
-- Name: guild_members_account_uuid_guild_uuid_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.guild_members
    ADD CONSTRAINT guild_members_account_uuid_guild_uuid_key UNIQUE (account_uuid, guild_uuid);


--
-- Name: guilds_name_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.guilds
    ADD CONSTRAINT guilds_name_key UNIQUE (name);


--
-- Name: guilds_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.guilds
    ADD CONSTRAINT guilds_pkey PRIMARY KEY (uuid);


--
-- Name: ip_bans_ip_mask_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.ip_bans
    ADD CONSTRAINT ip_bans_ip_mask_key UNIQUE (ip, mask);


--
-- Name: ip_bans_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.ip_bans
    ADD CONSTRAINT ip_bans_pkey PRIMARY KEY (uuid);


--
-- Name: mails_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.mails
    ADD CONSTRAINT mails_pkey PRIMARY KEY (uuid);


--
-- Name: players_name_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.players
    ADD CONSTRAINT players_name_key UNIQUE (name);


--
-- Name: players_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.players
    ADD CONSTRAINT players_pkey PRIMARY KEY (uuid);


--
-- Name: reserved_names_name_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.reserved_names
    ADD CONSTRAINT reserved_names_name_key UNIQUE (name);


--
-- Name: reserved_names_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.reserved_names
    ADD CONSTRAINT reserved_names_pkey PRIMARY KEY (uuid);


--
-- Name: services_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.services
    ADD CONSTRAINT services_pkey PRIMARY KEY (uuid);


--
-- Name: storage_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.storage
    ADD CONSTRAINT storage_pkey PRIMARY KEY (uuid);


--
-- Name: versions_name_key; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.versions
    ADD CONSTRAINT versions_name_key UNIQUE (name);


--
-- Name: versions_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.versions
    ADD CONSTRAINT versions_pkey PRIMARY KEY (uuid);


--
-- Name: account_ban_account_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX account_ban_account_uuid ON public.account_bans USING btree (account_uuid);


--
-- Name: account_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE UNIQUE INDEX account_uuid ON public.account_account_keys USING btree (account_uuid, account_key_uuid);


--
-- Name: accunt_key_status; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX accunt_key_status ON public.account_keys USING btree (status);


--
-- Name: bans_admin_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX bans_admin_uuid ON public.bans USING btree (admin_uuid);


--
-- Name: game_attributes_profession_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX game_attributes_profession_uuid ON public.game_attributes USING btree (profession_uuid);


--
-- Name: game_items_type; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX game_items_type ON public.game_items USING btree (type);


--
-- Name: game_maps_type; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX game_maps_type ON public.game_maps USING btree (type);


--
-- Name: guild_members_account_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX guild_members_account_uuid ON public.guild_members USING btree (account_uuid);


--
-- Name: guild_members_expires; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX guild_members_expires ON public.guild_members USING btree (expires);


--
-- Name: guild_members_guild_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX guild_members_guild_uuid ON public.guild_members USING btree (guild_uuid);


--
-- Name: ip_bans_ban_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX ip_bans_ban_uuid ON public.ip_bans USING btree (ban_uuid);


--
-- Name: ip_bans_ip; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX ip_bans_ip ON public.ip_bans USING btree (ip);


--
-- Name: mails_created; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX mails_created ON public.mails USING btree (created);


--
-- Name: mails_from_account_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX mails_from_account_uuid ON public.mails USING btree (from_account_uuid);


--
-- Name: mails_is_read; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX mails_is_read ON public.mails USING btree (is_read);


--
-- Name: mails_to_account_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX mails_to_account_uuid ON public.mails USING btree (to_account_uuid);


--
-- Name: players_account_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX players_account_uuid ON public.players USING btree (account_uuid);


--
-- Name: reserved_names_is_reserved; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX reserved_names_is_reserved ON public.reserved_names USING btree (is_reserved);


--
-- Name: reserved_names_until; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX reserved_names_until ON public.reserved_names USING btree (expires);


--
-- Name: services_type; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX services_type ON public.services USING btree (type);


--
-- Name: storage_owner_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX storage_owner_uuid ON public.storage USING btree (owner_uuid);


--
-- Name: SCHEMA public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

