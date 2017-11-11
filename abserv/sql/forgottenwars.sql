-- phpMyAdmin SQL Dump
-- version 4.2.12deb2+deb8u2
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Nov 07, 2017 at 08:38 AM
-- Server version: 5.5.58-0+deb8u1
-- PHP Version: 5.6.30-0+deb8u1

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
`id` int(10) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `password` varchar(255) NOT NULL,
  `email` varchar(255) NOT NULL DEFAULT '',
  `type` int(10) unsigned NOT NULL DEFAULT '1' COMMENT 'Account type',
  `blocked` tinyint(1) NOT NULL DEFAULT '0',
  `creation` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `accounts`
--

INSERT INTO `accounts` (`id`, `name`, `password`, `email`, `type`, `blocked`, `creation`) VALUES
(1, 'trill', '$2y$10$Hj00by/MFfilaVFshWCI7uBHkN9wDIh37pCLXrXtI/VQ1m9Hl9LNu', 'sa15@0x2a.wtf', 5, 0, 0);

-- --------------------------------------------------------

--
-- Table structure for table `account_bans`
--

CREATE TABLE IF NOT EXISTS `account_bans` (
  `ban_id` int(10) unsigned NOT NULL,
  `account_id` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `bans`
--

CREATE TABLE IF NOT EXISTS `bans` (
`id` int(10) unsigned NOT NULL,
  `expires` bigint(20) unsigned NOT NULL,
  `added` bigint(20) unsigned NOT NULL,
  `reason` int(5) NOT NULL DEFAULT '0',
  `active` tinyint(1) NOT NULL DEFAULT '0',
  `admin_id` int(10) unsigned DEFAULT NULL COMMENT 'account that issued the ban',
  `comment` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `guilds`
--

CREATE TABLE IF NOT EXISTS `guilds` (
`id` int(11) NOT NULL,
  `name` varchar(255) NOT NULL,
  `tag` varchar(4) NOT NULL,
  `ownerid` int(11) NOT NULL,
  `creation` bigint(20) NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `guilds`
--

INSERT INTO `guilds` (`id`, `name`, `tag`, `ownerid`, `creation`) VALUES
(1, 'Kreuz Retter', 'KR', 1, 0);

-- --------------------------------------------------------

--
-- Table structure for table `guild_invites`
--

CREATE TABLE IF NOT EXISTS `guild_invites` (
  `account_id` int(11) NOT NULL DEFAULT '0',
  `guild_id` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `guild_members`
--

CREATE TABLE IF NOT EXISTS `guild_members` (
  `account_id` int(11) unsigned NOT NULL,
  `guild_id` int(11) NOT NULL,
  `level` int(1) unsigned NOT NULL DEFAULT '1' COMMENT '0 = Guest, 1 = Member, 2 = Officer, 3 = Leader',
  `invited` bigint(20) NOT NULL DEFAULT '0',
  `expires` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `guild_members`
--

INSERT INTO `guild_members` (`account_id`, `guild_id`, `level`, `invited`, `expires`) VALUES
(1, 1, 3, 0, 0);

-- --------------------------------------------------------

--
-- Table structure for table `ip_bans`
--

CREATE TABLE IF NOT EXISTS `ip_bans` (
  `ban_id` int(10) unsigned NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mask` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `players`
--

CREATE TABLE IF NOT EXISTS `players` (
`id` int(11) NOT NULL,
  `name` varchar(255) NOT NULL,
  `account_id` int(11) NOT NULL DEFAULT '0',
  `level` int(11) NOT NULL DEFAULT '1',
  `experience` bigint(20) NOT NULL DEFAULT '0',
  `skillpoints` int(11) NOT NULL DEFAULT '0',
  `sex` int(11) NOT NULL DEFAULT '0',
  `lastlogin` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lastip` int(10) unsigned NOT NULL DEFAULT '0',
  `lastlogout` bigint(20) unsigned NOT NULL DEFAULT '0',
  `onlinetime` int(11) NOT NULL DEFAULT '0',
  `deleted` bigint(20) NOT NULL DEFAULT '0' COMMENT 'Time deleted or 0 for not deleted',
  `creation` bigint(20) NOT NULL DEFAULT '0',
  `last_map` varchar(255) DEFAULT NULL,
  `effects` blob NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `players`
--

INSERT INTO `players` (`id`, `name`, `account_id`, `level`, `experience`, `skillpoints`, `sex`, `lastlogin`, `lastip`, `lastlogout`, `onlinetime`, `deleted`, `creation`, `last_map`, `effects`) VALUES
(1, 'Schwester Trillian', 1, 1, 100, 0, 1, 0, 0, 0, 0, 0, 0, NULL, '');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `accounts`
--
ALTER TABLE `accounts`
 ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `name` (`name`);

--
-- Indexes for table `account_bans`
--
ALTER TABLE `account_bans`
 ADD KEY `ban_id` (`ban_id`), ADD KEY `account_id` (`account_id`);

--
-- Indexes for table `bans`
--
ALTER TABLE `bans`
 ADD PRIMARY KEY (`id`), ADD KEY `admin_id` (`admin_id`);

--
-- Indexes for table `guilds`
--
ALTER TABLE `guilds`
 ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `name` (`name`), ADD UNIQUE KEY `ownerid` (`ownerid`);

--
-- Indexes for table `guild_invites`
--
ALTER TABLE `guild_invites`
 ADD PRIMARY KEY (`account_id`,`guild_id`);

--
-- Indexes for table `guild_members`
--
ALTER TABLE `guild_members`
 ADD KEY `account_id` (`account_id`), ADD KEY `guild_id` (`guild_id`);

--
-- Indexes for table `ip_bans`
--
ALTER TABLE `ip_bans`
 ADD KEY `ban_id` (`ban_id`);

--
-- Indexes for table `players`
--
ALTER TABLE `players`
 ADD PRIMARY KEY (`id`), ADD UNIQUE KEY `name` (`name`), ADD KEY `account_id` (`account_id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `accounts`
--
ALTER TABLE `accounts`
MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `bans`
--
ALTER TABLE `bans`
MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `guilds`
--
ALTER TABLE `guilds`
MODIFY `id` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `players`
--
ALTER TABLE `players`
MODIFY `id` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;
--
-- Constraints for dumped tables
--

--
-- Constraints for table `account_bans`
--
ALTER TABLE `account_bans`
ADD CONSTRAINT `account_bans_ibfk_1` FOREIGN KEY (`ban_id`) REFERENCES `bans` (`id`) ON DELETE CASCADE,
ADD CONSTRAINT `account_bans_ibfk_2` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE;

--
-- Constraints for table `bans`
--
ALTER TABLE `bans`
ADD CONSTRAINT `bans_ibfk_1` FOREIGN KEY (`admin_id`) REFERENCES `accounts` (`id`) ON DELETE SET NULL;

--
-- Constraints for table `ip_bans`
--
ALTER TABLE `ip_bans`
ADD CONSTRAINT `ip_bans_ibfk_1` FOREIGN KEY (`ban_id`) REFERENCES `bans` (`id`) ON DELETE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
