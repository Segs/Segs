SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

DROP TABLE IF EXISTS `table_versions`;
DROP TABLE IF EXISTS `supergroups`;
DROP TABLE IF EXISTS `progress`;
/* The costume table is no more but for compatibility with older releases
   we want to make sure to still remove it. */
DROP TABLE IF EXISTS `costume`;
DROP TABLE IF EXISTS `emails`;
DROP TABLE IF EXISTS `characters`;
DROP TABLE IF EXISTS `accounts`;

CREATE TABLE `accounts` (
  `id` int(11) NOT NULL,
  `max_slots` int(11) NOT NULL DEFAULT '8'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `characters` (
  `id` int(11) NOT NULL,
  `account_id` int(11) NOT NULL,
  `slot_index` int(11) NOT NULL DEFAULT '0',
  `char_name` text CHARACTER SET latin1 NOT NULL,
  `costume_data` mediumblob,
  `chardata` mediumblob,
  `entitydata` blob,
  `player_data` mediumblob,
  `supergroup_id` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

CREATE TABLE `supergroups` (
  `id` int(11) NOT NULL,
  `sg_name` text NOT NULL,
  `sg_data` blob,
  `sg_members` blob
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `emails`(
	`id` int(11) NOT NULL,
	`sender_id` int(11) NOT NULL,
	`recipient_id` int(11) NOT NULL,
	`email_data` blob
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `table_versions` (
  `id` int(11) NOT NULL,
  `table_name` varchar(20) NOT NULL,
  `version` int(11) NOT NULL DEFAULT '0',
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO `table_versions` (`id`, `table_name`, `version`, `last_update`) VALUES
(1, 'db_version', 10, '2019-04-28 22:56:43'),
(2, 'table_versions', 0, '2017-11-11 08:57:42'),
(3, 'accounts', 1, '2018-05-03 12:52:03'),
(4, 'characters', 12, '2019-04-28 22:56:43'),
(7, 'supergroups', 2, '2018-10-22 22:56:43'),
(8, 'emails', 0, '2018-09-23 08:00:00');

ALTER TABLE `accounts`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `characters`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

ALTER TABLE `supergroups`
  ADD PRIMARY KEY (`id`);
  
ALTER TABLE `emails`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `table_versions`
  ADD PRIMARY KEY (`id`);


ALTER TABLE `characters`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `supergroups`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `emails`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `table_versions`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;

ALTER TABLE `characters`
  ADD CONSTRAINT `characters_ibfk_1` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE;
  
ALTER TABLE `emails`
  ADD CONSTRAINT `emails_ibfk_1` FOREIGN KEY (`sender_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE,
  ADD CONSTRAINT `emails_ibfk_2` FOREIGN KEY (`recipient_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
