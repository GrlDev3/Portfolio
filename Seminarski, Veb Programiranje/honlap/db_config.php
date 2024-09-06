<?php
if (defined('VARIABLE') AND VARIABLE=='jrUmrlbHRevgwhskwpLdpoxkksqe')
{
    define("HOST", "localhost");
    define("USER","root");
    define("PASSWORD","");
    define("DATABASE","makeup");

    $connection = mysqli_connect(HOST, USER, PASSWORD, DATABASE)or die(mysqli_connect_error());

    if (mysqli_connect_errno())
        echo "Failed to connect to MySQL: ".mysqli_connect_error();

    mysqli_query($connection,"SET NAMES utf8") or die (mysqli_error($connection));
    mysqli_query($connection,"SET CHARACTER SET utf8") or die (mysqli_error($connection));
  //  mysqli_query($connection,"SET COLLATION_CONNECTION='utf8_general_ci") or die(mysqli_error($connection));

}