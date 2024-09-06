-- phpMyAdmin SQL Dump
-- version 4.1.14
-- http://www.phpmyadmin.net
--
-- Hoszt: 127.0.0.1
-- Létrehozás ideje: 2016. Jún 28. 13:32
-- Szerver verzió: 5.6.17
-- PHP verzió: 5.5.12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Adatbázis: `makeup`
--
CREATE DATABASE IF NOT EXISTS `makeup` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `makeup`;

-- --------------------------------------------------------

--
-- Tábla szerkezet ehhez a táblához `people`
--

CREATE TABLE IF NOT EXISTS `people` (
  `id_user` int(11) NOT NULL AUTO_INCREMENT,
  `username` char(30) NOT NULL,
  `email` varchar(50) NOT NULL,
  `password` char(50) DEFAULT NULL,
  PRIMARY KEY (`id_user`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=8 ;

--
-- A tábla adatainak kiíratása `people`
--

INSERT INTO `people` (`id_user`, `username`, `email`, `password`) VALUES
(1, 'AdminNiki', 'nikoletta9553@hotmail.com', 'f04e5afb5355f7243aa0b17cdfa57cd0'),
(2, 'AdminValentina', 'valentinahodan@yahoo.com', '2a3733d1a02f5ee2eaeecbd2e79ef2dc'),
(3, 'test', 'test@test.com', '098f6bcd4621d373cade4e832627b4f6');

-- --------------------------------------------------------

--
-- Tábla szerkezet ehhez a táblához `products`
--

CREATE TABLE IF NOT EXISTS `products` (
  `id_product` int(11) NOT NULL AUTO_INCREMENT,
  `productname` char(30) NOT NULL,
  `image` varchar(100) NOT NULL,
  `description` char(100) NOT NULL,
  `price` double(5,2) NOT NULL,
  PRIMARY KEY (`id_product`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=13 ;

--
-- A tábla adatainak kiíratása `products`
--

INSERT INTO `products` (`id_product`, `productname`, `image`, `description`, `price`) VALUES
(1, 'Lipstick', 'ruzs.jpg', 'It is used for coloring the lips. It can be used everyday, or occasionally.', 5.99),
(2, 'Powder', 'puder.jpg', 'It is used for smoothing the face, and hiding blemishes. It can be used everyday, or occasionally.', 10.99),
(3, 'Mascara', 'szempillaspiral.jpg', 'It is used to style the eyes, by adding color to the eyelashes.It is good for everyday use.', 6.99),
(4, 'Nailpolish', 'koromlakk.jpg', 'It is used to color and decorate the nails. It is good for everyday use.', 4.99),
(5, 'Eyeshadow', 'szemhejfestek.jpg', 'It is used to style the eyes by coloring the eyelids. It is good for everyday use.', 8.99),
(6, 'Lip gloss', 'szajfeny.jpg', 'It gives shine to the lips. It can last all day.', 3.99),
(7, 'Blush', 'pirosito.jpg', 'It is used to redden the cheeks.', 6.55),
(8, 'Foundation', 'alapozo.jpg', 'It is applied to the face to create an even, uniform colour to the complexion, to cover flaws.', 9.98),
(9, 'Eye pencil', 'szemceruza.jpg', 'It is used to define the eyes.', 3.99),
(10, 'Eyeliner', 'szemkonturceruza.jpg', 'It is used to define the eyes.', 4.50),
(11, 'Corrector', 'korrektorstift.jpg', 'It is used to mask dark circles, age spots and large pores visible on the skin.', 5.56);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
