--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


--
-- Name: logout_all(); Type: FUNCTION; Schema: public; Owner: sa
--

CREATE FUNCTION public.logout_all() RETURNS void
    LANGUAGE sql
    AS $$update accounts set online_status = 0$$;


ALTER FUNCTION public.logout_all() OWNER TO sa;

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
    description text DEFAULT ''::text,
    status smallint DEFAULT 0::smallint NOT NULL,
    key_type bigint DEFAULT 0::bigint NOT NULL,
    email text DEFAULT ''::text NOT NULL
);


ALTER TABLE public.account_keys OWNER TO sa;

--
-- Name: accounts; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.accounts (
    uuid character(36) DEFAULT ''::bpchar NOT NULL,
    name text NOT NULL,
    password text NOT NULL,
    email text DEFAULT ''::character varying NOT NULL,
    type bigint DEFAULT 1::bigint NOT NULL,
    status bigint DEFAULT 0::bigint NOT NULL,
    creation bigint DEFAULT 0::bigint NOT NULL,
    char_slots bigint DEFAULT 6::bigint NOT NULL,
    current_character_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    current_server_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    online_status bigint DEFAULT 0::bigint NOT NULL,
    titles text,
    guild_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    chest_size integer DEFAULT 0 NOT NULL,
    auth_token character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    auth_token_expiry bigint DEFAULT 0 NOT NULL
);


ALTER TABLE public.accounts OWNER TO sa;

--
-- Name: TABLE accounts; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON TABLE public.accounts IS 'Player accounts';


--
-- Name: COLUMN accounts.name; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.accounts.name IS 'User/Login name';


--
-- Name: COLUMN accounts.email; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.accounts.email IS 'Email (optional)';


--
-- Name: COLUMN accounts.type; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.accounts.type IS 'Account type';


--
-- Name: COLUMN accounts.creation; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.accounts.creation IS 'Date this account was created';


--
-- Name: COLUMN accounts.char_slots; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.accounts.char_slots IS 'Number of character slots';


--
-- Name: COLUMN accounts.titles; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.accounts.titles IS 'Base64 encoded';


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
    comment text DEFAULT ''::text NOT NULL
);


ALTER TABLE public.bans OWNER TO sa;

--
-- Name: concrete_items; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.concrete_items (
    uuid character(36) NOT NULL,
    player_uuid character(36),
    storage_place integer DEFAULT 0 NOT NULL,
    storage_pos integer DEFAULT 0 NOT NULL,
    upgrade_1 character(36) NOT NULL,
    upgrade_2 character(36) NOT NULL,
    upgrade_3 character(36) NOT NULL,
    account_uuid character(36),
    item_uuid character(36) NOT NULL,
    stats text,
    count integer DEFAULT 1 NOT NULL,
    creation bigint DEFAULT 0 NOT NULL,
    value integer DEFAULT 0 NOT NULL,
    instance_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    map_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL
);


ALTER TABLE public.concrete_items OWNER TO sa;

--
-- Name: TABLE concrete_items; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON TABLE public.concrete_items IS 'Concrete items';


--
-- Name: COLUMN concrete_items.player_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.player_uuid IS 'Player this belongs to';


--
-- Name: COLUMN concrete_items.storage_place; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.storage_place IS 'Inventory, Chest or equipped';


--
-- Name: COLUMN concrete_items.storage_pos; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.storage_pos IS 'Sorage position in inventory or chest';


--
-- Name: COLUMN concrete_items.upgrade_1; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.upgrade_1 IS 'Upgrade item concrete UUID';


--
-- Name: COLUMN concrete_items.upgrade_2; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.upgrade_2 IS 'Upgrade item concrete UUID';


--
-- Name: COLUMN concrete_items.upgrade_3; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.upgrade_3 IS 'Upgrade item concrete UUID';


--
-- Name: COLUMN concrete_items.account_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.account_uuid IS 'Account this belongs to';


--
-- Name: COLUMN concrete_items.item_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.item_uuid IS 'UUID in game_items table';


--
-- Name: COLUMN concrete_items.stats; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.stats IS 'Stats of the item';


--
-- Name: COLUMN concrete_items.count; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.count IS 'How many of this item';


--
-- Name: COLUMN concrete_items.creation; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.creation IS 'Creation time';


--
-- Name: COLUMN concrete_items.value; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.value IS 'Value when sold';


--
-- Name: COLUMN concrete_items.instance_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.instance_uuid IS 'The game instance where this item dropped';


--
-- Name: COLUMN concrete_items.map_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.concrete_items.map_uuid IS 'The map where this item dropped';


--
-- Name: friend_list; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.friend_list (
    account_uuid character(36) NOT NULL,
    friend_uuid character(36) NOT NULL,
    friend_name text DEFAULT ''::text NOT NULL,
    relation bigint NOT NULL,
    creation bigint DEFAULT 0 NOT NULL
);


ALTER TABLE public.friend_list OWNER TO sa;

--
-- Name: COLUMN friend_list.account_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.friend_list.account_uuid IS 'The account this friendlist belongs to';


--
-- Name: COLUMN friend_list.friend_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.friend_list.friend_uuid IS 'Friend Account UUID that was friended';


--
-- Name: COLUMN friend_list.friend_name; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.friend_list.friend_name IS 'Name of character that was friended. Should be changeable by the user.';


--
-- Name: COLUMN friend_list.relation; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.friend_list.relation IS '0 = unknown, 1 = friend, 2 = ignore';


--
-- Name: COLUMN friend_list.creation; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.friend_list.creation IS 'Time stamp';


--
-- Name: game_attributes; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_attributes (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    profession_uuid character(36) NOT NULL,
    name text NOT NULL,
    is_primary bigint DEFAULT 0::bigint NOT NULL
);


ALTER TABLE public.game_attributes OWNER TO sa;

--
-- Name: game_effects; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_effects (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    name text DEFAULT ''::text NOT NULL,
    category bigint DEFAULT 0::bigint NOT NULL,
    script character varying(260) DEFAULT ''::character varying NOT NULL,
    icon character varying(260) DEFAULT ''::character varying NOT NULL,
    sound_effect character varying(260) DEFAULT ''::character varying NOT NULL,
    particle_effect character varying(260) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.game_effects OWNER TO sa;

--
-- Name: game_item_chances; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_item_chances (
    uuid character(36) NOT NULL,
    map_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    item_uuid character(36) NOT NULL,
    chance integer DEFAULT 0 NOT NULL
);


ALTER TABLE public.game_item_chances OWNER TO sa;

--
-- Name: COLUMN game_item_chances.map_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_item_chances.map_uuid IS 'Map this item can drop. Empty GUID = everywhere';


--
-- Name: COLUMN game_item_chances.chance; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_item_chances.chance IS 'Chance (promille) to find this item on the map. (0..1000)';


--
-- Name: game_items; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_items (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    name text DEFAULT ''::text NOT NULL,
    schript_file character varying(260) DEFAULT ''::character varying NOT NULL,
    object_file character varying(260) DEFAULT ''::character varying NOT NULL,
    icon_file character varying(260) DEFAULT ''::character varying NOT NULL,
    type bigint DEFAULT 0::bigint NOT NULL,
    stack_able integer DEFAULT 0 NOT NULL,
    belongs_to bigint DEFAULT 0 NOT NULL,
    value integer DEFAULT 0 NOT NULL,
    model_class integer DEFAULT 0 NOT NULL,
    spawn_item_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    actor_script character varying(260) DEFAULT ''::character varying
);


ALTER TABLE public.game_items OWNER TO sa;

--
-- Name: COLUMN game_items.object_file; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_items.object_file IS 'Prefab file somewhere in /Objects';


--
-- Name: COLUMN game_items.stack_able; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_items.stack_able IS 'Is the item stack able';


--
-- Name: COLUMN game_items.belongs_to; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_items.belongs_to IS 'This can be used as upgrade for types';


--
-- Name: COLUMN game_items.value; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_items.value IS 'Some items have a fixed value';


--
-- Name: COLUMN game_items.spawn_item_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_items.spawn_item_uuid IS 'This item can spawn another item, e.g. a projectile';


--
-- Name: COLUMN game_items.actor_script; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_items.actor_script IS 'If this Item is an Actor (e.g. Projectile) this is the actor script';


--
-- Name: game_maps; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_maps (
    uuid character(36) NOT NULL,
    name text NOT NULL,
    type bigint NOT NULL,
    directory character varying(260) NOT NULL,
    script_file character varying(260) NOT NULL,
    landing integer DEFAULT 0 NOT NULL,
    party_size integer DEFAULT 0 NOT NULL,
    map_coord_x integer DEFAULT 0 NOT NULL,
    map_coord_y integer DEFAULT 0 NOT NULL,
    default_level integer DEFAULT 1 NOT NULL,
    party_count integer DEFAULT 0 NOT NULL,
    random_party integer DEFAULT 0 NOT NULL,
    queue_map_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    game_mode integer DEFAULT 0 NOT NULL
);


ALTER TABLE public.game_maps OWNER TO sa;

--
-- Name: COLUMN game_maps.default_level; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_maps.default_level IS 'The default level of players and NPCs on this map';


--
-- Name: COLUMN game_maps.party_count; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_maps.party_count IS 'Number of opposing parties in this game';


--
-- Name: COLUMN game_maps.random_party; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_maps.random_party IS 'Party with random players';


--
-- Name: COLUMN game_maps.queue_map_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_maps.queue_map_uuid IS 'From this map you can queue to another map';


--
-- Name: game_music; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_music (
    uuid character(36) NOT NULL,
    map_uuid text NOT NULL,
    local_file character varying(260) NOT NULL,
    remote_file character varying(260) DEFAULT ''::character varying NOT NULL,
    sorting integer DEFAULT 0 NOT NULL,
    style integer DEFAULT 0 NOT NULL
);


ALTER TABLE public.game_music OWNER TO sa;

--
-- Name: COLUMN game_music.map_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_music.map_uuid IS 'Map UUIDs separated with a semicolon';


--
-- Name: game_professions; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_professions (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    name text NOT NULL,
    abbr character varying(2) NOT NULL,
    model_index_female bigint DEFAULT 0 NOT NULL,
    model_index_male bigint DEFAULT 0 NOT NULL,
    "position" integer DEFAULT 0 NOT NULL
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
-- Name: COLUMN game_professions."position"; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.game_professions."position" IS 'Front-, mid-, backline';


--
-- Name: game_quests; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_quests (
    uuid character(36) NOT NULL,
    idx integer NOT NULL,
    name text NOT NULL,
    script character varying(260) NOT NULL
);


ALTER TABLE public.game_quests OWNER TO sa;

--
-- Name: game_skills; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.game_skills (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    name text NOT NULL,
    attribute_uuid character(36) NOT NULL,
    type bigint NOT NULL,
    is_elite bigint DEFAULT 0::bigint NOT NULL,
    description text DEFAULT ''::text NOT NULL,
    short_description text DEFAULT ''::text NOT NULL,
    icon character varying(260) DEFAULT ''::character varying NOT NULL,
    script character varying(260) DEFAULT ''::character varying NOT NULL,
    is_locked bigint DEFAULT 0::bigint NOT NULL,
    profession_uuid character(36) DEFAULT '79b75ff4-92f0-11e8-a7ca-02100700d6f0'::bpchar NOT NULL,
    sound_effect character varying(260) DEFAULT ''::character varying NOT NULL,
    particle_effect character varying(260) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.game_skills OWNER TO sa;

--
-- Name: guild_members; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.guild_members (
    account_uuid character(36) DEFAULT NULL::bpchar,
    guild_uuid character(36) DEFAULT NULL::bpchar,
    role bigint DEFAULT 1::bigint NOT NULL,
    invite_name text DEFAULT ''::text NOT NULL,
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
    name text NOT NULL,
    tag character varying(4) NOT NULL,
    creator_account_uuid character(36) NOT NULL,
    creation bigint NOT NULL,
    guild_hall_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    creator_name text DEFAULT ''::bpchar NOT NULL,
    creator_player_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    guild_hall_instance_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    guild_hall_server_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL
);


ALTER TABLE public.guilds OWNER TO sa;

--
-- Name: COLUMN guilds.guild_hall_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.guilds.guild_hall_uuid IS 'Guild Hall Map UUID';


--
-- Name: COLUMN guilds.guild_hall_instance_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.guilds.guild_hall_instance_uuid IS 'If the guild hall instance is running, this is the UUID';


--
-- Name: COLUMN guilds.guild_hall_server_uuid; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON COLUMN public.guilds.guild_hall_server_uuid IS 'If the instance is running, this is the server where it runs on';


--
-- Name: instances; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.instances (
    uuid character(36) NOT NULL,
    game_uuid character(36) NOT NULL,
    server_uuid character(36) NOT NULL,
    name text NOT NULL,
    recording text DEFAULT ''::text NOT NULL,
    start_time bigint DEFAULT (0)::bigint NOT NULL,
    number integer DEFAULT 0 NOT NULL,
    is_running integer DEFAULT 0 NOT NULL,
    stop_time bigint DEFAULT (0)::bigint NOT NULL
);


ALTER TABLE public.instances OWNER TO sa;

--
-- Name: TABLE instances; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON TABLE public.instances IS 'Game instances';


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
    from_name text DEFAULT ''::text NOT NULL,
    to_name text DEFAULT ''::text NOT NULL,
    subject text DEFAULT ''::text NOT NULL,
    message text DEFAULT ''::text NOT NULL,
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
    name text NOT NULL,
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
    skills text DEFAULT ''::text NOT NULL,
    equipment text,
    titles text,
    last_outpost_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    inventory_size integer DEFAULT 0 NOT NULL
);


ALTER TABLE public.players OWNER TO sa;

--
-- Name: TABLE players; Type: COMMENT; Schema: public; Owner: sa
--

COMMENT ON TABLE public.players IS 'Characters';


--
-- Name: reserved_names; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.reserved_names (
    uuid character(36) NOT NULL,
    name text NOT NULL,
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
    name text DEFAULT ''::text NOT NULL,
    type bigint DEFAULT 0::bigint NOT NULL,
    location text DEFAULT ''::text NOT NULL,
    host text DEFAULT ''::character varying NOT NULL,
    port bigint DEFAULT 0::bigint NOT NULL,
    status bigint DEFAULT 0::bigint NOT NULL,
    start_time bigint DEFAULT 0::bigint NOT NULL,
    stop_time bigint DEFAULT 0::bigint NOT NULL,
    run_time bigint DEFAULT 0::bigint NOT NULL,
    file character varying(260) DEFAULT ''::character varying NOT NULL,
    path character varying(260) DEFAULT ''::character varying NOT NULL,
    arguments character varying(260) DEFAULT ''::character varying NOT NULL,
    machine character varying(260) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.services OWNER TO sa;

--
-- Name: versions; Type: TABLE; Schema: public; Owner: sa; Tablespace: 
--

CREATE TABLE public.versions (
    uuid character(36) NOT NULL,
    name text NOT NULL,
    value bigint NOT NULL,
    internal smallint DEFAULT (0)::smallint NOT NULL
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
-- Name: game_item_chances_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_item_chances
    ADD CONSTRAINT game_item_chances_pkey PRIMARY KEY (uuid);


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
-- Name: game_music_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_music
    ADD CONSTRAINT game_music_pkey PRIMARY KEY (uuid);


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
-- Name: game_quests_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.game_quests
    ADD CONSTRAINT game_quests_pkey PRIMARY KEY (uuid);


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
-- Name: instances_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.instances
    ADD CONSTRAINT instances_pkey PRIMARY KEY (uuid);


--
-- Name: inventory_items_pkey; Type: CONSTRAINT; Schema: public; Owner: sa; Tablespace: 
--

ALTER TABLE ONLY public.concrete_items
    ADD CONSTRAINT inventory_items_pkey PRIMARY KEY (uuid);


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
-- Name: concrete_items_storage_place; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX concrete_items_storage_place ON public.concrete_items USING btree (storage_place);


--
-- Name: friend_list_account_uuid_key; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX friend_list_account_uuid_key ON public.friend_list USING btree (account_uuid);


--
-- Name: friend_list_friend_uuid_key; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX friend_list_friend_uuid_key ON public.friend_list USING btree (friend_uuid);


--
-- Name: game_attributes_profession_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX game_attributes_profession_uuid ON public.game_attributes USING btree (profession_uuid);


--
-- Name: game_item_chances_may_uuid_key; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX game_item_chances_may_uuid_key ON public.game_item_chances USING btree (map_uuid);


--
-- Name: game_items_type; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX game_items_type ON public.game_items USING btree (type);


--
-- Name: game_maps_type; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX game_maps_type ON public.game_maps USING btree (type);


--
-- Name: game_music_sorting; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX game_music_sorting ON public.game_music USING btree (sorting);


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
-- Name: guilds_name_ci_index; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE UNIQUE INDEX guilds_name_ci_index ON public.guilds USING btree (lower(name));


--
-- Name: instances_recording_index; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX instances_recording_index ON public.instances USING btree (recording);


--
-- Name: inventory_items_account_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX inventory_items_account_uuid ON public.concrete_items USING btree (account_uuid);


--
-- Name: inventory_items_player_uuid; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX inventory_items_player_uuid ON public.concrete_items USING btree (player_uuid);


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
-- Name: players_name_ci_index; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE UNIQUE INDEX players_name_ci_index ON public.players USING btree (lower(name));


--
-- Name: reserved_names_is_reserved; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX reserved_names_is_reserved ON public.reserved_names USING btree (is_reserved);


--
-- Name: reserved_names_name_ci_index; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX reserved_names_name_ci_index ON public.reserved_names USING btree (lower(name));


--
-- Name: reserved_names_until; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX reserved_names_until ON public.reserved_names USING btree (expires);


--
-- Name: services_type; Type: INDEX; Schema: public; Owner: sa; Tablespace: 
--

CREATE INDEX services_type ON public.services USING btree (type);


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

