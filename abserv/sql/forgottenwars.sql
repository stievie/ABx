-- MySQL dump 10.13  Distrib 5.5.60, for debian-linux-gnu (armv7l)
--
-- Host: localhost    Database: forgottenwars
-- ------------------------------------------------------
-- Server version	5.5.60-0+deb8u1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `forgottenwars`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `forgottenwars` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `forgottenwars`;

--
-- Table structure for table `account_account_keys`
--

DROP TABLE IF EXISTS `account_account_keys`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `account_account_keys` (
  `account_uuid` char(36) NOT NULL,
  `account_key_uuid` char(36) NOT NULL,
  UNIQUE KEY `account_uuid` (`account_uuid`,`account_key_uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_bans`
--

DROP TABLE IF EXISTS `account_bans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `account_bans` (
  `uuid` char(36) NOT NULL,
  `ban_uuid` char(36) NOT NULL,
  `account_uuid` char(36) NOT NULL,
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `ban_uuid` (`ban_uuid`),
  KEY `account_uuid` (`account_uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `account_keys`
--

DROP TABLE IF EXISTS `account_keys`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `account_keys` (
  `uuid` char(36) NOT NULL,
  `used` int(10) NOT NULL DEFAULT '0',
  `total` int(10) NOT NULL DEFAULT '1',
  `description` varchar(255) DEFAULT NULL,
  `status` tinyint(1) NOT NULL DEFAULT '0' COMMENT '0 = unknown, 1 = not activated, 2 = ready for use, 3 = banned',
  `key_type` int(2) NOT NULL DEFAULT '0' COMMENT '0 = unknown, 1 = account, 2 char slot',
  `email` varchar(60) NOT NULL DEFAULT '',
  PRIMARY KEY (`uuid`),
  KEY `status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Temporary table structure for view `account_view`
--

DROP TABLE IF EXISTS `account_view`;
/*!50001 DROP VIEW IF EXISTS `account_view`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `account_view` (
  `uuid` tinyint NOT NULL,
  `name` tinyint NOT NULL,
  `password` tinyint NOT NULL,
  `email` tinyint NOT NULL,
  `type` tinyint NOT NULL,
  `creation` tinyint NOT NULL,
  `titles` tinyint NOT NULL,
  `account_key` tinyint NOT NULL,
  `used` tinyint NOT NULL,
  `total` tinyint NOT NULL,
  `keys_left` tinyint NOT NULL,
  `status` tinyint NOT NULL,
  `description` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `accounts`
--

DROP TABLE IF EXISTS `accounts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `accounts` (
  `uuid` char(36) NOT NULL DEFAULT '',
  `name` varchar(32) NOT NULL,
  `password` varchar(255) NOT NULL,
  `email` varchar(255) NOT NULL DEFAULT '',
  `type` int(10) unsigned NOT NULL DEFAULT '1' COMMENT 'Account type',
  `status` int(10) NOT NULL DEFAULT '0',
  `creation` bigint(20) NOT NULL DEFAULT '0',
  `char_slots` int(10) NOT NULL DEFAULT '6',
  `last_character_uuid` char(36) NOT NULL DEFAULT '',
  `titles` blob,
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bans`
--

DROP TABLE IF EXISTS `bans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bans` (
  `uuid` char(36) NOT NULL,
  `expires` bigint(20) unsigned NOT NULL,
  `added` bigint(20) unsigned NOT NULL,
  `reason` int(5) NOT NULL DEFAULT '0',
  `active` tinyint(1) NOT NULL DEFAULT '0',
  `admin_uuid` char(36) NOT NULL,
  `comment` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`uuid`),
  KEY `admin_uuid` (`admin_uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `friend_list`
--

DROP TABLE IF EXISTS `friend_list`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `friend_list` (
  `account_uuid` char(36) NOT NULL,
  `friend_uuid` char(36) NOT NULL COMMENT 'UUID of friend account',
  `relation` int(1) NOT NULL COMMENT '1 = Friend, 2 = Ignore',
  UNIQUE KEY `account_uuid` (`account_uuid`,`friend_uuid`) COMMENT 'You can befriend/ignore a preson only once'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `games`
--

DROP TABLE IF EXISTS `games`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `games` (
  `uuid` char(36) NOT NULL,
  `name` varchar(32) NOT NULL,
  `type` int(10) NOT NULL,
  `directory` varchar(255) NOT NULL,
  `script_file` varchar(255) NOT NULL,
  `landing` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guild_members`
--

DROP TABLE IF EXISTS `guild_members`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `guild_members` (
  `account_uuid` char(36) DEFAULT NULL,
  `guild_uuid` char(36) DEFAULT NULL,
  `role` int(1) unsigned NOT NULL DEFAULT '1' COMMENT '0 = Unknown, 1 = Guest, 2 = Invited, 3 = Member, 4 = Officer, 5 = Leader',
  `invited` bigint(20) NOT NULL DEFAULT '0',
  `joined` int(20) NOT NULL DEFAULT '0',
  `expires` bigint(20) NOT NULL DEFAULT '0',
  KEY `guild_uuid` (`guild_uuid`),
  KEY `account_uuid` (`account_uuid`) COMMENT 'An account can be a member of one guild and be guest of other guilds'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guilds`
--

DROP TABLE IF EXISTS `guilds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `guilds` (
  `uuid` char(36) NOT NULL DEFAULT '',
  `name` varchar(32) NOT NULL,
  `tag` varchar(4) NOT NULL,
  `creator_account_uuid` char(36) NOT NULL,
  `creation` bigint(20) NOT NULL,
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ip_bans`
--

DROP TABLE IF EXISTS `ip_bans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ip_bans` (
  `uuid` char(36) NOT NULL,
  `ban_uuid` char(36) NOT NULL,
  `ip` int(10) unsigned NOT NULL,
  `mask` int(10) unsigned NOT NULL,
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `ip_2` (`ip`,`mask`),
  KEY `ban_uuid` (`ban_uuid`),
  KEY `ip` (`ip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `mails`
--

DROP TABLE IF EXISTS `mails`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mails` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `from_player_id` int(11) unsigned NOT NULL,
  `from_account_id` int(11) unsigned NOT NULL,
  `to_player_id` int(11) unsigned NOT NULL,
  `to_account_id` int(11) unsigned NOT NULL,
  `subject` varchar(63) DEFAULT NULL,
  `message` varchar(255) DEFAULT NULL,
  `is_read` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `is_read` (`is_read`),
  KEY `from_player_id` (`from_player_id`),
  KEY `to_player_id` (`to_player_id`),
  KEY `from_account_id` (`from_account_id`),
  KEY `to_account_id` (`to_account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `players`
--

DROP TABLE IF EXISTS `players`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `players` (
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
  `lastip` int(10) unsigned NOT NULL DEFAULT '0',
  `lastlogout` bigint(20) unsigned NOT NULL DEFAULT '0',
  `onlinetime` int(20) NOT NULL DEFAULT '0' COMMENT 'Online time in seconds',
  `deleted` bigint(20) NOT NULL DEFAULT '0' COMMENT 'Time deleted or 0 for not deleted',
  `creation` bigint(20) NOT NULL DEFAULT '0',
  `last_map` varchar(50) NOT NULL DEFAULT '',
  `effects` blob COMMENT 'Persistent effects',
  `skills` blob,
  `equipment` blob,
  `titles` blob,
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `name` (`name`),
  KEY `account_uuid` (`account_uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `storage`
--

DROP TABLE IF EXISTS `storage`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `storage` (
  `uuid` char(36) NOT NULL,
  `owner_uuid` char(36) NOT NULL,
  `item_uuid` char(36) NOT NULL,
  `location` int(1) NOT NULL COMMENT '0 = Player, 1 = Account',
  `count` int(10) NOT NULL DEFAULT '1' COMMENT 'For stackable items',
  `position` int(10) NOT NULL,
  PRIMARY KEY (`uuid`),
  KEY `location` (`location`),
  KEY `owner_uuid` (`owner_uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Current Database: `forgottenwars`
--

USE `forgottenwars`;

--
-- Final view structure for view `account_view`
--

/*!50001 DROP TABLE IF EXISTS `account_view`*/;
/*!50001 DROP VIEW IF EXISTS `account_view`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `account_view` AS (select `accounts`.`uuid` AS `uuid`,`accounts`.`name` AS `name`,`accounts`.`password` AS `password`,`accounts`.`email` AS `email`,`accounts`.`type` AS `type`,`accounts`.`creation` AS `creation`,`accounts`.`titles` AS `titles`,`account_keys`.`uuid` AS `account_key`,`account_keys`.`used` AS `used`,`account_keys`.`total` AS `total`,(`account_keys`.`total` - `account_keys`.`used`) AS `keys_left`,`account_keys`.`status` AS `status`,`account_keys`.`description` AS `description` from (`accounts` left join (`account_account_keys` join `account_keys`) on(((`account_account_keys`.`account_uuid` = `accounts`.`uuid`) and (`account_account_keys`.`account_key_uuid` = `account_keys`.`uuid`))))) */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-04-29  3:59:15
