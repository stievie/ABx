--
-- Database: `forgottenwars`
--

CREATE TABLE IF NOT EXISTS `accounts` (
`id` int(10) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `password` varchar(255) NOT NULL,
  `email` varchar(255) NOT NULL DEFAULT '',
  `type` int(10) unsigned NOT NULL DEFAULT '1' COMMENT 'Account type',
  `blocked` tinyint(1) NOT NULL DEFAULT '0',
  `creation` bigint(20) NOT NULL DEFAULT '0',
  `storage` blob NOT NULL,
  `titles` blob NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `account_account_keys` (
  `account_id` int(10) NOT NULL,
  `account_keys_id` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `account_bans` (
  `ban_id` int(10) unsigned NOT NULL,
  `account_id` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `account_keys` (
`id` int(10) unsigned NOT NULL,
  `account_key` char(36) NOT NULL,
  `used` int(10) NOT NULL DEFAULT '0',
  `total` int(10) NOT NULL DEFAULT '1',
  `description` varchar(255) DEFAULT NULL,
  `status` tinyint(1) NOT NULL COMMENT '0 = not activated, 1 = ready for use, 2 = banned'
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `account_view` (
`id` int(10) unsigned
,`name` varchar(32)
,`password` varchar(255)
,`email` varchar(255)
,`type` int(10) unsigned
,`blocked` tinyint(1)
,`creation` bigint(20)
,`storage` blob
,`titles` blob
,`account_key` char(36)
,`used` int(10)
,`total` int(10)
,`status` tinyint(1)
,`description` varchar(255)
);

CREATE TABLE IF NOT EXISTS `bans` (
`id` int(10) unsigned NOT NULL,
  `expires` bigint(20) unsigned NOT NULL,
  `added` bigint(20) unsigned NOT NULL,
  `reason` int(5) NOT NULL DEFAULT '0',
  `active` tinyint(1) NOT NULL DEFAULT '0',
  `admin_id` int(10) unsigned DEFAULT NULL COMMENT 'account that issued the ban',
  `comment` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `games` (
`id` int(11) NOT NULL,
  `name` varchar(32) NOT NULL,
  `type` int(10) NOT NULL,
  `map` varchar(255) NOT NULL,
  `nav_mesh` varchar(255) NOT NULL,
  `script_file` varchar(255) NOT NULL,
  `landing` tinyint(1) DEFAULT '0'
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;

INSERT INTO `games` (`id`, `name`, `type`, `map`, `nav_mesh`, `script_file`, `landing`) VALUES
(1, 'Temple of Athene', 1, '/games/maps/temple_of_athene', '/games/maps/temple_of_athene/temple_of_athene.navmesh', '/games/scripts/temple_of_athene.lua', 1),
(2, 'Temple of Hephaistos', 1, '/games/maps/temple_of_hephaistos', '/games/maps/temple_of_hephaistos/temple_of_hephaistos.navmesh', '/games/scripts/temple_of_hephaistos.lua', 1);

CREATE TABLE IF NOT EXISTS `guilds` (
`id` int(11) NOT NULL,
  `name` varchar(255) NOT NULL,
  `tag` varchar(4) NOT NULL,
  `ownerid` int(11) NOT NULL,
  `creation` bigint(20) NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `guild_invites` (
  `account_id` int(11) NOT NULL DEFAULT '0',
  `guild_id` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `guild_members` (
  `account_id` int(11) unsigned NOT NULL,
  `guild_id` int(11) NOT NULL,
  `level` int(1) unsigned NOT NULL DEFAULT '1' COMMENT '0 = Guest, 1 = Member, 2 = Officer, 3 = Leader',
  `invited` bigint(20) NOT NULL DEFAULT '0',
  `expires` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `ip_bans` (
  `ban_id` int(10) unsigned NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mask` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `players` (
`id` int(11) NOT NULL,
  `profession` char(2) NOT NULL,
  `profession2` char(2) NOT NULL,
  `name` char(20) NOT NULL,
  `pvp` tinyint(1) NOT NULL DEFAULT '0',
  `account_id` int(11) NOT NULL DEFAULT '0',
  `level` int(11) NOT NULL DEFAULT '1',
  `experience` bigint(20) NOT NULL DEFAULT '0',
  `skillpoints` int(11) NOT NULL DEFAULT '0',
  `sex` int(11) NOT NULL DEFAULT '0',
  `lastlogin` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lastip` int(10) unsigned NOT NULL DEFAULT '0',
  `lastlogout` bigint(20) unsigned NOT NULL DEFAULT '0',
  `onlinetime` int(20) NOT NULL DEFAULT '0' COMMENT 'Online time in seconds',
  `deleted` bigint(20) NOT NULL DEFAULT '0' COMMENT 'Time deleted or 0 for not deleted',
  `creation` bigint(20) NOT NULL DEFAULT '0',
  `last_map` varchar(50) NOT NULL DEFAULT '',
  `effects` blob COMMENT 'Persistent effects',
  `skills` blob,
  `equipment` blob,
  `inventory` blob,
  `titles` blob
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `account_view`;

CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `account_view` AS (select `accounts`.`id` AS `id`,`accounts`.`name` AS `name`,`accounts`.`password` AS `password`,`accounts`.`email` AS `email`,`accounts`.`type` AS `type`,`accounts`.`blocked` AS `blocked`,`accounts`.`creation` AS `creation`,`accounts`.`storage` AS `storage`,`accounts`.`titles` AS `titles`,`account_keys`.`account_key` AS `account_key`,`account_keys`.`used` AS `used`,`account_keys`.`total` AS `total`,`account_keys`.`status` AS `status`,`account_keys`.`description` AS `description` from (`accounts` left join (`account_account_keys` join `account_keys`) on(((`account_account_keys`.`account_id` = `accounts`.`id`) and (`account_account_keys`.`account_keys_id` = `account_keys`.`id`)))));

ALTER TABLE `accounts`
 ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `name` (`name`);
ALTER TABLE `account_account_keys`
 ADD UNIQUE KEY `account_id` (`account_id`,`account_keys_id`);
ALTER TABLE `account_bans`
 ADD KEY `ban_id` (`ban_id`), ADD KEY `account_id` (`account_id`);
ALTER TABLE `account_keys`
 ADD PRIMARY KEY (`id`);
ALTER TABLE `bans`
 ADD PRIMARY KEY (`id`), ADD KEY `admin_id` (`admin_id`);
ALTER TABLE `games`
 ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `name` (`name`);
ALTER TABLE `guilds`
 ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `name` (`name`), ADD UNIQUE KEY `ownerid` (`ownerid`);
ALTER TABLE `guild_invites`
 ADD PRIMARY KEY (`account_id`,`guild_id`);
ALTER TABLE `guild_members`
 ADD KEY `account_id` (`account_id`), ADD KEY `guild_id` (`guild_id`);
ALTER TABLE `ip_bans`
 ADD KEY `ban_id` (`ban_id`);
ALTER TABLE `players`
 ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `name` (`name`), ADD KEY `account_id` (`account_id`);

ALTER TABLE `accounts`
MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
ALTER TABLE `account_keys`
MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
ALTER TABLE `bans`
MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
ALTER TABLE `games`
MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `guilds`
MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `players`
MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

ALTER TABLE `account_bans`
ADD CONSTRAINT `account_bans_ibfk_1` FOREIGN KEY (`ban_id`) REFERENCES `bans` (`id`) ON DELETE CASCADE,
ADD CONSTRAINT `account_bans_ibfk_2` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE;

ALTER TABLE `bans`
ADD CONSTRAINT `bans_ibfk_1` FOREIGN KEY (`admin_id`) REFERENCES `accounts` (`id`) ON DELETE SET NULL;

ALTER TABLE `ip_bans`
ADD CONSTRAINT `ip_bans_ibfk_1` FOREIGN KEY (`ban_id`) REFERENCES `bans` (`id`) ON DELETE CASCADE;
