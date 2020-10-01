SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;


DROP TABLE IF EXISTS `table_versions`;
DROP TABLE IF EXISTS `game_servers`;
DROP TABLE IF EXISTS `bans`;
DROP TABLE IF EXISTS `accounts`;
CREATE TABLE `accounts` (
  `id` int(11) NOT NULL,
  `username` varchar(20) NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '1',
  `creation_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `passw` blob NOT NULL,
  `salt` blob NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `bans` (
  `id` int(11) NOT NULL,
  `account_id` int(11) NOT NULL,
  `offending_ip` int(11) NOT NULL DEFAULT '0',
  `started` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `reason` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `game_servers` (
  `id` int(11) NOT NULL,
  `addr` varchar(20) NOT NULL,
  `port` int(11) NOT NULL DEFAULT '0',
  `name` text NOT NULL,
  `token` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `table_versions` (
  `id` int(11) NOT NULL,
  `table_name` varchar(20) NOT NULL,
  `version` int(11) NOT NULL DEFAULT '0',
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO `table_versions` (`id`, `table_name`, `version`, `last_update`) VALUES
(1, 'db_version', 1, '2019-02-17 03:11:58'),
(2, 'table_versions', 0, '2017-11-11 08:55:54'),
(3, 'accounts', 1, '2018-01-06 11:18:01'),
(4, 'game_servers', 0, '2017-11-11 09:12:37'),
(5, 'bans', 0, '2017-11-11 09:12:37');


ALTER TABLE `accounts`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `username` (`username`);

ALTER TABLE `bans`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `account_id` (`account_id`);

ALTER TABLE `game_servers`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `table_versions`
  ADD PRIMARY KEY (`id`);


ALTER TABLE `accounts`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
ALTER TABLE `bans`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
ALTER TABLE `game_servers`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

ALTER TABLE `bans`
  ADD CONSTRAINT `ban_account_id` FOREIGN KEY (`account_id`) REFERENCES `accounts` (`id`) ON DELETE CASCADE;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
