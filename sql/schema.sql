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
