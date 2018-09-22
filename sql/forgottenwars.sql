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
-- Dumping data for table `account_account_keys`
--

LOCK TABLES `account_account_keys` WRITE;
/*!40000 ALTER TABLE `account_account_keys` DISABLE KEYS */;
INSERT INTO `account_account_keys` VALUES ('16154f1a-dd92-42de-88a1-3fd5c0c0f87b','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('1b173096-5ec7-4b34-b5c4-54f3e2976c21','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('464cedf5-f62e-4dd9-bf1f-c741fc4c8878','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('6ead02f7-b7d2-4c91-be89-7131b1e183d3','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('7c9947c1-438b-11e8-980f-02100700d6f0','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('7c994ff2-438b-11e8-980f-02100700d6f0','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('7c995347-438b-11e8-980f-02100700d6f0','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('7c995634-438b-11e8-980f-02100700d6f0','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('7c9958ce-438b-11e8-980f-02100700d6f0','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('7c995b5c-438b-11e8-980f-02100700d6f0','9d637cd2-4aa2-4f41-8264-8b4c0d9ed5eb'),('7c995dd6-438b-11e8-980f-02100700d6f0','28cd49c7-5c83-4409-8cd0-fc4d8038597f'),('7c996067-438b-11e8-980f-02100700d6f0','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('7c9962e7-438b-11e8-980f-02100700d6f0','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('7c99657b-438b-11e8-980f-02100700d6f0','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('7c996d4d-438b-11e8-980f-02100700d6f0','28cd49c7-5c83-4409-8cd0-fc4d8038597f'),('b90acfa1-7e8a-4045-915a-864ac70ea293','1f0c324a-6eab-4071-b50b-bfdd3736eabf'),('ca3366ad-d5ba-4ada-9cda-9d060265adce','1f0c324a-6eab-4071-b50b-bfdd3736eabf');
/*!40000 ALTER TABLE `account_account_keys` ENABLE KEYS */;
UNLOCK TABLES;

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
-- Dumping data for table `account_bans`
--

LOCK TABLES `account_bans` WRITE;
/*!40000 ALTER TABLE `account_bans` DISABLE KEYS */;
/*!40000 ALTER TABLE `account_bans` ENABLE KEYS */;
UNLOCK TABLES;

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
-- Dumping data for table `account_keys`
--

LOCK TABLES `account_keys` WRITE;
/*!40000 ALTER TABLE `account_keys` DISABLE KEYS */;
INSERT INTO `account_keys` VALUES ('1f0c324a-6eab-4071-b50b-bfdd3736eabf',14,100,'My private key',2,1,''),('28cd49c7-5c83-4409-8cd0-fc4d8038597f',2,20,'Friends',2,1,''),('9d637cd2-4aa2-4f41-8264-8b4c0d9ed5eb',1,100,'Beta Key',2,1,''),('ffa9902d-5044-11e8-a7ca-02100700d6f0',0,20,'Guild Wars',2,1,'');
/*!40000 ALTER TABLE `account_keys` ENABLE KEYS */;
UNLOCK TABLES;

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
  `current_character_uuid` char(36) NOT NULL DEFAULT '',
  `current_server_uuid` char(36) NOT NULL DEFAULT '',
  `online_status` int(1) NOT NULL DEFAULT '0' COMMENT '0 = offline, 1 = away, 2 = dnd, 3 = online',
  `titles` blob,
  `guild_uuid` char(36) NOT NULL DEFAULT '',
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `accounts`
--

LOCK TABLES `accounts` WRITE;
/*!40000 ALTER TABLE `accounts` DISABLE KEYS */;
INSERT INTO `accounts` VALUES ('16154f1a-dd92-42de-88a1-3fd5c0c0f87b','trill11','$2b$10$zFntt7hktatESA6PdKA8x.AaWq2Gtixpem/e1FElwZ973fLZy4oFq','sa@0x2a.wtf',1,1,1525514925466,6,'c9892ba2-b9b4-4c20-b974-ea1455dd076d','230a6dd3-907e-4193-87e5-a25789d68016',0,NULL,''),('1b173096-5ec7-4b34-b5c4-54f3e2976c21','stievie9','$2b$10$FpWnX.oivcE3XYWuSTfUZOmAsRhOSa2fB0rQeS3QodhbMOseDZIoa','sa@0x2a.wtf',1,1,1531635940850,6,'2deeb001-77d9-4254-a7af-fb671821b8fc','14935aef-7cd9-4495-b704-ec9578411558',0,NULL,''),('464cedf5-f62e-4dd9-bf1f-c741fc4c8878','trill33','$2b$10$cnzcgxvJG0Blup9b8mQ9wuKakzeTx3qvJFphwpvcYLmfailzymWJS','sa@0x2a.wtf',5,1,1524918429660,6,'c0ae886f-379d-401c-98a8-e4f60de2b2ad','14935aef-7cd9-4495-b704-ec9578411558',0,NULL,'60e72a18-4797-11e8-ad09-02100700d6f0'),('6ead02f7-b7d2-4c91-be89-7131b1e183d3','trill22','$2b$10$MR9ebGTzNJrahwtpUyYPbeCVa6wveB.Lc0gKYuilDtsi/z9IYmrli\0','test',1,1,1525515804685,6,'','230a6dd3-907e-4193-87e5-a25789d68016',0,NULL,''),('7c9947c1-438b-11e8-980f-02100700d6f0','trill','$2a$10$h9jSRZoB8MAY2z5mRqdkZO8tpdRTJF6xGJt7q.Zkc//8g5daOvCfG','sa@0x2a.wtf',5,1,0,10,'75d51395-4796-11e8-ad09-02100700d6f0','230a6dd3-907e-4193-87e5-a25789d68016',0,'','60e72a18-4797-11e8-ad09-02100700d6f0'),('7c994ff2-438b-11e8-980f-02100700d6f0','stievie','$2a$10$h9jSRZoB8MAY2z5mRqdkZO8tpdRTJF6xGJt7q.Zkc//8g5daOvCfG','sa15@0x2a.wtf',5,1,0,6,'75d51f7d-4796-11e8-ad09-02100700d6f0','230a6dd3-907e-4193-87e5-a25789d68016',0,'',''),('7c995347-438b-11e8-980f-02100700d6f0','stievie2','$2a$10$h9jSRZoB8MAY2z5mRqdkZO8tpdRTJF6xGJt7q.Zkc//8g5daOvCfG','sa15@0x2a.wtf',5,1,0,6,'75d522bc-4796-11e8-ad09-02100700d6f0','230a6dd3-907e-4193-87e5-a25789d68016',0,'',''),('7c995634-438b-11e8-980f-02100700d6f0','stievie3','$2b$10$yQLLnJ1uKfxofg6mrnMrbuijIpHeT92AOD0YWPsBisyXOkMpBCujC','sa@0x2a.wtf',5,1,1514101068701,6,'75d52e7f-4796-11e8-ad09-02100700d6f0','230a6dd3-907e-4193-87e5-a25789d68016',0,'','60e72a18-4797-11e8-ad09-02100700d6f0'),('7c9958ce-438b-11e8-980f-02100700d6f0','stievie4','$2b$10$ijH5T8UaCS4VpHChRlNMEOEPCfEQRL6MesZAB.khZXU7Um69/OYgS','sa@0x2a.wtf',1,1,1514101667268,6,'75d53165-4796-11e8-ad09-02100700d6f0','230a6dd3-907e-4193-87e5-a25789d68016',0,'',''),('7c995b5c-438b-11e8-980f-02100700d6f0','stieviebeta','$2b$10$oDtSlKRoJcjLXECOhLxoveTH8V920SxpQ5FVGBA.sTT9qTzrS6CP2\0','sa@0x2a.wtf',1,1,1514191570663,6,'','230a6dd3-907e-4193-87e5-a25789d68016',0,'',''),('7c995dd6-438b-11e8-980f-02100700d6f0','trill2','$2b$10$C//uD5g/S7cxJqFnnFQSp.djMUSKJb3qBCo1njJMM5G7P6Pe81/US','test@test.2',1,1,1514458016607,6,'75d52b91-4796-11e8-ad09-02100700d6f0','230a6dd3-907e-4193-87e5-a25789d68016',0,'',''),('7c996067-438b-11e8-980f-02100700d6f0','stievie5','$2b$10$cxCN6NEYVnuCoWGHMzRP/u5/CTKW6obielVtHby.wyWrPyBEaJeSy','sa@0x2a.wtf',1,1,1514972822039,6,'75d53721-4796-11e8-ad09-02100700d6f0','230a6dd3-907e-4193-87e5-a25789d68016',0,'',''),('7c9962e7-438b-11e8-980f-02100700d6f0','stievie6','$2b$10$aIABfDa3dut4NmUncgKFMut5DZPQaXXIStf1b.AShxdzt19NO1Ymq','sa@0x2a.wtf',1,1,1514972937220,6,'75d53a02-4796-11e8-ad09-02100700d6f0','230a6dd3-907e-4193-87e5-a25789d68016',0,'',''),('7c99657b-438b-11e8-980f-02100700d6f0','stievie7','$2b$10$eaXZ0mYIgLNcJwU9mBR3r.FG2ASqji5dp5.3gh6nX8bJf7BHYZQzW','sa@0x2a.wtf',1,1,1515226744469,6,'75d543a3-4796-11e8-ad09-02100700d6f0','230a6dd3-907e-4193-87e5-a25789d68016',0,'',''),('7c996d4d-438b-11e8-980f-02100700d6f0','melonenpfluecker','$2b$10$ervH2vnOqpK2LiPXha7Esuw9/6hblgn2Bz17xu38VeOyuhoAoA61m\0','tom@10101.at',1,1,1517575196237,6,'','230a6dd3-907e-4193-87e5-a25789d68016',0,'',''),('b90acfa1-7e8a-4045-915a-864ac70ea293','stievie8','$2b$10$2VsQcHktlMeAjXvvh9Mf4uiF1A8xSgG7Q1/Pogat7o2GAn8Zpj8W6','sa@0x2a.wtf',1,1,1525407568919,6,'c503b2f7-883e-488b-b5d9-3e87f2df35df','230a6dd3-907e-4193-87e5-a25789d68016',0,NULL,''),('ca3366ad-d5ba-4ada-9cda-9d060265adce','trill00','$2b$10$ZzUVCcyavQdvwT5ziwa2eOeYASVQn9tF52M1arwBJmU2xnvCHFIUu','sa@0x2a.wtf',1,1,1525530305247,6,'b47c561d-597d-4f1a-bcd6-105686216153','230a6dd3-907e-4193-87e5-a25789d68016',0,NULL,'');
/*!40000 ALTER TABLE `accounts` ENABLE KEYS */;
UNLOCK TABLES;

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
-- Dumping data for table `bans`
--

LOCK TABLES `bans` WRITE;
/*!40000 ALTER TABLE `bans` DISABLE KEYS */;
/*!40000 ALTER TABLE `bans` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `friend_list`
--

DROP TABLE IF EXISTS `friend_list`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `friend_list` (
  `account_uuid` char(36) NOT NULL,
  `friend_uuid` char(36) NOT NULL COMMENT 'UUID of friend account',
  `friend_name` varchar(20) NOT NULL DEFAULT '' COMMENT 'Befriended with this name',
  `relation` int(1) NOT NULL COMMENT '1 = Friend, 2 = Ignore',
  UNIQUE KEY `account_uuid` (`account_uuid`,`friend_uuid`) COMMENT 'You can befriend/ignore a preson only once'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `friend_list`
--

LOCK TABLES `friend_list` WRITE;
/*!40000 ALTER TABLE `friend_list` DISABLE KEYS */;
/*!40000 ALTER TABLE `friend_list` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_attributes`
--

DROP TABLE IF EXISTS `game_attributes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `game_attributes` (
  `uuid` char(36) NOT NULL,
  `idx` int(10) unsigned NOT NULL,
  `profession_uuid` char(36) NOT NULL,
  `name` varchar(255) NOT NULL,
  `is_primary` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `idx` (`idx`),
  KEY `profession_uuid` (`profession_uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `game_attributes`
--

LOCK TABLES `game_attributes` WRITE;
/*!40000 ALTER TABLE `game_attributes` DISABLE KEYS */;
INSERT INTO `game_attributes` VALUES ('0739ed7d-50f6-11e8-a7ca-02100700d6f0',12,'85d0ef81-50f4-11e8-a7ca-02100700d6f0','Energy Storage',1),('073a57c1-50f6-11e8-a7ca-02100700d6f0',9,'85d0ef81-50f4-11e8-a7ca-02100700d6f0','Earth Magic',0),('0ff5523b-50f5-11e8-a7ca-02100700d6f0',19,'59f4493d-50f4-11e8-a7ca-02100700d6f0','Hammer Mastery',0),('0ff5b6cc-50f5-11e8-a7ca-02100700d6f0',20,'59f4493d-50f4-11e8-a7ca-02100700d6f0','Swordsmanship',0),('13582547-50f6-11e8-a7ca-02100700d6f0',10,'85d0ef81-50f4-11e8-a7ca-02100700d6f0','Fire Magic',0),('13598bc5-50f6-11e8-a7ca-02100700d6f0',8,'85d0ef81-50f4-11e8-a7ca-02100700d6f0','Air Magic',0),('18c57862-50f5-11e8-a7ca-02100700d6f0',21,'59f4493d-50f4-11e8-a7ca-02100700d6f0','Tactics',0),('1a19c3d5-50f6-11e8-a7ca-02100700d6f0',11,'85d0ef81-50f4-11e8-a7ca-02100700d6f0','Water Magic',0),('4e8d25fe-50f7-11e8-a7ca-02100700d6f0',99,'00000000-0000-0000-0000-000000000000','None',0),('536a55fe-50f5-11e8-a7ca-02100700d6f0',23,'59f4b30b-50f4-11e8-a7ca-02100700d6f0','Expertise',1),('536aba10-50f5-11e8-a7ca-02100700d6f0',25,'59f4b30b-50f4-11e8-a7ca-02100700d6f0','Markmanship',0),('67bf90ba-50f5-11e8-a7ca-02100700d6f0',22,'59f4b30b-50f4-11e8-a7ca-02100700d6f0','Beast Mastery',0),('67bffc43-50f5-11e8-a7ca-02100700d6f0',24,'59f4b30b-50f4-11e8-a7ca-02100700d6f0','Wilderness Survival',0),('9f9162c0-50f5-11e8-a7ca-02100700d6f0',16,'73156b15-50f4-11e8-a7ca-02100700d6f0','Divine Favor',1),('9f91d5ba-50f5-11e8-a7ca-02100700d6f0',13,'73156b15-50f4-11e8-a7ca-02100700d6f0','Healing Prayers',0),('b5a203ec-50f5-11e8-a7ca-02100700d6f0',14,'73156b15-50f4-11e8-a7ca-02100700d6f0','Smiting Prayers',0),('b5a2673e-50f5-11e8-a7ca-02100700d6f0',15,'73156b15-50f4-11e8-a7ca-02100700d6f0','Protection Prayers',0),('c5d4d491-50f5-11e8-a7ca-02100700d6f0',6,'7315cbd6-50f4-11e8-a7ca-02100700d6f0','Soul Reaping',1),('c5d5205c-50f5-11e8-a7ca-02100700d6f0',4,'7315cbd6-50f4-11e8-a7ca-02100700d6f0','Blood Magic',0),('d1e4b914-50f5-11e8-a7ca-02100700d6f0',7,'7315cbd6-50f4-11e8-a7ca-02100700d6f0','Curses',0),('d1e52aa1-50f5-11e8-a7ca-02100700d6f0',5,'7315cbd6-50f4-11e8-a7ca-02100700d6f0','Death Magic',0),('e8e10cb8-50f5-11e8-a7ca-02100700d6f0',0,'85d0939b-50f4-11e8-a7ca-02100700d6f0','Fast Casting',1),('e8e16f7e-50f5-11e8-a7ca-02100700d6f0',2,'85d0939b-50f4-11e8-a7ca-02100700d6f0','Domination Magic',0),('f74719b7-50f5-11e8-a7ca-02100700d6f0',1,'85d0939b-50f4-11e8-a7ca-02100700d6f0','Illusion Magic',0),('f7477bbe-50f5-11e8-a7ca-02100700d6f0',3,'85d0939b-50f4-11e8-a7ca-02100700d6f0','Inspiration Magic',0),('fcb26454-50f4-11e8-a7ca-02100700d6f0',17,'59f4493d-50f4-11e8-a7ca-02100700d6f0','Strength',1),('fcb2cbfc-50f4-11e8-a7ca-02100700d6f0',18,'59f4493d-50f4-11e8-a7ca-02100700d6f0','Axe Mastery',0);
/*!40000 ALTER TABLE `game_attributes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_effects`
--

DROP TABLE IF EXISTS `game_effects`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `game_effects` (
  `uuid` char(36) NOT NULL,
  `idx` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(63) NOT NULL DEFAULT '',
  `category` int(10) NOT NULL DEFAULT '0',
  `script` varchar(260) NOT NULL,
  `icon` varchar(260) NOT NULL DEFAULT '',
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `idx` (`idx`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `game_effects`
--

LOCK TABLES `game_effects` WRITE;
/*!40000 ALTER TABLE `game_effects` DISABLE KEYS */;
INSERT INTO `game_effects` VALUES ('62781eff-5286-11e8-a7ca-02100700d6f0',1001,'Morale',20,'/scripts/effects/general/morale.lua',''),('827f837a-5113-11e8-a7ca-02100700d6f0',1000,'PvP',254,'/scripts/effects/environment/pvp.lua',''),('827fe642-5113-11e8-a7ca-02100700d6f0',10,'Patient Spirit',2,'/scripts/effects/enchantment/patient_spirit.lua',''),('fa78edb8-528b-11e8-a7ca-02100700d6f0',6,'Mantra of Earth',7,'/scripts/effects/stance/mantra_of_earth.lua','');
/*!40000 ALTER TABLE `game_effects` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_items`
--

DROP TABLE IF EXISTS `game_items`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `game_items` (
  `uuid` char(36) NOT NULL,
  `idx` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL DEFAULT '',
  `schript_file` varchar(260) NOT NULL DEFAULT '',
  `server_icon_file` varchar(260) NOT NULL DEFAULT '',
  `server_model_file` varchar(260) NOT NULL DEFAULT '',
  `client_model_file` varchar(260) NOT NULL DEFAULT '',
  `client_icon_file` varchar(260) NOT NULL DEFAULT '',
  `type` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `idx` (`idx`),
  KEY `type` (`type`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `game_items`
--

LOCK TABLES `game_items` WRITE;
/*!40000 ALTER TABLE `game_items` DISABLE KEYS */;
INSERT INTO `game_items` VALUES ('010d902b-93f4-11e8-a7ca-02100700d6f0',7,'PC Female Elementarist Body 1','','','','Objects/PC_Human_E_Female1_Base.xml','',1),('3092b8ef-9250-11e8-a7ca-02100700d6f0',4,'PC Female Warrior Body 1','','','','Objects/PC_Human_W_Female1_Base.xml','',1),('3f57f41d-924b-11e8-a7ca-02100700d6f0',3,'PC Female Mesmer Body 1','','','','Objects/PC_Human_Me_Female1_Base.xml','',1),('6c6438a1-9319-11e8-a7ca-02100700d6f0',6,'PC Male Warrior Body 1','','','','Objects/PC_Human_W_Male1_Base.xml','',1),('7003a93f-b01c-11e8-a7ca-02100700d6f0',10,'NPC Pedestrian 1','','','','Objects/NPC_PedestrianFemale1_Base.xml','',1),('96254a1e-9958-11e8-a7ca-02100700d6f0',9,'NPC Merchant','','','','Objects/NPC_Merchant_Base.xml','',1),('9f7e64d1-9319-11e8-a7ca-02100700d6f0',2,'PC Male Monk Body 1','','','','Objects/PC_Human_Mo_Male1_Base.xml','',1),('c5d0a104-b588-11e8-a7ca-02100700d6f0',11,'Portal','','','','Objects/Portal.xml','',4),('d159a500-8f0c-11e8-a7ca-02100700d6f0',1,'PC Female Monk Body 1','','','','Objects/PC_Human_Mo_Female1_Base.xml','',1),('d9686531-9259-11e8-a7ca-02100700d6f0',5,'NPC Smith 1','','','','Objects/NPC_Smith1_Base.xml','',1),('fa5d7b1b-987a-11e8-a7ca-02100700d6f0',8,'Hair Female Long blonde','','','','Objects/Hair_Female_LongBlonde.xml','',2);
/*!40000 ALTER TABLE `game_items` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_maps`
--

DROP TABLE IF EXISTS `game_maps`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `game_maps` (
  `uuid` char(36) NOT NULL,
  `name` varchar(32) NOT NULL,
  `type` int(10) NOT NULL,
  `directory` varchar(255) NOT NULL,
  `script_file` varchar(255) NOT NULL,
  `landing` tinyint(1) DEFAULT '0',
  `party_size` int(10) unsigned NOT NULL DEFAULT '0',
  `map_coord_x` int(11) NOT NULL DEFAULT '0',
  `map_coord_y` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `game_maps`
--

LOCK TABLES `game_maps` WRITE;
/*!40000 ALTER TABLE `game_maps` DISABLE KEYS */;
INSERT INTO `game_maps` VALUES ('75e3dfcf-479a-11e8-ad09-02100700d6f0','Temple of Athene',1,'/maps/temple_of_athene','/scripts/games/temple_of_athene.lua',0,4,220,477),('75e3eeb5-479a-11e8-ad09-02100700d6f0','Temple of Hephaistos',1,'/maps/temple_of_hephaistos','/scripts/games/temple_of_hephaistos.lua',1,4,360,420),('75e3f5c3-479a-11e8-ad09-02100700d6f0','Highlands',1,'/maps/highlands','/scripts/games/highlands.lua',0,4,160,245),('75e3fc5c-479a-11e8-ad09-02100700d6f0','Rhodes',1,'/maps/rhodes','/scripts/games/rhodes.lua',0,4,840,620);
/*!40000 ALTER TABLE `game_maps` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_professions`
--

DROP TABLE IF EXISTS `game_professions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `game_professions` (
  `uuid` char(36) NOT NULL,
  `idx` int(10) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `abbr` char(2) NOT NULL,
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `index` (`idx`),
  UNIQUE KEY `abbr` (`abbr`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `game_professions`
--

LOCK TABLES `game_professions` WRITE;
/*!40000 ALTER TABLE `game_professions` DISABLE KEYS */;
INSERT INTO `game_professions` VALUES ('59f4493d-50f4-11e8-a7ca-02100700d6f0',1,'Warrior','W'),('59f4b30b-50f4-11e8-a7ca-02100700d6f0',2,'Ranger','R'),('73156b15-50f4-11e8-a7ca-02100700d6f0',3,'Monk','Mo'),('7315cbd6-50f4-11e8-a7ca-02100700d6f0',4,'Necromancer','N'),('79b75ff4-92f0-11e8-a7ca-02100700d6f0',0,'None','NA'),('85d0939b-50f4-11e8-a7ca-02100700d6f0',5,'Mesmer','Me'),('85d0ef81-50f4-11e8-a7ca-02100700d6f0',6,'Elementarist','E');
/*!40000 ALTER TABLE `game_professions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_skills`
--

DROP TABLE IF EXISTS `game_skills`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `game_skills` (
  `uuid` char(36) NOT NULL,
  `idx` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `attribute_uuid` char(36) NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `is_elite` tinyint(1) NOT NULL DEFAULT '0',
  `description` varchar(255) NOT NULL DEFAULT '',
  `short_description` varchar(255) NOT NULL DEFAULT '',
  `icon` varchar(260) NOT NULL DEFAULT '',
  `script` varchar(260) NOT NULL DEFAULT '',
  `is_locked` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `idx` (`idx`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `game_skills`
--

LOCK TABLES `game_skills` WRITE;
/*!40000 ALTER TABLE `game_skills` DISABLE KEYS */;
INSERT INTO `game_skills` VALUES ('0a5f957d-50fc-11e8-a7ca-02100700d6f0',6,'Mantra of Earth','f7477bbe-50f5-11e8-a7ca-02100700d6f0',11,0,'Stance. For 30...78...90 seconds, whenever you take earth damage, the damage is reduced by 26...45...50% and you gain 2 Energy.','Stance. (30...78...90 seconds.) Reduces earth damage you take by 26...45...50%. You gain 2 Energy when you take earth damage.','','/scripts/skills/mantra_of_earth.lua',0),('153eea82-50fa-11e8-a7ca-02100700d6f0',0,'(None)','4e8d25fe-50f7-11e8-a7ca-02100700d6f0',0,0,'No Skill','No Skill','','',0),('2cd9bf91-50fb-11e8-a7ca-02100700d6f0',3,'Signet of Capture','4e8d25fe-50f7-11e8-a7ca-02100700d6f0',9,0,'Signet. Choose one skill from a nearby dead Boss of your profession. Signet of Capture is permanently replaced by that skill. If that skill was elite, gain 250 XP for every level you have earned.','Signet. Choose one skill from a nearby dead Boss of your profession. Signet of Capture is permanently replaced by that skill. If that skill was elite, gain 250 XP for every level you have earned.','','/scripts/skills/signet_of_capture.lua',0),('5966a040-50fb-11e8-a7ca-02100700d6f0',4,'BAMPH!','4e8d25fe-50f7-11e8-a7ca-02100700d6f0',0,0,'Skill. BAMPH!','Skill. BAMPH!','','/scripts/skills/bamph.lua',1),('a76668f7-50fb-11e8-a7ca-02100700d6f0',5,'Power Block','e8e16f7e-50f5-11e8-a7ca-02100700d6f0',10,1,'Elite Spell. If target foe is casting a spell or chant, that skill and all skills of the same attribute are disabled for 1...10...12 seconds and that skill is interrupted.','Elite Spell. If target foe is casting a spell or chant, that skill and all skills of the same attribute are disabled for 1...10...12 seconds and that skill is interrupted.','','/scripts/skills/power_block.lua',0),('bc4ff444-50f9-11e8-a7ca-02100700d6f0',1,'Healing Signet','18c57862-50f5-11e8-a7ca-02100700d6f0',9,0,'Signet. You gain 82...154...172 Health. You have -40 armor while using this skill. ','Signet. You gain 82...154...172 Health. You have -40 armor while using this skill. ','','/scripts/skills/healing_signet.lua',0),('cd0722c1-50fa-11e8-a7ca-02100700d6f0',2,'Resurrection Signet','4e8d25fe-50f7-11e8-a7ca-02100700d6f0',9,0,'Signet. Resurrect target party member. That party member is returned to life with 100% Health and 25% Energy. This signet only recharges when you gain a morale boost.','Signet. Resurrects target party member (100% Health, 25% Energy). This signet only recharges when you gain a morale boost.','','/scripts/skills/resurrection_signet.lua',0);
/*!40000 ALTER TABLE `game_skills` ENABLE KEYS */;
UNLOCK TABLES;

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
  `invite_name` varchar(20) NOT NULL DEFAULT '',
  `invited` bigint(20) NOT NULL DEFAULT '0',
  `joined` int(20) NOT NULL DEFAULT '0',
  `expires` bigint(20) NOT NULL DEFAULT '0',
  UNIQUE KEY `account_uuid_2` (`account_uuid`,`guild_uuid`) COMMENT 'But can only be once a member of the same guild',
  KEY `guild_uuid` (`guild_uuid`),
  KEY `account_uuid` (`account_uuid`) COMMENT 'An account can be a member of one guild and be guest of other guilds',
  KEY `expires` (`expires`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `guild_members`
--

LOCK TABLES `guild_members` WRITE;
/*!40000 ALTER TABLE `guild_members` DISABLE KEYS */;
INSERT INTO `guild_members` VALUES ('7c9947c1-438b-11e8-980f-02100700d6f0','60e72a18-4797-11e8-ad09-02100700d6f0',4,'Schwester Trillian',0,0,0),('464cedf5-f62e-4dd9-bf1f-c741fc4c8878','60e72a18-4797-11e8-ad09-02100700d6f0',3,'Psycho Babe',0,0,0),('7c995634-438b-11e8-980f-02100700d6f0','60e72a18-4797-11e8-ad09-02100700d6f0',3,'Sun Shine',0,0,0);
/*!40000 ALTER TABLE `guild_members` ENABLE KEYS */;
UNLOCK TABLES;

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
-- Dumping data for table `guilds`
--

LOCK TABLES `guilds` WRITE;
/*!40000 ALTER TABLE `guilds` DISABLE KEYS */;
INSERT INTO `guilds` VALUES ('60e72a18-4797-11e8-ad09-02100700d6f0','Kreuz Retter','KR','7c9947c1-438b-11e8-980f-02100700d6f0',0);
/*!40000 ALTER TABLE `guilds` ENABLE KEYS */;
UNLOCK TABLES;

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
-- Dumping data for table `ip_bans`
--

LOCK TABLES `ip_bans` WRITE;
/*!40000 ALTER TABLE `ip_bans` DISABLE KEYS */;
/*!40000 ALTER TABLE `ip_bans` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `mails`
--

DROP TABLE IF EXISTS `mails`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `mails` (
  `uuid` char(36) NOT NULL,
  `from_account_uuid` char(36) NOT NULL,
  `to_account_uuid` char(36) NOT NULL,
  `from_name` char(20) NOT NULL DEFAULT '',
  `to_name` char(20) NOT NULL DEFAULT '',
  `subject` varchar(60) NOT NULL DEFAULT '',
  `message` varchar(255) NOT NULL DEFAULT '',
  `created` bigint(20) NOT NULL DEFAULT '0',
  `is_read` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`uuid`),
  KEY `is_read` (`is_read`),
  KEY `from_account_uuid` (`from_account_uuid`),
  KEY `to_account_uuid` (`to_account_uuid`),
  KEY `created` (`created`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `mails`
--

LOCK TABLES `mails` WRITE;
/*!40000 ALTER TABLE `mails` DISABLE KEYS */;
INSERT INTO `mails` VALUES ('00b2748a-ea97-4197-88ca-67a9351fc03f','7c9947c1-438b-11e8-980f-02100700d6f0','7c995347-438b-11e8-980f-02100700d6f0','Schwester Trillian','Mini Me','','hallo',1528431985947,1),('09b661db-b0f1-49dc-88f7-9e9a24ba379a','7c9947c1-438b-11e8-980f-02100700d6f0','7c995634-438b-11e8-980f-02100700d6f0','Schwester Trillian','Sun Shine','','Hallo',1528431671938,0),('1d5f911e-2282-44ee-a00d-38094a22c4f0','464cedf5-f62e-4dd9-bf1f-c741fc4c8878','7c9947c1-438b-11e8-980f-02100700d6f0','Psycho Babe','schwester trillian','Hallo','Hallo di!',1537417217141,1),('50ba9ba7-71fe-4099-9fb4-2b34fc0a8656','7c9947c1-438b-11e8-980f-02100700d6f0','464cedf5-f62e-4dd9-bf1f-c741fc4c8878','Schwester Trillian','Psycho Babe','Re: Hallo','halo',1537417240057,1),('a6e7c02f-a106-4197-95ad-21700a9d8b36','464cedf5-f62e-4dd9-bf1f-c741fc4c8878','7c996d4d-438b-11e8-980f-02100700d6f0','Psycho Babe','melonenpflueckerine','Es git Mail','Ist das geil!',1525259435600,0);
/*!40000 ALTER TABLE `mails` ENABLE KEYS */;
UNLOCK TABLES;

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
  `profession_uuid` char(36) NOT NULL DEFAULT '00000000-0000-0000-0000-000000000000',
  `profession2_uuid` char(36) NOT NULL DEFAULT '00000000-0000-0000-0000-000000000000',
  `name` varchar(20) NOT NULL,
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
  `current_map_uuid` char(36) NOT NULL DEFAULT '',
  `model_index` int(10) NOT NULL DEFAULT '0',
  `model_data` blob COMMENT 'Model index, scale etc.',
  `effects` blob COMMENT 'Persistent effects',
  `skills` varchar(36) NOT NULL DEFAULT '' COMMENT 'Base64 encoded skills',
  `equipment` blob,
  `titles` blob,
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `name` (`name`),
  KEY `account_uuid` (`account_uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `players`
--

LOCK TABLES `players` WRITE;
/*!40000 ALTER TABLE `players` DISABLE KEYS */;
INSERT INTO `players` VALUES ('14484624-f3b8-4741-8b39-35d027410208','W','','59f4493d-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psychotic Psyco',1,'ca3366ad-d5ba-4ada-9cda-9d060265adce',20,0,0,2,1526979817854,1526979913308,95,0,1526979817169,'75e3eeb5-479a-11e8-ad09-02100700d6f0',6,NULL,NULL,'',NULL,NULL),('2deeb001-77d9-4254-a7af-fb671821b8fc','E','','85d0ef81-50f4-11e8-a7ca-02100700d6f0','','Psycho Pet',1,'1b173096-5ec7-4b34-b5c4-54f3e2976c21',1,0,0,1,1531635993823,0,0,0,1531635993518,'75e3eeb5-479a-11e8-ad09-02100700d6f0',7,NULL,NULL,'',NULL,NULL),('33be12ca-1b73-41e3-9208-9f81ef75e004','W','','59f4493d-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Sy Borg',1,'464cedf5-f62e-4dd9-bf1f-c741fc4c8878',1,0,0,1,1536388501389,1536388644794,24834,0,1528521270842,'75e3eeb5-479a-11e8-ad09-02100700d6f0',4,NULL,NULL,'',NULL,NULL),('3793e458-ebbb-4ef9-bd38-bfa779a15107','Mo','','73156b15-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Sani Two',1,'b90acfa1-7e8a-4045-915a-864ac70ea293',1,0,0,2,1532851834143,1532851853156,19,0,1532851833489,'75e3eeb5-479a-11e8-ad09-02100700d6f0',1,NULL,NULL,'',NULL,NULL),('4bb92c55-dc58-4e48-ae8c-34af6318b11b','Mo','','73156b15-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Mini Me',1,'7c995347-438b-11e8-980f-02100700d6f0',1,0,0,1,1535363079737,1535363096976,4002,0,1527926642874,'75e3eeb5-479a-11e8-ad09-02100700d6f0',1,NULL,NULL,'',NULL,NULL),('59679349-1083-4156-afeb-2515f17b2cf7','E','','85d0ef81-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Green Wich',1,'7c9947c1-438b-11e8-980f-02100700d6f0',1,0,0,1,1536209142200,1536209864776,32389,0,1533278225275,'75e3eeb5-479a-11e8-ad09-02100700d6f0',7,NULL,NULL,'',NULL,NULL),('6ae9f17f-4492-490b-8437-df244cc96dce','Me','','85d0939b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Kitten',1,'ca3366ad-d5ba-4ada-9cda-9d060265adce',20,0,0,1,1531808060110,1531808259618,5939,0,1525530353490,'75e3eeb5-479a-11e8-ad09-02100700d6f0',3,NULL,NULL,'',NULL,NULL),('75d51395-4796-11e8-ad09-02100700d6f0','Mo','E','73156b15-50f4-11e8-a7ca-02100700d6f0','85d0ef81-50f4-11e8-a7ca-02100700d6f0','Schwester Trillian',1,'7c9947c1-438b-11e8-980f-02100700d6f0',20,100,0,1,1537532092052,1537533326049,609856,0,1514451557328,'75e3eeb5-479a-11e8-ad09-02100700d6f0',1,NULL,'','','',''),('75d51b66-4796-11e8-ad09-02100700d6f0','Me','E','85d0939b-50f4-11e8-a7ca-02100700d6f0','85d0ef81-50f4-11e8-a7ca-02100700d6f0','Sonnen Schein',1,'7c9947c1-438b-11e8-980f-02100700d6f0',20,100,0,1,1536643804865,1536644908489,78403,0,1514451557328,'75e3eeb5-479a-11e8-ad09-02100700d6f0',3,NULL,'','','',''),('75d51f7d-4796-11e8-ad09-02100700d6f0','Mo','E','73156b15-50f4-11e8-a7ca-02100700d6f0','85d0ef81-50f4-11e8-a7ca-02100700d6f0','Schwester Tank',1,'7c994ff2-438b-11e8-980f-02100700d6f0',20,100,0,1,1537005564290,1537005579782,147636,0,1514451557328,'75e3eeb5-479a-11e8-ad09-02100700d6f0',1,NULL,'','',NULL,NULL),('75d522bc-4796-11e8-ad09-02100700d6f0','E','Mo','85d0ef81-50f4-11e8-a7ca-02100700d6f0','73156b15-50f4-11e8-a7ca-02100700d6f0','Shy Borg',1,'7c995347-438b-11e8-980f-02100700d6f0',20,100,0,1,1537159014434,1537076619015,101520,0,1514451557328,'75e3eeb5-479a-11e8-ad09-02100700d6f0',7,NULL,'','',NULL,NULL),('75d525a8-4796-11e8-ad09-02100700d6f0','Me','','85d0939b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Laila Lovely',1,'7c9947c1-438b-11e8-980f-02100700d6f0',20,100,0,1,1535900207592,1535900218654,12917,0,1514451557328,'75e3eeb5-479a-11e8-ad09-02100700d6f0',3,NULL,'','',NULL,NULL),('75d528ae-4796-11e8-ad09-02100700d6f0','Me','','85d0939b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Amy Downers',1,'7c9947c1-438b-11e8-980f-02100700d6f0',20,100,0,1,1535364038712,1535364043171,18033,0,1514451660897,'75e3dfcf-479a-11e8-ad09-02100700d6f0',3,NULL,'','',NULL,NULL),('75d52b91-4796-11e8-ad09-02100700d6f0','E','','85d0ef81-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Green Witch',1,'7c995dd6-438b-11e8-980f-02100700d6f0',20,100,0,1,1531651090474,1531651400262,1191,0,1514458056489,'75e3eeb5-479a-11e8-ad09-02100700d6f0',7,NULL,'','',NULL,NULL),('75d52e7f-4796-11e8-ad09-02100700d6f0','Me','','85d0939b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Sun Shine',1,'7c995634-438b-11e8-980f-02100700d6f0',20,100,0,1,1535878308146,1535878315734,55700,0,1514458709448,'75e3eeb5-479a-11e8-ad09-02100700d6f0',3,NULL,'','',NULL,NULL),('75d53165-4796-11e8-ad09-02100700d6f0','N','','7315cbd6-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Drowning Witch',1,'7c9958ce-438b-11e8-980f-02100700d6f0',20,100,0,1,1527924940660,1527925217979,6486,0,1514458769257,'75e3fc5c-479a-11e8-ad09-02100700d6f0',1,NULL,'','',NULL,NULL),('75d53449-4796-11e8-ad09-02100700d6f0','R','','59f4b30b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Test Test',1,'7c994ff2-438b-11e8-980f-02100700d6f0',20,100,0,2,1525236027175,1525236659411,1439,0,1514459291352,'75e3eeb5-479a-11e8-ad09-02100700d6f0',2,NULL,'','',NULL,NULL),('75d53721-4796-11e8-ad09-02100700d6f0','W','','59f4493d-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Dad',1,'7c996067-438b-11e8-980f-02100700d6f0',20,100,0,2,1533277100398,1533277387562,17388,0,1514972875023,'75e3eeb5-479a-11e8-ad09-02100700d6f0',6,NULL,'','',NULL,NULL),('75d53a02-4796-11e8-ad09-02100700d6f0','W','','59f4493d-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Mum',1,'7c9962e7-438b-11e8-980f-02100700d6f0',20,100,0,1,1533465551219,1533466393031,13489,0,1514972950036,'75e3eeb5-479a-11e8-ad09-02100700d6f0',4,NULL,'','',NULL,NULL),('75d53cd4-4796-11e8-ad09-02100700d6f0','R','','59f4b30b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Max Test',1,'7c9947c1-438b-11e8-980f-02100700d6f0',20,100,0,2,1535020729262,1535020738420,591,0,1515228484573,'75e3eeb5-479a-11e8-ad09-02100700d6f0',2,NULL,'','',NULL,NULL),('75d540ab-4796-11e8-ad09-02100700d6f0','Mo','','73156b15-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Testine Max',1,'7c9947c1-438b-11e8-980f-02100700d6f0',20,100,0,1,1534837241718,1534837269872,1807,0,1515242987321,'75e3fc5c-479a-11e8-ad09-02100700d6f0',1,NULL,'','',NULL,NULL),('75d543a3-4796-11e8-ad09-02100700d6f0','Me','','85d0939b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Girl',1,'7c99657b-438b-11e8-980f-02100700d6f0',20,100,0,1,1533983478643,1533983919773,8314,0,1515487828990,'75e3eeb5-479a-11e8-ad09-02100700d6f0',3,NULL,'','',NULL,NULL),('75d5467e-4796-11e8-ad09-02100700d6f0','Mo','','73156b15-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','melonenpflueckerine',1,'7c996d4d-438b-11e8-980f-02100700d6f0',20,100,0,1,1517575500604,1517577081051,1804,0,1517575222495,'75e3eeb5-479a-11e8-ad09-02100700d6f0',1,NULL,'','',NULL,NULL),('769140f3-c8a0-4148-a3e8-e1c6ebab1fe2','N','','7315cbd6-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Whatever',1,'ca3366ad-d5ba-4ada-9cda-9d060265adce',20,0,0,1,1527924771375,1527925217950,465,0,1526978128337,'75e3eeb5-479a-11e8-ad09-02100700d6f0',1,NULL,NULL,'',NULL,NULL),('821f662c-6a26-42c2-aba6-4d657572cf7c','R','','59f4b30b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Me',1,'ca3366ad-d5ba-4ada-9cda-9d060265adce',1,0,0,2,1527925570978,1527926349109,853,0,1527401840174,'75e3eeb5-479a-11e8-ad09-02100700d6f0',2,NULL,NULL,'',NULL,NULL),('918e19bc-18c4-4cf0-9abe-76dfd2b2d969','R','','59f4b30b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Bunny',1,'464cedf5-f62e-4dd9-bf1f-c741fc4c8878',20,0,0,2,1536392128852,1536392135522,1598,0,1525496707556,'75e3eeb5-479a-11e8-ad09-02100700d6f0',2,NULL,NULL,'',NULL,NULL),('b47c561d-597d-4f1a-bcd6-105686216153','Me','','85d0ef81-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Chick',1,'ca3366ad-d5ba-4ada-9cda-9d060265adce',20,0,0,1,1531808272158,1531809733946,3620,0,1526376824603,'75e3eeb5-479a-11e8-ad09-02100700d6f0',7,NULL,NULL,'',NULL,NULL),('c0ae886f-379d-401c-98a8-e4f60de2b2ad','W','','59f4493d-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Babe',1,'464cedf5-f62e-4dd9-bf1f-c741fc4c8878',20,0,0,1,1537417128761,1537418435021,62458,0,1524918956502,'75e3eeb5-479a-11e8-ad09-02100700d6f0',4,NULL,NULL,'',NULL,NULL),('c503b2f7-883e-488b-b5d9-3e87f2df35df','Mo','','73156b15-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Sani Toeter',1,'b90acfa1-7e8a-4045-915a-864ac70ea293',1,0,0,2,1533277184264,1533277386623,9516,0,1532850451254,'75e3eeb5-479a-11e8-ad09-02100700d6f0',2,NULL,NULL,'',NULL,NULL),('c9892ba2-b9b4-4c20-b974-ea1455dd076d','R','','59f4b30b-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Bee',1,'16154f1a-dd92-42de-88a1-3fd5c0c0f87b',20,0,0,1,1525514951461,1525514960240,8,0,1525514951099,'75e3eeb5-479a-11e8-ad09-02100700d6f0',1,NULL,NULL,'',NULL,NULL),('dbfbef63-fd67-4add-97e3-73e3829256ef','N','','7315cbd6-50f4-11e8-a7ca-02100700d6f0','00000000-0000-0000-0000-000000000000','Psycho Baby',1,'464cedf5-f62e-4dd9-bf1f-c741fc4c8878',20,0,0,2,1532755866752,1532755905738,22233,0,1524918638720,'75e3eeb5-479a-11e8-ad09-02100700d6f0',2,NULL,NULL,'',NULL,NULL);
/*!40000 ALTER TABLE `players` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Temporary table structure for view `players_online_view`
--

DROP TABLE IF EXISTS `players_online_view`;
/*!50001 DROP VIEW IF EXISTS `players_online_view`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `players_online_view` (
  `uuid` tinyint NOT NULL,
  `profession` tinyint NOT NULL,
  `profession2` tinyint NOT NULL,
  `profession_uuid` tinyint NOT NULL,
  `profession2_uuid` tinyint NOT NULL,
  `name` tinyint NOT NULL,
  `pvp` tinyint NOT NULL,
  `account_uuid` tinyint NOT NULL,
  `level` tinyint NOT NULL,
  `experience` tinyint NOT NULL,
  `skillpoints` tinyint NOT NULL,
  `sex` tinyint NOT NULL,
  `lastlogin` tinyint NOT NULL,
  `lastlogout` tinyint NOT NULL,
  `onlinetime` tinyint NOT NULL,
  `deleted` tinyint NOT NULL,
  `creation` tinyint NOT NULL,
  `current_map_uuid` tinyint NOT NULL,
  `effects` tinyint NOT NULL,
  `skills` tinyint NOT NULL,
  `equipment` tinyint NOT NULL,
  `titles` tinyint NOT NULL,
  `online_status` tinyint NOT NULL,
  `account_name` tinyint NOT NULL,
  `game_name` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `professions_view`
--

DROP TABLE IF EXISTS `professions_view`;
/*!50001 DROP VIEW IF EXISTS `professions_view`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `professions_view` (
  `uuid` tinyint NOT NULL,
  `idx` tinyint NOT NULL,
  `profession_uuid` tinyint NOT NULL,
  `name` tinyint NOT NULL,
  `is_primary` tinyint NOT NULL,
  `profession_name` tinyint NOT NULL,
  `profession_abbr` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `reserved_names`
--

DROP TABLE IF EXISTS `reserved_names`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `reserved_names` (
  `uuid` char(36) NOT NULL,
  `name` varchar(40) NOT NULL,
  `is_reserved` int(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `name` (`name`),
  KEY `is_reserved` (`is_reserved`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `reserved_names`
--

LOCK TABLES `reserved_names` WRITE;
/*!40000 ALTER TABLE `reserved_names` DISABLE KEYS */;
INSERT INTO `reserved_names` VALUES ('e4be8e9f-6ba3-11e8-a7ca-02100700d6f0','Stefan Ascher',1);
/*!40000 ALTER TABLE `reserved_names` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `services`
--

DROP TABLE IF EXISTS `services`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `services` (
  `uuid` char(36) NOT NULL,
  `name` varchar(64) NOT NULL DEFAULT '',
  `type` int(3) NOT NULL DEFAULT '0' COMMENT 'Also startup order',
  `location` char(10) NOT NULL DEFAULT '',
  `host` varchar(64) NOT NULL DEFAULT '' COMMENT 'If empty it assumes it''s the same host as the login host',
  `port` int(10) NOT NULL DEFAULT '0',
  `status` int(1) NOT NULL DEFAULT '0',
  `start_time` int(20) NOT NULL DEFAULT '0',
  `stop_time` int(20) NOT NULL DEFAULT '0',
  `run_time` int(20) NOT NULL DEFAULT '0' COMMENT 'Time the server was runnig in seconds',
  `file` varchar(260) NOT NULL DEFAULT '',
  `path` varchar(260) NOT NULL DEFAULT '',
  `arguments` varchar(260) NOT NULL DEFAULT '',
  PRIMARY KEY (`uuid`),
  KEY `type` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `services`
--

LOCK TABLES `services` WRITE;
/*!40000 ALTER TABLE `services` DISABLE KEYS */;
INSERT INTO `services` VALUES ('0e9cf876-bb63-4926-a1a9-271bcf4a1c39','ATF1',3,'AT','',8081,0,2147483647,2147483647,1536264386,'c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin\\abfile.exe','c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin',''),('14935aef-7cd9-4495-b704-ec9578411558','AT2',5,'AT','',2750,0,2147483647,2147483647,44868,'c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin\\abserv.exe','c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin','-conf abserv2.lua'),('230a6dd3-907e-4193-87e5-a25789d68016','AT1',5,'AT','',2749,0,2147483647,2147483647,1535956432,'c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin\\abserv.exe','c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin',''),('3cfa2c3f-b426-4508-a880-217944716c54','abmsgs',2,'AT','',2771,0,2147483647,2147483647,2147483647,'c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin\\abmsgs.exe','c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin',''),('76d18b89-23d8-49cb-815e-d575122a27e8','AT1',5,'AT','',64060,0,2147483647,0,0,'C:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\abserv\\abserv\\..\\..\\Bin\\abserv_d.exe','C:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\abserv\\abserv\\..\\..\\Bin','-id 00000000-0000-0000-0000-000000000000 -port 0 -autoterm'),('961226da-0010-45c1-9141-b45872c46991','ablb',255,'AT','0.0.0.0',2740,0,2147483647,2147483647,182,'C:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin\\ablb.exe','C:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin',''),('9b53f954-db11-413b-85b6-b7080b0f4063','AT3',5,'AT','',2751,0,2147483647,2147483647,11045,'c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin\\abserv.exe','c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin','-conf abserv3.lua'),('b7a49132-abec-4195-a429-c60d4a60e918','AT1',5,'AT','',63875,0,2147483647,0,0,'C:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\abserv\\abserv\\..\\..\\Bin\\abserv_d.exe','C:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\abserv\\abserv\\..\\..\\Bin','-id 00000000-0000-0000-0000-000000000000 -port 0 -autoterm'),('cd00b30c-fc7f-416d-be9e-cec9fb34fb79','abdata',1,'AT','',2770,0,2147483647,2147483647,933087,'c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin\\abdata.exe','c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin',''),('ef5ac3ec-45f9-40dc-8523-99c7937c69ba','ablogin',4,'AT','',2748,0,2147483647,2147483647,1536274080,'c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin\\ablogin.exe','c:\\Users\\Stefan Ascher\\Documents\\Visual Studio 2015\\Projects\\ABx\\Bin','');
/*!40000 ALTER TABLE `services` ENABLE KEYS */;
UNLOCK TABLES;

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
-- Dumping data for table `storage`
--

LOCK TABLES `storage` WRITE;
/*!40000 ALTER TABLE `storage` DISABLE KEYS */;
/*!40000 ALTER TABLE `storage` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `versions`
--

DROP TABLE IF EXISTS `versions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `versions` (
  `uuid` char(36) NOT NULL,
  `name` char(20) NOT NULL,
  `value` int(10) NOT NULL,
  `internal` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`uuid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `versions`
--

LOCK TABLES `versions` WRITE;
/*!40000 ALTER TABLE `versions` DISABLE KEYS */;
INSERT INTO `versions` VALUES ('710f3575-5bfd-11e8-a7ca-02100700d6f0','game_attributes',1,0),('710f8be3-5bfd-11e8-a7ca-02100700d6f0','game_effects',1,0),('8180a2cb-5bfd-11e8-a7ca-02100700d6f0','game_items',1,0),('ac35736a-5bfa-11e8-a7ca-02100700d6f0','game_skills',1,0),('f57fda7d-5bf8-11e8-a7ca-02100700d6f0','game_maps',1,0),('f5804256-5bf8-11e8-a7ca-02100700d6f0','game_professions',1,0);
/*!40000 ALTER TABLE `versions` ENABLE KEYS */;
UNLOCK TABLES;

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

--
-- Final view structure for view `players_online_view`
--

/*!50001 DROP TABLE IF EXISTS `players_online_view`*/;
/*!50001 DROP VIEW IF EXISTS `players_online_view`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `players_online_view` AS select `players`.`uuid` AS `uuid`,`players`.`profession` AS `profession`,`players`.`profession2` AS `profession2`,`players`.`profession_uuid` AS `profession_uuid`,`players`.`profession2_uuid` AS `profession2_uuid`,`players`.`name` AS `name`,`players`.`pvp` AS `pvp`,`players`.`account_uuid` AS `account_uuid`,`players`.`level` AS `level`,`players`.`experience` AS `experience`,`players`.`skillpoints` AS `skillpoints`,`players`.`sex` AS `sex`,`players`.`lastlogin` AS `lastlogin`,`players`.`lastlogout` AS `lastlogout`,`players`.`onlinetime` AS `onlinetime`,`players`.`deleted` AS `deleted`,`players`.`creation` AS `creation`,`players`.`current_map_uuid` AS `current_map_uuid`,`players`.`effects` AS `effects`,`players`.`skills` AS `skills`,`players`.`equipment` AS `equipment`,`players`.`titles` AS `titles`,`accounts`.`online_status` AS `online_status`,`accounts`.`name` AS `account_name`,`game_maps`.`name` AS `game_name` from ((`players` left join `accounts` on((`players`.`account_uuid` = `accounts`.`uuid`))) left join `game_maps` on((`players`.`current_map_uuid` = `game_maps`.`uuid`))) where ((`accounts`.`online_status` = 3) and (`accounts`.`current_character_uuid` = `players`.`uuid`)) */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `professions_view`
--

/*!50001 DROP TABLE IF EXISTS `professions_view`*/;
/*!50001 DROP VIEW IF EXISTS `professions_view`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`root`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `professions_view` AS select `game_attributes`.`uuid` AS `uuid`,`game_attributes`.`idx` AS `idx`,`game_attributes`.`profession_uuid` AS `profession_uuid`,`game_attributes`.`name` AS `name`,`game_attributes`.`is_primary` AS `is_primary`,`game_professions`.`name` AS `profession_name`,`game_professions`.`abbr` AS `profession_abbr` from (`game_attributes` left join `game_professions` on((`game_attributes`.`profession_uuid` = `game_professions`.`uuid`))) */;
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

-- Dump completed on 2018-09-22  3:59:13
