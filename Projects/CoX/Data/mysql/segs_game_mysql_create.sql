SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

DROP TABLE IF EXISTS `table_versions`;
DROP TABLE IF EXISTS `supergroups`;
DROP TABLE IF EXISTS `progress`;
DROP TABLE IF EXISTS `costume`;
DROP TABLE IF EXISTS `characters`;
DROP TABLE IF EXISTS `accounts`;
DROP TABLE IF EXISTS `emails`;

CREATE TABLE `accounts` (
  `id` int(11) NOT NULL,
  `max_slots` int(11) NOT NULL DEFAULT '8'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `characters` (
  `id` int(11) NOT NULL,
  `account_id` int(11) NOT NULL,
  `slot_index` int(11) NOT NULL DEFAULT '0',
  `char_name` text CHARACTER SET latin1 NOT NULL,
  `chardata` mediumblob,
  `entitydata` blob,
  `bodytype` int(11) NOT NULL DEFAULT '4',
  `height` double NOT NULL DEFAULT '0',
  `physique` double NOT NULL DEFAULT '0',
  `supergroup_id` int(11) NOT NULL DEFAULT '0',
  `player_data` mediumblob
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

CREATE TABLE `costume` (
  `id` int(11) NOT NULL,
  `character_id` int(11) NOT NULL,
  `costume_index` int(11) NOT NULL,
  `skin_color` int(11) UNSIGNED NOT NULL,
  `parts` blob
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `supergroups` (
  `id` int(11) NOT NULL,
  `sg_name` text NOT NULL,
  `sg_motto` text,
  `sg_motd` text,
  `sg_rank_names` blob,
  `sg_rank_perms` blob,
  `sg_emblem` blob,
  `sg_colors` blob,
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
(1, 'db_version', 7, '2018-05-03 17:54:32'),
(2, 'table_versions', 0, '2017-11-11 08:57:42'),
(3, 'accounts', 1, '2018-05-03 12:52:03'),
(4, 'characters', 8, '2018-05-04 14:58:27'),
(5, 'costume', 0, '2017-11-11 08:57:43'),
(7, 'supergroups', 1, '2018-05-03 12:52:53'),
(8, 'emails', 1, '2018-09-23 08:00;00');

ALTER TABLE `accounts`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `characters`
  ADD PRIMARY KEY (`id`),
  ADD KEY `account_id` (`account_id`);

ALTER TABLE `costume`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `character_id` (`character_id`,`costume_index`);

ALTER TABLE `supergroups`
  ADD PRIMARY KEY (`id`);
  
ALTER TABLE `emails`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `table_versions`
  ADD PRIMARY KEY (`id`);


ALTER TABLE `characters`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `costume`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `supergroups`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `emails`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `table_versions`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;

ALTER TABLE `characters`
  ADD CONSTRAINT `characters_ibfk_1` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE;

ALTER TABLE `costume`
  ADD CONSTRAINT `costume_ibfk_1` FOREIGN KEY (`character_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;
  
ALTER TABLE `emails`
  ADD CONSTRAINT `emails_ibfk_1` FOREIGN KEY (`sender_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;
  ADD CONSTRAINT `emails_ibfk_2` FOREIGN KEY (`recipient_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
