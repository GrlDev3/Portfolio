<!DOCTYPE html
    PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
    <title>Avon Cosmetics</title>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
    <meta http-equiv="content-language" content="en"/>
    <link type="text/css" rel="stylesheet" href="../css/mainstyle.css"/>
    <link href="../css/sliderstyle.css" rel="stylesheet" type="text/css"/>
    <script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.2/jquery.min.js"></script>
    <script type="text/javascript" src="../js/slider.js"></script>
</head>
<body>
<div id="header">
    <div class="slideshow">
        <ul class="slider">
            <li><img src="image/avon01.jpg" alt="smink"/></li>
            <li><img src="image/avon02.jpg" alt="smink"/></li>
            <li><img src="image/avon03.jpg" alt="smink"/></li>
            <li><img src="image/avon04.jpg" alt="smink"/></li>
            <li><img src="image/avon05.jpg" alt="smink"/></li>
            <li><img src="image/avon06.jpg" alt="smink"/></li>
            <li><img src="image/avon07.jpg" alt="smink"/></li>
            <li><img src="image/avon08.jpg" alt="smink"/></li>
            <li><img src="image/avon09.jpg" alt="smink"/></li>
            <li><img src="image/avon10.jpg" alt="smink"/></li>
        </ul>
    </div>
    <h1>Avon Cosmetics</h1>
</div>
<div id="navigation">
    <ol>
        <li><a href="index.php">About us</a></li>
        <li><a href="index.php?op=contact">Contact</a></li>
        <li><a href="index.php?op=products">Products</a></li>
        <li><a href="index.php?op=search">Search</a></li>
        <li><a href="index.php?op=login">Login</a></li>
        <li><a href="index.php?op=signup">Sign Up</a></li>
    </ol>
</div>
<?php
define("VARIABLE","jrUmrlbHRevgwhskwpLdpoxkksqe");
require("../db_config.php");

$op= "aboutus";

if(isset($_GET['op']))
    $op= mysqli_real_escape_string($connection,$_GET['op']);

switch($op)
{
    case "aboutus";
    default: include("aboutus.xhtml");
        break;

    case "contact": include("contact.xhtml");
        break;

    case "login": include("login.php");
        break;

    /*case "logging-in":
        if(isset($_GET['error'])){
            include("verify.php");
        }
        break;*/

    case "products": include("products.php");
        break;

    /*case "registrate": include("registration.html");
        break;*/

    case "signup":
            include ("signup.php");
        break;

    case "search": include("search.xhtml");
        break;

    case "search-result": include ("search.php");
        break;
}
?>

<div id="footer">
    <p>This is only a seminar work. This is not the actual Avon website. This website will be taken down by the end of the week. All rights belong to the real <a href="https://www.avon.com/">Avon Cosmetics Company</a>.</p>
</div>
</body>
</html>