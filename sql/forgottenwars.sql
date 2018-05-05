-- phpMyAdmin SQL Dump
-- version 4.2.12deb2+deb8u2
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: May 05, 2018 at 09:26 AM
-- Server version: 5.5.60-0+deb8u1
-- PHP Version: 5.6.33-0+deb8u1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `forgottenwars`
--

-- --------------------------------------------------------

--
-- Table structure for table `accounts`
--

CREATE TABLE IF NOT EXISTS `accounts` (
  `uuid` char(36) NOT NULL DEFAULT '',
  `name` varchar(32) NOT NULL,
  `password` varchar(255) NOT NULL,
  `email` varchar(255) NOT NULL DEFAULT '',
  `type` int(10) unsigned NOT NULL DEFAULT '1' COMMENT 'Account type',
  `status` int(10) NOT NULL DEFAULT '0',
  `creation` bigint(20) NOT NULL DEFAULT '0',
  `char_slots` int(10) NOT NULL DEFAULT '6',
  `last_character_uuid` char(36) NOT NULL DEFAULT '',
  `titles` blob
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `account_account_keys`
--

CREATE TABLE IF NOT EXISTS `account_account_keys` (
  `account_uuid` char(36) NOT NULL,
  `account_key_uuid` char(36) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `account_bans`
--

CREATE TABLE IF NOT EXISTS `account_bans` (
  `uuid` char(36) NOT NULL,
  `ban_uuid` char(36) NOT NULL,
  `account_uuid` char(36) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `account_keys`
--

CREATE TABLE IF NOT EXISTS `account_keys` (
  `uuid` char(36) NOT NULL,
  `used` int(10) NOT NULL DEFAULT '0',
  `total` int(10) NOT NULL DEFAULT '1',
  `description` varchar(255) DEFAULT NULL,
  `status` tinyint(1) NOT NULL DEFAULT '0' COMMENT '0 = unknown, 1 = not activated, 2 = ready for use, 3 = banned',
  `key_type` int(2) NOT NULL DEFAULT '0' COMMENT '0 = unknown, 1 = account, 2 char slot',
  `email` varchar(60) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Stand-in structure for view `account_view`
--
CREATE TABLE IF NOT EXISTS `account_view` (
`uuid` char(36)
,`name` varchar(32)
,`password` varchar(255)
,`email` varchar(255)
,`type` int(10) unsigned
,`creation` bigint(20)
,`titles` blob
,`account_key` char(36)
,`used` int(10)
,`total` int(10)
,`keys_left` bigint(12)
,`status` tinyint(1)
,`description` varchar(255)
);
-- --------------------------------------------------------

--
-- Table structure for table `bans`
--

CREATE TABLE IF NOT EXISTS `bans` (
  `uuid` char(36) NOT NULL,
  `expires` bigint(20) unsigned NOT NULL,
  `added` bigint(20) unsigned NOT NULL,
  `reason` int(5) NOT NULL DEFAULT '0',
  `active` tinyint(1) NOT NULL DEFAULT '0',
  `admin_uuid` char(36) NOT NULL,
  `comment` varchar(255) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `friend_list`
--

CREATE TABLE IF NOT EXISTS `friend_list` (
  `account_uuid` char(36) NOT NULL,
  `friend_uuid` char(36) NOT NULL COMMENT 'UUID of friend account',
  `relation` int(1) NOT NULL COMMENT '1 = Friend, 2 = Ignore'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `games`
--

CREATE TABLE IF NOT EXISTS `games` (
  `uuid` char(36) NOT NULL,
  `name` varchar(32) NOT NULL,
  `type` int(10) NOT NULL,
  `directory` varchar(255) NOT NULL,
  `script_file` varchar(255) NOT NULL,
  `landing` tinyint(1) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `guilds`
--

CREATE TABLE IF NOT EXISTS `guilds` (
  `uuid` char(36) NOT NULL DEFAULT '',
  `name` varchar(32) NOT NULL,
  `tag` varchar(4) NOT NULL,
  `creator_account_uuid` char(36) NOT NULL,
  `creation` bigint(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `guild_members`
--

CREATE TABLE IF NOT EXISTS `guild_members` (
  `account_uuid` char(36) DEFAULT NULL,
  `guild_uuid` char(36) DEFAULT NULL,
  `role` int(1) unsigned NOT NULL DEFAULT '1' COMMENT '0 = Unknown, 1 = Guest, 2 = Invited, 3 = Member, 4 = Officer, 5 = Leader',
  `invited` bigint(20) NOT NULL DEFAULT '0',
  `joined` int(20) NOT NULL DEFAULT '0',
  `expires` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ip_bans`
--

CREATE TABLE IF NOT EXISTS `ip_bans` (
  `uuid` char(36) NOT NULL,
  `ban_uuid` char(36) NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mask` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mails`
--

CREATE TABLE IF NOT EXISTS `mails` (
  `uuid` char(36) NOT NULL,
  `from_account_uuid` char(36) NOT NULL,
  `to_account_uuid` char(36) NOT NULL,
  `from_name` char(20) NOT NULL DEFAULT '',
  `to_name` char(20) NOT NULL DEFAULT '',
  `subject` varchar(60) NOT NULL DEFAULT '',
  `message` varchar(255) NOT NULL DEFAULT '',
  `created` bigint(20) NOT NULL DEFAULT '0',
  `is_read` tinyint(1) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `players`
--

CREATE TABLE IF NOT EXISTS `players` (
  `uuid` char(36) NOT NULL DEFAULT '',
  `profession` char(2) NOT NULL,
  `profession2` char(2) NOT NULL DEFAULT '',
  `name` char(20) NOT NULL,
  `pvp` tinyint(1) NOT NULL DEFAULT '0',
  `account_uuid` char(36) NOT NULL,
  `level` int(11) NOT NULL DEFAULT '1',
  `experience` bigint(20) NOT NULL DEFAULT '100',
  `skillpoints` int(11) NOT NULL DEFAULT '0',
  `sex` int(11) NOT NULL DEFAULT '0',
  `lastlogin` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lastlogout` bigint(20) unsigned NOT NULL DEFAULT '0',
  `onlinetime` int(20) NOT NULL DEFAULT '0' COMMENT 'Online time in seconds',
  `deleted` bigint(20) NOT NULL DEFAULT '0' COMMENT 'Time deleted or 0 for not deleted',
  `creation` bigint(20) NOT NULL DEFAULT '0',
  `last_map_uuid` char(36) NOT NULL DEFAULT '',
  `effects` blob COMMENT 'Persistent effects',
  `skills` blob,
  `equipment` blob,
  `titles` blob
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `storage`
--

CREATE TABLE IF NOT EXISTS `storage` (
  `uuid` char(36) NOT NULL,
  `owner_uuid` char(36) NOT NULL,
  `item_uuid` char(36) NOT NULL,
  `location` int(1) NOT NULL COMMENT '0 = Player, 1 = Account',
  `count` int(10) NOT NULL DEFAULT '1' COMMENT 'For stackable items',
  `position` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Structure for view `account_view`
--
DROP TABLE IF EXISTS `account_view`;

CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `account_view` AS (select `accounts`.`uuid` AS `uuid`,`accounts`.`name` AS `name`,`accounts`.`password` AS `password`,`accounts`.`email` AS `email`,`accounts`.`type` AS `type`,`accounts`.`creation` AS `creation`,`accounts`.`titles` AS `titles`,`account_keys`.`uuid` AS `account_key`,`account_keys`.`used` AS `used`,`account_keys`.`total` AS `total`,(`account_keys`.`total` - `account_keys`.`used`) AS `keys_left`,`account_keys`.`status` AS `status`,`account_keys`.`description` AS `description` from (`accounts` left join (`account_account_keys` join `account_keys`) on(((`account_account_keys`.`account_uuid` = `accounts`.`uuid`) and (`account_account_keys`.`account_key_uuid` = `account_keys`.`uuid`)))));

--
-- Indexes for dumped tables
--

--
-- Indexes for table `accounts`
--
ALTER TABLE `accounts`
 ADD PRIMARY KEY (`uuid`), ADD UNIQUE KEY `name` (`name`);

--
-- Indexes for table `account_account_keys`
--
ALTER TABLE `account_account_keys`
 ADD UNIQUE KEY `account_uuid` (`account_uuid`,`account_key_uuid`);

--
-- Indexes for table `account_bans`
--
ALTER TABLE `account_bans`
 ADD PRIMARY KEY (`uuid`), ADD UNIQUE KEY `ban_uuid` (`ban_uuid`), ADD KEY `account_uuid` (`account_uuid`);

--
-- Indexes for table `account_keys`
--
ALTER TABLE `account_keys`
 ADD PRIMARY KEY (`uuid`), ADD KEY `status` (`status`);

--
-- Indexes for table `bans`
--
ALTER TABLE `bans`
 ADD PRIMARY KEY (`uuid`), ADD KEY `admin_uuid` (`admin_uuid`);

--
-- Indexes for table `friend_list`
--
ALTER TABLE `friend_list`
 ADD UNIQUE KEY `account_uuid` (`account_uuid`,`friend_uuid`) COMMENT 'You can befriend/ignore a preson only once';

--
-- Indexes for table `games`
--
ALTER TABLE `games`
 ADD PRIMARY KEY (`uuid`), ADD UNIQUE KEY `name` (`name`);

--
-- Indexes for table `guilds`
--
ALTER TABLE `guilds`
 ADD PRIMARY KEY (`uuid`), ADD UNIQUE KEY `name` (`name`);

--
-- Indexes for table `guild_members`
--
ALTER TABLE `guild_members`
 ADD KEY `guild_uuid` (`guild_uuid`), ADD KEY `account_uuid` (`account_uuid`) COMMENT 'An account can be a member of one guild and be guest of other guilds';

--
-- Indexes for table `ip_bans`
--
ALTER TABLE `ip_bans`
 ADD PRIMARY KEY (`uuid`), ADD UNIQUE KEY `ip_2` (`ip`,`mask`), ADD KEY `ban_uuid` (`ban_uuid`), ADD KEY `ip` (`ip`);

--
-- Indexes for table `mails`
--
ALTER TABLE `mails`
 ADD PRIMARY KEY (`uuid`), ADD KEY `is_read` (`is_read`), ADD KEY `from_account_uuid` (`from_account_uuid`), ADD KEY `to_account_uuid` (`to_account_uuid`), ADD KEY `created` (`created`);

--
-- Indexes for table `players`
--
ALTER TABLE `players`
 ADD PRIMARY KEY (`uuid`), ADD UNIQUE KEY `name` (`name`), ADD KEY `account_uuid` (`account_uuid`);

--
-- Indexes for table `storage`
--
ALTER TABLE `storage`
 ADD PRIMARY KEY (`uuid`), ADD KEY `location` (`location`), ADD KEY `owner_uuid` (`owner_uuid`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
