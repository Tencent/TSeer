-- MySQL dump 10.15  Distrib 10.0.24-MariaDB, for Linux (x86_64)
--
-- Server version	10.0.10-proxy

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
-- Table structure for table `t_idc_priority_info`
--

DROP TABLE IF EXISTS `t_idc_priority_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `t_idc_priority_info` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `priority_name` varchar(128) NOT NULL,
  `order` varchar(128) NOT NULL,
  `group_list` text,
  PRIMARY KEY (`id`),
  UNIQUE KEY `priority_name` (`priority_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='t_idc_priority_info';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `t_idc_rule_info`
--

DROP TABLE IF EXISTS `t_idc_rule_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `t_idc_rule_info` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `group_name` varchar(128) NOT NULL,
  `allow_ip` text,
  PRIMARY KEY (`id`),
  UNIQUE KEY `group_name` (`group_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='t_idc_rule_info';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `t_node_info`
--

DROP TABLE IF EXISTS `t_node_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `t_node_info` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `node_name` varchar(128) DEFAULT '',
  `node_networkId` varchar(128) NOT NULL DEFAULT 'dcnode',
  `node_obj` varchar(128) DEFAULT '',
  `endpoint_ip` varchar(16) DEFAULT '',
  `endpoint_port` int(11) DEFAULT '0',
  `data_dir` varchar(128) DEFAULT '',
  `load_avg1` float DEFAULT '0',
  `load_avg5` float DEFAULT '0',
  `load_avg15` float DEFAULT '0',
  `cpuuse_avg1` int(11) DEFAULT '0',
  `cpu_num` int(11) NOT NULL DEFAULT '0',
  `cpu_hz` int(11) NOT NULL DEFAULT '0',
  `memory_size` int(15) NOT NULL DEFAULT '0',
  `core_file_size` varchar(16) DEFAULT '0',
  `open_files_size` int(11) DEFAULT '0',
  `last_reg_time` datetime DEFAULT '1970-01-01 00:08:00',
  `last_heartbeat` datetime DEFAULT '1970-01-01 00:08:00',
  `setting_state` enum('active','inactive') DEFAULT 'inactive',
  `present_state` enum('active','inactive') DEFAULT 'inactive',
  `version` varchar(128) NOT NULL COMMENT 'agent软件版本号',
  `ostype` varchar(128) NOT NULL DEFAULT '操作系统版本号',
  `docker_node_version` varchar(128) NOT NULL DEFAULT '',
  `template_name` varchar(128) NOT NULL DEFAULT '',
  `modify_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `group_id` int(11) DEFAULT '-1',
  `locator` varchar(256) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uniq_node_name` (`node_name`,`node_networkId`),
  KEY `present_state_idx` (`present_state`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COMMENT='存放agent信息';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `t_os_description`
--

DROP TABLE IF EXISTS `t_os_description`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `t_os_description` (
  `id` int(4) NOT NULL AUTO_INCREMENT,
  `os_version` varchar(128) NOT NULL DEFAULT '' COMMENT '操作系统版本号',
  `os_description` varchar(255) NOT NULL DEFAULT '' COMMENT '描述支持gcc等版本',
  PRIMARY KEY (`id`),
  UNIQUE KEY `version` (`os_version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='操作系统版本号';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `t_registry_info`
--

DROP TABLE IF EXISTS `t_registry_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `t_registry_info` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `locator_id` varchar(128) NOT NULL DEFAULT '',
  `servant` varchar(128) NOT NULL DEFAULT '',
  `endpoint` varchar(128) NOT NULL DEFAULT '',
  `last_heartbeat` datetime DEFAULT '1970-01-01 00:08:00',
  `present_state` enum('active','inactive') DEFAULT 'inactive',
  `tars_version` varchar(128) NOT NULL DEFAULT '',
  `modify_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `enable_group` char(1) DEFAULT 'N',
  PRIMARY KEY (`id`),
  UNIQUE KEY `locator_id` (`locator_id`,`servant`)
) ENGINE=InnoDB AUTO_INCREMENT=5554 DEFAULT CHARSET=utf8 COMMENT='t_registry_info';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `t_router_priv_info`
--

DROP TABLE IF EXISTS `t_servicegroup_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `t_servicegroup_info` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `service_group` varchar(128) NOT NULL,
  `key` varchar(128) NOT NULL,
  `userlist` varchar(256) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `service_group` (`service_group`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COMMENT='t_servicegroup_info';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `t_server_conf`
--

DROP TABLE IF EXISTS `t_server_conf`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `t_server_conf` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `moduletype` varchar(128) DEFAULT '',
  `application` varchar(128) DEFAULT '',
  `service_name` varchar(128) DEFAULT '',
  `node_name` varchar(50) NOT NULL DEFAULT '',
  `container_name` varchar(50) NOT NULL DEFAULT '',
  `registry_timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `bak_flag` varchar(16) NOT NULL DEFAULT '0',
  `grid_flag` varchar(16) NOT NULL DEFAULT '0',
  `setting_state` enum('active','inactive') NOT NULL DEFAULT 'inactive',
  `present_state` enum('active','inactive','activating','deactivating','destroyed') NOT NULL DEFAULT 'inactive',
  `process_id` varchar(16) NOT NULL DEFAULT '0',
  `node_port` varchar(16) NOT NULL DEFAULT '0',
  `container_port` varchar(16) NOT NULL DEFAULT '0',
  `weight` varchar(16) NOT NULL DEFAULT '',
  `weighttype` varchar(16) NOT NULL DEFAULT '',
  `enable_heartbeat` varchar(16) NOT NULL DEFAULT 'N',
  `tars_version` varchar(128) NOT NULL DEFAULT '',
  `enable_group` char(1) DEFAULT 'N',
  `enable_set` char(1) NOT NULL DEFAULT 'N',
  `set_name` varchar(16) DEFAULT NULL,
  `set_area` varchar(16) DEFAULT NULL,
  `set_group` varchar(64) DEFAULT NULL,
  `ip_group_name` varchar(64) DEFAULT NULL,
  `servant` varchar(512) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `application` (`moduletype`,`application`,`service_name`,`node_name`,`container_name`),
  KEY `container_name` (`container_name`),
  KEY `index_regtime` (`registry_timestamp`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COMMENT='t_server_conf';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `t_server_patchs`
--

DROP TABLE IF EXISTS `t_server_patchs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `t_server_patchs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `version` varchar(64) DEFAULT '',
  `package_name` varchar(256) DEFAULT '发布包文件名',
  `md5` varchar(50) DEFAULT NULL,
  `upload_time` datetime DEFAULT NULL,
  `upload_user` varchar(30) DEFAULT NULL,
  `package_type` tinyint(4) DEFAULT '0' COMMENT 'packettype=2,3',
  `ostype` varchar(64) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `packagename_key` (`ostype`,`md5`),
  KEY `grayversion_key` (`ostype`,`package_type`),
  KEY `index_i_2` (`tgz`(50))
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8 COMMENT='存放版本信息';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `t_server_publish`
--

DROP TABLE IF EXISTS `t_server_publish`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `t_server_publish` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `node_name` varchar(40) DEFAULT NULL COMMENT 'node IP',
  `grayscale_version` varchar(256) DEFAULT '' COMMENT 'v1.0',
  `grayscale_state` enum('0','1','2','3') NOT NULL DEFAULT '0',
  `publish_time` datetime DEFAULT NULL,
  `publish_user` varchar(30) DEFAULT NULL,
  `package_name` varchar(512) NOT NULL DEFAULT '发布包文件名',
  `ostype` varchar(128) DEFAULT 'CentOs-6.5-64',
  `md5` varchar(64) DEFAULT NULL COMMENT 'md5',
  PRIMARY KEY (`id`),
  UNIQUE KEY `ip` (`node_name`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COMMENT='存放版本信息';
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-09-12 17:28:17
