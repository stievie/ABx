SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET xmloption = content;

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;
COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';

CREATE OR REPLACE FUNCTION public.logout_all() RETURNS void
    LANGUAGE sql
    AS $$update accounts set online_status = 0$$;

CREATE OR REPLACE FUNCTION public.random_guid() RETURNS character
    LANGUAGE sql
    AS $$SELECT uuid_in(md5(random()::text || clock_timestamp()::text)::cstring)::text$$;

SET default_tablespace = '';
SET default_with_oids = false;

CREATE TABLE public.account_account_keys (
    account_uuid character(36) NOT NULL,
    account_key_uuid character(36) NOT NULL
);

CREATE TABLE public.account_bans (
    uuid character(36) NOT NULL,
    ban_uuid character(36) NOT NULL,
    account_uuid character(36) NOT NULL
);

CREATE TABLE public.account_keys (
    uuid character(36) NOT NULL,
    used bigint DEFAULT 0::bigint NOT NULL,
    total bigint DEFAULT 1::bigint NOT NULL,
    description text DEFAULT ''::text,
    status smallint DEFAULT 0::smallint NOT NULL,
    key_type bigint DEFAULT 0::bigint NOT NULL,
    email text DEFAULT ''::text NOT NULL
);

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
COMMENT ON TABLE public.accounts IS 'Player accounts';
COMMENT ON COLUMN public.accounts.name IS 'User/Login name';
COMMENT ON COLUMN public.accounts.email IS 'Email (optional)';
COMMENT ON COLUMN public.accounts.type IS 'Account type';
COMMENT ON COLUMN public.accounts.creation IS 'Date this account was created';
COMMENT ON COLUMN public.accounts.char_slots IS 'Number of character slots';
COMMENT ON COLUMN public.accounts.titles IS 'Base64 encoded';

CREATE TABLE public.bans (
    uuid character(36) NOT NULL,
    expires bigint NOT NULL,
    added bigint NOT NULL,
    reason bigint DEFAULT 0::bigint NOT NULL,
    active bigint DEFAULT 0::bigint NOT NULL,
    admin_uuid character(36) NOT NULL,
    comment text DEFAULT ''::text NOT NULL
);

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
COMMENT ON TABLE public.concrete_items IS 'Concrete items';
COMMENT ON COLUMN public.concrete_items.player_uuid IS 'Player this belongs to';
COMMENT ON COLUMN public.concrete_items.storage_place IS 'Inventory, Chest or equipped';
COMMENT ON COLUMN public.concrete_items.storage_pos IS 'Sorage position in inventory or chest';
COMMENT ON COLUMN public.concrete_items.upgrade_1 IS 'Upgrade item concrete UUID';
COMMENT ON COLUMN public.concrete_items.upgrade_2 IS 'Upgrade item concrete UUID';
COMMENT ON COLUMN public.concrete_items.upgrade_3 IS 'Upgrade item concrete UUID';
COMMENT ON COLUMN public.concrete_items.account_uuid IS 'Account this belongs to';
COMMENT ON COLUMN public.concrete_items.item_uuid IS 'UUID in game_items table';
COMMENT ON COLUMN public.concrete_items.stats IS 'Stats of the item';
COMMENT ON COLUMN public.concrete_items.count IS 'How many of this item';
COMMENT ON COLUMN public.concrete_items.creation IS 'Creation time';
COMMENT ON COLUMN public.concrete_items.value IS 'Value when sold';
COMMENT ON COLUMN public.concrete_items.instance_uuid IS 'The game instance where this item dropped';
COMMENT ON COLUMN public.concrete_items.map_uuid IS 'The map where this item dropped';

CREATE TABLE public.friend_list (
    account_uuid character(36) NOT NULL,
    friend_uuid character(36) NOT NULL,
    friend_name text DEFAULT ''::text NOT NULL,
    relation bigint NOT NULL,
    creation bigint DEFAULT 0 NOT NULL
);
COMMENT ON COLUMN public.friend_list.account_uuid IS 'The account this friendlist belongs to';
COMMENT ON COLUMN public.friend_list.friend_uuid IS 'Friend Account UUID that was friended';
COMMENT ON COLUMN public.friend_list.friend_name IS 'Name of character that was friended. Should be changeable by the user.';
COMMENT ON COLUMN public.friend_list.relation IS '0 = unknown, 1 = friend, 2 = ignore';
COMMENT ON COLUMN public.friend_list.creation IS 'Time stamp';

CREATE TABLE public.game_attributes (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    profession_uuid character(36) NOT NULL,
    name text NOT NULL,
    is_primary bigint DEFAULT 0::bigint NOT NULL
);

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

CREATE TABLE public.game_item_chances (
    uuid character(36) NOT NULL,
    map_uuid character(36) DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar NOT NULL,
    item_uuid character(36) NOT NULL,
    chance integer DEFAULT 0 NOT NULL
);
COMMENT ON COLUMN public.game_item_chances.map_uuid IS 'Map this item can drop. Empty GUID = everywhere';
COMMENT ON COLUMN public.game_item_chances.chance IS 'Chance (promille) to find this item on the map. (0..1000)';

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
COMMENT ON COLUMN public.game_items.object_file IS 'Prefab file somewhere in /Objects';
COMMENT ON COLUMN public.game_items.stack_able IS 'Is the item stack able';
COMMENT ON COLUMN public.game_items.belongs_to IS 'This can be used as upgrade for types';
COMMENT ON COLUMN public.game_items.value IS 'Some items have a fixed value';
COMMENT ON COLUMN public.game_items.spawn_item_uuid IS 'This item can spawn another item, e.g. a projectile';
COMMENT ON COLUMN public.game_items.actor_script IS 'If this Item is an Actor (e.g. Projectile) this is the actor script';

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
COMMENT ON COLUMN public.game_maps.default_level IS 'The default level of players and NPCs on this map';
COMMENT ON COLUMN public.game_maps.party_count IS 'Number of opposing parties in this game';
COMMENT ON COLUMN public.game_maps.random_party IS 'Party with random players';
COMMENT ON COLUMN public.game_maps.queue_map_uuid IS 'From this map you can queue to another map';

CREATE TABLE public.game_music (
    uuid character(36) NOT NULL,
    map_uuid text NOT NULL,
    local_file character varying(260) NOT NULL,
    remote_file character varying(260) DEFAULT ''::character varying NOT NULL,
    sorting integer DEFAULT 0 NOT NULL,
    style integer DEFAULT 0 NOT NULL
);
COMMENT ON COLUMN public.game_music.map_uuid IS 'Map UUIDs separated with a semicolon';

CREATE TABLE public.game_professions (
    uuid character(36) NOT NULL,
    idx bigint NOT NULL,
    name text NOT NULL,
    abbr character varying(2) NOT NULL,
    model_index_female bigint DEFAULT 0 NOT NULL,
    model_index_male bigint DEFAULT 0 NOT NULL,
    "position" integer DEFAULT 0 NOT NULL
);
COMMENT ON COLUMN public.game_professions.model_index_female IS 'Index in game_items table';
COMMENT ON COLUMN public.game_professions.model_index_male IS 'Index in game_items table';
COMMENT ON COLUMN public.game_professions."position" IS 'Front-, mid-, backline';

CREATE TABLE public.game_quests (
    uuid character(36) NOT NULL,
    idx integer NOT NULL,
    name text NOT NULL,
    script character varying(260) NOT NULL
);

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

CREATE TABLE public.guild_members (
    account_uuid character(36) DEFAULT NULL::bpchar,
    guild_uuid character(36) DEFAULT NULL::bpchar,
    role bigint DEFAULT 1::bigint NOT NULL,
    invite_name text DEFAULT ''::text NOT NULL,
    invited bigint DEFAULT 0::bigint NOT NULL,
    joined bigint DEFAULT 0::bigint NOT NULL,
    expires bigint DEFAULT 0::bigint NOT NULL
);

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
COMMENT ON COLUMN public.guilds.guild_hall_uuid IS 'Guild Hall Map UUID';
COMMENT ON COLUMN public.guilds.guild_hall_instance_uuid IS 'If the guild hall instance is running, this is the UUID';
COMMENT ON COLUMN public.guilds.guild_hall_server_uuid IS 'If the instance is running, this is the server where it runs on';

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
COMMENT ON TABLE public.instances IS 'Game instances';

CREATE TABLE public.ip_bans (
    uuid character(36) NOT NULL,
    ban_uuid character(36) NOT NULL,
    ip bigint NOT NULL,
    mask bigint NOT NULL
);

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
COMMENT ON TABLE public.players IS 'Characters';

CREATE TABLE public.reserved_names (
    uuid character(36) NOT NULL,
    name text NOT NULL,
    is_reserved smallint DEFAULT (0)::smallint NOT NULL,
    reserved_for_account_uuid character(36) DEFAULT ''::bpchar,
    expires bigint DEFAULT 0 NOT NULL
);
COMMENT ON TABLE public.reserved_names IS 'Names that can not be used to create characters';
COMMENT ON COLUMN public.reserved_names.reserved_for_account_uuid IS 'When a user deletes a character the name will be reserved for some time';
COMMENT ON COLUMN public.reserved_names.expires IS 'Timestamp when this reservation expires';

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

CREATE TABLE public.versions (
    uuid character(36) NOT NULL,
    name text NOT NULL,
    value bigint NOT NULL,
    internal smallint DEFAULT (0)::smallint NOT NULL
);

ALTER TABLE ONLY public.account_bans ADD CONSTRAINT account_bans_ban_uuid_key UNIQUE (ban_uuid);
ALTER TABLE ONLY public.account_bans ADD CONSTRAINT account_bans_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.account_keys ADD CONSTRAINT account_keys_uuid PRIMARY KEY (uuid);
ALTER TABLE ONLY public.accounts ADD CONSTRAINT accounts_name_key UNIQUE (name);
ALTER TABLE ONLY public.accounts ADD CONSTRAINT accounts_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.bans ADD CONSTRAINT bans_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.friend_list ADD CONSTRAINT friend_list_account_uuid_friend_uuid_key UNIQUE (account_uuid, friend_uuid);
ALTER TABLE ONLY public.game_attributes ADD CONSTRAINT game_attributes_idx_key UNIQUE (idx);
ALTER TABLE ONLY public.game_attributes ADD CONSTRAINT game_attributes_uuid_key UNIQUE (uuid);
ALTER TABLE ONLY public.game_effects ADD CONSTRAINT game_effects_idx_key UNIQUE (idx);
ALTER TABLE ONLY public.game_effects ADD CONSTRAINT game_effects_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.game_item_chances ADD CONSTRAINT game_item_chances_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.game_items ADD CONSTRAINT game_items_idx_key UNIQUE (idx);
ALTER TABLE ONLY public.game_items ADD CONSTRAINT game_items_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.game_maps ADD CONSTRAINT game_maps_name_key UNIQUE (name);
ALTER TABLE ONLY public.game_maps ADD CONSTRAINT game_maps_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.game_music ADD CONSTRAINT game_music_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.game_professions ADD CONSTRAINT game_professions_abbr_key UNIQUE (abbr);
ALTER TABLE ONLY public.game_professions ADD CONSTRAINT game_professions_idx_key UNIQUE (idx);
ALTER TABLE ONLY public.game_professions ADD CONSTRAINT game_professions_name_key UNIQUE (name);
ALTER TABLE ONLY public.game_professions ADD CONSTRAINT game_professions_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.game_quests ADD CONSTRAINT game_quests_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.game_skills ADD CONSTRAINT game_skills_idx_key UNIQUE (idx);
ALTER TABLE ONLY public.game_skills ADD CONSTRAINT game_skills_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.guild_members ADD CONSTRAINT guild_members_account_uuid_guild_uuid_key UNIQUE (account_uuid, guild_uuid);
ALTER TABLE ONLY public.guilds ADD CONSTRAINT guilds_name_key UNIQUE (name);
ALTER TABLE ONLY public.guilds ADD CONSTRAINT guilds_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.instances ADD CONSTRAINT instances_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.concrete_items ADD CONSTRAINT inventory_items_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.ip_bans ADD CONSTRAINT ip_bans_ip_mask_key UNIQUE (ip, mask);
ALTER TABLE ONLY public.ip_bans ADD CONSTRAINT ip_bans_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.mails ADD CONSTRAINT mails_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.players ADD CONSTRAINT players_name_key UNIQUE (name);
ALTER TABLE ONLY public.players ADD CONSTRAINT players_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.reserved_names ADD CONSTRAINT reserved_names_name_key UNIQUE (name);
ALTER TABLE ONLY public.reserved_names ADD CONSTRAINT reserved_names_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.services ADD CONSTRAINT services_pkey PRIMARY KEY (uuid);
ALTER TABLE ONLY public.versions ADD CONSTRAINT versions_name_key UNIQUE (name);
ALTER TABLE ONLY public.versions ADD CONSTRAINT versions_pkey PRIMARY KEY (uuid);

CREATE INDEX account_ban_account_uuid ON public.account_bans USING btree (account_uuid);
CREATE UNIQUE INDEX account_uuid ON public.account_account_keys USING btree (account_uuid, account_key_uuid);
CREATE INDEX accunt_key_status ON public.account_keys USING btree (status);
CREATE INDEX bans_admin_uuid ON public.bans USING btree (admin_uuid);
CREATE INDEX concrete_items_storage_place ON public.concrete_items USING btree (storage_place);
CREATE INDEX friend_list_account_uuid_key ON public.friend_list USING btree (account_uuid);
CREATE INDEX friend_list_friend_uuid_key ON public.friend_list USING btree (friend_uuid);
CREATE INDEX game_attributes_profession_uuid ON public.game_attributes USING btree (profession_uuid);
CREATE INDEX game_item_chances_may_uuid_key ON public.game_item_chances USING btree (map_uuid);
CREATE INDEX game_items_type ON public.game_items USING btree (type);
CREATE INDEX game_maps_type ON public.game_maps USING btree (type);
CREATE INDEX game_music_sorting ON public.game_music USING btree (sorting);
CREATE INDEX guild_members_account_uuid ON public.guild_members USING btree (account_uuid);
CREATE INDEX guild_members_expires ON public.guild_members USING btree (expires);
CREATE INDEX guild_members_guild_uuid ON public.guild_members USING btree (guild_uuid);
CREATE UNIQUE INDEX guilds_name_ci_index ON public.guilds USING btree (lower(name));
CREATE INDEX instances_recording_index ON public.instances USING btree (recording);
CREATE INDEX inventory_items_account_uuid ON public.concrete_items USING btree (account_uuid);
CREATE INDEX inventory_items_player_uuid ON public.concrete_items USING btree (player_uuid);
CREATE INDEX ip_bans_ban_uuid ON public.ip_bans USING btree (ban_uuid);
CREATE INDEX ip_bans_ip ON public.ip_bans USING btree (ip);
CREATE INDEX mails_created ON public.mails USING btree (created);
CREATE INDEX mails_from_account_uuid ON public.mails USING btree (from_account_uuid);
CREATE INDEX mails_is_read ON public.mails USING btree (is_read);
CREATE INDEX mails_to_account_uuid ON public.mails USING btree (to_account_uuid);
CREATE INDEX players_account_uuid ON public.players USING btree (account_uuid);
CREATE UNIQUE INDEX players_name_ci_index ON public.players USING btree (lower(name));
CREATE INDEX reserved_names_is_reserved ON public.reserved_names USING btree (is_reserved);
CREATE INDEX reserved_names_name_ci_index ON public.reserved_names USING btree (lower(name));
CREATE INDEX reserved_names_until ON public.reserved_names USING btree (expires);
CREATE INDEX services_type ON public.services USING btree (type);

INSERT INTO public.game_attributes VALUES ('0739ed7d-50f6-11e8-a7ca-02100700d6f0', 12, '85d0ef81-50f4-11e8-a7ca-02100700d6f0', 'Energy Storage', 1);
INSERT INTO public.game_attributes VALUES ('073a57c1-50f6-11e8-a7ca-02100700d6f0', 9, '85d0ef81-50f4-11e8-a7ca-02100700d6f0', 'Earth Magic', 0);
INSERT INTO public.game_attributes VALUES ('0ff5523b-50f5-11e8-a7ca-02100700d6f0', 19, '59f4493d-50f4-11e8-a7ca-02100700d6f0', 'Hammer Mastery', 0);
INSERT INTO public.game_attributes VALUES ('0ff5b6cc-50f5-11e8-a7ca-02100700d6f0', 20, '59f4493d-50f4-11e8-a7ca-02100700d6f0', 'Swordsmanship', 0);
INSERT INTO public.game_attributes VALUES ('13582547-50f6-11e8-a7ca-02100700d6f0', 10, '85d0ef81-50f4-11e8-a7ca-02100700d6f0', 'Fire Magic', 0);
INSERT INTO public.game_attributes VALUES ('13598bc5-50f6-11e8-a7ca-02100700d6f0', 8, '85d0ef81-50f4-11e8-a7ca-02100700d6f0', 'Air Magic', 0);
INSERT INTO public.game_attributes VALUES ('18c57862-50f5-11e8-a7ca-02100700d6f0', 21, '59f4493d-50f4-11e8-a7ca-02100700d6f0', 'Tactics', 0);
INSERT INTO public.game_attributes VALUES ('1a19c3d5-50f6-11e8-a7ca-02100700d6f0', 11, '85d0ef81-50f4-11e8-a7ca-02100700d6f0', 'Water Magic', 0);
INSERT INTO public.game_attributes VALUES ('4e8d25fe-50f7-11e8-a7ca-02100700d6f0', 99, '00000000-0000-0000-0000-000000000000', 'None', 0);
INSERT INTO public.game_attributes VALUES ('536a55fe-50f5-11e8-a7ca-02100700d6f0', 23, '59f4b30b-50f4-11e8-a7ca-02100700d6f0', 'Expertise', 1);
INSERT INTO public.game_attributes VALUES ('536aba10-50f5-11e8-a7ca-02100700d6f0', 25, '59f4b30b-50f4-11e8-a7ca-02100700d6f0', 'Markmanship', 0);
INSERT INTO public.game_attributes VALUES ('67bf90ba-50f5-11e8-a7ca-02100700d6f0', 22, '59f4b30b-50f4-11e8-a7ca-02100700d6f0', 'Beast Mastery', 0);
INSERT INTO public.game_attributes VALUES ('67bffc43-50f5-11e8-a7ca-02100700d6f0', 24, '59f4b30b-50f4-11e8-a7ca-02100700d6f0', 'Wilderness Survival', 0);
INSERT INTO public.game_attributes VALUES ('9f9162c0-50f5-11e8-a7ca-02100700d6f0', 16, '73156b15-50f4-11e8-a7ca-02100700d6f0', 'Divine Favor', 1);
INSERT INTO public.game_attributes VALUES ('9f91d5ba-50f5-11e8-a7ca-02100700d6f0', 13, '73156b15-50f4-11e8-a7ca-02100700d6f0', 'Healing Prayers', 0);
INSERT INTO public.game_attributes VALUES ('b5a203ec-50f5-11e8-a7ca-02100700d6f0', 14, '73156b15-50f4-11e8-a7ca-02100700d6f0', 'Smiting Prayers', 0);
INSERT INTO public.game_attributes VALUES ('b5a2673e-50f5-11e8-a7ca-02100700d6f0', 15, '73156b15-50f4-11e8-a7ca-02100700d6f0', 'Protection Prayers', 0);
INSERT INTO public.game_attributes VALUES ('c5d4d491-50f5-11e8-a7ca-02100700d6f0', 6, '7315cbd6-50f4-11e8-a7ca-02100700d6f0', 'Soul Reaping', 1);
INSERT INTO public.game_attributes VALUES ('c5d5205c-50f5-11e8-a7ca-02100700d6f0', 4, '7315cbd6-50f4-11e8-a7ca-02100700d6f0', 'Blood Magic', 0);
INSERT INTO public.game_attributes VALUES ('d1e4b914-50f5-11e8-a7ca-02100700d6f0', 7, '7315cbd6-50f4-11e8-a7ca-02100700d6f0', 'Curses', 0);
INSERT INTO public.game_attributes VALUES ('d1e52aa1-50f5-11e8-a7ca-02100700d6f0', 5, '7315cbd6-50f4-11e8-a7ca-02100700d6f0', 'Death Magic', 0);
INSERT INTO public.game_attributes VALUES ('e8e10cb8-50f5-11e8-a7ca-02100700d6f0', 0, '85d0939b-50f4-11e8-a7ca-02100700d6f0', 'Fast Casting', 1);
INSERT INTO public.game_attributes VALUES ('e8e16f7e-50f5-11e8-a7ca-02100700d6f0', 2, '85d0939b-50f4-11e8-a7ca-02100700d6f0', 'Domination Magic', 0);
INSERT INTO public.game_attributes VALUES ('f74719b7-50f5-11e8-a7ca-02100700d6f0', 1, '85d0939b-50f4-11e8-a7ca-02100700d6f0', 'Illusion Magic', 0);
INSERT INTO public.game_attributes VALUES ('f7477bbe-50f5-11e8-a7ca-02100700d6f0', 3, '85d0939b-50f4-11e8-a7ca-02100700d6f0', 'Inspiration Magic', 0);
INSERT INTO public.game_attributes VALUES ('fcb26454-50f4-11e8-a7ca-02100700d6f0', 17, '59f4493d-50f4-11e8-a7ca-02100700d6f0', 'Strength', 1);
INSERT INTO public.game_attributes VALUES ('fcb2cbfc-50f4-11e8-a7ca-02100700d6f0', 18, '59f4493d-50f4-11e8-a7ca-02100700d6f0', 'Axe Mastery', 0);

INSERT INTO public.game_effects VALUES ('827f837a-5113-11e8-a7ca-02100700d6f0', 1000, 'PvP', 254, '/scripts/effects/environment/pvp.lua', '/Textures/Effects/pvp.png', '', '');
INSERT INTO public.game_effects VALUES ('decedb0a-1c81-419e-ba36-38c655cbc792', 1043, 'Dash', 9, '/scripts/effects/stance/dash.lua', '/Textures/Skills/dash.png', '', '');
INSERT INTO public.game_effects VALUES ('bcda113a-74d9-416a-a630-df1c9b34e70a', 10003, 'Dazed', 1, 'scripts/effects/condition/dazed.lua', '/Textures/Effects/dazed.png', '', '');
INSERT INTO public.game_effects VALUES ('60fe583b-fba8-477f-8b32-44e971fac86b', 10001, 'Crippled', 1, '/scripts/effects/condition/crippled.lua', '', '', '');
INSERT INTO public.game_effects VALUES ('95bdcf5a-5bf3-41ac-8391-bb360b111f76', 10002, 'Poison', 1, '/scripts/effects/condition/poison.lua', '/Textures/Effects/poison.png', '', '');
INSERT INTO public.game_effects VALUES ('a40a6807-f687-4de8-867b-488c59353da7', 900000, 'Undestroyable', 20, '/scripts/effects/general/undestroyable.lua', '/Textures/Effects/god.png', '', '');
INSERT INTO public.game_effects VALUES ('4d021639-612e-4fe2-b359-d272eef353ed', 288, 'Healing Breeze', 2, '/scripts/effects/enchantment/healing_breeze.lua', '', '', '');
INSERT INTO public.game_effects VALUES ('5c389b57-bed1-4554-bb5f-4e48d70f1b86', 10004, 'Burning', 1, 'scripts/effects/condition/burning.lua', '/Textures/Effects/fire.png', '', '');
INSERT INTO public.game_effects VALUES ('62781eff-5286-11e8-a7ca-02100700d6f0', 1001, 'Morale', 20, '/scripts/effects/general/morale.lua', '/Textures/Effects/morale.png', '', '');

INSERT INTO public.game_item_chances VALUES ('c70c8b6d-4027-4e3f-a15d-0de55f694b56', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', 'a53eb26d-39e4-4900-b6f4-12c7da07b68b', 1);
INSERT INTO public.game_item_chances VALUES ('baa149ed-9e4a-4a4a-bede-bb1835538a20', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', '00000000-0000-0000-0000-000000000000', 300);
INSERT INTO public.game_item_chances VALUES ('1ca72c3d-05ad-4151-8bb8-d59c9e3ed39e', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', 'f113f917-6276-408c-a56c-86b9d7ec1dcb', 100);
INSERT INTO public.game_item_chances VALUES ('c07ef20e-1846-45f1-9692-65a39224fa39', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', 'eda759a6-f8f9-45f3-9464-efbef6843423', 100);
INSERT INTO public.game_item_chances VALUES ('74789191-e133-4c3b-be74-62c84ac3c0c0', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', 'ecf36a5c-ff07-4c24-822b-579af7c8339e', 100);
INSERT INTO public.game_item_chances VALUES ('6f4e4ba1-74e3-4d77-bbb8-28e3ab140310', '00000000-0000-0000-0000-000000000000', 'e79bc08f-0052-4ee3-ac79-51edcc454a48', 0);
INSERT INTO public.game_item_chances VALUES ('80e10ea1-f2b9-448f-a2e6-8cd4cb868b2f', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', 'e342391b-5150-4543-a43e-ecaed823f65a', 1);
INSERT INTO public.game_item_chances VALUES ('8f4dbb5f-8bba-4723-a346-1b2b7b23a77f', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', '3187c3e-119f-4754-8285-de9a6595473d ', 100);
INSERT INTO public.game_item_chances VALUES ('7a84de95-690e-4187-834f-3c4b0777678d', '00000000-0000-0000-0000-000000000000', '08fbf9bd-b84f-412f-ae4a-bc499784fadf', 300);
INSERT INTO public.game_item_chances VALUES ('4846f62d-8249-4c78-97d6-600fd75a49da', '00000000-0000-0000-0000-000000000000', 'ba4e40ed-8c70-4916-bc76-b30feca42d88', 100);
INSERT INTO public.game_item_chances VALUES ('64f636ff-668b-417c-b250-9267f18635d4', '00000000-0000-0000-0000-000000000000', 'f87b5cf3-c8e5-4f9f-b2c1-3768940484a7', 100);
INSERT INTO public.game_item_chances VALUES ('55e6d7fd-3087-4997-9a58-43c715b9f469', '00000000-0000-0000-0000-000000000000', '19e4420f-2af3-4039-822f-4be1e3e5450e', 100);
INSERT INTO public.game_item_chances VALUES ('6914b2d1-49d8-4130-9f19-a0b867b76f08', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', '7ccd5ff5-4788-42b1-9dc7-f644bcc5766f', 1);

INSERT INTO public.game_items VALUES ('fa5d7b1b-987a-11e8-a7ca-02100700d6f0', 8, 'Hair Female Long blonde', '', 'Objects/Hair_Female_LongBlonde.xml', '', 2, 0, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('00000000-0000-0000-0000-000000000000', 0, 'No Item', '', '', '', 0, 0, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('a53eb26d-39e4-4900-b6f4-12c7da07b68b', 503, 'Eternal Blade', '/scripts/items/weapons/leadhand/swords.lua', '/Objects/EternalBlade.xml', '/Textures/Icons/Items/EternalBlade.png', 31, 0, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('e3187c3e-119f-4754-8285-de9a6595473d', 506, 'Xiphos', '/scripts/items/weapons/leadhand/swords.lua', '/Objects/Xiphos.xml', '/Textures/Icons/Items/Xiphos.png', 31, 0, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('e342391b-5150-4543-a43e-ecaed823f65a', 504, 'Eternal Shield', '/scripts/items/weapons/offhand/shields.lua', '/Objects/EternalShield.xml', '/Textures/Icons/Items/EternalShield.png', 44, 0, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('f113f917-6276-408c-a56c-86b9d7ec1dcb', 502, 'Spartan Shield', '/scripts/items/weapons/offhand/shields.lua', '/Objects/SpartanShield.xml', '/Textures/Icons/Items/SpartanShield.png', 44, 0, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('e79bc08f-0052-4ee3-ac79-51edcc454a48', 505, 'Survivor Insignia', '/scripts/items/upgrades/insignias/survivor.lua', '/Objects/Placeholder.xml', '/Textures/Icons/placeholder.png', 15, 0, 0, 30, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('ecf36a5c-ff07-4c24-822b-579af7c8339e', 500, 'Basic Wand', '/scripts/items/weapons/leadhand/wands.lua', '/Objects/BasicWand.xml', '/Textures/Icons/Items/BasicWand.png', 39, 0, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('08fbf9bd-b84f-412f-ae4a-bc499784fadf', 9999999, 'Drachma', '', '/Objects/GoldBag.xml', '/Textures/Icons/Items/CoinBag.png', 65534, 1, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('eda759a6-f8f9-45f3-9464-efbef6843423', 501, 'Smith Hammer', '/scripts/items/weapons/twohanded/hammers.lua', '/Objects/SmithHammer.xml', '/Textures/Icons/Items/SmithHammer.png', 32, 0, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('ba4e40ed-8c70-4916-bc76-b30feca42d88', 100001, 'Wood', '', '/Objects/Item_Wood.xml', '/Textures/Icons/Items/Wood.png', 1000, 1, 0, 2, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('f87b5cf3-c8e5-4f9f-b2c1-3768940484a7', 100002, 'Bones', '', '/Objects/Bones.xml', '/Textures/Icons/Items/Bones.png', 1000, 1, 0, 2, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('19e4420f-2af3-4039-822f-4be1e3e5450e', 100000, 'Iron', '', '/Objects/Iron.xml', '/Textures/Icons/Items/Iron.png', 1000, 1, 0, 2, 0, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('3092b8ef-9250-11e8-a7ca-02100700d6f0', 4, 'PC Female Warrior Body 1', '', 'Objects/PC_Human_W_Female1_Base.xml', '', 1, 0, 0, 0, 1, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('6c6438a1-9319-11e8-a7ca-02100700d6f0', 6, 'PC Male Warrior Body 1', '', 'Objects/PC_Human_W_Male1_Base.xml', '', 1, 0, 0, 0, 2, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('010d902b-93f4-11e8-a7ca-02100700d6f0', 7, 'PC Female Elementarist Body 1', '', 'Objects/PC_Human_E_Female1_Base.xml', '', 1, 0, 0, 0, 3, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('3f57f41d-924b-11e8-a7ca-02100700d6f0', 3, 'PC Female Mesmer Body 1', '', 'Objects/PC_Human_Me_Female1_Base.xml', '', 1, 0, 0, 0, 5, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('9f7e64d1-9319-11e8-a7ca-02100700d6f0', 2, 'PC Male Monk Body 1', '', 'Objects/PC_Human_Mo_Male1_Base.xml', '', 1, 0, 0, 0, 10, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('d159a500-8f0c-11e8-a7ca-02100700d6f0', 1, 'PC Female Monk Body 1', '', 'Objects/PC_Human_Mo_Female1_Base.xml', '', 1, 0, 0, 0, 9, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('7003a93f-b01c-11e8-a7ca-02100700d6f0', 10, 'NPC Pedestrian 1', '', 'Objects/NPC_PedestrianFemale1_Base.xml', '', 1, 0, 0, 0, 9, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('a79dd25b-a280-41c3-b91e-6b61f4e0c56d', 12, 'NPC Pedestrian 2', '', 'Objects/NPC_PedestrianFemale2_Base.xml', '', 1, 0, 0, 0, 5, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('96254a1e-9958-11e8-a7ca-02100700d6f0', 9, 'NPC Merchant', '', 'Objects/NPC_Merchant_Base.xml', '', 1, 0, 0, 0, 8, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('d9686531-9259-11e8-a7ca-02100700d6f0', 5, 'NPC Smith 1', '', 'Objects/NPC_Smith1_Base.xml', '', 1, 0, 0, 0, 2, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('c5d0a104-b588-11e8-a7ca-02100700d6f0', 11, 'Portal', '', 'Objects/Portal.xml', '', 3, 0, 0, 0, 101, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('0cf61eda-2f41-40d2-bbd9-2061d10f57b9', 13, 'Chest', '', 'Objects/AccountChest.xml', '', 5, 0, 0, 0, 102, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('5a340ae9-59cf-43c3-b8a1-4477d5c857c7', 507, 'Longbow', '/scripts/items/weapons/twohanded/longbow.lua', '', '', 36, 0, 0, 0, 0, 'c277c616-4908-4d56-8ce7-7fd0b78e57b7', '');
INSERT INTO public.game_items VALUES ('7ccd5ff5-4788-42b1-9dc7-f644bcc5766f', 508, 'Eternal Bow', '/scripts/items/weapons/twohanded/recurve_bow.lua', 'Objects/EternalBow.xml', '/Textures/Icons/Items/EternalBow.png', 37, 0, 0, 0, 0, 'c277c616-4908-4d56-8ce7-7fd0b78e57b7', '');
INSERT INTO public.game_items VALUES ('c277c616-4908-4d56-8ce7-7fd0b78e57b7', 5000, 'Bow Arrow', '', 'Objects/BowArrow.xml', '', 45, 0, 0, 0, 501, '00000000-0000-0000-0000-000000000000', '/scripts/actors/projectiles/bow_arrow.lua');
INSERT INTO public.game_items VALUES ('f8480e5b-1497-4d65-a45d-a497c1e7a035', 14, 'Electra Staneli Model', '', 'Objects/ElectraStaneli.xml', '', 1, 0, 0, 0, 3, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('1cab266c-4cd6-417d-ad3f-4636dcb526e1', 10001, 'Poison Dart Launcher', '', '/Objects/PoisonDartLaucher.xml', '', 1, 0, 0, 0, 103, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('e945dcab-b383-4247-9da5-0a01c4bd4bfd', 5001, 'Poison Dart', '', '/Objects/PoisonDart.xml', '', 45, 0, 0, 0, 501, '00000000-0000-0000-0000-000000000000', '/scripts/actors/projectiles/poison_dart.lua');
INSERT INTO public.game_items VALUES ('ce54bf1a-af80-44d2-b4b6-82393407c946', 6000, 'Magic Mushroom', '', '/Objects/MagicMushroom.xml', '', 49, 0, 0, 0, 1000, '00000000-0000-0000-0000-000000000000', '');
INSERT INTO public.game_items VALUES ('2cea6cb6-a77e-4808-baef-c1b11601dc08', 6001, 'Meteor Shower', '', '/Objects/MeteorShower.xml', '/Textures/Icons/placeholder.png', 49, 0, 0, 0, 1000, '00000000-0000-0000-0000-000000000000', '');

INSERT INTO public.game_maps VALUES ('964152eb-b3e3-4a2b-b8d1-45eb01cce0ea', 'Pergamon', 1, '/maps/pergamon', '/scripts/games/pergamon.lua', 0, 4, 1080, 402, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('f39eef5f-b946-4661-8a46-a4490e466e15', 'Argos', 1, '/maps/argos', '/scripts/games/argos.lua', 0, 4, 642, 740, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('8604e197-6e54-46b0-945a-16e265cf0e0c', 'Cos', 1, '/maps/cos', '/scripts/games/cos.lua', 0, 4, 1199, 731, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('0f7e54b4-f9a6-4408-88a8-9585f50d3861', 'Delphi', 1, '/maps/delphi', '/scripts/games/delphi.lua', 0, 4, 569, 614, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('a12f3a86-9fab-4ff9-9623-999e8070ccb3', 'Dion', 1, '/maps/dion', '/scripts/games/dion.lua', 0, 4, 528, 362, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('9ad4b59c-0ae6-4ac6-be8a-091f36525850', 'Samos', 1, '/maps/samos', '/scripts/games/samos.lua', 0, 4, 1106, 627, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('3ea9544c-d96f-421e-9113-e25a2c396ae2', 'Spartha', 1, '/maps/spartha', '/scripts/games/spartha.lua', 0, 4, 611, 835, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('59c9fd36-51ad-4a95-bd6a-422085c9102d', 'Thebes', 1, '/maps/thebes', '/scripts/games/thebes.lua', 0, 4, 672, 623, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('4c43346f-dc73-43bc-beb0-d119db10032a', 'Troy', 1, '/maps/troy', '/scripts/games/troy.lua', 0, 4, 955, 315, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('75e3f5c3-479a-11e8-ad09-02100700d6f0', 'Dodona', 1, '/maps/dodona', '/scripts/games/dodona.lua', 0, 4, 347, 505, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('75e3dfcf-479a-11e8-ad09-02100700d6f0', 'Bassae', 1, '/maps/bassae', '/scripts/games/bassae.lua', 0, 4, 533, 791, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0', 'Claros', 1, '/maps/claros', '/scripts/games/claros.lua', 0, 4, 1128, 566, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('052b9cd1-ecd0-410f-bbaf-6b472cdc38fe', 'Magnesia', 1, '/maps/magnesia', '/scripts/games/magnesia.lua', 0, 4, 1199, 586, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('a9c02902-ce0c-47d0-9e74-499219ae71ee', 'Didymes', 1, '/maps/didymes', '/scripts/games/didymes.lua', 0, 4, 1200, 660, 1, 0, 0, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Rhodes', 1, '/maps/rhodes', '/scripts/games/rhodes.lua', 0, 4, 1318, 770, 1, 0, 0, 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', 0);
INSERT INTO public.game_maps VALUES ('a13b71f8-fe19-4bf5-bba7-c7642c796c0f', 'Rhodes Arena', 4, '/maps/rhodes_arena', '/scripts/games/rhodes_arena.lua', 0, 4, 0, 0, 1, 2, 1, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('3c081fd5-3966-433a-bc61-50a33084eac2', 'Athena Arena', 4, '/maps/athena_arena', '/scripts/games/athena_arena.lua', 0, 4, 0, 0, 1, 2, 1, '00000000-0000-0000-0000-000000000000', 0);
INSERT INTO public.game_maps VALUES ('75e3eeb5-479a-11e8-ad09-02100700d6f0', 'Athena', 2, '/maps/athena', '/scripts/games/athena.lua', 1, 4, 741, 668, 1, 0, 0, '3c081fd5-3966-433a-bc61-50a33084eac2', 0);

INSERT INTO public.game_music VALUES ('67a1233a-f391-4e2b-870e-ab6e073795fa', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Eastern/Jalandhar.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('7f621c3b-9f84-4fcc-93f9-36ea41a41b51', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Eastern/Lord of the Land.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('453ea70a-9b90-43fc-abb3-6a43b34a6fc8', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Eastern/Mystery Bazaar.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('8384e3f4-81f1-48c8-9e55-8d51da58edf1', '75e3eeb5-479a-11e8-ad09-02100700d6f0;59c9fd36-51ad-4a95-bd6a-422085c9102d;f39eef5f-b946-4661-8a46-a4490e466e15;3ea9544c-d96f-421e-9113-e25a2c396ae2;75e3dfcf-479a-11e8-ad09-02100700d6f0;0f7e54b4-f9a6-4408-88a8-9585f50d3861;  75e3f5c3-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Western/The Pyre.ogg', '', 0, 4);
INSERT INTO public.game_music VALUES ('5e7c64f5-bd37-4a7a-9b95-afaebd112ec5', '75e3eeb5-479a-11e8-ad09-02100700d6f0;59c9fd36-51ad-4a95-bd6a-422085c9102d;f39eef5f-b946-4661-8a46-a4490e466e15;3ea9544c-d96f-421e-9113-e25a2c396ae2;75e3dfcf-479a-11e8-ad09-02100700d6f0;0f7e54b4-f9a6-4408-88a8-9585f50d3861;  75e3f5c3-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Western/String Impromptu Number 1.ogg', '', 0, 4);
INSERT INTO public.game_music VALUES ('e300d73d-6588-4ca6-ac1c-775067ec5ab7', '75e3eeb5-479a-11e8-ad09-02100700d6f0;59c9fd36-51ad-4a95-bd6a-422085c9102d;f39eef5f-b946-4661-8a46-a4490e466e15;3ea9544c-d96f-421e-9113-e25a2c396ae2;75e3dfcf-479a-11e8-ad09-02100700d6f0;0f7e54b4-f9a6-4408-88a8-9585f50d3861;  75e3f5c3-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Western/Teller of the Tales.ogg', '', 0, 4);
INSERT INTO public.game_music VALUES ('815f442a-6041-4c0d-bfd4-c248c096afcd', '75e3eeb5-479a-11e8-ad09-02100700d6f0;59c9fd36-51ad-4a95-bd6a-422085c9102d;f39eef5f-b946-4661-8a46-a4490e466e15;3ea9544c-d96f-421e-9113-e25a2c396ae2;75e3dfcf-479a-11e8-ad09-02100700d6f0;0f7e54b4-f9a6-4408-88a8-9585f50d3861;  75e3f5c3-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Western/Virtutes Instrumenti.ogg', '', 0, 4);
INSERT INTO public.game_music VALUES ('7d064264-5aa1-4e10-9ebc-34b36c8dbf80', 'CreateAccount;CreateCharacter;SelectCharacter;00000000-0000-0000-0000-000000000000', 'Sounds/Music/General/Chee Zee Jungle.ogg', '', 0, 0);
INSERT INTO public.game_music VALUES ('c3c61589-a273-4303-92a1-fdbb9d1faaa5', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Eastern/Curse of the Scarab.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('26f5f670-0fd3-41e3-a2ab-ff58d1259f89', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Eastern/Desert City.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('ad71b9e9-c0a6-485e-89bd-4558a9dba1f0', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Eastern/Hidden Wonders.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('1231ac88-b87e-420b-8bc5-3825d61ece24', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Eastern/Ibn Al-Noor.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('51ab2924-1bd6-4bf8-9732-0b70202fb012', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Eastern/Naraina.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('0c2a9dbd-928e-49f1-9e1e-128f66011ac1', '75e3eeb5-479a-11e8-ad09-02100700d6f0;59c9fd36-51ad-4a95-bd6a-422085c9102d;f39eef5f-b946-4661-8a46-a4490e466e15;3ea9544c-d96f-421e-9113-e25a2c396ae2;75e3dfcf-479a-11e8-ad09-02100700d6f0;0f7e54b4-f9a6-4408-88a8-9585f50d3861;  75e3f5c3-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Western/Agnus Dei X.ogg', '', 0, 4);
INSERT INTO public.game_music VALUES ('c7332ad9-0b46-4e39-8516-364217e889c0', '75e3eeb5-479a-11e8-ad09-02100700d6f0;59c9fd36-51ad-4a95-bd6a-422085c9102d;f39eef5f-b946-4661-8a46-a4490e466e15;3ea9544c-d96f-421e-9113-e25a2c396ae2;75e3dfcf-479a-11e8-ad09-02100700d6f0;0f7e54b4-f9a6-4408-88a8-9585f50d3861;  75e3f5c3-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Western/Ghost Dance.ogg', '', 0, 4);
INSERT INTO public.game_music VALUES ('a8f78770-8bba-4805-896f-ae85fb2043a5', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Clash Defiant.ogg', '', 0, 528);
INSERT INTO public.game_music VALUES ('37068c82-7706-404b-96c9-d9a0b1226f07', '75e3eeb5-479a-11e8-ad09-02100700d6f0;59c9fd36-51ad-4a95-bd6a-422085c9102d;f39eef5f-b946-4661-8a46-a4490e466e15;3ea9544c-d96f-421e-9113-e25a2c396ae2;75e3dfcf-479a-11e8-ad09-02100700d6f0;0f7e54b4-f9a6-4408-88a8-9585f50d3861;  75e3f5c3-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Western/Relent.ogg', '', 0, 4);
INSERT INTO public.game_music VALUES ('db2e9b6a-db3d-4018-96f3-41877ca15fdf', '75e3eeb5-479a-11e8-ad09-02100700d6f0;59c9fd36-51ad-4a95-bd6a-422085c9102d;f39eef5f-b946-4661-8a46-a4490e466e15;3ea9544c-d96f-421e-9113-e25a2c396ae2;75e3dfcf-479a-11e8-ad09-02100700d6f0;0f7e54b4-f9a6-4408-88a8-9585f50d3861;  75e3f5c3-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Western/Virtutes Vocis.ogg', '', 0, 4);
INSERT INTO public.game_music VALUES ('a5642b8a-cea1-4f32-a8f3-b2f6e14ac499', 'CreateAccount;CreateCharacter;SelectCharacter;00000000-0000-0000-0000-000000000000', 'Sounds/Music/General/Chee Zee Caves V2.ogg', '', 0, 0);
INSERT INTO public.game_music VALUES ('92f3fdaf-6290-4583-b00b-d2d21249cd41', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Prelude and Action.ogg', '', 0, 336);
INSERT INTO public.game_music VALUES ('4dccaa3a-6cf9-4de1-ac04-db8e6a0dba0c', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Hiding Your Reality.ogg', '', 0, 112);
INSERT INTO public.game_music VALUES ('b4f05bcd-214f-4707-9cc3-7c873f25f02d', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Hot Pursuit.ogg', '', 0, 80);
INSERT INTO public.game_music VALUES ('94f2403b-93cb-46ef-91de-c4379973e482', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/I Can Feel it Coming.ogg', '', 0, 144);
INSERT INTO public.game_music VALUES ('13930d3d-cee1-401f-87a4-08bfc76480de', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Killers.ogg', '', 0, 352);
INSERT INTO public.game_music VALUES ('5ffe27a7-e809-4f10-b8f2-8f884f2beda7', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Outfoxing the Fox.ogg', '', 0, 80);
INSERT INTO public.game_music VALUES ('9de4a78d-ac7c-4a17-b7dc-6df9a1a0a230', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Twisting.ogg', '', 0, 144);
INSERT INTO public.game_music VALUES ('77cc63e6-63e3-4e16-bfc6-5e166e47d545', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/Music/Eastern/Return of the Mummy.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('a2fb027d-3bfd-4d55-9078-c78413b41007', '4c43346f-dc73-43bc-beb0-d119db10032a;964152eb-b3e3-4a2b-b8d1-45eb01cce0ea;e3c8fa12-3c04-48d6-aa45-59c7cb0a3ee0;9ad4b59c-0ae6-4ac6-be8a-091f36525850;a9c02902-ce0c-47d0-9e74-499219ae71ee;8604e197-6e54-46b0-945a-16e265cf0e0c;75e3fc5c-479a-11e8-ad09-02100700d6f0', 'Sounds/music/Eastern/Tabuk.ogg', '', 0, 2);
INSERT INTO public.game_music VALUES ('0c8dd5dc-2f8d-4109-911e-b9e1afcd5d7b', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Five Armies.ogg', '', 0, 880);
INSERT INTO public.game_music VALUES ('a3646ac1-6c7a-4841-a75f-d6857ffcbfc0', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Darkling.ogg', '', 0, 608);
INSERT INTO public.game_music VALUES ('04e0adc9-222b-4fa9-b39c-98c6cca7ddea', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Death and Axes.ogg', '', 0, 608);
INSERT INTO public.game_music VALUES ('6cfd46ed-bb6b-4d80-ab9c-1904455e0864', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Final Battle of the Dark Wizards.ogg', '', 0, 288);
INSERT INTO public.game_music VALUES ('1978a7b7-2d68-4692-80be-5893188890c4', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Graveyard Shift.ogg', '', 0, 2048);
INSERT INTO public.game_music VALUES ('2fbe5493-6788-4ace-aa36-d2cb976d782e', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Grim Idol.ogg', '', 0, 64);
INSERT INTO public.game_music VALUES ('d12adbf7-df05-44b6-87dd-dcca77dbc08b', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Lost Frontier.ogg', '', 0, 528);
INSERT INTO public.game_music VALUES ('80cfe279-abe3-4bab-85fd-f0047d536af1', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Malicious.ogg', '', 0, 16);
INSERT INTO public.game_music VALUES ('b0e27c92-dcf1-4a1f-be60-abcade01f061', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Nightmare Machine.ogg', '', 0, 80);
INSERT INTO public.game_music VALUES ('9eb43b5b-a552-47ff-8b95-4a93b81fb789', '00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Obliteration.ogg', '', 0, 64);
INSERT INTO public.game_music VALUES ('af88b7e6-60ad-4432-bb91-20a0f6cdd47d', 'CreateAccount;CreateCharacter;SelectCharacter;00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Floating Cities.ogg', '', 0, 1024);
INSERT INTO public.game_music VALUES ('e5d638ab-9c8a-4c5d-8579-2341aec89029', 'CreateAccount;CreateCharacter;SelectCharacter;00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Rynos Theme.ogg', '', 0, 352);
INSERT INTO public.game_music VALUES ('f0b3bba0-f697-4f23-81ac-e6ee58ce765c', 'CreateAccount;CreateCharacter;SelectCharacter;00000000-0000-0000-0000-000000000000', 'Sounds/Music/Action/Unholy Knight.ogg', '', 0, 192);

INSERT INTO public.game_professions VALUES ('79b75ff4-92f0-11e8-a7ca-02100700d6f0', 0, 'None', 'NA', 0, 0, 0);
INSERT INTO public.game_professions VALUES ('59f4493d-50f4-11e8-a7ca-02100700d6f0', 1, 'Warrior', 'W', 4, 6, 1);
INSERT INTO public.game_professions VALUES ('85d0ef81-50f4-11e8-a7ca-02100700d6f0', 6, 'Elementarist', 'E', 7, 1, 2);
INSERT INTO public.game_professions VALUES ('85d0939b-50f4-11e8-a7ca-02100700d6f0', 5, 'Mesmer', 'Me', 3, 1, 3);
INSERT INTO public.game_professions VALUES ('7315cbd6-50f4-11e8-a7ca-02100700d6f0', 4, 'Necromancer', 'N', 1, 2, 2);
INSERT INTO public.game_professions VALUES ('73156b15-50f4-11e8-a7ca-02100700d6f0', 3, 'Monk', 'Mo', 1, 2, 3);
INSERT INTO public.game_professions VALUES ('59f4b30b-50f4-11e8-a7ca-02100700d6f0', 2, 'Ranger', 'R', 1, 2, 2);

INSERT INTO public.game_skills VALUES ('177e1b60-0796-4acc-bc34-7c4ab2e482de', 9998, 'Sudden Death', '4e8d25fe-50f7-11e8-a7ca-02100700d6f0', 0, 0, 'Sudden Death', 'Instant suicide', '/Textures/Skills/sudden_death.png', '/scripts/skills/sudden_death.lua', 1, '79b75ff4-92f0-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('b1b9761a-3fca-4302-acb1-6f1a537dd548', 9997, 'Instant Kill', '4e8d25fe-50f7-11e8-a7ca-02100700d6f0', 512, 0, 'Instant Kill', 'Instantly kills the target', '/Textures/Skills/instant_kill.png', '/scripts/skills/instant_kill.lua', 1, '79b75ff4-92f0-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('e87bc838-c2a5-4eac-8aaf-d01db556ddb3', 9996, 'Instant Rezz', '4e8d25fe-50f7-11e8-a7ca-02100700d6f0', 512, 0, 'Instant Rezz', 'Instantly resurrects the target', '/Textures/Skills/instant_rezz.png', '/scripts/skills/instant_rezz.lua', 1, '79b75ff4-92f0-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('3713d062-a8e7-4dea-9d9c-90ab9697f8cb', 312, 'Holy Strike', 'b5a203ec-50f5-11e8-a7ca-02100700d6f0', 0, 0, 'Touch Skill. Deal 10..55 holy damage. Deal 10..55 additional dam,age if target is knocked down.', 'Touch Skill. Deal 10..55 holy damage. Deal 10..55 additional dam,age if target is knocked down.', '/Textures/Skills/holy_strike.png', '/scripts/skills/smiting_prayers/holy_strike.lua', 0, '73156b15-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('97b7b756-19e3-40d7-8e25-3de18f5a07f1', 240, 'Smite', 'b5a203ec-50f5-11e8-a7ca-02100700d6f0', 512, 0, 'Spell. Deal 10..55 holy damage. If target is attacking it takes 10..35 additional damage.', 'Spell. Deal 10..55 holy damage. If target is attacking it takes 10..35 additional damage.', '/Textures/Skills/placeholder.png', '/scripts/skills/smiting_prayers/smite.lua', 0, '73156b15-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('ee6923e1-ef49-4355-a636-1512b51574e1', 287, 'Heal Party', '9f91d5ba-50f5-11e8-a7ca-02100700d6f0', 512, 0, 'Spell. Heal party for 30..75 Health.', 'Spell. Heal party for 30..75 Health.', '/Textures/Skills/placeholder.png', '/scripts/skills/healing_prayers/heal_party.lua', 0, '73156b15-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('153eea82-50fa-11e8-a7ca-02100700d6f0', 0, '(None)', '4e8d25fe-50f7-11e8-a7ca-02100700d6f0', 0, 0, 'No Skill', 'No Skill', '', '', 0, '79b75ff4-92f0-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('e4fdaa0b-4ecf-49c6-a3f5-ca6119117341', 281, 'Orison of Healing', '9f91d5ba-50f5-11e8-a7ca-02100700d6f0', 512, 0, 'Spell. Heals for 20..70.', 'Spell. Heals for 20..70.', '/Textures/Skills/placeholder.png', '/scripts/skills/healing_prayers/orison_of_healing.lua', 0, '73156b15-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('339fb8f3-8bcd-48b5-bb87-2fc2635e60f7', 10001, 'Poison Spout', '00000000-0000-0000-0000-000000000000', 512, 0, 'Spell. Deals 50 damage and poisoning target for 12 seconds.', 'Spell. Deals 50 damage and poisoning target for 12 seconds.', '/Textures/Skills/placeholder.png', '', 1, '79b75ff4-92f0-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('cd0722c1-50fa-11e8-a7ca-02100700d6f0', 2, 'Resurrection Signet', '4e8d25fe-50f7-11e8-a7ca-02100700d6f0', 256, 0, 'Signet. Resurrect target party member with 100% health and 25% energy.', 'Signet. Resurrect target party member with 100% health and 25% energy.', '/Textures/Skills/placeholder.png', '/scripts/skills/resurrection_signet.lua', 0, '79b75ff4-92f0-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('bc4ff444-50f9-11e8-a7ca-02100700d6f0', 1, 'Healing Signet', '18c57862-50f5-11e8-a7ca-02100700d6f0', 256, 0, 'Signet. You get 82..172 Health. You have -40 armor while using this skill.', 'Signet. You get 82..172 Health. You have -40 armor while using this skill.', '/Textures/Skills/placeholder.png', '/scripts/skills/tactics/healing_signet.lua', 0, '73156b15-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('1cee4a88-a386-414e-b2eb-dcc2390dcd29', 280, 'Heal Area', '9f91d5ba-50f5-11e8-a7ca-02100700d6f0', 512, 0, 'Spell. Heal all adjacent creatures for 30..180 points.', 'Spell. Heal all adjacent creatures for 30..180 points.', '/Textures/Skills/placeholder.png', '/scripts/skills/healing_prayers/heal_area.lua', 0, '73156b15-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('b1db8982-6914-4150-aa3c-e293f88151a5', 61, 'Leech Signet', 'f7477bbe-50f5-11e8-a7ca-02100700d6f0', 256, 0, 'Signet. Interrupt targets action. You gain 3..15 energy if it was a spell.', 'Signet. Interrupt targets action. You gain 3..15 energy if it was a spell.', '/Textures/Skills/placeholder.png', '/scripts/skills/inspiration_magic/leech_signet.lua', 0, '85d0939b-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('edf646da-b914-4be1-9b34-58ede0c5c3d4', 39, 'Energy Surge', 'e8e16f7e-50f5-11e8-a7ca-02100700d6f0', 512, 1, 'Elite Spell. Target foe loses 1..10 Energy. For each lost energy point, nearby foes take 9 damage.', 'Elite Spell. Target foe loses 1..10 Energy. For each lost energy point, nearby foes take 9 damage.', '/Textures/Skills/placeholder.png', '/scripts/skills/domination_magic/energy_surge.lua', 0, '85d0939b-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('5595ea05-99fd-44e4-bd15-fb890a21eee1', 42, 'Energy Burn', 'e8e16f7e-50f5-11e8-a7ca-02100700d6f0', 512, 0, 'Spell. Target loses 1..10 Energy and takes 9 damage for each lost energy point.', 'Spell. Target loses 1..10 Energy and takes 9 damage for each lost energy point.', '/Textures/Skills/placeholder.png', '/scripts/skills/domination_magic/energy_burn.lua', 0, '85d0939b-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('d53e8cfa-861e-4986-8631-b72c127cf276', 288, 'Healing Breeze', '9f91d5ba-50f5-11e8-a7ca-02100700d6f0', 512, 0, 'Enchantment Spell. Target gets +4..9 health regeneration (15 seconds).', 'Enchantment Spell. Target gets +4..9 health regeneration (15 seconds).', '/Textures/Skills/placeholder.png', '/scripts/skills/healing_prayers/healing_breeze.lua', 0, '73156b15-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('6dc7c172-4101-400e-a710-c7439766f128', 191, 'Immolate', '13582547-50f6-11e8-a7ca-02100700d6f0', 512, 0, 'Spell. Target gets 20..75 fire damage and burns for 1..3 seconds.', 'Spell. Target gets 20..75 fire damage and burns for 1..3 seconds.', '/Textures/Skills/placeholder.png', '/scripts/skills/fire_magic/immolate.lua', 0, '85d0ef81-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('083290ca-38c3-421f-8a83-6afece46425b', 1043, 'Dash', '4e8d25fe-50f7-11e8-a7ca-02100700d6f0', 1024, 0, 'Stance. You move 50% faster (3 seconds).', 'Stance. You move 50% faster (3 seconds).', '/Textures/Skills/dash.png', '/scripts/skills/dash.lua', 0, '79b75ff4-92f0-11e8-a7ca-02100700d6f0', 'Sounds/FX/Effects/Dash.wav', '');
INSERT INTO public.game_skills VALUES ('688ca399-8f67-4cd3-a6b4-af4c10939d4d', 192, 'Meteor Shower', '13582547-50f6-11e8-a7ca-02100700d6f0', 512, 0, 'Spell. Deal 7..112 damage and knock down every 3 seconds (9 seconds) to adjacent foes at targets initial location.', 'Spell. Deal 7..112 damage and knock down every 3 seconds (9 seconds) to adjacent foes at targets initial location.', '/Textures/Skills/placeholder.png', '/scripts/skills/fire_magic/meteor_shower.lua', 0, '85d0ef81-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('e7062cfe-daf2-4b50-8f07-d1b20c032355', 884, 'Searing Flames', '13582547-50f6-11e8-a7ca-02100700d6f0', 512, 1, 'Elite Spell. Burning foes get 10..1000 fire damage. Foes not burning burn for 1..7 seconds.', 'Elite Spell. Burning foes get 10..1000 fire damage. Foes not burning burn for 1..7 seconds.', '/Textures/Skills/placeholder.png', '/scripts/skills/fire_magic/searing_flames.lua', 0, '85d0ef81-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('fab4bcad-5fe1-4616-bf29-b9148740c869', 1379, 'Glowing Gaze', '13582547-50f6-11e8-a7ca-02100700d6f0', 512, 0, 'Spell. Deal 5..50 fire damage. If the target is burning, you get 5 energy plus 1 Energy for every 2 ranks of Energy Storage.', 'Spell. Deal 5..50 fire damage. If the target is burning, you get 5 energy plus 1 Energy for every 2 ranks of Energy Storage.', '/Textures/Skills/placeholder.png', '/scripts/skills/fire_magic/glowing_gaze.lua', 0, '85d0ef81-50f4-11e8-a7ca-02100700d6f0', '', '');
INSERT INTO public.game_skills VALUES ('eecf3e66-d7fb-499c-872f-2824e34fdae1', 40, 'Ether Feast', 'f7477bbe-50f5-11e8-a7ca-02100700d6f0', 512, 0, 'Sudden Death', 'Spell. Target loses 3 energy, you are healed for 20..65 health by lost energy point.', '/Textures/Skills/placeholder.png', '/scripts/skills/inspiration_magic/ether_feast.lua', 0, '85d0939b-50f4-11e8-a7ca-02100700d6f0', '', '');

INSERT INTO public.versions (uuid, name, value, internal) VALUES (public.random_guid(), 'game_attributes', 1, 0);
INSERT INTO public.versions (uuid, name, value, internal) VALUES (public.random_guid(), 'game_effects', 1, 0);
INSERT INTO public.versions (uuid, name, value, internal) VALUES (public.random_guid(), 'game_item_chances', 1, 0);
INSERT INTO public.versions (uuid, name, value, internal) VALUES (public.random_guid(), 'game_items', 1, 0);
INSERT INTO public.versions (uuid, name, value, internal) VALUES (public.random_guid(), 'game_maps', 1, 0);
INSERT INTO public.versions (uuid, name, value, internal) VALUES (public.random_guid(), 'game_music', 1, 0);
INSERT INTO public.versions (uuid, name, value, internal) VALUES (public.random_guid(), 'game_professions', 1, 0);
INSERT INTO public.versions (uuid, name, value, internal) VALUES (public.random_guid(), 'game_skills', 1, 0);

INSERT INTO public.versions (uuid, name, value, internal) VALUES (public.random_guid(), 'schema', 0, 1);
